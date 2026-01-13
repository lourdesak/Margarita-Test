#ifndef MargaritaTrackingAction_h
#define MargaritaTrackingAction_h

#include "G4UserTrackingAction.hh"

class MargaritaTrackingAction : public G4UserTrackingAction
{
public:
  MargaritaTrackingAction();
  ~MargaritaTrackingAction() override;

  void PreUserTrackingAction(const G4Track*) override;
  void PostUserTrackingAction(const G4Track*) override;
};

#endif