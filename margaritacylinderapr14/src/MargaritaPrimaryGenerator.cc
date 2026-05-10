#include "MargaritaPrimaryGenerator.hh"
#include "G4GeneralParticleSource.hh"
#include "G4Event.hh"
#include "G4PrimaryVertex.hh"
#include "G4PrimaryParticle.hh"
#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"

MargaritaPrimaryGenerator::MargaritaPrimaryGenerator()
: fGPS(new G4GeneralParticleSource()) {}

MargaritaPrimaryGenerator::~MargaritaPrimaryGenerator() {
  delete fGPS;
}

void MargaritaPrimaryGenerator::GeneratePrimaries(G4Event* event)
{
  fGPS->GeneratePrimaryVertex(event);

  // Fill beam initial KE for every generated primary
  G4PrimaryVertex* vertex = event->GetPrimaryVertex();
  if (!vertex) return;

  G4PrimaryParticle* particle = vertex->GetPrimary();
  if (!particle) return;

  G4double eKin = particle->GetKineticEnergy();

  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  analysisManager->FillH1(4, eKin / MeV);   
}