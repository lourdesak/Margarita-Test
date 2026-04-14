#include "MargaritaPrimaryGenerator.hh"
#include "G4GeneralParticleSource.hh"
#include "G4SingleParticleSource.hh"
#include "G4SPSEneDistribution.hh"
#include "G4Event.hh"
#include "G4SystemOfUnits.hh"

// Uniform energy scan: 1000 muons at each energy from 10 to 100 MeV in 1 MeV steps
// Total events = 91 energies x 1000 muons = 91 000  (set /run/beamOn 91000 in run.mac)
static const G4int kNEnergies      = 91;   // 10, 11, …, 100 MeV
static const G4int kMuonsPerEnergy = 1000;

MargaritaPrimaryGenerator::MargaritaPrimaryGenerator()
: fGPS(new G4GeneralParticleSource()) {}

MargaritaPrimaryGenerator::~MargaritaPrimaryGenerator() {
  delete fGPS;
}

void MargaritaPrimaryGenerator::GeneratePrimaries(G4Event* event) {
    static G4long eventCount = 0;

    // Determine which energy step we are on and set it
    G4int   energyIdx = (eventCount / kMuonsPerEnergy) % kNEnergies;
    G4double energy   = (10 + energyIdx) * MeV;

    G4SPSEneDistribution* eneDist = fGPS->GetCurrentSource()->GetEneDist();
    eneDist->SetEnergyDisType("Mono");
    eneDist->SetMonoEnergy(energy);

    fGPS->GeneratePrimaryVertex(event);
    ++eventCount;
}
