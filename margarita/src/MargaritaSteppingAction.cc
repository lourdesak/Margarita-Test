#include "MargaritaSteppingAction.hh"
#include "MargaritaRunAction.hh"

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

#include "G4AnalysisManager.hh"
#include <cmath>
#include <array>
#include <map>

MargaritaSteppingAction::MargaritaSteppingAction(MargaritaRunAction* run)
: frunAction(run) {}

MargaritaSteppingAction::~MargaritaSteppingAction() = default;

void MargaritaSteppingAction::UserSteppingAction(const G4Step* aStep)
{
  // Handles
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

  // --- Stopping power: one entry per muon ---
  // Accumulate total eDep and path length per track.  Fill h2.2 once
  // when the muon stops (KE~0) or is about to leave CylPV.
  struct TrackAccum { G4double eDep = 0.; G4double pathLen = 0.; };
  static std::map<G4int, TrackAccum> fSPaccum;

  const G4int    trkIDsp = trk->GetTrackID();
  const G4double stepLen = aStep->GetStepLength();
  const G4double eDep    = aStep->GetTotalEnergyDeposit();

  fSPaccum[trkIDsp].eDep    += eDep;
  fSPaccum[trkIDsp].pathLen += stepLen;

  // Check if this muon is done: stopped or about to leave the cylinder
  const G4double postKE = post->GetKineticEnergy();
  const G4bool   stopped = (postKE < 1.0 * keV);
  const G4bool   exiting = (trk->GetNextVolume() == nullptr ||
                            trk->GetNextVolume()->GetName() != "CylPV");

  if (stopped || exiting) {
    TrackAccum& acc = fSPaccum[trkIDsp];
    const G4double eKinInit = trk->GetVertexKineticEnergy();
    if (acc.pathLen > 0. && eKinInit >= 1.*MeV && eKinInit <= 100.*MeV) {
      const G4double dEdx = acc.eDep / acc.pathLen;
      analysisManager->FillH2(2, eKinInit/MeV, dEdx/(MeV/cm));
    }
    fSPaccum.erase(trkIDsp);
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
  

  trk->SetTrackStatus(fStopAndKill);
}