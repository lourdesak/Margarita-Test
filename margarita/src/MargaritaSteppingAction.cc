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

#include <iomanip>
#include <iostream>

MargaritaSteppingAction::MargaritaSteppingAction(MargaritaActionBeam* eventAction,
                                                 MargaritaRunAction* runAction): G4UserSteppingAction(),
  fEventAction(eventAction),
  fRunAction(runAction)
{
}

MargaritaSteppingAction::~MargaritaSteppingAction() 
{
}

void MargaritaSteppingAction::UserSteppingAction(const G4Step* theStep)
{
    // Check for NULL volumes
    if(theStep->GetPreStepPoint()->GetPhysicalVolume() == NULL || 
       theStep->GetPostStepPoint()->GetPhysicalVolume() == NULL) return;

    // Get PDG encoding and filter out neutrinos
    G4int pdg = abs(theStep->GetTrack()->GetDefinition()->GetPDGEncoding());
    if (pdg == 12 || pdg == 14 || pdg == 16) return; // Don't save neutrino info

    // Get volume names
    std::string preStepPointName = 
        theStep->GetPreStepPoint()->GetPhysicalVolume()->GetLogicalVolume()->GetName();
    std::string postStepPointName = 
        theStep->GetPostStepPoint()->GetPhysicalVolume()->GetLogicalVolume()->GetName();

    // Check if step involves the cylinder detector volume
    if ((preStepPointName == G4String("CylLV")) || (postStepPointName == G4String("CylLV")))
    {
        // Get track information
        G4Track* track = theStep->GetTrack();
        G4int eventID = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();
        G4int trackID = track->GetTrackID();
        G4int parentID = track->GetParentID();
        G4int stepNo = track->GetCurrentStepNumber();

        // Get position at pre-step point
        G4ThreeVector position = theStep->GetPreStepPoint()->GetPosition();

        // Get momentum at pre-step point
        G4ThreeVector momentum = theStep->GetPreStepPoint()->GetMomentum();

        // Get kinetic energy at pre-step point
        G4double kineticEnergy = theStep->GetPreStepPoint()->GetKineticEnergy();

        // Write to output file
        std::ofstream& outFile = fRunAction->GetOutputStream();
        if (outFile.is_open()) {
            outFile << eventID << ","
                    << trackID << ","
                    << parentID << ","
                    << pdg << ","
                    << stepNo << ","
                    << std::fixed << std::setprecision(4)
                    << position.x()/mm << ","
                    << position.y()/mm << ","
                    << position.z()/mm << ","
                    << momentum.x()/MeV << ","
                    << momentum.y()/MeV << ","
                    << momentum.z()/MeV << ","
                    << kineticEnergy/MeV
                    << std::endl;
        }
    }
}

