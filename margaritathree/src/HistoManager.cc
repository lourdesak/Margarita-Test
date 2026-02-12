#include "HistoManager.hh"
#include "G4UnitsTable.hh"
#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"

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

  // Directories / IDs
  analysis->SetHistoDirectoryName("histo");
  analysis->SetFirstHistoId(1);  // H1 ids start at 1

  // ---- 12 histograms total: 4 per cylinder x 3 cylinders ----
  // Cylinder 1 (CylPV):   H1 ids 1,2,3 and H2 id 1
  // Cylinder 2 (CylPV2):  H1 ids 4,5,6 and H2 id 2
  // Cylinder 3 (CylPV3):  H1 ids 7,8,9 and H2 id 3

  // ---------------- CylPV ----------------
  G4int id = analysis->CreateH1("h1.1", "CylPV Stop: E_{kin} [MeV]", 200, -50., 50.); // H1 id=1
  analysis->SetH1Activation(id, true);

  id = analysis->CreateH1("h1.2", "CylPV Stop: Z [mm]", 200, -50., 50.);           // H1 id=2
  analysis->SetH1Activation(id, true);

  id = analysis->CreateH2("h2.1", "CylPV Stop: XY;x [mm];y [mm]",
                          120, -50., 50., 120, -50., 50.);                           // H2 id=1
  analysis->SetH2Activation(id, true);

  id = analysis->CreateH1("h1.3", "CylPV Stop: E_{kin}^{init} [MeV];counts", 200, 0., 50.); // H1 id=3
  analysis->SetH1Activation(id, true);

  // ---------------- CylPV2 ----------------
  id = analysis->CreateH1("h1.4", "CylPV2 Stop: E_{kin} [MeV]", 200, -50., 50.);     // H1 id=4
  analysis->SetH1Activation(id, true);

  id = analysis->CreateH1("h1.5", "CylPV2 Stop: Z [mm]", 200, -50., 50.);          // H1 id=5
  analysis->SetH1Activation(id, true);

  id = analysis->CreateH2("h2.2", "CylPV2 Stop: XY;x [mm];y [mm]",
                          120, -50., 50., 120, -50., 50.);                           // H2 id=2
  analysis->SetH2Activation(id, true);

  id = analysis->CreateH1("h1.6", "CylPV2 Stop: E_{kin}^{init} [MeV];counts", 200, 0., 50.); // H1 id=6
  analysis->SetH1Activation(id, true);

  // ---------------- CylPV3 ----------------
  id = analysis->CreateH1("h1.7", "CylPV3 Stop: E_{kin} [MeV]", 200, -50., 50.);     // H1 id=7
  analysis->SetH1Activation(id, true);

  id = analysis->CreateH1("h1.8", "CylPV3 Stop: Z [mm]", 200, -50., 50.);          // H1 id=8
  analysis->SetH1Activation(id, true);

  id = analysis->CreateH2("h2.3", "CylPV3 Stop: XY;x [mm];y [mm]",
                          120, -50., 50., 120, -50., 50.);                           // H2 id=3
  analysis->SetH2Activation(id, true);

  id = analysis->CreateH1("h1.9", "CylPV3 Stop: E_{kin}^{init} [MeV];counts", 200, 0., 50.); // H1 id=9
  analysis->SetH1Activation(id, true);
}
