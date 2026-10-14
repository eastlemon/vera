#pragma once

#include <SDL2/SDL.h>

#include "BaseClasses.h"

class BNavSatWnd {
  bool m_bTracking;
  SDL_Thread *m_hThread;

public:
  BNavSatWnd();
  ~BNavSatWnd();

  bool IsTracking() { return m_bTracking; }
  void StartTracking(BVector vPos);
  BVector Track(BVector vPos);
  BVector GetRelLoc(BVector vPos);
  void EndTracking();
  int ActivateCurrentMapTexture();
  void SetResolution(double dRes);
};
