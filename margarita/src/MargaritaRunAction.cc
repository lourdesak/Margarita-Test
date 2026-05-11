#include "MargaritaRunAction.hh"
#include "HistoManager.hh"
#include "MargaritaDetectorConstruct.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "TROOT.h"

MargaritaRunAction::MargaritaRunAction() : G4UserRunAction(), fHistoManager(0)
{
  fHistoManager = new HistoManager();
}

MargaritaRunAction::~MargaritaRunAction()
{
  delete fHistoManager;
}

void MargaritaRunAction::BeginOfRunAction(const G4Run*)
{
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  if (analysisManager->IsActive()) {
    auto* det = dynamic_cast<const DetectorConstruction*>(
        G4RunManager::GetRunManager()->GetUserDetectorConstruction());
    if (det) {
      const G4String& g = det->GetGeometry();
      analysisManager->SetFileName(g == "cyl" ? "g4marg_cyl" : "g4marg");
    }
    analysisManager->OpenFile();
  }
}

void MargaritaRunAction::EndOfRunAction(const G4Run*)
{
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  if (analysisManager->IsActive()) {
    analysisManager->Write();
    analysisManager->CloseFile();

    auto* det = dynamic_cast<const DetectorConstruction*>(
        G4RunManager::GetRunManager()->GetUserDetectorConstruction());
    const bool isCyl = det && det->GetGeometry() == "cyl";
    const G4String rootFile = isCyl ? "g4marg_cyl.root" : "g4marg.root";
    const G4String cmd = ".x plotstoppingmuons.C(\"" + rootFile + "\")";

    G4cout << "[Margarita] EndOfRunAction: det=" << (det ? "OK" : "NULL")
           << " geometry=\"" << (det ? det->GetGeometry() : G4String("?"))
           << "\" rootFile=" << rootFile
           << " cmd=" << cmd << G4endl;

    gROOT->ProcessLine(cmd.c_str());
  }
}
