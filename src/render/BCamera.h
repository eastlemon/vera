// Camera

#pragma once

#include "BaseClasses.h"

class BCamera {
public:
  enum TCameraLoc { FIXED, FOLLOW, OVERVIEW, INCAR, ONSIDE, SIDEVIEW };

  BCamera();

  void SetLocation(BVector vLoc);

  BOrientation m_orientation;
  BVector m_vLocation;
  BVector m_vPrevLocation;
  BVector m_vSpeed;
  BVector m_vOverview;
  BVector m_vFixLocation;
  TCameraLoc m_locMode;
  bool m_bInitLoc;
  double m_dAngleOfView;
  bool m_bNeedsToBeInitialized;
  double m_dFollowHeight;
};
