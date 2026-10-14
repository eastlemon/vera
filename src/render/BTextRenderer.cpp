// BTextRenderer: Misc text rendering routines
#include "BTextRenderer.h"
#include "BSimulation.h"
#include "BTextures.h"
#include <algorithm>

using namespace std;

BTextRenderer::BTextRenderer() {
  m_dCharWidth = 14.5;
  m_dCharHeight = 28.75;
}

void BTextRenderer::StartRenderingText() {
  OpenGLHelpers::SwitchToTexture(0);
  // BTextures::Use(BTextures::Texture::PANEL);
  BTextures::Use(BTextures::PANEL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void BTextRenderer::StopRenderingText() { glDisable(GL_TEXTURE_2D); }

void BTextRenderer::DrawTextAt(double dX, double dY,
                               string sText, TTextAlign textalign, double dRed,
                               double dGreen, double dBlue, double dAlpha) {

  OpenGLHelpers::SetColorFull(dRed, dGreen, dBlue, dAlpha);

  glPushMatrix();
  glTranslated(dX, dY - m_dCharHeight / 2.0, 0);
  if (textalign == ALIGN_RIGHT) {
    // glTranslated(-m_dCharWidth * sText.GetLength(), 0, 0);
    glTranslated(-m_dCharWidth * sText.length(), 0, 0);
  } else if (textalign == ALIGN_CENTER) {
    // glTranslated(-(m_dCharWidth * double(sText.GetLength())) / 2.0, 0, 0);
    glTranslated(-(m_dCharWidth * double(sText.length())) / 2.0, 0, 0);
  }

  // Draw text char by char
  // for(int i = 0; i < sText.GetLength(); ++i) {
  for (int i = 0; i < sText.length(); ++i) {
    char c = sText[i];
    c = toupper(c);
    c -= ' ';

    glBegin(GL_TRIANGLE_STRIP);
    int xOff = c % 5;
    int yOff = c / 5;
    OpenGLHelpers::SetTexCoord((1.0 + double(xOff) * m_dCharWidth) / 512.0,
                               (511.0 - (0.0 + double(yOff) * m_dCharHeight)) /
                                   512.0);
    glVertex3f(0, m_dCharHeight - 4.0, 0);
    OpenGLHelpers::SetTexCoord(
        (1.0 + double(xOff) * m_dCharWidth) / 512.0,
        (511.0 - (0.0 + double(yOff) * m_dCharHeight + m_dCharHeight - 4.0)) /
            512.0);
    glVertex3f(0, 0, 0);
    OpenGLHelpers::SetTexCoord(
        (1.0 + double(xOff) * m_dCharWidth + m_dCharWidth) / 512.0,
        (511.0 - (0.0 + double(yOff) * m_dCharHeight)) / 512.0);
    glVertex3f(m_dCharWidth, m_dCharHeight - 4.0, 0);
    OpenGLHelpers::SetTexCoord(
        (1.0 + double(xOff) * m_dCharWidth + m_dCharWidth) / 512.0,
        (511.0 - (0.0 + double(yOff) * m_dCharHeight + m_dCharHeight - 4.0)) /
            512.0);
    glVertex3f(m_dCharWidth, 0, 0);
    glEnd();
    glTranslated(m_dCharWidth, 0, 0);
  }

  glPopMatrix();
}

void BTextRenderer::DrawSmallTextAt(double dX, double dY,
                                    string sText, int nChars,
                                    TTextAlign textalign, double dRed,
                                    double dGreen, double dBlue,
                                    double dAlpha) {

  double xSize = 8.25;
  double ySize = 13.9;

  OpenGLHelpers::SetColorFull(dRed, dGreen, dBlue, dAlpha);

  glPushMatrix();
  glTranslated(dX, dY, 0);
  if (textalign == ALIGN_RIGHT) {
    // glTranslated(-xSize * sText.GetLength(), 0, 0);
    glTranslated(-xSize * sText.length(), 0, 0);
  } else if (textalign == ALIGN_CENTER) {
    // glTranslated(-(xSize * double(sText.GetLength())) / 2.0, 0, 0);
    glTranslated(-(xSize * double(sText.length())) / 2.0, 0, 0);
  }

  // Draw text char by char
  // for(int i = 0; i < min(nChars, sText.GetLength()); ++i) {
  for (int i = 0; i < min(nChars, (int)sText.length()); ++i) {
    char c = sText[i];
    c = toupper(c);
    c -= ' ';

    glBegin(GL_TRIANGLE_STRIP);
    int xOff = c % 10;
    int yOff = c / 10;
    OpenGLHelpers::SetTexCoord((401.0 + double(xOff) * xSize) / 512.0,
                               (511.0 - (284.0 + double(yOff) * ySize)) /
                                   512.0);
    glVertex3f(0, ySize, 0);
    OpenGLHelpers::SetTexCoord(
        (401.0 + double(xOff) * xSize) / 512.0,
        (511.0 - (284.0 + double(yOff) * ySize + ySize - 0.8)) / 512.0);
    glVertex3f(0, 0, 0);
    OpenGLHelpers::SetTexCoord((401.0 + double(xOff) * xSize + xSize) / 512.0,
                               (511.0 - (284.0 + double(yOff) * ySize)) /
                                   512.0);
    glVertex3f(xSize, ySize, 0);
    OpenGLHelpers::SetTexCoord(
        (401.0 + double(xOff) * xSize + xSize) / 512.0,
        (511.0 - (284.0 + double(yOff) * ySize + ySize - 0.8)) / 512.0);
    glVertex3f(xSize, 0, 0);
    glEnd();
    glTranslated(xSize, 0, 0);
  }

  glPopMatrix();
}
