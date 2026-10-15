// BTextRenderer: text rendering via TextRenderer2D (shader-based).

#include "GLCompat.h"

#include "BTextRenderer.h"
#include "BSimulation.h"
#include "BTextures.h"
#include "TextRenderer2D.h"
#include <algorithm>

using namespace std;

// Singleton text renderer instance.
static TextRenderer2D &textRenderer2D() {
  static TextRenderer2D instance;
  return instance;
}

BTextRenderer::BTextRenderer() {
  m_dCharWidth = 14.5;
  m_dCharHeight = 28.75;
}

void BTextRenderer::StartRenderingText() {
  // Bind the glyph atlas texture. The shader will sample from it.
  OpenGLHelpers::SwitchToTexture(0);
  BTextures::Use(BTextures::PANEL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glEnable(GL_TEXTURE_2D);
}

void BTextRenderer::StopRenderingText() {
  // No explicit unbind needed - the next GL call will set up its own state.
}

void BTextRenderer::DrawTextAt(double dX, double dY,
                               string sText, TTextAlign textalign, double dRed,
                               double dGreen, double dBlue, double dAlpha) {
  // Big font parameters (must match the original glBegin/glEnd code):
  //   glyph size:   14.5 x 28.75
  //   grid:         5 columns
  //   atlas origin: (1, 0)  - first glyph at pixel (1, 0)
  //   glyph Y gap:  -4.0    - the visible glyph is 4px shorter than m_dCharHeight
  //                           (legacy: glVertex3f(0, m_dCharHeight - 4.0, 0))
  //
  // The legacy code translates to (dX, dY - m_dCharHeight/2) before drawing,
  // so the baseline anchor is at (dX, dY - m_dCharHeight/2). We replicate
  // this by passing that Y as the text origin.
  float originX = float(dX);
  float originY = float(dY - m_dCharHeight / 2.0);

  textRenderer2D().drawText(
      sText, (int)sText.length(),
      originX, originY,
      float(m_dCharWidth), float(m_dCharHeight),
      -4.0f,                // glyphOffsetY (big font has -4px visible gap)
      5,                    // atlasCols
      1.0f, 0.0f,           // atlasOriginX, atlasOriginY
      float(m_dCharWidth),  // alignWidth
      (int)textalign,
      glm::vec4(float(dRed), float(dGreen), float(dBlue), float(dAlpha)));
}

void BTextRenderer::DrawSmallTextAt(double dX, double dY,
                                    string sText, int nChars,
                                    TTextAlign textalign, double dRed,
                                    double dGreen, double dBlue,
                                    double dAlpha) {
  // Small font parameters:
  //   glyph size:   8.25 x 13.9
  //   grid:         10 columns
  //   atlas origin: (401, 284)
  //   glyph Y gap:  -0.8  (legacy: glVertex3f(0, ySize, 0) top,
  //                        glVertex3f(0, 0, 0) bottom, so ySize is the full
  //                        height; the -0.8 is from the UV calc)
  float xSize = 8.25f;
  float ySize = 13.9f;

  textRenderer2D().drawText(
      sText, std::min(nChars, (int)sText.length()),
      float(dX), float(dY),
      xSize, ySize,
      -0.8f,         // glyphOffsetY (small font has -0.8px visible gap)
      10,            // atlasCols
      401.0f, 284.0f,  // atlasOriginX, atlasOriginY
      xSize,         // alignWidth
      (int)textalign,
      glm::vec4(float(dRed), float(dGreen), float(dBlue), float(dAlpha)));
}
