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
  G4int id = analysis->CreateH1("h1.1", "Stop: E_{kin} [MeV]", 200, -50., 50.);       // id = 1
  analysis->SetH1Activation(id, true);
  id = analysis->CreateH1("h1.2", "Stop: Z[cm]", 200, 0, 3.65);     
  analysis->SetH1Activation(id, true);
  id = analysis->CreateH2("h2.1", "Stop: XY;x [mm];y [mm]", 120, -50., 50., 120, -50., 50.);    // id = 3
  analysis->SetH2Activation(id, true);
  id = analysis->CreateH1("h1.3", "Stop: E_{kin}^{init} [MeV];counts", 200, 0., 50.);   // id = 4
  analysis->SetH1Activation(id, true);
  id = analysis->CreateH1("h1.4", "Beam initial KE;E [MeV];counts", 200, 0., 50.);
  analysis->SetH1Activation(id, true);

  // Incident muons in CylPV vs initial KE (denominator for stopping efficiency)
  id = analysis->CreateH1("h1.5", "Incident: E_{kin}^{init} [MeV];counts", 200, 0., 50.);  // id = 5
  analysis->SetH1Activation(id, true);

  // Stopping power curve: dE/dx [MeV/cm] vs KE [MeV], 1–100 MeV window
  id = analysis->CreateH2("h2.2",
                          "Stopping Power versus Kinetic Energy;Kinetic Energy [MeV];dE/dx [MeV/cm]",
                           49, 1., 50.,     // x: KE, 1 MeV/bin
                           200, 0., 40.);    // y: dE/dx, covers Bragg peak at low KE
  analysis->SetH2Activation(id, true);
  // id = analysis->CreateH1("h1.4", "Stopping efficiency vs. time", 200, 0., 50.);   // id = 5
  // analysis->SetH1Activation(id, true);

}
