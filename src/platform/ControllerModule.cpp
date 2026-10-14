// Full-access controller module for all users of the game

#include "ControllerModule.h"
#include <SDL2/SDL.h>

bool ControllerModule::m_bInitialized = false;
int ControllerModule::m_nControllers = 0;
int ControllerModule::m_nCurrent = -1;
bool ControllerModule::m_bCurrentInitialized = false;
string ControllerModule::m_sControllers[];
BKeyMap ControllerModule::m_keymap;
BControllerMap ControllerModule::m_controllermap;

ControllerModule::ControllerModule() {}

bool ControllerModule::SwitchToController(int nIndex) { return false; }

double ControllerModule::GetValueFromRaw(BControllerMapItem *pcmi) {}

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

BControllerMap::BControllerMap() {
  m_cmiLeft.m_bButton = false;
  m_cmiLeft.m_bFlipped = false;
  m_cmiLeft.m_bPlusSide = false;
  m_cmiLeft.m_bWhole = false;
  m_cmiLeft.m_jiPart = BControllerMapItem::X;

  m_cmiRight.m_bButton = false;
  m_cmiRight.m_bFlipped = false;
  m_cmiRight.m_bPlusSide = true;
  m_cmiRight.m_bWhole = false;
  m_cmiRight.m_jiPart = BControllerMapItem::X;

  m_cmiAccelerate.m_bButton = false;
  m_cmiAccelerate.m_bFlipped = false;
  m_cmiAccelerate.m_bPlusSide = false;
  m_cmiAccelerate.m_bWhole = false;
  m_cmiAccelerate.m_jiPart = BControllerMapItem::Y;

  m_cmiReverse.m_bButton = false;
  m_cmiReverse.m_bFlipped = false;
  m_cmiReverse.m_bPlusSide = true;
  m_cmiReverse.m_bWhole = false;
  m_cmiReverse.m_jiPart = BControllerMapItem::Y;

  m_cmiBreak.m_bButton = true;
  m_cmiBreak.m_nButton = 0;

  m_cmiJet.m_bButton = true;
  m_cmiJet.m_nButton = 2;

  m_cmiHeli.m_bButton = true;
  m_cmiHeli.m_nButton = 3;

  m_cmiLift.m_bButton = true;
  m_cmiLift.m_nButton = 9;

  m_cmiCamera.m_bButton = true;
  m_cmiCamera.m_nButton = 1;
}

BControllerMapItem::BControllerMapItem() {
  m_bButton = false;
  m_nButton = 0;
  m_bWhole = true;
  m_bPlusSide = false;
  m_bFlipped = false;
  m_jiPart = X;
}

string BControllerMapItem::ToString(bool &rbFlipped) {
  string sRet;
  if (m_bButton) {
    // sRet.Format("%d (button)", m_nButton + 1); //FIXME
  } else {
    switch (m_jiPart) {
    case X:
      sRet = "X";
      break;
    case Y:
      sRet = "Y";
      break;
    case Z:
      sRet = "Z";
      break;
    case XROT:
      sRet = "X Rot";
      break;
    case YROT:
      sRet = "Y Rot";
      break;
    case ZROT:
      sRet = "Z Rot";
      break;
    case SLIDER1:
      sRet = "Slider1";
      break;
    case SLIDER2:
      sRet = "Slider2";
      break;
    }

    if (!m_bWhole) {
      if (m_bPlusSide) {
        sRet += "+";
      } else {
        sRet += "-";
      }
    }
  }

  rbFlipped = m_bFlipped;

  return sRet;
}

bool BControllerMapItem::FromString(string sItem) {
  if (sItem.empty()) {
    return false;
  }

  return true;
}
