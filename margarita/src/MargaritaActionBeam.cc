#include "MargaritaActionBeam.hh"
#include "MargaritaSteppingAction.hh"
#include "MargaritaPrimaryGenerator.hh"
#include "MargaritaRunAction.hh"

ActionInitialization::ActionInitialization() : G4VUserActionInitialization() {}

ActionInitialization::~ActionInitialization() {}
void ActionInitialization::BuildForMaster() const
{
  SetUserAction(new MargaritaRunAction);
}

void ActionInitialization::Build() const {
  auto runAction = new MargaritaRunAction();
  SetUserAction(runAction);

  SetUserAction(new MargaritaPrimaryGenerator());
  SetUserAction(new MargaritaSteppingAction(runAction));
}
