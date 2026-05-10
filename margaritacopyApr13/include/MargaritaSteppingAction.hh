#ifndef MARGARITA_STEPPING_ACTION_HH
#define MARGARITA_STEPPING_ACTION_HH

#include "G4UserSteppingAction.hh"
#include "globals.hh"
#include <set>

class G4Step;
class MargaritaRunAction;

class MargaritaSteppingAction : public G4UserSteppingAction {
public:
    MargaritaSteppingAction(MargaritaRunAction* run);
    ~MargaritaSteppingAction();   

    void UserSteppingAction(const G4Step* step) override;
    void ClearStoppedTracks() { fStoppedTrackIDs.clear(); }

private:
    MargaritaRunAction* frunAction = nullptr;
    std::set<G4int> fStoppedTrackIDs;
};

#endif
