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
  int idx = -1;
  if      (namePost == "CylPV")  idx = 0;
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

  // If this track is already being terminated, don't count it again
  if (trk->GetTrackStatus() == fStopAndKill) return;

  // Values to fill
  const G4double eKinInit = trk->GetVertexKineticEnergy();
  const auto     posPost  = post->GetPosition();
  const G4double x        = posPost.x();
  const G4double y        = posPost.y();
  const G4double z        = posPost.z();

  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();

  const std::array<G4int,3> h1_keStop  = {1, 4, 7};
  const std::array<G4int,3> h1_zStop   = {2, 5, 8};
  const std::array<G4int,3> h2_xyStop  = {1, 2, 3};
  const std::array<G4int,3> h1_initKE  = {3, 6, 9};

  analysisManager->FillH1(h1_keStop[idx], eKinPost_step);
  analysisManager->FillH1(h1_zStop[idx],  z);
  analysisManager->FillH2(h2_xyStop[idx], x, y);
  analysisManager->FillH1(h1_initKE[idx], eKinInit);

  // Ensure this muon cannot be counted again on subsequent steps
  trk->SetTrackStatus(fStopAndKill);
}