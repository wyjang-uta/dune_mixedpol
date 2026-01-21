//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
/// \file exampleB1.cc
/// \brief Main program of the B1 example

#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"
#include "FTFP_BERT.hh"

#include "G4Version.hh"
#if G4VERSION_NUMBER >= 1100
#include "G4RunManagerFactory.hh"
#else
  #ifdef G4MULTITHREADED
#include "G4MTRunManager.hh"
  #else
#include "G4RunManager.hh"
  #endif
#endif

#include "G4SystemOfUnits.hh"
#include "G4SteppingVerbose.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"

// #include "Randomize.hh"

using namespace B1;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

int main(int argc, char** argv)
{
  // Detect interactive mode (if no arguments) and define UI session
  //
  G4UIExecutive* ui = nullptr;
  if (argc == 1) {
    ui = new G4UIExecutive(argc, argv);
  }

  // Default arguments
  G4double Bmag = 3.0 * tesla;
  G4long mySeed = 1234;
  G4String fileName = "output.root";
  // Parse arguments (ex: ./exampleB1 run1.mac 1.5 1234 /exp/dune/data/users/wyjang/dune_mixedpol/dipole/result_1.5.root)
  if( argc >= 5 ) {
    Bmag = std::stod(argv[2]) * tesla;
    mySeed = std::stol(argv[3]);
    fileName = argv[4];
  }

  // Optionally: choose a different Random engine...
  G4Random::setTheEngine(new CLHEP::MTwistEngine);
  G4Random::setTheSeed(mySeed);

  // use G4SteppingVerboseWithUnits
  //G4int precision = 4;
  //G4SteppingVerbose::UseBestUnit(precision);

  // Construct the default run manager
  //
  #if G4VERSION_NUMBER >= 1100
  auto runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);
  #else
    #ifdef G4MULTITHREADED
  auto* runManager = new G4MTRunManager;
    #else
  auto* runManager = new G4RunManager;
    #endif
  #endif

  // Set mandatory initialization classes
  //
  // Detector construction
  auto* detector = new DetectorConstruction();
  detector->SetDipoleBField(Bmag);
  runManager->SetUserInitialization(detector);

  // Physics list
  auto physicsList = new FTFP_BERT;
  physicsList->SetVerboseLevel(1);
  runManager->SetUserInitialization(physicsList);

  // User action initialization
  runManager->SetUserInitialization(new ActionInitialization(fileName));

  // Initialize visualization with the default graphics system
  G4VisManager* visManager = new G4VisExecutive;
  // Constructors can also take optional arguments:
  // - a graphics system of choice, eg. "OGL"
  // - and a verbosity argument - see /vis/verbose guidance.
  // auto visManager = new G4VisExecutive(argc, argv, "OGL", "Quiet");
  // auto visManager = new G4VisExecutive("Quiet");
  visManager->Initialize();

  // Get the pointer to the User Interface manager
  auto UImanager = G4UImanager::GetUIpointer();

  // Process macro or start UI session
  //
  if (!ui) {
    // batch mode
    G4String command = "/control/execute ";
    G4String fileName = argv[1];
    UImanager->ApplyCommand(command + fileName);
  }
  else {
    // interactive mode
    UImanager->ApplyCommand("/control/execute init_vis.mac");
    ui->SessionStart();
    delete ui;
  }

  // Job termination
  // Free the store: user actions, physics_list and detector_description are
  // owned and deleted by the run manager, so they should not be deleted
  // in the main() program !

  delete visManager;
  delete runManager;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo.....
