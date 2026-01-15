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

#include "G4Event.hh"
#include "G4LogicalVolume.hh"
#include "G4RunManager.hh"
#include "G4Step.hh"
#include "G4AnalysisManager.hh"

namespace B1
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SteppingAction::SteppingAction(EventAction* eventAction) : fEventAction(eventAction) {}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SteppingAction::UserSteppingAction(const G4Step* step)
{
    auto analysisManager = G4AnalysisManager::Instance();
    const std::vector<const G4Track*>* secondaries = step->GetSecondaryInCurrentStep();
    for( size_t lp = 0; lp < (*secondaries).size(); lp++ )
    {
        analysisManager->FillNtupleSColumn(0, (*secondaries)[lp]->GetDefinition()->GetParticleName());
        analysisManager->FillNtupleSColumn(1, (*secondaries)[lp]->GetCreatorProcess()->GetProcessName());
        analysisManager->FillNtupleDColumn(2, (*secondaries)[lp]->GetKineticEnergy()/CLHEP::GeV);
        analysisManager->FillNtupleDColumn(3, (*secondaries)[lp]->GetTotalEnergy()/CLHEP::GeV);
        analysisManager->FillNtupleDColumn(4, (*secondaries)[lp]->GetMomentum().getX()/CLHEP::GeV);
        analysisManager->FillNtupleDColumn(5, (*secondaries)[lp]->GetMomentum().getY()/CLHEP::GeV);
        analysisManager->FillNtupleDColumn(6, (*secondaries)[lp]->GetMomentum().getZ()/CLHEP::GeV);
        analysisManager->FillNtupleDColumn(7, (*secondaries)[lp]->GetPosition().getX()/CLHEP::cm);
        analysisManager->FillNtupleDColumn(8, (*secondaries)[lp]->GetPosition().getY()/CLHEP::cm);
        analysisManager->FillNtupleDColumn(9, (*secondaries)[lp]->GetPosition().getZ()/CLHEP::cm);
        analysisManager->AddNtupleRow();
    }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}  // namespace B1
