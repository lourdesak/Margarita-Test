#include "MargaritaRunAction.hh"
#include "HistoManager.hh"
#include "MargaritaDetectorConstruct.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"

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
  // Histograms
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  if (analysisManager->IsActive()) {
    // Pick filename from the active geometry: g4marg.root (box) or g4marg_cyl.root (cyl)
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
  // Save histograms
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  if (analysisManager->IsActive()) {
    analysisManager->Write();
    analysisManager->CloseFile();
  }
}

