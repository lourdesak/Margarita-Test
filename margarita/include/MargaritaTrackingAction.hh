#ifndef MargaritaTrackingAction_h
#define MargaritaTrackingAction_h 1

#include "G4UserTrackingAction.hh"
#include "globals.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class MargaritaTrackingAction : public G4UserTrackingAction
{
  public:
    MargaritaTrackingAction();
    ~MargaritaTrackingAction() {};

    virtual void PreUserTrackingAction(const G4Track*);
    virtual void PostUserTrackingAction(const G4Track*);
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif

