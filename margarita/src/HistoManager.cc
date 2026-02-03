#include "HistoManager.hh"
#include "G4UnitsTable.hh"
#include "G4AnalysisManager.hh"   

HistoManager::HistoManager() : fFileName("g4marg")
{
  Book();
}
HistoManager::~HistoManager() {}

void HistoManager::Book()
{
  G4AnalysisManager* analysis = G4AnalysisManager::Instance();

  analysis->SetDefaultFileType("root");
  analysis->SetFileName(fFileName);
  analysis->SetVerboseLevel(1);
  analysis->SetActivation(true);

  G4int nbins = 100;
  G4double vmin = 0;
  G4double vmax = 100;

  // Beam Stuff
  analysis->SetHistoDirectoryName("histo");
  analysis->SetFirstHistoId(1);

  // Stopping Muons stuff
  G4int id = analysis->CreateH1("h1.1", "Stop: KE;E_{kin} [MeV];counts", 200, -50., 50.);       // id = 1
  analysis->SetH1Activation(id, true);
  id = analysis->CreateH1("h1.2", "Stop: Z;z [mm];counts", 200, -100, 100);                 // id = 2
  analysis->SetH1Activation(id, true);
  id = analysis->CreateH2("h2.3", "Stop: XY;x [mm];y [mm]", 120, -50., 50., 120, -50., 50.);    // id = 3
  analysis->SetH2Activation(id, true);     
  id = analysis->CreateH1("h1.4", "Stop: Initial KE;E_{kin}^{init} [MeV];counts", 200, 0., 50.);// id = 4
  analysis->SetH1Activation(id, true);
}
