// Controller module - keyboard input bindings implementation.
//
// Only BKeyMap (the keyboard binding table) is implemented here. All
// DirectInput/joystick code (BControllerState, BControllerMap,
// BControllerMapItem, SwitchToController, GetValueFromRaw) has been
// removed - the game is keyboard-only.

#include "ControllerModule.h"
#include <SDL2/SDL.h>

BKeyMap ControllerModule::m_keymap;

ControllerModule::ControllerModule() {}

// Default keyboard bindings.
// Matches the layout documented in BUILD_MACOS.md / QuickHelp.tga:
//   Arrows       - drive
//   Left Ctrl    - brake
//   Enter        - propeller on
//   Left Shift   - propeller reverse
//   J            - toggle jet
//   H            - toggle heli
//   L            - lift
//   C            - switch camera
BKeyMap::BKeyMap() {
  m_unLeft = SDLK_LEFT;
  m_unRight = SDLK_RIGHT;
  m_unAccelerate = SDLK_UP;
  m_unReverse = SDLK_DOWN;
  m_unBreak = SDLK_LCTRL;
  m_unPropeller = SDLK_RETURN;
  m_unPropellerReverse = SDLK_LSHIFT;
  m_unJet = 'J';
  m_unHeli = 'H';
  m_unLift = 'L';
  m_unCamera = 'C';
}
