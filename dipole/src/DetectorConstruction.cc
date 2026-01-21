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

void DetectorConstruction::ConstructHornA(G4LogicalVolume* logicWorld)
{
  G4NistManager* nist = G4NistManager::Instance();
  G4Material* aluminum_mat = nist->FindOrBuildMaterial("G4_Al");
  G4Material* helium_mat = nist->FindOrBuildMaterial("G4_He");

  // Define Horn parameters
  G4double current = 300.0 * 1000.0 * ampere; // 300 * kA not working?? why?
  //G4double current = 0; // test for no magnetic field

  // G4Polycone definition
  const G4int numZPlanes = 3;

  // Z positions of horn start
                               //  Start       Neck        End
  G4double zPlane[numZPlanes] = { 000.0 * cm, 140.0 * cm, 280.0 * cm };

  // define radii of horns at each Z positions
  // r0: radii of inner conductors
  G4double rInner_Inner[numZPlanes] = {
    0.35 * cm, 
    0.35 * cm, 
    13.0 * cm
  };

  // r1: radii of outer conductors
  // r1: 내부 도체의 외부 반경 (자기장 영역의 내부 반경)
  G4double rOuter_Inner[numZPlanes] = {
    2.35 * cm, // beginning of the neck
    2.35 * cm, // end of the neck
    15.0 * cm  // 
  };

  // r2: 자기장 영역의 외부 반경 (외부 도체의 내부 반경)
  G4double rInner_Outer[numZPlanes] = {
    40.0 * cm, // 좁은 넥
    40.0 * cm, // 넥 끝
    40.0 * cm // 넓게 퍼짐
  };

  // r3: 외부 도체의 외부 반경
  G4double rOuter_Outer[numZPlanes] = {
    42.0 * cm, // 좁은 넥 (두께 2cm)
    42.0 * cm, // 넥 끝 (두께 2cm)
    42.0 * cm // 넓게 퍼짐 (두께 2cm)
  };


  // --- 4. 혼 지오메트리 생성 (3개의 G4Polycone) ---

  // (A) 내부 도체 (자기장 없음)
  G4Polycone* solidInnerCondA = new G4Polycone("SolidInnerCondA",
                                              0., 2. * M_PI, // 0 ~ 360도
                                              numZPlanes,
                                              zPlane,
                                              rInner_Inner, // rInner 배열
                                              rOuter_Inner  // rOuter 배열
                                              );

  logicInnerCondA = new G4LogicalVolume(solidInnerCondA,
                                       aluminum_mat,
                                       "LogicInnerCondA");

  // (B) 필드 영역 (자기장 있음!)
  G4Polycone* solidFieldRegionA = new G4Polycone("SolidFieldRegionA",
                                                0., 2. * M_PI,
                                                numZPlanes,
                                                zPlane,
                                                rOuter_Inner, // rInner = 내부 도체의 rOuter
                                                rInner_Outer  // rOuter = 외부 도체의 rInner
                                                );

  logicFieldRegionA = new G4LogicalVolume(solidFieldRegionA,
                                          helium_mat,
                                          "LogicFieldRegionA");

  // (C) 외부 도체 (자기장 없음)
  G4Polycone* solidOuterCondA = new G4Polycone("SolidOuterCondA",
                                              0., 2. * M_PI,
                                              numZPlanes,
                                              zPlane,
                                              rInner_Outer, // rInner = 자기장 영역의 rOuter
                                              rOuter_Outer  // rOuter = 최종 바깥 반경
                                              );

  logicOuterCondA = new G4LogicalVolume(solidOuterCondA,
                                        aluminum_mat,
                                        "LogicOuterCondA");
  // --- 5. 자기장 생성 및 할당 ---
  fMagFieldA = new SimpleHornMagneticField(current);

  fFieldMgrA = new G4FieldManager();
  fFieldMgrA->SetDetectorField(fMagFieldA);

  G4Mag_UsualEqRhs* equationOfMotion = new G4Mag_UsualEqRhs(fMagFieldA);
  G4MagIntegratorStepper* stepper = new G4NystromRK4(equationOfMotion);
  G4double minStep = 0.5 * mm; // 최소 스텝
  G4ChordFinder* chordFinder = new G4ChordFinder(fMagFieldA, minStep, stepper);
  chordFinder->SetDeltaChord(0.1 * mm);
  fFieldMgrA->SetChordFinder(chordFinder);
  fFieldMgrA->SetDeltaIntersection(1e-4 * mm); // 경계 교차 정확도 설정
  fFieldMgrA->SetDeltaOneStep(1e-4 * mm);      // 한 스텝의 정확도 설정

  logicFieldRegionA->SetFieldManager(fFieldMgrA, true); // Assign magnetic field only to this volume.


  // --- 6. 볼륨 배치 (logicWorld에 배치) ---
  // G4Polycone은 zPlane에 정의된 절대 Z 좌표에 이미 위치하므로,
  // G4PVPlacement의 위치 벡터는 (0,0,0)으로 둡니다.
  G4ThreeVector origin(0, 0, 0);
  new G4PVPlacement(0, origin, logicInnerCondA, "InnerCond_PV", logicWorld, false, 0);
  new G4PVPlacement(0, origin, logicFieldRegionA, "FieldRegion_PV", logicWorld, false, 0);
  new G4PVPlacement(0, origin, logicOuterCondA, "OuterCond_PV", logicWorld, false, 0);

  // --- 7. 시각화 속성 (선택 사항) ---
  G4VisAttributes* visAttrInner = new G4VisAttributes(G4Colour(0.5, 0.5, 0.5)); // Grey
  logicInnerCondA->SetVisAttributes(visAttrInner);

  G4VisAttributes* visAttrField = new G4VisAttributes(G4Colour(0.0, 0.5, 1.0, 0.1)); // Transparent Blue
  visAttrField->SetForceWireframe(true);
  logicFieldRegionA->SetVisAttributes(visAttrField);

  G4VisAttributes* visAttrOuter = new G4VisAttributes(G4Colour(0.5, 0.5, 0.5, 0.2)); // Grey
  logicOuterCondA->SetVisAttributes(visAttrOuter);
}

void DetectorConstruction::ConstructHornB(G4LogicalVolume* logicWorld)
{
  G4NistManager* nist = G4NistManager::Instance();
  G4Material* aluminum_mat = nist->FindOrBuildMaterial("G4_Al");
  G4Material* helium_mat = nist->FindOrBuildMaterial("G4_He");

  // --- 3. 혼 파라미터 정의 ---
  G4double current = -300.0 * 1000.0 * ampere; // 300 kA (kiloampere -> 1000*ampere)
  //G4double current = 0; // 자기장 없는 상태에서 테스트

  // G4Polycone을 위한 Z 평면 및 반경 정의
  // 여기서는 3개의 Z 평면으로 간단한 깔때기 모양을 만듭니다.
  const G4int numZPlanes = 5;

  // Z 위치
  //                          //  Start       Neck        End
  G4double zPlane[numZPlanes] = { 000.0 * cm, 100.0 * cm, 200.0 * cm, 270.0 * cm, 370.0 * cm};

  // 각 Z 평면에서의 반경 정의

  G4double aluminum_thickness = 2.0 * cm;
  // r0: 내부 도체의 내부 반경
  G4double rInner_Inner[numZPlanes] = {
    20 * cm, 
    20 * cm, 
    10 * cm,
    20 * cm,
    20 * cm
  };

  // r1: 내부 도체의 외부 반경 (자기장 영역의 내부 반경)
  G4double rOuter_Inner[numZPlanes] = {
    rInner_Inner[0] + aluminum_thickness,
    rInner_Inner[1] + aluminum_thickness,
    rInner_Inner[2] + aluminum_thickness,
    rInner_Inner[3] + aluminum_thickness,
    rInner_Inner[4] + aluminum_thickness
  };

  // r2: 자기장 영역의 외부 반경 (외부 도체의 내부 반경)
  G4double rInner_Outer[numZPlanes] = {
    60.0 * cm,
    60.0 * cm,
    60.0 * cm,
    60.0 * cm,
    60.0 * cm
  };

  // r3: 외부 도체의 외부 반경
  G4double rOuter_Outer[numZPlanes] = {
    rInner_Outer[0] + aluminum_thickness,
    rInner_Outer[1] + aluminum_thickness,
    rInner_Outer[2] + aluminum_thickness,
    rInner_Outer[3] + aluminum_thickness,
    rInner_Outer[4] + aluminum_thickness
  };

  // --- 4. 혼 지오메트리 생성 (3개의 G4Polycone) ---

  // (A) 내부 도체 (자기장 없음)
  G4Polycone* solidInnerCondB = new G4Polycone("SolidInnerCondB",
                                              0., 2. * M_PI, // 0 ~ 360도
                                              numZPlanes,
                                              zPlane,
                                              rInner_Inner, // rInner 배열
                                              rOuter_Inner  // rOuter 배열
                                              );

  logicInnerCondB = new G4LogicalVolume(solidInnerCondB,
                                        aluminum_mat,
                                        "LogicInnerCondB");

  // (B) 필드 영역 (자기장 있음!)
  G4Polycone* solidFieldRegionB = new G4Polycone("SolidFieldRegionB",
                                                  0., 2. * M_PI,
                                                  numZPlanes,
                                                  zPlane,
                                                  rOuter_Inner, // rInner = 내부 도체의 rOuter
                                                  rInner_Outer  // rOuter = 외부 도체의 rInner
                                                );

  logicFieldRegionB = new G4LogicalVolume(solidFieldRegionB,
                                          helium_mat,
                                          "LogicFieldRegionB");

  // (C) 외부 도체 (자기장 없음)
  G4Polycone* solidOuterCondB = new G4Polycone("SolidOuterCondB",
                                              0., 2. * M_PI,
                                              numZPlanes,
                                              zPlane,
                                              rInner_Outer, // rInner = 자기장 영역의 rOuter
                                              rOuter_Outer  // rOuter = 최종 바깥 반경
                                              );

  logicOuterCondB = new G4LogicalVolume(solidOuterCondB,
                                        aluminum_mat,
                                        "LogicOuterCondB");

  // --- 5. 자기장 생성 및 할당 ---
  fMagFieldB = new SimpleHornMagneticField(current);

  fFieldMgrB = new G4FieldManager();
  fFieldMgrB->SetDetectorField(fMagFieldB);
  G4Mag_UsualEqRhs* equationOfMotion = new G4Mag_UsualEqRhs(fMagFieldB);
  G4MagIntegratorStepper* stepper = new G4NystromRK4(equationOfMotion);
  G4double minStep = 0.5 * mm; // 최소 스텝
  G4ChordFinder* chordFinder = new G4ChordFinder(fMagFieldB, minStep, stepper);
  chordFinder->SetDeltaChord(0.1 * mm);
  fFieldMgrB->SetChordFinder(chordFinder);
  fFieldMgrB->SetDeltaIntersection(1e-4 * mm); // 경계 교차 정확도 설정
  fFieldMgrB->SetDeltaOneStep(1e-4 * mm);      // 한 스텝의 정확도 설정

  logicFieldRegionB->SetFieldManager(fFieldMgrB, true); // Assign magnetic field only to this volume.

  // --- 6. 볼륨 배치 (logicWorld에 배치) ---
  // G4Polycone은 zPlane에 정의된 절대 Z 좌표에 이미 위치하므로,
  // G4PVPlacement의 위치 벡터는 (0,0,0)으로 둡니다.
  G4ThreeVector origin(0, 0, 363.7 * cm);
  new G4PVPlacement(0, origin, logicInnerCondB, "InnerCondB_PV", logicWorld, false, 0);
  new G4PVPlacement(0, origin, logicFieldRegionB, "FieldRegionB_PV", logicWorld, false, 0);
  new G4PVPlacement(0, origin, logicOuterCondB, "OuterCondB_PV", logicWorld, false, 0);

  // --- 7. 시각화 속성 (선택 사항) ---
  G4VisAttributes* visAttrInner = new G4VisAttributes(G4Colour(0.5, 0.5, 0.5)); // Grey
  logicInnerCondB->SetVisAttributes(visAttrInner);
  G4VisAttributes* visAttrField = new G4VisAttributes(G4Colour(0.0, 0.5, 1.0, 0.1)); // Transparent Blue
  visAttrField->SetForceWireframe(true);
  logicFieldRegionB->SetVisAttributes(visAttrField);

  G4VisAttributes* visAttrOuter = new G4VisAttributes(G4Colour(0.5, 0.5, 0.5, 0.2)); // Grey
  logicOuterCondB->SetVisAttributes(visAttrOuter);

}

void DetectorConstruction::ConstructHornC(G4LogicalVolume* logicWorld)
{
  // 혼 C 지오메트리 및 자기장 생성 코드 (유사하게 작성)
  G4NistManager* nist = G4NistManager::Instance();
  G4Material* aluminum_mat = nist->FindOrBuildMaterial("G4_Al");
  G4Material* helium_mat = nist->FindOrBuildMaterial("G4_He");

  // --- 3. 혼 파라미터 정의 ---
  G4double current = 300.0 * 1000.0 * ampere; // 300 kA (kiloampere -> 1000*ampere)
  //G4double current = 0; // 자기장 없는 상태에서 테스트

  // G4Polycone을 위한 Z 평면 및 반경 정의
  // 여기서는 3개의 Z 평면으로 간단한 깔때기 모양을 만듭니다.
  const G4int numZPlanes = 6;

  // Z 위치
  //                          //  Start       Neck        End
  G4double zPlane[numZPlanes] = { 000.0 * cm, 20.0 * cm, 50.0 * cm, 60.0 * cm, 140.0 * cm, 190.0 * cm};

  // 각 Z 평면에서의 반경 정의

  G4double aluminum_thickness = 2.0 * cm;
  // r0: 내부 도체의 내부 반경
  G4double rInner_Inner[numZPlanes] = {
    30.0 * cm, 
    30.0 * cm, 
    10.0 * cm,
    10.0 * cm,
    40.0 * cm,
    40.0 * cm
  };

  // r1: 내부 도체의 외부 반경 (자기장 영역의 내부 반경)
  G4double rOuter_Inner[numZPlanes] = {
    rInner_Inner[0] + aluminum_thickness,
    rInner_Inner[1] + aluminum_thickness,
    rInner_Inner[2] + aluminum_thickness,
    rInner_Inner[3] + aluminum_thickness,
    rInner_Inner[4] + aluminum_thickness,
    rInner_Inner[5] + aluminum_thickness
  };

  // r2: 자기장 영역의 외부 반경 (외부 도체의 내부 반경)
  G4double rInner_Outer[numZPlanes] = {
    60.0 * cm,
    60.0 * cm,
    60.0 * cm,
    60.0 * cm,
    60.0 * cm,
    60.0 * cm
  };

  // r3: 외부 도체의 외부 반경
  G4double rOuter_Outer[numZPlanes] = {
    rInner_Outer[0] + aluminum_thickness,
    rInner_Outer[1] + aluminum_thickness,
    rInner_Outer[2] + aluminum_thickness,
    rInner_Outer[3] + aluminum_thickness,
    rInner_Outer[4] + aluminum_thickness,
    rInner_Outer[5] + aluminum_thickness
  };

  // --- 4. 혼 지오메트리 생성 (3개의 G4Polycone) ---

  // (A) 내부 도체 (자기장 없음)
  G4Polycone* solidInnerCondC = new G4Polycone("SolidInnerCondC",
                                              0., 2. * M_PI, // 0 ~ 360도
                                              numZPlanes,
                                              zPlane,
                                              rInner_Inner, // rInner 배열
                                              rOuter_Inner  // rOuter 배열
                                              );

  logicInnerCondC = new G4LogicalVolume(solidInnerCondC,
                                        aluminum_mat,
                                        "LogicInnerCondC");

  // (B) 필드 영역 (자기장 있음!)
  G4Polycone* solidFieldRegionC = new G4Polycone("SolidFieldRegionC",
                                                  0., 2. * M_PI,
                                                  numZPlanes,
                                                  zPlane,
                                                  rOuter_Inner, // rInner = 내부 도체의 rOuter
                                                  rInner_Outer  // rOuter = 외부 도체의 rInner
                                                );

  logicFieldRegionC = new G4LogicalVolume(solidFieldRegionC,
                                          helium_mat,
                                          "LogicFieldRegionC");

  // (C) 외부 도체 (자기장 없음)
  G4Polycone* solidOuterCondC = new G4Polycone("SolidOuterCondC",
                                              0., 2. * M_PI,
                                              numZPlanes,
                                              zPlane,
                                              rInner_Outer, // rInner = 자기장 영역의 rOuter
                                              rOuter_Outer  // rOuter = 최종 바깥 반경
                                              );

  logicOuterCondC = new G4LogicalVolume(solidOuterCondC,
                                        aluminum_mat,
                                        "LogicOuterCondC");

  // --- 5. 자기장 생성 및 할당 ---
  fMagFieldC = new SimpleHornMagneticField(current);

  fFieldMgrC = new G4FieldManager();
  fFieldMgrC->SetDetectorField(fMagFieldC);
  G4Mag_UsualEqRhs* equationOfMotion = new G4Mag_UsualEqRhs(fMagFieldC);
  G4MagIntegratorStepper* stepper = new G4NystromRK4(equationOfMotion);
  G4double minStep = 0.5 * mm; // 최소 스텝
  G4ChordFinder* chordFinder = new G4ChordFinder(fMagFieldC, minStep, stepper);
  chordFinder->SetDeltaChord(0.1 * mm);
  fFieldMgrC->SetChordFinder(chordFinder);
  fFieldMgrC->SetDeltaIntersection(1e-4 * mm); // 경계 교차 정확도 설정
  fFieldMgrC->SetDeltaOneStep(1e-4 * mm);      // 한 스텝의 정확도 설정

  logicFieldRegionC->SetFieldManager(fFieldMgrC, true); // Assign magnetic field only to this volume.

  // --- 6. 볼륨 배치 (logicWorld에 배치) ---
  // G4Polycone은 zPlane에 정의된 절대 Z 좌표에 이미 위치하므로,
  // G4PVPlacement의 위치 벡터는 (0,0,0)으로 둡니다.
  G4ThreeVector origin(0, 0, 1747.8 * cm);
  new G4PVPlacement(0, origin, logicInnerCondC, "InnerCondC_PV", logicWorld, false, 0);
  new G4PVPlacement(0, origin, logicFieldRegionC, "FieldRegionC_PV", logicWorld, false, 0);
  new G4PVPlacement(0, origin, logicOuterCondC, "OuterCondC_PV", logicWorld, false, 0);
  // --- 7. 시각화 속성 (선택 사항) ---
  G4VisAttributes* visAttrInner = new G4VisAttributes(G4Colour(0.5, 0.5, 0.5)); // Grey
  logicInnerCondC->SetVisAttributes(visAttrInner);
  G4VisAttributes* visAttrField = new G4VisAttributes(G4Colour(0.0, 0.5, 1.0, 0.1)); // Transparent Blue
  visAttrField->SetForceWireframe(true);
  logicFieldRegionC->SetVisAttributes(visAttrField);

  G4VisAttributes* visAttrOuter = new G4VisAttributes(G4Colour(0.5, 0.5, 0.5, 0.2)); // Grey
  logicOuterCondC->SetVisAttributes(visAttrOuter);

  // --- 8. 월드 반환 ---
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
