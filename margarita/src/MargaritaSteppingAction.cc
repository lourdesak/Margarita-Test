#include "MargaritaSteppingAction.hh"
#include "MargaritaRunAction.hh"

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

#include "G4AnalysisManager.hh"
#include <cmath>
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

  // Particle selection: mu- only (PDG 13), primaries only
  const G4int pdg = trk->GetDefinition()->GetPDGEncoding();
  if (pdg != 13) return;
  if (trk->GetParentID() != 0) return;

  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();

  // --- Stopping power: per-step dE/dx vs pre-step KE ---
  // Uses PRE-step volume (where the energy deposit actually occurs).
  // Both BoxPV and CylPV fill h2.2 (only one geometry active per run).
  {
    G4VPhysicalVolume* volPre =
        pre->GetTouchableHandle() ? pre->GetTouchableHandle()->GetVolume() : nullptr;
    if (volPre) {
      const G4String namePre = volPre->GetName();
      if (namePre == "BoxPV" || namePre == "CylPV") {
        const G4double eKinPre = pre->GetKineticEnergy();
        const G4double stepLen = aStep->GetStepLength();
        const G4double eDep    = aStep->GetTotalEnergyDeposit();

        if (stepLen > 0. && eDep > 0. && eKinPre <= 100.*MeV) {
          const G4double dEdx = eDep / stepLen;
          analysisManager->FillH2(2, eKinPre/MeV, dEdx/(MeV/cm));
        }
      }
    }
  }

  // --- Post-step volume gate (incident counter + stop detection) ---
  G4VPhysicalVolume* volPost =
      post->GetTouchableHandle() ? post->GetTouchableHandle()->GetVolume() : nullptr;
  if (!volPost) return;

  const G4String namePost = volPost->GetName();

  int idx = -1;
  if      (namePost == "CylPV" || namePost == "BoxPV")  idx = 0;
  else return;

  // --- Incident muon counter (one per muon entering the target) ---
  // Used as denominator for stopping efficiency: h1.5
  static std::set<G4int> fIncidentCounted;
  const G4int trkIDinc = trk->GetTrackID();
  if (!fIncidentCounted.count(trkIDinc)) {
    const G4double eKinInit_inc = trk->GetVertexKineticEnergy();
    analysisManager->FillH1(5, eKinInit_inc/MeV);
    fIncidentCounted.insert(trkIDinc);
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