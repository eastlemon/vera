// Controller module - keyboard input bindings.
//
// All DirectInput/joystick infrastructure has been removed; the game uses
// keyboard input exclusively. The BKeyMap class stores the SDL keycodes
// bound to each in-game action (left, right, accelerate, brake, jet, etc.)
// and is persisted in settings.dat.

#pragma once

#include "BaseClasses.h"
#include <string>

using namespace std;

// Keyboard binding table. SDL keycodes for each in-game action.
// Persisted in settings.dat as "KEYMAP-<Action> = <code>".
class BKeyMap {
public:
  int m_unPropeller;
  int m_unPropellerReverse;
  int m_unLeft;
  int m_unRight;
  int m_unAccelerate;
  int m_unReverse;
  int m_unBreak;
  int m_unJet;
  int m_unHeli;
  int m_unLift;
  int m_unCamera;

  BKeyMap();
};

// ControllerModule - global accessor for the keyboard map.
// (Historical name kept for compatibility; only m_keymap is live.)
class ControllerModule {
public:
  static BKeyMap m_keymap;

  ControllerModule();
};
