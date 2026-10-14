// Camera
#include "BCamera.h"

BCamera::BCamera() {
  SetLocation(BVector(0, 0, 0));
  m_vPrevLocation = m_vLocation;
  m_locMode = FIXED;
  m_bInitLoc = true;
}

void BCamera::SetLocation(BVector vLoc) {
  m_vFixLocation = vLoc;
  m_vLocation = m_vFixLocation;
  m_orientation.m_vForward = BVector(100, 60, -3.0) - m_vLocation;
  m_orientation.m_vUp = BVector(0, 0, -1);
  m_orientation.m_vRight =
      m_orientation.m_vUp.CrossProduct(m_orientation.m_vForward);
  m_orientation.Normalize();
  m_locMode = FOLLOW;
  m_dFollowHeight = -3.0;
  m_dAngleOfView = 75.0;
}
