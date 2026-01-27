#ifndef MARGARITA_STEPPING_ACTION_HH
#define MARGARITA_STEPPING_ACTION_HH

#include "G4UserSteppingAction.hh"

class G4Step;
class MargaritaRunAction;

class MargaritaSteppingAction : public G4UserSteppingAction {
public:
  explicit MargaritaSteppingAction(MargaritaRunAction* runAction);

  void UserSteppingAction(const G4Step* step) override;

private:
  MargaritaRunAction* frunAction = nullptr; // non-owning
};

#endif
