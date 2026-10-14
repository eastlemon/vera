// BMenu: start menu components

#include "BMenu.h"
#include "BGame.h"
#include "BTextRenderer.h"

BMenu::BMenu() {
  m_nItems = 0;
  m_items = 0;

  m_nTitleWidth = 0;
  m_nTitleHeight = 0;
  m_dTitleX = 0;
  m_dTitleY = 0;

  m_bDrawLine = true;

  m_listMenu.SetItems(0, 0);
  // m_align = BTextRenderer::TTextAlign::ALIGN_CENTER;
  m_align = BTextRenderer::ALIGN_CENTER;
}
