#include "MargaritaRunAction.hh"
#include "HistoManager.hh"
#include "G4Run.hh"

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

