//
// Bare core classes for the Vera

#pragma once

#include <math.h>

class BIndexGuard {
  int m_nMax;
  int m_nMapTo;

public:
  BIndexGuard(int nMax, int nMapTo) {
    m_nMax = nMax;
    m_nMapTo = nMapTo;
  };
  inline int operator()(int nCandidate) {
    if ((nCandidate < 0) || (nCandidate > m_nMax)) {
      return m_nMapTo;
    } else {
      return nCandidate;
    }
  };
};

class BVector {
public:
  double m_dX;
  double m_dY;
  double m_dZ;

  inline BVector() {
    m_dX = 0.0;
    m_dY = 0.0;
    m_dZ = 0.0;
  };
  inline BVector(double a, double b, double c) {
    m_dX = a;
    m_dY = b;
    m_dZ = c;
  };

  inline void Set(double a, double b, double c) {
    m_dX = a;
    m_dY = b;
    m_dZ = c;
  };
  inline BVector operator+(BVector p) {
    BVector vRet;
    vRet.m_dX = m_dX + p.m_dX;
    vRet.m_dY = m_dY + p.m_dY;
    vRet.m_dZ = m_dZ + p.m_dZ;
    return vRet;
  };
  inline BVector &operator+=(BVector p) {
    m_dX += p.m_dX;
    m_dY += p.m_dY;
    m_dZ += p.m_dZ;
    return *this;
  };
  inline BVector operator-(BVector p) {
    BVector vRet;
    vRet.m_dX = m_dX - p.m_dX;
    vRet.m_dY = m_dY - p.m_dY;
    vRet.m_dZ = m_dZ - p.m_dZ;
    return vRet;
  };
  inline BVector &operator-=(BVector p) {
    m_dX -= p.m_dX;
    m_dY -= p.m_dY;
    m_dZ -= p.m_dZ;
    return *this;
  };
  inline BVector operator*(double d) {
    BVector vRet;
    vRet.m_dX = m_dX * d;
    vRet.m_dY = m_dY * d;
    vRet.m_dZ = m_dZ * d;
    return vRet;
  };
  inline double Length() {
    return sqrt((m_dX * m_dX) + (m_dY * m_dY) + (m_dZ * m_dZ));
  };
  inline BVector CrossProduct(BVector p) {
    BVector vRet;
    vRet.Set(m_dY * p.m_dZ - m_dZ * p.m_dY, m_dZ * p.m_dX - m_dX * p.m_dZ,
             m_dX * p.m_dY - m_dY * p.m_dX);
    return vRet;
  };
  inline double ScalarProduct(BVector p) {
    return m_dX * p.m_dX + m_dY * p.m_dY + m_dZ * p.m_dZ;
  };
  inline void ToUnitLength() {
    double dLen = Length();
    if (dLen) {
      m_dX *= 1.0 / dLen;
      m_dY *= 1.0 / dLen;
      m_dZ *= 1.0 / dLen;
    }
  };
};

class BOrientation {
public:
  BVector m_vForward;
  BVector m_vRight;
  BVector m_vUp;

  BOrientation() {
    m_vForward.Set(0, 1, 0);
    m_vRight.Set(1, 0, 0);
    m_vUp.Set(0, 0, -1);
  };

  void Normalize() {
    m_vForward.ToUnitLength();
    m_vRight.ToUnitLength();
    m_vUp.ToUnitLength();
  };
};