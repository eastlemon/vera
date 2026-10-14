// BMessages: Show on-screen messages

#pragma once

#include "BCmdModule.h"
#include "BGame.h"
#include "BSimulation.h"
#include "BaseClasses.h"
#include <string>

using namespace std;

class BLetter {
public:
  char m_cParts[15];
};

class BMessage {
public:
  int m_nCharPartWidth;
  int m_nCharPartGap;
  int m_nCharWidth;
  int m_nCharHeight;
  int m_nCharGap;
  int m_nTotalWidth;
  double m_dR;
  double m_dG;
  double m_dB;
  bool m_blink;
  int m_nCharsPerLine;
  string m_sId;
  string m_sText;
  time_t m_timeRemove;
  clock_t m_clockStarted;
  bool m_bFaint;
  bool m_bSecondary;
  BMessage *m_pNext;
};

class BMessages {
  static BLetter m_letters[65];
  static BMessage *m_pMessages;

public:
  BMessages();
  ~BMessages();

  static void Show(int nCharsPerLine, string sId, string sText, int nSeconds,
                   bool blink = false, double dR = 1, double dG = 1,
                   double dB = 1, bool bFaint = false, bool bSecondary = false);
  static void Remove(string sId);
  static void RemoveAll();
  static void Render();
  static bool Find(string sId);

  static BMessage *RenderMessage(BMessage *pMsg);
  static void RenderLetter(BMessage *pMsg, char c, bool bInside);
};
