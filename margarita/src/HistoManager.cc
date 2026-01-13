#include "HistoManager.hh"
#include "G4UnitsTable.hh"
#include "G4AnalysisManager.hh"   // <-- needed for G4AnalysisManager

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
  G4double vmin = 0.;
  G4double vmax = 100.;

  // Beam Stuff
  analysis->SetHistoDirectoryName("histo");
  analysis->SetFirstHistoId(1);

  // --- Removed the four initial Beam H1s so that the Stopping Muon H1s take IDs 1..4 ---
  // G4int id = analysis->CreateH1("h1.1", "KE of primary muons", nbins, vmin, vmax);
  // analysis->SetH1Activation(id, false);
  // id = analysis->CreateH1("h1.2", "vertex dist dN/dv = f(r)", nbins, vmin, vmax);
  // analysis->SetH1Activation(id, false);
  // id = analysis->CreateH1("h1.3", "direction: cos(theta)", nbins, vmin, vmax);
  // analysis->SetH1Activation(id, false);
  // id = analysis->CreateH1("h1.4", "direction: phi", nbins, vmin, vmax);
  // analysis->SetH1Activation(id, false);

  // histos 2D (unchanged)
  G4int id = analysis->CreateH2("h2.1", "vertex: XY", nbins, vmin, vmax, nbins, vmin, vmax);
  analysis->SetH2Activation(id, false);

  id = analysis->CreateH2("h2.2", "vertex: YZ", nbins, vmin, vmax, nbins, vmin, vmax);
  analysis->SetH2Activation(id, false);

  id = analysis->CreateH2("h2.3", "vertex: ZX", nbins, vmin, vmax, nbins, vmin, vmax);
  analysis->SetH2Activation(id, false);

  id = analysis->CreateH2("h2.4", "direction: phi-cos(theta)", nbins, vmin, vmax, nbins, vmin, vmax);
  analysis->SetH2Activation(id, false);

  id = analysis->CreateH2("h2.5", "direction: phi-theta", nbins, vmin, vmax, nbins, vmin, vmax);
  analysis->SetH2Activation(id, false);

  // Stopping Muons stuff
  // After removing the four initial H1s above, the following H1s become IDs 1..4:
  id = analysis->CreateH1("h3.1", "Stop: KE;E_{kin} [MeV];counts", 200, -50., 50.); // id = 1
  analysis->SetH1Activation(id, true);

  id = analysis->CreateH1("h3.2", "Stop: Z;z [mm];counts", 200, -1000., 1000.);   // id = 2
  analysis->SetH1Activation(id, true);

//  id = analysis->CreateH1("h3.3", "Stop: cos#theta;cos#theta;counts", 100, -1., 1.); // id = 3
//  analysis->SetH1Activation(id, true);

  id = analysis->CreateH1("h3.4", "Stop: Initial KE;E_{kin}^{init} [MeV];counts",
                          200, 0., 50.);                                             // id = 4
  analysis->SetH1Activation(id, true);

  // Extra H2 (unchanged)
  id = analysis->CreateH2("h3.5", "Stop: XY;x [mm];y [mm]",
                          120, -50., 50., 120, -50., 50.);
  analysis->SetH2Activation(id, true);     // H2 id continues after h2.5

  // nTuples for Beam stuff (unchanged)
  analysis->SetNtupleDirectoryName("ntuple");
  analysis->SetFirstNtupleId(1);

  analysis->CreateNtuple("101", "Primary Particle Tuple");
  analysis->CreateNtupleIColumn("particleID");   // 0
  analysis->CreateNtupleDColumn("Ekin");         // 1
  analysis->CreateNtupleDColumn("posX");         // 2
  analysis->CreateNtupleDColumn("posY");         // 3
  analysis->CreateNtupleDColumn("posZ");         // 4
  analysis->CreateNtupleDColumn("dirTheta");     // 5
  analysis->CreateNtupleDColumn("dirPhi");       // 6
  analysis->CreateNtupleDColumn("weight");       // 7
  analysis->FinishNtuple();

  analysis->SetNtupleActivation(true);

  // nTuples for Stopping Muon stuff (unchanged)
  analysis->CreateNtuple("201", "Stopped primary mu-");
  analysis->CreateNtupleDColumn("eKin");       // 0  (KE at stop)
  analysis->CreateNtupleDColumn("posX");       // 1
  analysis->CreateNtupleDColumn("posY");       // 2
  analysis->CreateNtupleDColumn("posZ");       // 3
  analysis->CreateNtupleDColumn("momX");       // 4
  analysis->CreateNtupleDColumn("momY");       // 5
  analysis->CreateNtupleDColumn("momZ");       // 6
  analysis->CreateNtupleIColumn("pdg");        // 7
  analysis->CreateNtupleDColumn("ekin_init");  // 8
  analysis->FinishNtuple();

  analysis->SetNtupleActivation(false);
}
