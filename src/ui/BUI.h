//
// BUI: Misc UI components

#pragma once

#include "BTextRenderer.h"
#include "BaseClasses.h"
#include <string>

using namespace std;

class CVeraView;

class BUIEdit {
  int m_nMaxLength;
  string m_sValue;
  string m_sPrompt;

public:
  enum TStatus { EDITING, READY, CANCELED };

private:
  TStatus status;

public:
  BUIEdit();
  void Setup(string sPrompt, string sValue, int nMaxLength);
  void ProcessChar(unsigned char c);
  string GetValue(TStatus &rStatus);
  void
  DrawAt(double dX, double dY, bool bCursor,
         BTextRenderer::TTextAlign textAlign = BTextRenderer::ALIGN_CENTER);
};

class BUISelectionList {
  string m_sPrompt;
  int m_nSelected;
  string *m_psItems;
  int m_nSavedSelection;
  int m_nItems;

public:
  BUISelectionList();
  void SetItems(string *psItems, int nItems, string sPrompt = "");
  int GetNofItems() { return m_nItems; }
  int SelectItem(string sItem);
  void AdvanceSelection(int nAmount);
  void SaveSelection();
  void Cancel();
  int GetSelectedItem(string &sItemText);
  void DrawAt(double dX, double dY,
              BTextRenderer::TTextAlign textAlign = BTextRenderer::ALIGN_LEFT,
              double dRed = 1, double dGreen = 1, double dBlue = 1,
              bool bScrolling = true, bool bWithBackground = false);
  bool OnFingerUp(float x, float y, double dX, double dY, bool bScrolling);
};

class BUI {
public:
  BUI();

  static BTextRenderer *TextRenderer() { return &m_textRenderer; }
  static BUISelectionList *GetActiveSelectionList() { return m_pSelList; }
  static void StartUsingSelectionList(
      BUISelectionList *pList,
      void (CVeraView::*pPrevKeyDownFunction)(int, int, int));
  static void (CVeraView::*StopUsingSelectionList())(int, int, int) {
    m_pSelList = 0;
    return m_pPrevKeyDownFunction;
  }
  static void StartUsingSlider(int *pnSliderValue,
                               void (CVeraView::*pPrevKeyDownFunction)(int, int,
                                                                       int));
  static void (CVeraView::*StopUsingSlider())(int, int, int) {
    m_pnSliderValue = 0;
    return m_pPrevKeyDownFunction;
  }

  static int m_nPrevSliderValue;
  static int *m_pnSliderValue;
  static BTextRenderer m_textRenderer;
  static BUISelectionList *m_pSelList;
  static void (CVeraView::*m_pPrevKeyDownFunction)(int, int, int);
};
