#include "MargaritaSteppingAction.hh"
#include "MargaritaRunAction.hh"
#include "G4Step.hh"

MargaritaSteppingAction::MargaritaSteppingAction(MargaritaRunAction* runAction)
: fMargaritaRunAction(runAction) {}

MargaritaSteppingAction::~MargaritaSteppingAction() = default;

void MargaritaSteppingAction::UserSteppingAction(const G4Step* /*aStep*/)
{
  // intentionally left blank — add step-level logic here later if needed
}
