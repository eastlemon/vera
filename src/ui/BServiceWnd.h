

#pragma once

#include "BaseClasses.h"
#include <string>

using namespace std;

class BServiceWnd {
  bool m_bPrompting;
  int m_nCommandStart;
  int m_nCursor;
  char m_sText[41 * 17];
  unsigned char m_cColors[41 * 17 * 3];
  unsigned char m_cBright[41 * 17];
  void AddString(string sText, double dR = 1, double dG = 1, double dB = 1);
  void Prompt();

public:
  BServiceWnd();
  ~BServiceWnd();
  void AddChar(char c, double dR = 0, double dG = 1, double dB = 0);
  void Output(string sText, double dR = 0.75, double dG = 0.75,
              double dB = 0.75);
  void Newline(bool bForce = false);
  void DrawTexts();
};
