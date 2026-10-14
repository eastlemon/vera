// BTextRenderer: Misc text rendering routines

#pragma once

#include "BaseClasses.h"
#include <string>

using namespace std;

class BTextRenderer {

  double m_dCharWidth;
  double m_dCharHeight;

public:
  BTextRenderer();

  enum TTextAlign { ALIGN_LEFT, ALIGN_RIGHT, ALIGN_CENTER };

  void StartRenderingText();
  void DrawTextAt(double nX, double nY, string sText,
                  TTextAlign textalign = ALIGN_LEFT, double dRed = 1,
                  double dGreen = 1, double dBlue = 1, double dAlpha = 1);
  void DrawSmallTextAt(double dX, double dY, string sText, int nChars,
                       TTextAlign textalign = ALIGN_LEFT, double dRed = 1,
                       double dGreen = 1, double dBlue = 1, double dAlpha = 1);

  void StopRenderingText();
  double GetCharHeight() { return m_dCharHeight; }
  double GetCharWidth() { return m_dCharWidth; }
};
