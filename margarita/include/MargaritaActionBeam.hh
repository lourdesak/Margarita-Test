#ifndef ACTION_HH
#define ACTION_HH

#include "G4VUserActionInitialization.hh"
#include "MargaritaPrimaryGenerator.hh"

class ActionInitialization : public G4VUserActionInitialization
{
public:
    ActionInitialization();
    ~ActionInitialization();

    // Geant4 interface
    virtual void BuildForMaster() const;
    virtual void Build() const;
};

#endif

