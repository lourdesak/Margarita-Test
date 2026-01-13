MARGARITA PROJECT
=================

Overview of components:
Beam -> Physics List -> Detector Geom -> Histograms

------------------------------------------------------------------
  BEAM (General Particle Source GPS)
  
  Macro specifications (controls all beam parameters)
    =======================
    /gps/particle mu-
    /gps/pos/type Plane
    /gps/pos/shape Circle
    /gps/pos/centre 0 0 1 m
    /gps/pos/radius 3.65 cm
    /gps/direction 0 0 -1


    /gps/ene/type Lin
    /gps/ene/min 1 MeV
    /gps/ene/max 25 MeV
    /gps/ene/gradient 1
    /gps/ene/intercept 1
    ========================
    
  Files involved:
  1. run.mac
  2. MargaritaPrimaryGenerator.cc
  3. MargaritaPrimaryGenerator.hh
  4. MargaritaActionBeam.cc
  5. MargaritaActionBeam.hh
  
------------------------------------------------------------------
  PHYSICS LIST
  
  QGSP_BERT

  Files involved:
  1. marg.cc

------------------------------------------------------------------
  DETECTOR GEOMETRY
  
  Files involved:
  A simple non-sensitive cylinder filled with water placed in a world-volume of air (G4Tubs)
  
  1. MargaritaDetectorConstruct.cc
  2. MargaritaDetectorConstruct.hh
  
  ------------------------------------------------------------------
  HISTOGRAMS (Beam Stuff)
  
  Files involved:
  1. HistoManager.cc
  2. HistoManager.hh
  3. run.mac
  4. TrackingAction.cc
  5. RunAction.cc
  6. TrackingAction.hh
  7. RunAction.hh

  This example implements an histo manager which creates histograms and
  ntuples using Geant4 analysis tools.
     
  The output file contains 6 histograms and one ntuple:

  histo1D 1: energy spectrum.
  histo1D 2: vertex: radial distribution dN/dv.
  histo1D 3: angular distribution: cos(theta).
  histo1D 4: angular distribution: phi.      
  histo2D 1: vertex position in the X-Y plane.
  histo2D 2: vertex position in the X-Z plane.
  histo2D 3: vertex position in the Y-Z plane.
  histo2D 4: angular distribution: phi-cos(theta).
  histo2D 5: angular distribution: of phi-theta.
 
  In the ntuple the following data are recorded for each incident particle:  

  Particle ID
  Incident Position (x,y,z);
  Incident Angle (theta,phi);
  Particle weight;
	
  The histograms are managed by G4AnalysisManager class and its Messenger. 
  The histos can be individually activated with the command :
  /analysis/h1/set id nbBins  valMin valMax unit 
  where unit is the desired unit for the histo (MeV or keV, deg or mrad, etc..)
   
  One can control the name of the histograms file with the command:
  /analysis/setFileName  name  (default exgps)
   
  It is possible to choose the format of the histogram file : root (default),
  xml, csv, by using namespace in HistoManager.hh 
	
