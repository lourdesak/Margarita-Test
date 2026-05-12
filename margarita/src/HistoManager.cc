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
  id = analysis->CreateH2("h2.1", "Stop: XY;x [cm];y [cm]", 120, -2., 7.5, 120, -2., 7.5);    // id = 3
  analysis->SetH2Activation(id, true);
  id = analysis->CreateH1("h1.3", "Stop: E_{kin}^{init} [MeV];counts", 200, 0., 50.);   // id = 4
  analysis->SetH1Activation(id, true);
  id = analysis->CreateH1("h1.4", "Beam initial KE;E [MeV];counts", 200, 0., 50.);
  analysis->SetH1Activation(id, true);

  // Incident muons in CylPV vs initial KE (denominator for stopping efficiency)
  id = analysis->CreateH1("h1.5", "Incident: E_{kin}^{init} [MeV];counts", 200, 0., 50.);  // id = 5
  analysis->SetH1Activation(id, true);

  // Stopping power curve: dE/dx [MeV/cm] vs KE [MeV], 0–100 MeV window
  id = analysis->CreateH2("h2.2",
                          "Stopping Power versus Kinetic Energy;Kinetic Energy [MeV];dE/dx [MeV/cm]",
                           100, 0., 100.,    // x: KE, 1 MeV/bin, starts at 0
                           200, 0., 40.);    // y: dE/dx, covers Bragg peak at low KE
  analysis->SetH2Activation(id, true);
  // id = analysis->CreateH1("h1.4", "Stopping efficiency vs. time", 200, 0., 50.);   // id = 5
  // analysis->SetH1Activation(id, true);

  // ------------------------------------------------------------------
  // Single-track diagnostic ntuple (per-step records of stopping muons)
  // Each row = one step of a primary mu- that ultimately stops in the
  // active volume. Track-level summary fields are repeated on each row
  // so a single (eventID, trackID) group is self-contained.
  // ------------------------------------------------------------------
  analysis->SetNtupleDirectoryName("ntuple");
  analysis->SetFirstNtupleId(0);
  analysis->CreateNtuple("stoppingSteps", "Per-step records of stopping muons");
  analysis->CreateNtupleIColumn("eventID");   // 0
  analysis->CreateNtupleIColumn("trackID");   // 1
  analysis->CreateNtupleIColumn("stepNum");   // 2
  analysis->CreateNtupleDColumn("eKinInit");  // 3  initial KE [MeV]
  analysis->CreateNtupleDColumn("kePre");     // 4  pre-step KE [MeV]
  analysis->CreateNtupleDColumn("dEdx");      // 5  per-step dE/dx [MeV/cm]
  analysis->CreateNtupleDColumn("x");         // 6  post-step x [cm]
  analysis->CreateNtupleDColumn("y");         // 7  post-step y [cm]
  analysis->CreateNtupleDColumn("z");         // 8  post-step z [cm]
  analysis->CreateNtupleDColumn("stepLen");   // 9  step length [cm]
  analysis->CreateNtupleDColumn("eDep");      // 10 step energy deposit [MeV]
  analysis->CreateNtupleDColumn("cumLen");    // 11 cumulative path length up to this step [cm]
  analysis->CreateNtupleIColumn("nSteps");    // 12 total steps for this track
  analysis->CreateNtupleDColumn("totalLen");  // 13 total track length [cm]
  analysis->CreateNtupleDColumn("totalEDep"); // 14 total deposited energy [MeV]
  analysis->CreateNtupleDColumn("stopX");     // 15 stopping x [cm]
  analysis->CreateNtupleDColumn("stopY");     // 16 stopping y [cm]
  analysis->CreateNtupleDColumn("stopZ");     // 17 stopping z [cm]
  analysis->CreateNtupleDColumn("vtxX");      // 18 vertex x [cm]
  analysis->CreateNtupleDColumn("vtxY");      // 19 vertex y [cm]
  analysis->CreateNtupleDColumn("vtxZ");      // 20 vertex z [cm]
  analysis->CreateNtupleDColumn("dirX");      // 21 initial momentum direction x
  analysis->CreateNtupleDColumn("dirY");      // 22 initial momentum direction y
  analysis->CreateNtupleDColumn("dirZ");      // 23 initial momentum direction z
  analysis->FinishNtuple();

  // ------------------------------------------------------------------
  // Incident-muon ntuple (ntupleId = 1).
  // One row per primary mu- the first time it is seen inside the
  // active detector volume — i.e. the *denominator* for an absolute
  // stopping-efficiency map. Vertex (vtxX/Y/Z) and initial KE are
  // recorded so downstream macros can bin by entry coordinate.
  // ------------------------------------------------------------------
  analysis->CreateNtuple("incidentMuons", "One row per incident primary mu- in the detector");
  analysis->CreateNtupleIColumn("eventID");   // 0
  analysis->CreateNtupleIColumn("trackID");   // 1
  analysis->CreateNtupleDColumn("vtxX");      // 2  vertex x [cm]
  analysis->CreateNtupleDColumn("vtxY");      // 3  vertex y [cm]
  analysis->CreateNtupleDColumn("vtxZ");      // 4  vertex z [cm]
  analysis->CreateNtupleDColumn("eKinInit");  // 5  initial KE [MeV]
  analysis->FinishNtuple();
}
