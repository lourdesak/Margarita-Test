#include "MargaritaDetectorConstruct.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"

#include "G4Tubs.hh"
#include "G4RotationMatrix.hh"

// --- Paraview necessity: Minimal helper: make "Vacuum" visible ---
//    return new G4VisAttributes(true); // originally false; must be true for ParaView

DetectorConstruction::DetectorConstruction()
{
    fMessenger = new G4GenericMessenger(this, "/margarita/", "Margarita commands");
    fMessenger->DeclareProperty("detectorGeometry", fGeometry,
        "Target geometry: 'box' or 'cyl'. Set before /run/initialize.");
}
DetectorConstruction::~DetectorConstruction() { delete fMessenger; }

G4VPhysicalVolume* DetectorConstruction::Construct()
{
    // NIST manager for materials
    G4NistManager* nist = G4NistManager::Instance();
    G4Material* worldMat = nist->FindOrBuildMaterial("G4_AIR");

    // World solid (1 m cube)
    G4Box* WorldVol = new G4Box("WorldVol", 25 * cm, 25 * cm, 25 * cm); //Half-meas so 50 cm cube

    // World logical volume
    G4LogicalVolume* logicWorld = new G4LogicalVolume(WorldVol,worldMat,"logicWorld");

    // World placement
    G4VPhysicalVolume* physWorld = new G4PVPlacement(0,G4ThreeVector(0., 0., 0.),logicWorld,"physWorld",0,false,0,true);

    auto* blueOutline = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0));
    blueOutline->SetVisibility(true);
    blueOutline->SetForceWireframe(true);
    logicWorld->SetVisAttributes(blueOutline);

    G4Material* targetMat = nist->FindOrBuildMaterial("G4_WATER");

    if (fGeometry == "cyl") {
        auto* cylSolid = new G4Tubs("CylSolid",
                                    0.*cm,      // inner radius
                                    3.65*cm,    // outer radius
                                    3.65*cm,    // half-length (height = 7.3 cm)
                                    0.*deg, 360.*deg);
        auto* cylLV = new G4LogicalVolume(cylSolid, targetMat, "CylLV");

        auto* cylVis = new G4VisAttributes(G4Colour(0.2, 0.7, 0.9, 1.0));
        cylVis->SetForceSolid(true);
        cylLV->SetVisAttributes(cylVis);

        auto* rotX90 = new G4RotationMatrix();
        rotX90->rotateX(90.*deg);  // Tubs axis Z -> points along -Y
        new G4PVPlacement(rotX90,
                          G4ThreeVector(0., 0., 0.),
                          cylLV, "CylPV",
                          logicWorld, false, 0, true);

        G4cout << "[Mat] World: " << logicWorld->GetMaterial()->GetName() << G4endl;
        G4cout << "[Mat] Cyl  : " << cylLV->GetMaterial()->GetName() << G4endl;
    }
    else {
        // Default: box
        auto* boxSolid = new G4Box("BoxSolid",
                                3.65 * cm,   // half x  -> 7.3 cm
                                3.65 * cm,   // half y  -> 7.3 cm
                                3.65 * cm);  // half z  -> 7.3 cm depth

        auto* boxLV = new G4LogicalVolume(boxSolid, targetMat, "BoxLV");

        auto* boxVis = new G4VisAttributes(G4Colour(0.2, 0.7, 0.9, 1.0));
        boxVis->SetForceSolid(true);
        boxLV->SetVisAttributes(boxVis);

        new G4PVPlacement(0,
                        G4ThreeVector(0., 0., 0.),
                        boxLV, "BoxPV",
                        logicWorld, false, 0, true);

        G4cout << "[Mat] World: " << logicWorld->GetMaterial()->GetName() << G4endl;
        G4cout << "[Mat] Box  : " << boxLV->GetMaterial()->GetName() << G4endl;
    }

    return physWorld;
}
