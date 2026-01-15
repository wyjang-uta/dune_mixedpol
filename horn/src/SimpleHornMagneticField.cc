#include "SimpleHornMagneticField.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh" // mu0, twopi 상수를 사용하기 위해

SimpleHornMagneticField::SimpleHornMagneticField(G4double peakCurrent)
  : G4MagneticField(), fCurrent(peakCurrent)
{
  // Geant4의 G4PhysicalConstants에서 mu_0 값을 가져옵니다.
  fMu0 = mu0;
}

SimpleHornMagneticField::~SimpleHornMagneticField()
{
}

// 자기장을 계산하는 핵심 메소드
void SimpleHornMagneticField::GetFieldValue(const G4double Point[4],
                                      G4double* Bfield) const
{
  // Point[0] = x, Point[1] = y

  // 1. z축(빔 축)으로부터의 반경 거리 계산
  G4double r = std::sqrt(Point[0] * Point[0] + Point[1] * Point[1]);

  // 2. r=0 (축)에서의 특이점(singularity) 방지
  // (이 필드는 r=0인 G4Tubs에는 적용되지 않겠지만, 안전을 위해 추가)
  if (r < 1.0e-6 * m) {
      Bfield[0] = 0.0;
      Bfield[1] = 0.0;
      Bfield[2] = 0.0;
      return;
  }
  
  // 3. 자기장 크기 계산 (암페어 법칙)
  // B_phi = (mu_0 * I) / (2 * pi * r)
  G4double B_mag = (fMu0 * fCurrent) / (twopi * r);

  // 4. 토로이달 필드 (B_phi)를 카테시안 좌표 (Bx, By)로 변환
  // Bx = -B_mag * sin(phi) = -B_mag * (y / r)
  // By = +B_mag * cos(phi) = +B_mag * (x / r)
  Bfield[0] = -B_mag * (Point[1] / r);
  Bfield[1] =  B_mag * (Point[0] / r);
  Bfield[2] = 0.0;
}
