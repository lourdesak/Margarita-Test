#ifndef CONSTRUCTION_HH
#define CONSTRUCTION_HH

#include "G4SystemOfUnits.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4VPhysicalVolume.hh"
#include "G4Box.hh"
#include "G4PVPlacement.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4GenericMessenger.hh"

class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
    DetectorConstruction();
    ~DetectorConstruction();

    virtual G4VPhysicalVolume* Construct(); //Strictly just for Paraview

    const G4String& GetGeometry() const { return fGeometry; }

private:
  G4VisAttributes* GetMaterialVisAttrib(const G4String& mName); //End for paraview
  G4GenericMessenger* fMessenger = nullptr;
  G4String fGeometry = "box";  // /margarita/detectorGeometry box|cyl
};

#endif
