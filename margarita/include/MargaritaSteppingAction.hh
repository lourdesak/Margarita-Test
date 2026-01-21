#ifndef MARGARITA_STEPPING_ACTION_HH
#define MARGARITA_STEPPING_ACTION_HH

#include "G4UserSteppingAction.hh"  

class G4Step;
class MargaritaRunAction;
class MargaritaActionBeam;

class MargaritaSteppingAction : public G4UserSteppingAction {
public:
    
  MargaritaSteppingAction(MargaritaActionBeam* eventAction, MargaritaRunAction* runAction);

  void UserSteppingAction(const G4Step* step) override;

private:
    MargaritaRunAction* frunAction = nullptr; // non-owning
    MargaritaActionBeam* feventAction = nullptr; // non-owning
};

#endif 
