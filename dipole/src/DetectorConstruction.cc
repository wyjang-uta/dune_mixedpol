#include "DetectorConstruction.hh"

// Geometries
#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Polycone.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

// Magnetic Fields
#include "SimpleHornMagneticField.hh"
#include "G4FieldManager.hh"
#include "G4TransportationManager.hh"
#include "G4ChordFinder.hh"
#include "G4NystromRK4.hh"
#include "G4ClassicalRK4.hh"
#include "G4DormandPrince745.hh"
#include "G4Mag_UsualEqRhs.hh"
#include "G4UniformMagField.hh"
#include "G4UserLimits.hh"

// Visualization
#include "G4VisAttributes.hh"
#include "G4Colour.hh"

DetectorConstruction::DetectorConstruction()
: G4VUserDetectorConstruction(),
  fMagFieldA(nullptr), fMagFieldB(nullptr), fMagFieldC(nullptr),
  fFieldMgrA(nullptr), fFieldMgrB(nullptr), fFieldMgrC(nullptr),
  logicInnerCondA(nullptr), logicFieldRegionA(nullptr), logicOuterCondA(nullptr),
  logicInnerCondB(nullptr), logicFieldRegionB(nullptr), logicOuterCondB(nullptr),
  logicInnerCondC(nullptr), logicFieldRegionC(nullptr), logicOuterCondC(nullptr)
{
}

DetectorConstruction::~DetectorConstruction()
{
  delete fMagFieldA;
  delete fMagFieldB;
  delete fMagFieldC;
  delete fFieldMgrA;
  delete fFieldMgrB;
  delete fFieldMgrC;
}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  G4VPhysicalVolume* physWorld = nullptr;

  // 1. Construct World
  ConstructWorld(physWorld);

  // Get world logical volume
  G4LogicalVolume* logicWorld = physWorld->GetLogicalVolume();

  ConstructTarget(logicWorld);

  // 2. Construct Horn Geometry and Magnetic Fields
  // disabled for simple dipole magnetic field version
  //ConstructHornA(logicWorld);
  //ConstructHornB(logicWorld);
  //ConstructHornC(logicWorld);
  // instead construct dipoles
  ConstructDipoleA(logicWorld);
  ConstructDipoleB(logicWorld);
  ConstructDipoleC(logicWorld);

  // (Optional) Additional geometry components can be constructed here

  return physWorld;
}

void DetectorConstruction::ConstructWorld(G4VPhysicalVolume*& physWorld)
{
  G4NistManager* nist = G4NistManager::Instance();

  G4Material* world_mat = nist->FindOrBuildMaterial("G4_Galactic");

  G4double world_size_xy = 20.0 * m;
  G4double world_size_z = 300.0 * m;
  G4Box* solidWorld = new G4Box("SolidWorld",
                                0.5 * world_size_xy,
                                0.5 * world_size_xy,
                                0.5 * world_size_z);

  G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld,
                                                    world_mat,
                                                    "LogicWorld");

  physWorld = new G4PVPlacement(0, G4ThreeVector(),
                                logicWorld, "PhysWorld",
                                0, false, 0);

  logicWorld->SetVisAttributes(G4VisAttributes::GetInvisible());
}

void DetectorConstruction::ConstructTarget(G4LogicalVolume* logicWorld)
{
  // --- Define the target volume and locate it ---
  G4Tubs* solidTarget = new G4Tubs("SolidTarget",
                                     0.0 * cm,
                                     0.85 * cm,
                                     0.5 * 1.5 * m,
                                     0.0 * deg,
                                     360.0 * deg);

  G4double density = 2.267 * g/cm3;
  G4double a = 12.0107 * g/mole;
  G4double z = 6.0;
  G4Material* target_mat = new G4Material("Graphite", z, a, density);
  G4LogicalVolume* logicTarget = new G4LogicalVolume(solidTarget,
                                                     target_mat,
                                                     "LogicTarget");
  G4double world_z_halflen = dynamic_cast<G4Box*>(logicWorld->GetSolid())->GetZHalfLength();
  G4double zpos = -world_z_halflen + solidTarget->GetZHalfLength();
  new G4PVPlacement(0,
                    G4ThreeVector(0., 0., zpos),
                    logicTarget,
                    "PhysTarget",
                    logicWorld,
                    false,
                    0);
}

void DetectorConstruction::ConstructDipoleA(G4LogicalVolume* logicWorld) {
  G4double sizeXY = 50.0 * cm;
  G4double sizeZ = 50.0 * cm;
  G4NistManager* nist = G4NistManager::Instance();
  G4Material* helium_mat = nist->FindOrBuildMaterial("G4_Galactic");

  // Create dipole volume
  G4Box* solidDipole = new G4Box("DipoleA_SV", sizeXY/2.0, sizeXY/2.0, sizeZ/2.0);
  G4LogicalVolume* logicDipole = new G4LogicalVolume(solidDipole, helium_mat, "DipoleA_LV");
  G4double world_z_halflen = dynamic_cast<G4Box*>(logicWorld->GetSolid())->GetZHalfLength();
  G4double zpos = -world_z_halflen + 1.5 * m + 0.5 * m + solidDipole->GetZHalfLength();
  new G4PVPlacement(0, G4ThreeVector(0,0,zpos), logicDipole, "DipoleA_PV", logicWorld, false, 0);

  // uniform magnetic field
  G4double angleDeg = 0.0;
  G4double angleRad = angleDeg * (CLHEP::pi/180.0);
  G4double Bmag = fBFieldVal;
  G4double Bx = Bmag * std::sin(angleRad);
  G4double By = Bmag * std::cos(angleRad);
  G4double Bz = 0.0;
  G4UniformMagField* magField = new G4UniformMagField(G4ThreeVector(Bx, By, Bz));

  G4FieldManager* fieldMgr = new G4FieldManager();
  fieldMgr->SetDetectorField(magField);
  G4Mag_UsualEqRhs* fEquation = new G4Mag_UsualEqRhs(magField);
  G4MagIntegratorStepper* fStepper = new G4ClassicalRK4(fEquation);
  //G4MagIntegratorStepper* fStepper = new G4DormandPrince745(fEquation);
  G4double minStep = 0.5 * mm;

  G4ChordFinder* fChordFinder = new G4ChordFinder(magField, minStep, fStepper);
  fieldMgr->SetChordFinder(fChordFinder);

  fieldMgr->SetDeltaOneStep(0.5 * mm);
  fieldMgr->SetDeltaIntersection(0.1 * mm);

  logicDipole->SetFieldManager(fieldMgr, true);

  // --- VisAttributes
  G4VisAttributes* visAttrInner = new G4VisAttributes(G4Colour(0.5, 0.5, 0.5)); // Grey
  logicDipole->SetVisAttributes(visAttrInner);

  // --- Step Limiter (optional)
  G4UserLimits* userLimits = new G4UserLimits();
  userLimits->SetMaxAllowedStep(10.0 * mm);
  logicDipole->SetUserLimits(userLimits);
}

void DetectorConstruction::ConstructDipoleB(G4LogicalVolume* logicWorld) {
  G4double sizeXY = 50.0 * cm;
  G4double sizeZ = 50.0 * cm;
  G4NistManager* nist = G4NistManager::Instance();
  G4Material* helium_mat = nist->FindOrBuildMaterial("G4_Galactic");
  G4Box* solidDipole = new G4Box("DipoleB_SV", sizeXY/2.0, sizeXY/2.0, sizeZ/2.0);
  G4LogicalVolume* logicDipole = new G4LogicalVolume(solidDipole, helium_mat, "DipoleB_LV");
  G4double world_z_halflen = dynamic_cast<G4Box*>(logicWorld->GetSolid())->GetZHalfLength();
  G4double zpos = -world_z_halflen + 1.5 * m + 0.5 * m + solidDipole->GetZHalfLength() * 2.0 + 0.5 * m + solidDipole->GetZHalfLength();
  new G4PVPlacement(0, G4ThreeVector(0,0,zpos), logicDipole, "DipoleB_PV", logicWorld, false, 0);

  // uniform magnetic field
  G4double angleDeg = 120.0;
  G4double angleRad = angleDeg * (CLHEP::pi/180.0);
  G4double Bmag = fBFieldVal;
  G4double Bx = Bmag * std::sin(angleRad);
  G4double By = Bmag * std::cos(angleRad);
  G4double Bz = 0.0;
  G4UniformMagField* magField = new G4UniformMagField(G4ThreeVector(Bx, By, Bz));

  G4FieldManager* fieldMgr = new G4FieldManager();
  fieldMgr->SetDetectorField(magField);
  G4Mag_UsualEqRhs* fEquation = new G4Mag_UsualEqRhs(magField);
  G4MagIntegratorStepper* fStepper = new G4ClassicalRK4(fEquation);
  //G4MagIntegratorStepper* fStepper = new G4DormandPrince745(fEquation);
  G4double minStep = 0.5 * mm;

  G4ChordFinder* fChordFinder = new G4ChordFinder(magField, minStep, fStepper);
  fieldMgr->SetChordFinder(fChordFinder);

  fieldMgr->SetDeltaOneStep(0.5 * mm);
  fieldMgr->SetDeltaIntersection(0.1 * mm);

  logicDipole->SetFieldManager(fieldMgr, true);
  // --- VisAttributes
  G4VisAttributes* visAttrInner = new G4VisAttributes(G4Colour(0.5, 0.5, 0.5)); // Grey
  logicDipole->SetVisAttributes(visAttrInner);

  // --- Step Limiter (optional)
  G4UserLimits* userLimits = new G4UserLimits();
  userLimits->SetMaxAllowedStep(10.0 * mm);
  logicDipole->SetUserLimits(userLimits);
}

void DetectorConstruction::ConstructDipoleC(G4LogicalVolume* logicWorld) {
  G4double sizeXY = 50.0 * cm;
  G4double sizeZ = 50.0 * cm;
  G4NistManager* nist = G4NistManager::Instance();
  G4Material* helium_mat = nist->FindOrBuildMaterial("G4_Galactic");
  G4Box* solidDipole = new G4Box("DipoleC_SV", sizeXY/2.0, sizeXY/2.0, sizeZ/2.0);
  G4LogicalVolume* logicDipole = new G4LogicalVolume(solidDipole, helium_mat, "DipoleC_LV");
  G4double world_z_halflen = dynamic_cast<G4Box*>(logicWorld->GetSolid())->GetZHalfLength();
  G4double zpos = -world_z_halflen + 1.5 * m + 0.5 * m + solidDipole->GetZHalfLength() * 2.0 + 0.5 * m + solidDipole->GetZHalfLength() * 2.0 + 0.5 * m + solidDipole->GetZHalfLength();
  new G4PVPlacement(0, G4ThreeVector(0,0,zpos), logicDipole, "DipoleC_PV", logicWorld, false, 0);

  // uniform magnetic field
  G4double angleDeg = 240.0;
  G4double angleRad = angleDeg * (CLHEP::pi/180.0);
  G4double Bmag = fBFieldVal;
  G4double Bx = Bmag * std::sin(angleRad);
  G4double By = Bmag * std::cos(angleRad);
  G4double Bz = 0.0;
  G4UniformMagField* magField = new G4UniformMagField(G4ThreeVector(Bx, By, Bz));

  G4FieldManager* fieldMgr = new G4FieldManager();
  fieldMgr->SetDetectorField(magField);
  G4Mag_UsualEqRhs* fEquation = new G4Mag_UsualEqRhs(magField);
  G4MagIntegratorStepper* fStepper = new G4ClassicalRK4(fEquation); 
  //G4MagIntegratorStepper* fStepper = new G4DormandPrince745(fEquation);
  G4double minStep = 0.5 * mm;

  G4ChordFinder* fChordFinder = new G4ChordFinder(magField, minStep, fStepper);
  fieldMgr->SetChordFinder(fChordFinder);

  fieldMgr->SetDeltaOneStep(0.5 * mm);
  fieldMgr->SetDeltaIntersection(0.1 * mm);

  logicDipole->SetFieldManager(fieldMgr, true);
  // --- VisAttributes
  G4VisAttributes* visAttrInner = new G4VisAttributes(G4Colour(0.5, 0.5, 0.5)); // Grey
  logicDipole->SetVisAttributes(visAttrInner);

  // --- Step Limiter (optional)
  G4UserLimits* userLimits = new G4UserLimits();
  userLimits->SetMaxAllowedStep(10.0 * mm);
  logicDipole->SetUserLimits(userLimits);
}
