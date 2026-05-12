#include "MargaritaSteppingAction.hh"
#include "MargaritaRunAction.hh"

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

#include "G4AnalysisManager.hh"
#include <array>

MargaritaSteppingAction::MargaritaSteppingAction(MargaritaRunAction* run)
: frunAction(run) {}

MargaritaSteppingAction::~MargaritaSteppingAction() = default;

void MargaritaSteppingAction::UserSteppingAction(const G4Step* aStep)
{
  // Handles
  G4StepPoint* pre  = aStep->GetPreStepPoint();
  G4StepPoint* post = aStep->GetPostStepPoint();
  G4Track*     trk  = aStep->GetTrack();

  // Volume at post-step
  G4VPhysicalVolume* volPost =
      post->GetTouchableHandle() ? post->GetTouchableHandle()->GetVolume() : nullptr;
  if (!volPost) return;

  const G4String namePost = volPost->GetName();

  // Select which cylinder we are in
  int idx = -1;
  if      (namePost == "CylPV" || namePost == "BoxPV")  idx = 0;
  else return;

  // Volume at pre-step. We require the step to *start* inside the active
  // volume so the entry step (pre = air, post = detector boundary) is not
  // counted — its step length is the path through air from the source plane
  // to the detector face, which would otherwise inflate the recorded
  // track length and pollute h2.2 with a zero-eDep, large-stepLen entry.
  G4VPhysicalVolume* volPre =
      pre->GetTouchableHandle() ? pre->GetTouchableHandle()->GetVolume() : nullptr;
  const G4String namePre = volPre ? volPre->GetName() : G4String("");
  if (namePre != "CylPV" && namePre != "BoxPV") return;

  // Particle selection: mu- only (PDG 13)
  const G4int pdg = trk->GetDefinition()->GetPDGEncoding();
  if (pdg != 13) return;

  // Primaries only
  if (trk->GetParentID() != 0) return;

  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();

  // --- Incident muon counter (one per muon entering CylPV) ---
  // Used as denominator for stopping efficiency: h1.5
  static std::set<G4int> fIncidentCounted;
  const G4int trkIDinc = trk->GetTrackID();
  if (!fIncidentCounted.count(trkIDinc)) {
    const G4double eKinInit_inc = trk->GetVertexKineticEnergy();
    analysisManager->FillH1(5, eKinInit_inc/MeV);
    fIncidentCounted.insert(trkIDinc);
  }

  // --- Stopping power: per-step dE/dx vs pre-step KE → h2.2 ---
  const G4double stepLen = aStep->GetStepLength();
  const G4double eDep    = aStep->GetTotalEnergyDeposit();
  const G4double eKinPre = aStep->GetPreStepPoint()->GetKineticEnergy();
  const G4double postKE  = post->GetKineticEnergy();
  const G4bool   stopped = (postKE < 1.0 * keV);
  const G4bool   exiting = (trk->GetNextVolume() == nullptr ||
                            trk->GetNextVolume()->GetName() != namePost);

  if (stepLen > 0. && eDep > 0.)
    analysisManager->FillH2(2, eKinPre/MeV, eDep/stepLen/(MeV/cm));

  // -----------------------------------------------------------------
  // [NEW] Per-track step buffer for single-track diagnostic
  // Append this step's record. We don't yet know if the track stops;
  // we only flush to the ntuple when we detect a stop below.
  //
  // Track identity is (eventID, trackID): trackID alone is event-local
  // (every event's primary is trackID=1) so a pure trackID check would
  // silently concatenate consecutive events' primaries into one buffer.
  // -----------------------------------------------------------------
  {
    const G4Event* evtBuf = G4RunManager::GetRunManager()->GetCurrentEvent();
    const G4int eventIDbuf = evtBuf ? evtBuf->GetEventID() : -1;
    const G4int trkIDbuf   = trk->GetTrackID();
    if (trkIDbuf != fCurTrackID || eventIDbuf != fCurEventID) {
      // New primary mu- (either new event, or previous one exited w/o stopping)
      fCurEventID  = eventIDbuf;
      fCurTrackID  = trkIDbuf;
      fCurEKinInit = trk->GetVertexKineticEnergy() / MeV;
      fCurVertex   = trk->GetVertexPosition();
      fCurDir      = trk->GetVertexMomentumDirection();
      fCurCumLen   = 0.;
      fCurSteps.clear();

      // [NEW] One row per incident primary mu- entering the active
      // volume -> denominator ntuple for the absolute (z,x) stopping-
      // efficiency map. ntupleId = 1 ("incidentMuons").
      analysisManager->FillNtupleIColumn(1, 0, eventIDbuf);
      analysisManager->FillNtupleIColumn(1, 1, trkIDbuf);
      analysisManager->FillNtupleDColumn(1, 2, fCurVertex.x() / cm);
      analysisManager->FillNtupleDColumn(1, 3, fCurVertex.y() / cm);
      analysisManager->FillNtupleDColumn(1, 4, fCurVertex.z() / cm);
      analysisManager->FillNtupleDColumn(1, 5, fCurEKinInit);
      analysisManager->AddNtupleRow(1);
    }
    if (stepLen > 0.) {
      fCurCumLen += stepLen / cm;
      StepRec rec;
      rec.stepNum = (G4int)fCurSteps.size() + 1;
      rec.kePre   = eKinPre / MeV;
      rec.dEdx    = (eDep > 0.) ? eDep / stepLen / (MeV / cm) : 0.;
      const auto pp = post->GetPosition();
      rec.x       = pp.x() / cm;
      rec.y       = pp.y() / cm;
      rec.z       = pp.z() / cm;
      rec.stepLen = stepLen / cm;
      rec.eDep    = eDep / MeV;
      rec.cumLen  = fCurCumLen;
      fCurSteps.push_back(rec);
    }
  }

  // --- Stop detection ---
  // Already counted this track — skip
  const G4int trkID = trk->GetTrackID();
  if (fStoppedTrackIDs.count(trkID)) return;

  // Stop condition: KE ~ 0 (at post-step) inside one of the cylinders
  const G4double eKinPost_step = post->GetKineticEnergy();
  const G4double keEps         = 1.0 * keV;
  if (eKinPost_step > keEps) return;

  // Values to fill
  const G4double eKinInit = trk->GetVertexKineticEnergy();
  const auto     posPost  = post->GetPosition();
  const G4double x = posPost.x()/cm;
  const G4double y = posPost.y()/cm;
  const G4double z = posPost.z()/cm;

  const std::array<G4int,3> h1_keStop  = {1, 4, 7};
  const std::array<G4int,3> h1_zStop   = {2, 5, 8};
  const std::array<G4int,3> h2_xyStop  = {1, 2, 3};
  const std::array<G4int,3> h1_initKE  = {3, 6, 9};

  analysisManager->FillH1(h1_keStop[idx], eKinPost_step);
  analysisManager->FillH1(h1_zStop[idx], z);
  analysisManager->FillH2(h2_xyStop[idx], x, y);
  analysisManager->FillH1(h1_initKE[idx], eKinInit);

  // -----------------------------------------------------------------
  // [NEW] Flush buffered step records of this stopping track to the
  // stoppingSteps ntuple (ntupleId = 0). Track-level summary fields
  // are written on every row so a single (eventID, trackID) group is
  // self-contained for downstream selection.
  // -----------------------------------------------------------------
  if (fCurTrackID == trkID && !fCurSteps.empty()) {
    const G4Event* evt = G4RunManager::GetRunManager()->GetCurrentEvent();
    const G4int eventID = evt ? evt->GetEventID() : -1;
    const G4int nSteps  = (G4int)fCurSteps.size();
    G4double totalLen = 0., totalEDep = 0.;
    for (const auto& s : fCurSteps) { totalLen += s.stepLen; totalEDep += s.eDep; }

    for (const auto& s : fCurSteps) {
      analysisManager->FillNtupleIColumn(0,  0, eventID);
      analysisManager->FillNtupleIColumn(0,  1, fCurTrackID);
      analysisManager->FillNtupleIColumn(0,  2, s.stepNum);
      analysisManager->FillNtupleDColumn(0,  3, fCurEKinInit);
      analysisManager->FillNtupleDColumn(0,  4, s.kePre);
      analysisManager->FillNtupleDColumn(0,  5, s.dEdx);
      analysisManager->FillNtupleDColumn(0,  6, s.x);
      analysisManager->FillNtupleDColumn(0,  7, s.y);
      analysisManager->FillNtupleDColumn(0,  8, s.z);
      analysisManager->FillNtupleDColumn(0,  9, s.stepLen);
      analysisManager->FillNtupleDColumn(0, 10, s.eDep);
      analysisManager->FillNtupleDColumn(0, 11, s.cumLen);
      analysisManager->FillNtupleIColumn(0, 12, nSteps);
      analysisManager->FillNtupleDColumn(0, 13, totalLen);
      analysisManager->FillNtupleDColumn(0, 14, totalEDep);
      analysisManager->FillNtupleDColumn(0, 15, x);
      analysisManager->FillNtupleDColumn(0, 16, y);
      analysisManager->FillNtupleDColumn(0, 17, z);
      analysisManager->FillNtupleDColumn(0, 18, fCurVertex.x() / cm);
      analysisManager->FillNtupleDColumn(0, 19, fCurVertex.y() / cm);
      analysisManager->FillNtupleDColumn(0, 20, fCurVertex.z() / cm);
      analysisManager->FillNtupleDColumn(0, 21, fCurDir.x());
      analysisManager->FillNtupleDColumn(0, 22, fCurDir.y());
      analysisManager->FillNtupleDColumn(0, 23, fCurDir.z());
      analysisManager->AddNtupleRow(0);
    }
    fCurSteps.clear();
    fCurTrackID = -1;  // prevent double-flush
    fCurEventID = -1;
  }

  trk->SetTrackStatus(fStopAndKill);
}
