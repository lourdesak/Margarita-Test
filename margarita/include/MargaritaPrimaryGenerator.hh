#ifndef MARGARITA_PRIMARY_GENERATOR_HH
#define MARGARITA_PRIMARY_GENERATOR_HH

#include "G4VUserPrimaryGeneratorAction.hh"

class G4GeneralParticleSource;
class G4Event;

class MargaritaPrimaryGenerator : public G4VUserPrimaryGeneratorAction {
public:
  MargaritaPrimaryGenerator();
  ~MargaritaPrimaryGenerator() override;
  void GeneratePrimaries(G4Event* event) override;

private:
  G4GeneralParticleSource* fGPS;  // owned
};

#endif
