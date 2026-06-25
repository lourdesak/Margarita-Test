#ifndef MARGARITA_STEPPING_ACTION_HH
#define MARGARITA_STEPPING_ACTION_HH

#include "G4UserSteppingAction.hh"
#include "globals.hh"
#include "G4ThreeVector.hh"
#include <vector>

class G4Step;
class MargaritaRunAction;

class MargaritaSteppingAction : public G4UserSteppingAction {
public:
    MargaritaSteppingAction(MargaritaRunAction* run);
    ~MargaritaSteppingAction();

    void UserSteppingAction(const G4Step* step) override;

private:
    MargaritaRunAction* frunAction = nullptr;

    // ---- Per-track step buffer for single-track diagnostic ----
    // We accumulate every step of a primary mu- inside the active volume.
    // When the track stops, we flush all rows to the stoppingSteps ntuple.
    // When a new track enters or the buffered track exits without stopping,
    // the buffer is discarded.
    struct StepRec {
        G4int    stepNum;
        G4double kePre;     // MeV
        G4double dEdx;      // MeV/cm
        G4double x, y, z;   // cm  (post-step position)
        G4double stepLen;   // cm
        G4double eDep;      // MeV
        G4double cumLen;    // cm  (cumulative path length up to this step)
    };
    std::vector<StepRec> fCurSteps;
    G4int    fCurEventID  = -1;     // event-scoped identity for the buffer
    G4int    fCurTrackID  = -1;
    G4double fCurEKinInit = 0.;     // MeV
    G4double fCurCumLen   = 0.;     // cm
    G4ThreeVector fCurVertex;       // cm
    G4ThreeVector fCurDir;          // unit
};

#endif
