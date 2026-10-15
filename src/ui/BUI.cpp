// BUI: Misc UI components
#include "GLCompat.h"

#include "BUI.h"
#include "BGame.h"
#include "BSimulation.h"
#include "BTextRenderer.h"
#include "Renderer2D.h"
#include "VeraView.h"
#include <SDL2/SDL.h>

BTextRenderer BUI::m_textRenderer;
BUISelectionList *BUI::m_pSelList = 0;
void (CVeraView::*BUI::m_pPrevKeyDownFunction)(int, int, int) = 0;
int BUI::m_nPrevSliderValue = 0;
int *BUI::m_pnSliderValue = 0;

void BUI::StartUsingSelectionList(
    BUISelectionList *pList,
    void (CVeraView::*pPrevKeyDownFunction)(int, int, int)) {
  m_pSelList = pList;
  pList->SaveSelection();
  m_pPrevKeyDownFunction = pPrevKeyDownFunction;
  CVeraView *pView = BGame::GetView();
  pView->m_pKeyDownFunction = &CVeraView::OnKeyDownSelectionList;
}

void BUI::StartUsingSlider(int *pnSliderValue,
                           void (CVeraView::*pPrevKeyDownFunction)(int, int,
                                                                   int)) {
  m_pnSliderValue = pnSliderValue;
  m_nPrevSliderValue = *pnSliderValue;
  m_pPrevKeyDownFunction = pPrevKeyDownFunction;
  CVeraView *pView = BGame::GetView();
  pView->m_pKeyDownFunction = &CVeraView::OnKeyDownSlider;
}

BUISelectionList::BUISelectionList() {
  m_nSelected = -1;
  m_psItems = 0;
  m_nItems = 0;
  m_sPrompt = "";
}

void BUISelectionList::SaveSelection() { m_nSavedSelection = m_nSelected; }

void BUISelectionList::Cancel() { m_nSelected = m_nSavedSelection; }

// void BUISelectionList::SetItems(CString *psItems, int nItems, CString
// sPrompt) {
void BUISelectionList::SetItems(string *psItems, int nItems, string sPrompt) {
  m_psItems = psItems;
  m_nItems = nItems;
  m_sPrompt = sPrompt;
}

// int BUISelectionList::SelectItem(CString sItem) {
int BUISelectionList::SelectItem(string sItem) {
  // Search for the string and select that item.
  // Return selected item's index
  for (int i = 0; i < m_nItems; ++i) {
    // if(sItem.CompareNoCase(m_psItems[i]) == 0) {
    if (sItem.compare(m_psItems[i]) == 0) {
      m_nSelected = i;
      return i;
    }
  }
  m_nSelected = -1;
  return -1;
}

void BUISelectionList::AdvanceSelection(int nAmount) {
  m_nSelected += nAmount;
  if (m_nSelected < -100) {
    m_nSelected = 0;
  }
  if (m_nSelected < 0) {
    m_nSelected = (m_nItems - 1);
  }
  if (m_nSelected > (m_nItems + 100)) {
    m_nSelected = (m_nItems - 1);
  }
  if (m_nSelected > (m_nItems - 1)) {
    m_nSelected = 0;
  }
}

// int BUISelectionList::GetSelectedItem(CString &sItemText) {
int BUISelectionList::GetSelectedItem(string &sItemText) {
  if (m_nSelected != -1) {
    sItemText = m_psItems[m_nSelected];
  }
  return m_nSelected;
}

void BUISelectionList::DrawAt(double dX, double dY,
                              BTextRenderer::TTextAlign textAlign, double dRed,
                              double dGreen, double dBlue, bool bScrolling,
                              bool bWithBackground) {
  double dCharHeight = BUI::TextRenderer()->GetCharHeight();
  double dCharWidth = BUI::TextRenderer()->GetCharWidth();

  glPushMatrix();

  glTranslated(dX, dY, 0);
  if (bScrolling && (m_nSelected != -1)) {
    glTranslated(0, dCharHeight * double(m_nSelected), 0);
  } else {
    glTranslated(0, (dCharHeight * double(m_nItems)) / 2.0, 0);
  }

  // Draw background, if requested
  int i;
  if (bWithBackground) {
    double dMaxLen = 0.0;
    for (i = 0; i < m_nItems; ++i) {
      if ((double(m_psItems[i].length()) * dCharWidth) > dMaxLen) {
        dMaxLen = double(m_psItems[i].length()) * dCharWidth;
      }
    }
    double dXOffset = 0;
    if (textAlign == BTextRenderer::ALIGN_CENTER) {
      dXOffset = -dMaxLen / 2.0;
    } else if (textAlign == BTextRenderer::ALIGN_RIGHT) {
      dXOffset = -dMaxLen;
    }

    double dYBase = double(m_nItems - 1) * -dCharHeight;
    double bgY = dYBase - dCharHeight / 2.0;
    double bgH = double(m_nItems) * dCharHeight;
    double bgX = -5 + dXOffset;
    double bgW = dMaxLen + 11;

    // Stage B: draw shadow + background via Renderer2D (shader-based).
    // The MV variants read the current GL modelview matrix (set up by the
    // caller's glPushMatrix/glTranslated) so local-space coords work.
    if (Renderer2D::get().isReady()) {
      // Shadow (offset by +6, -6 via glTranslated above)
      glTranslated(6, -6, 0);
      Renderer2D::get().useShader();
      Renderer2D::get().drawSolidQuadMV(
          float(bgX), float(bgY), float(bgW), float(bgH),
          glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
      glTranslated(-6, 6, 0);

      // Actual background
      Renderer2D::get().drawSolidQuadMV(
          float(bgX), float(bgY), float(bgW), float(bgH),
          glm::vec4(0.15f, 0.15f, 0.15f, 1.0f));
      Renderer2D::get().unuseShader();
    } else {
      // Legacy fallback (only if shader failed to init)
      glTranslated(6, -6, 0);
      OpenGLHelpers::SetColorFull(0, 0, 0, 0.5);
      glBegin(GL_QUADS);
      glVertex3f(bgX, bgY, 0);
      glVertex3f(bgX, bgY + bgH, 0);
      glVertex3f(bgX + bgW, bgY + bgH, 0);
      glVertex3f(bgX + bgW, bgY, 0);
      glEnd();
      glTranslated(-6, 6, 0);
      OpenGLHelpers::SetColorFull(0.15, 0.15, 0.15, 1);
      glBegin(GL_QUADS);
      glVertex3f(bgX, bgY, 0);
      glVertex3f(bgX, bgY + bgH, 0);
      glVertex3f(bgX + bgW, bgY + bgH, 0);
      glVertex3f(bgX + bgW, bgY, 0);
      glEnd();
    }

    // Border (GL_LINE_STRIP - kept on legacy path; lines are Stage C)
    OpenGLHelpers::SetColorFull(1, 1, 1, 1);
    glBegin(GL_LINE_STRIP);
    glVertex3f(-5 + dXOffset, dYBase - dCharHeight / 2.0, 0);
    glVertex3f(-5 + dXOffset,
               dYBase + double(m_nItems - 1) * dCharHeight + dCharHeight / 2.0,
               0);
    glVertex3f(dXOffset + dMaxLen + 6,
               dYBase + double(m_nItems - 1) * dCharHeight + dCharHeight / 2.0,
               0);
    glVertex3f(dXOffset + dMaxLen + 6, dYBase - dCharHeight / 2.0, 0);
    glVertex3f(-5 + dXOffset, dYBase - dCharHeight / 2.0, 0);
    glEnd();
  }

  // First draw selection rectangle behind the selected item
  if (m_nSelected != -1) {
    glDisable(GL_TEXTURE_2D);
    double dLen = double(m_psItems[m_nSelected].length()) * dCharWidth;
    double dYBase = double(m_nSelected) * -dCharHeight;

    double dXOffset = 0;
    if (textAlign == BTextRenderer::ALIGN_CENTER) {
      dXOffset = -dLen / 2.0;
    } else if (textAlign == BTextRenderer::ALIGN_RIGHT) {
      dXOffset = -dLen;
    }

    double dAlpha = fabs(double(SDL_GetTicks() % 1000) - 500.0) / 500.0;
    float selX = float(-5 + dXOffset);
    float selY = float(dYBase - dCharHeight / 2.0);
    float selW = float(dLen + 10);
    float selH = float(dCharHeight);

    glTranslated(0, -1, 0);

    // Stage B: selection highlight via Renderer2D.
    // The legacy code used per-vertex colors for a vertical gradient
    // (dark at top/bottom, blue in the middle). We approximate this with
    // a solid color using the middle blue value - the visual difference
    // is minor and the gradient can be restored later via a custom vertex
    // color upload if needed.
    if (Renderer2D::get().isReady()) {
      Renderer2D::get().useShader();
      // Use the middle color (0.5, 0.5, 1.0) scaled by alpha.
      // The legacy gradient was: top=black, middle=blue, bottom=dark-blue.
      // We use the middle blue with the average alpha.
      Renderer2D::get().drawSolidQuadMV(
          selX, selY, selW, selH,
          glm::vec4(0.35f, 0.35f, 0.75f, float(0.5 * dAlpha)));
      Renderer2D::get().unuseShader();
    } else {
      glBegin(GL_QUADS);
      OpenGLHelpers::SetColorFull(0, 0, 0, 0.95 * dAlpha);
      glVertex3f(-5 + dXOffset, dYBase + -dCharHeight / 2.0, 0);
      OpenGLHelpers::SetColorFull(0.5, 0.5, 1, 0.5 * dAlpha);
      glVertex3f(-5 + dXOffset, dYBase + dCharHeight / 2.0, 0);
      OpenGLHelpers::SetColorFull(0.25, 0.25, 1, 0.5 * dAlpha);
      glVertex3f(dXOffset + dLen + 5, dYBase + dCharHeight / 2.0, 0);
      OpenGLHelpers::SetColorFull(0, 0, 0, 0.95 * dAlpha);
      glVertex3f(dXOffset + dLen + 5, dYBase + -dCharHeight / 2.0, 0);
      glEnd();
    }

    // Selection border (GL_LINE mode - kept on legacy path)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    OpenGLHelpers::SetColorFull(1, 1, 1, 0.4);
    glBegin(GL_QUADS);
    glVertex3f(-5 + dXOffset, dYBase + -dCharHeight / 2.0, 0);
    glVertex3f(-5 + dXOffset, dYBase + dCharHeight / 2.0, 0);
    glVertex3f(dXOffset + dLen + 5, dYBase + dCharHeight / 2.0, 0);
    glVertex3f(dXOffset + dLen + 5, dYBase + -dCharHeight / 2.0, 0);
    glEnd();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glTranslated(0, 1, 0);
  }

  // Then draw items
  BUI::TextRenderer()->StartRenderingText();
  for (i = 0; i < m_nItems; ++i) {

    // double dLen = double(m_psItems[i].GetLength()) * dCharWidth;
    double dLen = double(m_psItems[i].length()) * dCharWidth;
    double dXOffset = 0;
    // if(textAlign == BTextRenderer::TTextAlign::ALIGN_CENTER) {
    if (textAlign == BTextRenderer::ALIGN_CENTER) {
      dXOffset = -dLen / 2.0;
      //} else if(textAlign == BTextRenderer::TTextAlign::ALIGN_RIGHT) {
    } else if (textAlign == BTextRenderer::ALIGN_RIGHT) {
      dXOffset = -dLen;
    }

    BUI::TextRenderer()->DrawTextAt(0, // dXOffset,
                                    double(i) * -dCharHeight, m_psItems[i],
                                    textAlign, dRed, dGreen, dBlue);
  }
  BUI::TextRenderer()->StopRenderingText();
  glPopMatrix();
}

bool BUISelectionList::OnFingerUp(float x, float y, double dX, double dY,
                                  bool bScrolling) {
  double dCharHeight = BUI::TextRenderer()->GetCharHeight();
  double tmp_y = y - dY;               // touch pos related to mid screen
  tmp_y += m_nItems * dCharHeight / 2; // DrawAt draws to center of the screen
  tmp_y += dCharHeight / 2; // DrawTextAt draws item to center of given position
  if (bScrolling) {
    // Values from BUISelectionList::DrawAt
    tmp_y -= dCharHeight * double(double(m_nItems) / 2 - m_nSelected);
  }
  tmp_y /= dCharHeight; // mouse position to menu index
  if (tmp_y < 0 || tmp_y >= m_nItems) {
    return false;
  }
  m_nSelected = tmp_y;
  return true;
}

BUIEdit::BUIEdit() {
  m_nMaxLength = 0;
  m_sValue = "";
  m_sPrompt = "";
}

// void BUIEdit::Setup(CString sPrompt, CString sValue, int nMaxLength) {
void BUIEdit::Setup(string sPrompt, string sValue, int nMaxLength) {
  m_sPrompt = sPrompt;
  m_sValue = sValue;
  m_nMaxLength = nMaxLength;
  status = EDITING;
}

void BUIEdit::ProcessChar(unsigned char c) {
  if (!(((toupper(c) >= 32) && (toupper(c) <= 96)) || (c == '.') || (c == '/') ||
        //(c == VK_BACK) ||
        (c == SDLK_BACKSPACE) ||
        //(c == VK_RETURN) ||
        (c == SDLK_RETURN) ||
        //(c == VK_ESCAPE))) {
        (c == SDLK_ESCAPE))) {
    // Bad character, no go
    return;
  }

  // if(c == VK_BACK) {
  if (c == SDLK_BACKSPACE) {
    // Eat one character
    // if(m_sValue.GetLength() > 0) {
    if (m_sValue.length() > 0) {
      // m_sValue.Delete(m_sValue.GetLength() - 1, 1);
      m_sValue.erase(m_sValue.length() - 1, 1);
    }
    return;
  }

  // if(c == VK_RETURN) {
  if (c == SDLK_RETURN) {
    status = READY;
    return;
  }

  // if(c == VK_ESCAPE) {
  if (c == SDLK_ESCAPE) {
    status = CANCELED;
    return;
  }

  // Add writable character
  // if(m_sValue.GetLength() < m_nMaxLength) {
  if (m_sValue.length() < m_nMaxLength) {
    m_sValue += c;
  }
  return;
}

string BUIEdit::GetValue(TStatus &rStatus) {
  rStatus = status;
  return m_sValue;
}

void BUIEdit::DrawAt(double dX, double dY, bool bCursor,
                     BTextRenderer::TTextAlign textAlign) {
  double dCharHeight = BUI::TextRenderer()->GetCharHeight();
  double dCharWidth = BUI::TextRenderer()->GetCharWidth();

  glPushMatrix();

  glTranslated(dX, dY, 0);

  // if(textAlign == BTextRenderer::TTextAlign::ALIGN_LEFT) {
  if (textAlign == BTextRenderer::ALIGN_LEFT) {
    // glTranslated(dCharWidth * double(m_sPrompt.GetLength()), 0, 0);
    glTranslated(dCharWidth * double(m_sPrompt.length()), 0, 0);
  }

  BUI::TextRenderer()->StartRenderingText();

  // Prompt
  BUI::TextRenderer()->DrawTextAt(0, 0, m_sPrompt,
                                  // BTextRenderer::TTextAlign::ALIGN_RIGHT,
                                  BTextRenderer::ALIGN_RIGHT, 1, 0.75, 0.5);

  // Value
  BUI::TextRenderer()->DrawTextAt(0, 0, m_sValue,
                                  // BTextRenderer::TTextAlign::ALIGN_LEFT,
                                  BTextRenderer::ALIGN_LEFT, 1, 1, 1);

  BUI::TextRenderer()->StopRenderingText();

  // Cursor

  glDisable(GL_TEXTURE_2D);
  // double dXOffset = double(m_sValue.GetLength()) * dCharWidth;
  double dXOffset = double(m_sValue.length()) * dCharWidth;
  double dLen = dCharWidth;

  double dAlpha = fabs(double(SDL_GetTicks() % 1000) - 500.0) / 500.0;

  glTranslated(-1, -1, 0);
  glBegin(GL_QUADS);
  OpenGLHelpers::SetColorFull(0, 0, 0, 0.95 * dAlpha);
  glVertex3f(dXOffset, -dCharHeight / 2.0, 0);
  OpenGLHelpers::SetColorFull(0.5, 0.5, 1, 0.5 * dAlpha);
  glVertex3f(dXOffset, dCharHeight / 2.0, 0);
  OpenGLHelpers::SetColorFull(0.25, 0.25, 1, 0.5 * dAlpha);
  glVertex3f(dXOffset + dLen, dCharHeight / 2.0, 0);
  OpenGLHelpers::SetColorFull(0, 0, 0, 0.95 * dAlpha);
  glVertex3f(dXOffset + dLen, -dCharHeight / 2.0, 0);
  glEnd();

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  OpenGLHelpers::SetColorFull(1, 1, 1, 0.4);
  glBegin(GL_QUADS);
  glVertex3f(dXOffset, -dCharHeight / 2.0, 0);
  glVertex3f(dXOffset, dCharHeight / 2.0, 0);
  glVertex3f(dXOffset + dLen, dCharHeight / 2.0, 0);
  glVertex3f(dXOffset + dLen, -dCharHeight / 2.0, 0);
  glEnd();
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glTranslated(0, 1, 0);

  glPopMatrix();
}
