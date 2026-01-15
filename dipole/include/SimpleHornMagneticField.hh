#ifndef SimpleHornMagneticField_h
#define SimpleHornMagneticField_h 1

#include "G4MagneticField.hh"
#include "globals.hh"

class SimpleHornMagneticField : public G4MagneticField
{
public:
  // 생성자: 피크 전류(Ampere 단위)만 받습니다.
  SimpleHornMagneticField(G4double peakCurrent);
  virtual ~SimpleHornMagneticField();

  // GetFieldValue: Geant4가 호출할 핵심 함수
  virtual void GetFieldValue(const G4double Point[4], // [x,y,z,t]
                             G4double* Bfield) const; // [Bx,By,Bz]

private:
  G4double fCurrent; // 피크 전류 (Ampere)
  G4double fMu0;     // 진공 투자율 (mu_0)
};

#endif
