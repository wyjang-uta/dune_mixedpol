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
/// \file B1/src/RunAction.cc
/// \brief Implementation of the B1::RunAction class

#include "RunAction.hh"

#include "DetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"

#include "G4AccumulableManager.hh"
#include "G4LogicalVolume.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleGun.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4Version.hh"
#if G4VERSION_NUMBER >= 1100
  #include "G4AnalysisManager.hh"
#else
  #include "g4root.hh"
#endif

namespace B1
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

RunAction::RunAction(G4String fileName)
  : G4UserRunAction(),
    fOutputName(fileName)
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::BeginOfRunAction(const G4Run*)
{
  // inform the runManager to save random number seed
  G4RunManager::GetRunManager()->SetRandomNumberStore(false);

  // analysis manager
  auto analysisManager = G4AnalysisManager::Instance();
  G4cout << "Using " << analysisManager->GetType() << G4endl;

  analysisManager->SetNtupleMerging(true);
  analysisManager->SetVerboseLevel(1);
  analysisManager->SetFileName(fOutputName);
  analysisManager->OpenFile();
  analysisManager->CreateNtuple("mirage", "MIRAGE simulation TTree");
  analysisManager->CreateNtupleIColumn("parentPDG");
  analysisManager->CreateNtupleDColumn("parentPx");
  analysisManager->CreateNtupleDColumn("parentPy");
  analysisManager->CreateNtupleDColumn("parentPz");
  analysisManager->CreateNtupleDColumn("parentE");
  analysisManager->CreateNtupleDColumn("vertexX");
  analysisManager->CreateNtupleDColumn("vertexY");
  analysisManager->CreateNtupleDColumn("vertexZ");
  analysisManager->CreateNtupleIColumn("daughterPDG");
  analysisManager->CreateNtupleDColumn("daughterE");
  analysisManager->CreateNtupleDColumn("daughterPx");
  analysisManager->CreateNtupleDColumn("daughterPy");
  analysisManager->CreateNtupleDColumn("daughterPz");
  analysisManager->CreateNtupleDColumn("projXat574m");
  analysisManager->CreateNtupleDColumn("projYat574m");
  analysisManager->FinishNtuple();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::EndOfRunAction(const G4Run* run)
{
  // print run summary
  G4int nofEvents = run->GetNumberOfEvent();
  if (nofEvents == 0) return;
  if( IsMaster() ) {
    G4cout
      << G4endl
      << "--------------------End of Global Run-----------------------"
      << G4endl;
  }
  else {
    G4cout
      << G4endl
      << "--------------------End of Local Run------------------------"
      << G4endl;
  }

  // save histograms & ntuple
  auto analysisManager = G4AnalysisManager::Instance();
  analysisManager->Write();
  analysisManager->CloseFile();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}  // namespace B1
