//Source code for the Margarita Standalone project -LasR

#include <iostream>
#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4VisManager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "globals.hh"
#include "MargaritaDetectorConstruct.hh"

#include "QGSP_BERT.hh"
#include "MargaritaActionBeam.hh"

int main(int argc, char** argv)
{
    G4RunManager *runManager = new G4RunManager();
    
    runManager ->SetUserInitialization(new DetectorConstruction ()); //Cyl stuff
    runManager->SetUserInitialization(new QGSP_BERT); //Physics list stuff
    runManager->SetUserInitialization(new ActionInitialization); //Beam stuff
    runManager ->Initialize(); //responsible for error gen
    
    G4UIExecutive *ui = new G4UIExecutive(argc, argv);
    G4VisManager *visManager = new G4VisExecutive();
    visManager->Initialize();
    G4UImanager *UImanager = G4UImanager::GetUIpointer();
    
    // === Minimal addition: run macro if provided ===
    if (argc >= 2) {
        G4String command = "/control/execute ";
        G4String fileName = argv[1];
        UImanager->ApplyCommand(command + fileName);
    }
    // ===============================================
    UImanager->ApplyCommand("/vis/list"); //Test: Gives list of visual drivers you got eg. QT, heprep
    UImanager -> ApplyCommand("/vis/open OGL");
    UImanager -> ApplyCommand("/vis/drawVolume");
    UImanager -> ApplyCommand("/vis/viewer/set/autoRefresh true");
    UImanager -> ApplyCommand("/vis/scene/add/trajectories smooth");
//Beam specifications: Avoiding macro use temporarily
//    UImanager->ApplyCommand("/gps/pos/type Plane");
//    UImanager->ApplyCommand("/gps/pos/shape Circle");
//    UImanager->ApplyCommand("/gps/direction 0 0 1.0 * CLHEP::m");
//    UImanager->ApplyCommand("/gps/pos/centre 0 0 0 * CLHEP::m");
//    UImanager->ApplyCommand("/gps/pos/radius 3.65 * CLHEP::cm");
    
    //ui->SessionStart();
    return 0;
}
