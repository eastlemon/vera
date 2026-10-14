//
// VeraView.h : interface of the CVeraView class

#if !defined(AFX_VeraVIEW_H__73E6FC07_BC82_11D4_B532_0060B0F1F5DD__INCLUDED_)
#define AFX_VeraVIEW_H__73E6FC07_BC82_11D4_B532_0060B0F1F5DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>

using namespace std;

#include <SDL2/SDL.h>

#include "BGame.h"
#include "BMessages.h"
#include "BaseClasses.h"
#include "dialogs/DlgOnScreenKbd.h"

class CVeraView {

  void InitVideo();
  void DrawVideo();
  bool m_bShowVideo;

  BGame m_game;
  BMessages m_messages;

  bool m_bInitClock;

  bool m_bWireframe;
  bool m_bNormals;
  bool m_bCreateDLs;

public:
  unsigned m_nMenuTime; // Time spent viewing menu

  int window_width;
  int window_height;

  int mouse_x;
  int mouse_y;

  enum MouseFlags { MOUSE_NONE, MOUSE_DOWN, MOUSE_UP };

  bool exit;

private:
  int m_chaMenuMusic; // Channel for the menu background music

  bool m_bIgnoreNextChar;

  SDL_Thread *m_pThreadLoading;

  enum TMenuScroll { SCROLL_LEFT, SCROLL_RIGHT, SCROLL_UP, SCROLL_DOWN };

  TMenuScroll m_scrollDir;
  unsigned m_clockMenuScroll;

  // void ProcessMouseInput(int nFlags, CPoint point);
  void ProcessMouseInput(MouseFlags nFlags, SDL_Point point);

  void StartMenuScroll(TMenuScroll scroll);
  void DrawMenu(BMenu *pMenu);
  void DrawMenuItemTextAtRelPos(int nX, int nY, int nItems, int nIndex,
                                BMenuItem *pMenuItem);
  void DrawMenuItemSliderAtRelPos(int nX, int nY, int nItems, int nIndex,
                                  BMenuItem *pMenuItem);

  // void DrawMenuItem(CDC* pDC, CFont *pFont, int m, int nY, string sText,
  // COLORREF color, CRect rectWnd);
public:
  bool m_bDrawOnlyMenu;
  bool m_bFullRedraw;

  // void OnDrawIntro(CDC* pDC);     // OBSOLETE
  void OnDrawIntro(); // OBSOLETE
  // void OnDrawCurrentMenu(CDC* pDC);
  void OnDrawCurrentMenu();
  // void OnDrawGame(CDC* pDC);
  void OnDrawGame();

  void OnKeyDownGame(int nChar, int nRepCnt, int nFlags);
  void OnKeyDownGameMenu(int nChar, int nRepCnt, int nFlags);
  void OnKeyDownFueling(int nChar, int nRepCnt, int nFlags);
  void OnKeyDownCancelQuestion(int nChar, int nRepCnt, int nFlags);
  void OnKeyDownSceneEditor(int nChar, int nRepCnt, int nFlags);
  void OnKeyDownSelectionList(int nChar, int nRepCnt, int nFlags);
  void OnKeyDownSlider(int nChar, int nRepCnt, int nFlags);
  void OnKeyDownCurrentMenu(int nChar, int nRepCnt, int nFlags);

  void ReturnPressedOnCurrentMenu();
  void CancelPressedOnCurrentMenu();

  void CheckForFueling();
  void CheckForPickup();
  void CheckForDelivery();

  void Setup2DRendering();
  void End2DRendering();
  // void DrawDashboard(CDC* pDC);
  void DrawDashboard();
  // void DrawFuelingButtons(CDC* pDC, CRect &rectWnd);
  void DrawFuelingButtons();
  // void DrawNavSat(CDC* pDC);
  void DrawNavSat();
  // void DrawQuickHelp(CDC* pDC);
  void DrawQuickHelp();
  // void DrawServiceWnd(CDC* pDC);
  void DrawServiceWnd();
  // void DrawServiceWndTexts(CRect &rectWnd);
  void DrawServiceWndTexts();
  void DrawSrvText(string sText);
  // void DrawPickupInfo(CDC* pDC, CRect &rectWnd);
  void DrawPickupInfo();
  // void DrawDeliveryInfo(CDC* pDC, CRect &rectWnd);
  void DrawDeliveryInfo();
  // void DrawTrackingTarget(BTrackingTarget *pTarget, CRect &rectWnd);
  void DrawTrackingTarget(BTrackingTarget *pTarget);
  // void UpdateTrackingTargetScreenPos(BTrackingTarget *pTarget, CRect
  // &rectWnd);
  void UpdateTrackingTargetScreenPos(BTrackingTarget *pTarget);
  // void DrawTrackingTargetOnScreen(BTrackingTarget *pTarget, CRect &rectWnd);
  void DrawTrackingTargetOnScreen(BTrackingTarget *pTarget);
  void DrawNavSatIndicator(bool bCar, double dAngle, double dR, double dG,
                           double dB, double dAlpha,
                           // BVector vLoc,
                           BVector vLoc);
  void DrawPanel(double dWidth, double dHeight, double dRed = 0,
                 double dGreen = 0, double dBlue = 0, double dAlpha = 1);

  // void DrawMouseCursor(CRect &rectWnd);
  void DrawMouseCursor();
  BVector ColorForGForce(double dGForce);

private:

  // protected: // create from serialization only
public:
  CVeraView();

  // Attributes
public:
  // void (CVeraView::*m_pDrawFunction)(CDC*);
  void (CVeraView::*m_pDrawFunction)();
  void (CVeraView::*m_pKeyDownFunction)(int, int, int);


  // Operations
public:
  // Overrides
public:
  // virtual void OnDraw(CDC* pDC);  // overridden to draw this view
  virtual void OnDraw(); // overridden to draw this view
  // virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
  virtual bool PreCreateWindow();

protected:

  void InitializeOpenGL();

  // Implementation
public:
  virtual ~CVeraView();
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

protected:
  // Generated message map functions
  // protected:
public:
  // FIXME
  int OnCreate();
  bool OnEraseBkgnd();
  void OnDestroy();
  void OnChar(int nChar, int nRepCnt, int nFlags);
  void OnKeyDown(int nChar, int nRepCnt, int nFlags);
  void OnKeyUp(int nChar, int nRepCnt, int nFlags);
  void OnMouseMove(SDL_Point point);
  void OnLButtonDown(SDL_Point point);
  void OnLButtonUp(SDL_Point point);
  void OnFingerDown(float x, float y, int finger_id);
  void OnFingerUp(float x, float y, int finger_id);
  void HandleBUITouch(SDL_Point point);
  bool touch_seen;

  vector<Dialog *> dialogs;

  void drawDialogs();

  template <typename D> vector<Dialog *>::iterator getFirstDialogOfType();
  template <typename D> bool isDialogOpen();

  void drawSurface(double x1, double y1, double x2, double y2, GLenum format,
                   SDL_Surface *surface);



  SDL_Window *window;

  void setWindow(SDL_Window *window) { this->window = window; }

  void setExit();

  bool isExit() { return exit; }
};

//

//{{AFX_INSERT_LOCATION}}
// the previous line.

#endif // !defined(AFX_VeraVIEW_H__73E6FC07_BC82_11D4_B532_0060B0F1F5DD__INCLUDED_)
