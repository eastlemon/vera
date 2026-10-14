//
// BMenu: start menu components

#pragma once

#include "BTextRenderer.h"
#include "BUI.h"
#include "BaseClasses.h"
#include <ctime>
#include <string>

using namespace std;

class BMenuItem {
public:
  enum TType { BASIC, STRING_FROM_LIST, SLIDER };

  TType m_type;
  string m_sText;
  int m_nValue;
  int m_nValue2;
  string m_sAssocFile;
  int m_nAssocImage;
  bool m_bDisabled;
  int m_nAssocListItems;
  string *m_sAssocListItems;
  BUISelectionList m_listMenu;
  bool m_bOpen;

  BMenuItem() {
    m_type = BASIC;
    m_nValue = 0;
    m_sText = "";
    m_sAssocFile = "";
    m_nAssocImage = -1;
    m_bOpen = false;
    m_bDisabled = false;
  }
};

class BMenu {
public:
  enum TType {
    MAIN,
    CHOOSE_SCENE,
    CHOOSE_VEHICLE,
    PRECACHING_TERRAIN,
    SETTINGS,
    CREDITS
  };

  TType m_type;
  int m_nItems;
  BMenuItem *m_items;
  string *m_sItems;

  int m_nTitleWidth;
  int m_nTitleHeight;
  double m_dTitleX;
  double m_dTitleY;

  bool m_bDrawLine;
  unsigned m_clockStarted;

  BUISelectionList m_listMenu;
  BTextRenderer::TTextAlign m_align;

  BMenu();
};
