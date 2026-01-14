#include "MargaritaPrimaryGenerator.hh"
#include "G4GeneralParticleSource.hh"
#include "G4Event.hh"

MargaritaPrimaryGenerator::MargaritaPrimaryGenerator()
: fGPS(new G4GeneralParticleSource()) {}

MargaritaPrimaryGenerator::~MargaritaPrimaryGenerator() {
  delete fGPS;
}

void MargaritaPrimaryGenerator::GeneratePrimaries(G4Event* event) {
    fGPS->GeneratePrimaryVertex(event);
    // Sanity check: Access the single primary and print out the values in the start up log
    auto vtx  = event->GetPrimaryVertex();
    auto part = vtx->GetPrimary();
    
    if (part) {
        G4double ke = part->GetKineticEnergy();
        G4cout << "Muon KE: " << ke/CLHEP::MeV << " MeV" << G4endl;
    }
}
