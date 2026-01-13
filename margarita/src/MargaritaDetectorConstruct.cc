#include "MargaritaDetectorConstruct.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"

#include "G4Tubs.hh"

// --- Minimal helper: make "Vacuum" visible (needed for ParaView) ---
//static G4VisAttributes* GetMaterialVisAttrib(const G4String& mName) {
//  if (mName == "Vacuum" || mName == "G4_Galactic") {
//    return new G4VisAttributes(true); // originally false; must be true for ParaView
//  }
//  return nullptr; // no change for other materials
//}

DetectorConstruction::DetectorConstruction() {}
DetectorConstruction::~DetectorConstruction() {}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
    // NIST manager for materials
    G4NistManager* nist = G4NistManager::Instance();
    G4Material* worldMat = nist->FindOrBuildMaterial("G4_AIR");

    // World solid (1 m cube)
    G4Box* WorldVol = new G4Box("WorldVol", 5 * m, 5 * m, 10 * m); //Half-meas so 10x10x20m cube

    // World logical volume
    G4LogicalVolume* logicWorld = new G4LogicalVolume(WorldVol,
                                                      worldMat,
                                                      "logicWorld");

    // World placement
    G4VPhysicalVolume* physWorld = new G4PVPlacement(0,
                                                     G4ThreeVector(0., 0., 0.),
                                                     logicWorld,
                                                     "physWorld",
                                                     0,
                                                     false,
                                                     0,
                                                     true);
    // --- ADDED: cylinder placed inside logicWorld (can be a mother for daughters) ---
    G4Material* cylMat = nist->FindOrBuildMaterial("G4_WATER");  // choose any NIST material
    auto* cylSolid = new G4Tubs("CylSolid",
                                0.*cm,     // inner radius
                                3.65*cm,    // outer radius
                                3.65*cm,    // half-length (height = 7.3 cm)
                                0.*deg, 360.*deg);

    auto* cylLV = new G4LogicalVolume(cylSolid, cylMat, "CylLV");
    auto cylVis = new G4VisAttributes(G4Colour(0.2,0.7,0.9,1.0));
    cylVis->SetForceSolid(true);
    cylLV->SetVisAttributes(cylVis);

    // place at the world origin; parent is logicWorld
    auto rotX90 = new G4RotationMatrix();
    rotX90->rotateX(90.*deg);       // +90° about X (Tubs axis Z -> points along -Y)

    new G4PVPlacement(rotX90,                     // rotation
                      G4ThreeVector(0.,0.,0*m), // position
                      //G4ThreeVector(0.,0.,2.5*m), // position
                      cylLV, "CylPV",
                      logicWorld, false, 0, true);

//    new G4PVPlacement(rotX90,
//                      G4ThreeVector(0., 0., 1.*m),
//                      cylLV, "CylPV2",
//                      logicWorld, false, 1, true);
//
//    new G4PVPlacement(rotX90,
//                      G4ThreeVector(0., 0., 2.*m),
//                      cylLV, "CylPV3",
//                      logicWorld, false, 2, true);
    // --- END ADDED ---
    G4cout << "[Mat] World: " << logicWorld->GetMaterial()->GetName() << G4endl;
    G4cout << "[Mat] Cyl  : " << cylLV->GetMaterial()->GetName() << G4endl;

    return physWorld;
}

