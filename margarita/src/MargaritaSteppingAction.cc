// #include "MargaritaSteppingAction.hh"
// #include "MargaritaRunAction.hh"
// #include "G4Step.hh"

// MargaritaSteppingAction::MargaritaSteppingAction(MargaritaRunAction* runAction)
// : fMargaritaRunAction(runAction) {}

// MargaritaSteppingAction::~MargaritaSteppingAction() = default;

// void MargaritaSteppingAction::UserSteppingAction(const G4Step* /*aStep*/)
// {
// }

//
// ********************************************************************
// * SteppingAction.cc                                                *
// * Implementation of particle track recording                       *
// ********************************************************************
//

#include "MargaritaSteppingAction.hh"
#include "MargaritaActionBeam.hh"
#include "MargaritaRunAction.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4EventManager.hh"
#include "G4TrackingManager.hh"


#include "HistoManager.hh"
#include "G4AnalysisManager.hh"
#include "G4TrackingManager.hh"
#include "G4PhysicalConstants.hh"

#include <iomanip>
#include <iostream>
#include <fstream>

MargaritaSteppingAction::MargaritaSteppingAction( MargaritaRunAction* run)
{
    frunAction = run;
}

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

  // Target volume name
  static const G4String kTargetVolumeName = "CylPV";

  // Particle selection: mu- only (PDG 13)
  const G4int pdg = trk->GetDefinition()->GetPDGEncoding();
  if (pdg != 13) return;

  // Primaries only
  if (trk->GetParentID() != 0) return;

  // Stop condition: inside target and KE ~ 0
  const G4double eKinPost_step = post->GetKineticEnergy();
  const G4double keEps         = 1.0*keV;
  if (namePost != kTargetVolumeName || eKinPost_step > keEps) return;

  const G4double eKinInit = trk->GetVertexKineticEnergy();   // initial KE of this primary
  const auto     posPost = post->GetPosition();
  const G4double x       = posPost.x();
  const G4double y       = posPost.y();
  const G4double z       = posPost.z();

  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();

  static const G4int kH1_KE_Stop_Id  = 1; 
  static const G4int kH1_Z_Stop_Id   = 2; 
  static const G4int kH2_XY_Stop_Id  = 1; 
  static const G4int kH1_InitKE_Id   = 3; 

  analysisManager->FillH1(kH1_KE_Stop_Id, eKinPost_step);
  analysisManager->FillH1(kH1_Z_Stop_Id,  z);
  analysisManager->FillH2(kH2_XY_Stop_Id, x, y);
  analysisManager->FillH1(kH1_InitKE_Id,  eKinInit);
}

