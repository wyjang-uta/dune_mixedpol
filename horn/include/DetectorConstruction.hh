#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"

class G4VPhysicalVolume;
class G4LogicalVolume;
class G4FieldManager;
class SimpleHornMagneticField;

/**
 * @brief Geant4 지오메트리를 정의하는 메인 클래스
 *
 * 이 클래스는 월드 볼륨과 단순화된 마그네틱 혼 지오메트리를 생성합니다.
 * 또한 SimpleHornMagneticField를 생성하여 적절한 볼륨에 할당합니다.
 */
class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
  DetectorConstruction();
  virtual ~DetectorConstruction();

  // Geant4가 호출하는 지오메트리 생성 함수
  virtual G4VPhysicalVolume* Construct();

  SimpleHornMagneticField* GetHornAMagneticField() { return fMagFieldA; }
  SimpleHornMagneticField* GetHornBMagneticField() { return fMagFieldB; }
  SimpleHornMagneticField* GetHornCMagneticField() { return fMagFieldC; }

private:
  // Helper functions
  void ConstructWorld(G4VPhysicalVolume*& physWorld);
  void ConstructHornA(G4LogicalVolume* logicWorld);
  void ConstructHornB(G4LogicalVolume* logicWorld);
  void ConstructHornC(G4LogicalVolume* logicWorld);

  // 자기장 멤버 변수
  SimpleHornMagneticField* fMagFieldA;
  SimpleHornMagneticField* fMagFieldB;
  SimpleHornMagneticField* fMagFieldC;
  G4FieldManager* fFieldMgrA;
  G4FieldManager* fFieldMgrB;
  G4FieldManager* fFieldMgrC;

  // 볼륨 멤버 변수 (필요시 사용)
  G4LogicalVolume* logicInnerCondA;
  G4LogicalVolume* logicFieldRegionA;
  G4LogicalVolume* logicOuterCondA;

  G4LogicalVolume* logicInnerCondB;;
  G4LogicalVolume* logicFieldRegionB;
  G4LogicalVolume* logicOuterCondB;

  G4LogicalVolume* logicInnerCondC;
  G4LogicalVolume* logicFieldRegionC;
  G4LogicalVolume* logicOuterCondC;
};

#endif
