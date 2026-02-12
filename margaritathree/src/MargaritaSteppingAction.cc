#include "MargaritaSteppingAction.hh"
#include "MargaritaRunAction.hh"

#include "G4Step.hh"
#include "G4Track.hh"
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
  G4StepPoint* post = aStep->GetPostStepPoint();
  G4Track*     trk  = aStep->GetTrack();

  // Volume at post-step
  G4VPhysicalVolume* volPost =
      post->GetTouchableHandle() ? post->GetTouchableHandle()->GetVolume() : nullptr;
  if (!volPost) return;

  const G4String namePost = volPost->GetName();

  // Select which cylinder we are in
  // idx = 0 -> CylPV, idx = 1 -> CylPV2, idx = 2 -> CylPV3
  int idx = -1;
  if      (namePost == "CylPV")  idx = 0;
  else if (namePost == "CylPV2") idx = 1;
  else if (namePost == "CylPV3") idx = 2;
  else return;

  // Particle selection: mu- only (PDG 13)
  const G4int pdg = trk->GetDefinition()->GetPDGEncoding();
  if (pdg != 13) return;

  // Primaries only
  if (trk->GetParentID() != 0) return;

  // Stop condition: KE ~ 0 (at post-step) inside one of the cylinders
  const G4double eKinPost_step = post->GetKineticEnergy();
  const G4double keEps         = 1.0 * keV;
  if (eKinPost_step > keEps) return;

  // Values to fill
  const G4double eKinInit = trk->GetVertexKineticEnergy();
  const auto     posPost  = post->GetPosition();
  const G4double x        = posPost.x();
  const G4double y        = posPost.y();
  const G4double z        = posPost.z();

  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();

  // IMPORTANT: These IDs must match what you CreateH1/CreateH2 in HistoManager.
  //
  // We assume you will create 12 histograms total, in this order:
  // CylPV:   H1(KEstop)=1, H1(Z)=2, H2(XY)=1, H1(InitKE)=3   (example layout below uses per-cylinder blocks)
  //
  // A clean scheme is per-cylinder blocks:
  // Cylinder 0 (CylPV):   H1 ids 1,2,3 and H2 id 1
  // Cylinder 1 (CylPV2):  H1 ids 4,5,6 and H2 id 2
  // Cylinder 2 (CylPV3):  H1 ids 7,8,9 and H2 id 3

  const std::array<G4int,3> h1_keStop  = {1, 4, 7};
  const std::array<G4int,3> h1_zStop   = {2, 5, 8};
  const std::array<G4int,3> h2_xyStop  = {1, 2, 3};
  const std::array<G4int,3> h1_initKE  = {3, 6, 9};

  analysisManager->FillH1(h1_keStop[idx], eKinPost_step);
  analysisManager->FillH1(h1_zStop[idx],  z);
  analysisManager->FillH2(h2_xyStop[idx], x, y);
  analysisManager->FillH1(h1_initKE[idx], eKinInit);
}
