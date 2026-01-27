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
/// \file B1/src/SteppingAction.cc
/// \brief Implementation of the B1::SteppingAction class

#include "SteppingAction.hh"

#include "DetectorConstruction.hh"
#include "EventAction.hh"

#include "G4Step.hh"
#include "G4VProcess.hh"
#include "G4Track.hh"

#include "G4PionPlus.hh"
#include "G4PionMinus.hh"
#include "G4KaonPlus.hh"
#include "G4KaonMinus.hh"

#include "G4Version.hh"
#if G4VERSION_NUMBER >= 1100
  #include "G4AnalysisManager.hh"
#else
  #include "g4root.hh"
#endif
namespace B1
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SteppingAction::SteppingAction(EventAction* eventAction) : fEventAction(eventAction) {}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SteppingAction::UserSteppingAction(const G4Step* step)
{
    auto analysisManager = G4AnalysisManager::Instance();
    G4Track* track = step->GetTrack();

    G4StepPoint* postPoint = step->GetPostStepPoint();

    // Find out parent particle that produces neutrinos
    if( postPoint->GetProcessDefinedStep() &&
        postPoint->GetProcessDefinedStep()->GetProcessName() == "Decay" )
    {
      const std::vector<const G4Track*>* secondaries = step->GetSecondaryInCurrentStep();

      // Parent particle info
      G4int parentPDG = track->GetDefinition()->GetPDGEncoding();
      G4ThreeVector parentMom = track->GetMomentum();
      G4double parentE = track->GetTotalEnergy();
      G4ThreeVector decayPos = track->GetPosition();

      for (size_t i = 0; i < secondaries->size(); ++i) {
        const G4Track* secTrack = (*secondaries)[i];
        G4int secPDG = secTrack->GetDefinition()->GetPDGEncoding();
        G4String partName = secTrack->GetDefinition()->GetParticleName();

        //if( G4StrUtil::contains(partName, "nu") || G4StrUtil::contains(partName,"anti_nu") ) {
        if( partName.contains("nu") || partName.contains("anti_nu") ) {
          G4ThreeVector nuMom = secTrack->GetMomentum();
          G4double x_proj = -9999.0 * CLHEP::m;
          G4double y_proj = -9999.0 * CLHEP::m;

          // Calculate projection at 574 m
          if( nuMom.getZ() > 0.0) {
            G4double z_target = (574.0 - 150.0) * CLHEP::m; // WARNING! world center coordinate <-> DUNE MCzero coorinate tranformation is hardcoded here.
            G4double deltaZ = z_target - decayPos.z();
            x_proj = decayPos.x() + nuMom.getX()/nuMom.getZ() * deltaZ;
            y_proj = decayPos.y() + nuMom.getY()/nuMom.getZ() * deltaZ;
          }

          // Fill ntuple
          // parent particle info
          analysisManager->FillNtupleIColumn(0, parentPDG);
          analysisManager->FillNtupleDColumn(1, parentMom.getX()/CLHEP::GeV);
          analysisManager->FillNtupleDColumn(2, parentMom.getY()/CLHEP::GeV);
          analysisManager->FillNtupleDColumn(3, parentMom.getZ()/CLHEP::GeV);
          analysisManager->FillNtupleDColumn(4, parentE/CLHEP::GeV);
          analysisManager->FillNtupleDColumn(5, decayPos.getX()/CLHEP::m);
          analysisManager->FillNtupleDColumn(6, decayPos.getY()/CLHEP::m);
          analysisManager->FillNtupleDColumn(7, decayPos.getZ()/CLHEP::m);
          // neutrino info
          analysisManager->FillNtupleIColumn(8, secPDG);
          analysisManager->FillNtupleDColumn(9, secTrack->GetTotalEnergy()/CLHEP::GeV);
          analysisManager->FillNtupleDColumn(10, nuMom.getX()/CLHEP::GeV);
          analysisManager->FillNtupleDColumn(11, nuMom.getY()/CLHEP::GeV);
          analysisManager->FillNtupleDColumn(12, nuMom.getZ()/CLHEP::GeV);
          analysisManager->FillNtupleDColumn(13, x_proj/CLHEP::m);
          analysisManager->FillNtupleDColumn(14, y_proj/CLHEP::m);
          analysisManager->AddNtupleRow();
        }
      }

    }




}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}  // namespace B1
