// VeraView.cpp - skeleton: constructor, lifecycle, OnDraw, InitializeOpenGL.
// For menu rendering see VeraView_Menu.cpp.
// For in-game frame rendering see VeraView_Game.cpp.
// For input dispatch see VeraView_Input.cpp.

#include <cstdlib>
#include <iostream>
#include <sstream>

#include <SDL2/SDL.h>

using namespace std;

#include "VeraView.h"

#include "BTextures.h"
#include "OpenGLExtFunctions.h"
#include "OpenGLHelpers.h"
#if defined(__APPLE__) && defined(__MACH__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#if defined(__APPLE__) && defined(__MACH__)
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif
#include "BMessages.h"
#include "BNavSatWnd.h"
#include "BServiceWnd.h"
#include "BTextRenderer.h"
#include "BUI.h"
#include "HeightMap.h"
#include "Settings.h"
#include "SoundModule.h"

extern bool g_cbBlackAndWhite;
extern double Random(double dRange);
extern double g_dPhysicsStepsInSecond;
extern bool g_bControl;
extern bool g_bShift;

// Globals defined in this file:
double g_dPhysicsStepsInSecond = 200.0; // Was 200.0
bool g_bControl = false;
bool g_bShift = false;
double g_dRate = 30.0;
double g_d10LastFPS[10];
double g_dAveRate = 30.0;
bool g_bMouseButtonDown = false;
static double g_cdPI = 3.1415926;
GLint g_nMatrixMode;

// Externs (defined in other files / not in this TU):
extern double g_dExtraAlpha;
extern BVector g_vCenter;

// ----------------------------------------------------------------

CVeraView::CVeraView() {
  m_nMenuTime = 0;
  m_pDrawFunction = &CVeraView::OnDrawCurrentMenu;
  m_pKeyDownFunction = &CVeraView::OnKeyDownCurrentMenu;
  m_bDrawOnlyMenu = false;
  m_bFullRedraw = true;
  // m_hCursor = 0;
  SoundModule::Initialize();
  m_bInitClock = true;
  m_bCreateDLs = false;
  m_bWireframe = false;
  m_bNormals = false;
  m_bIgnoreNextChar = false;
  m_clockMenuScroll = 0;
  // m_pThreadLoading = 0;

  // omat
  exit = false;
  mouse_x = 0;
  mouse_y = 0;
  touch_seen = false;
}

CVeraView::~CVeraView() {}

bool CVeraView::PreCreateWindow() {
  // TODO: Modify the Window class or styles here by modifying
  //  the CREATESTRUCT cs
  // return CView::PreCreateWindow(cs);
  return false;
}

int CVeraView::OnCreate() {

  // Save current screen resolution
  // FIXME
  /*EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &g_devmodeOrig);

  BOOL retval = CView::OnCreate(lpCreateStruct);

  HWND hWnd = GetSafeHwnd();
  HDC hDC = ::GetDC(hWnd);
  PIXELFORMATDESCRIPTOR pfd = {
  sizeof(PIXELFORMATDESCRIPTOR), // size of this pfd
  1, // version number
  PFD_DRAW_TO_WINDOW |    // support window
    PFD_SUPPORT_OPENGL |  // support OpenGL
    // PFD_SUPPORT_GDI |     // support GDI
    PFD_DOUBLEBUFFER |    // double buffered
    PFD_NEED_SYSTEM_PALETTE,
  PFD_TYPE_RGBA, // RGBA type
  32, // 32-bit color depth
  0, 0, 0, 0, 0, 0, // color bits ignored
  0, // no alpha buffer
  0, // shift bit ignored
  0, // no accumulation buffer
  0, 0, 0, 0, // accum bits ignored
  32, // 32-bit z-buffer
  8, // 8-bit stencil buffer for reflections
  0, // no auxiliary buffer
  PFD_MAIN_PLANE, // main layer
  0, // reserved
  0, 0, 0 // layer masks ignored
  };
  int pixelformat = ChoosePixelFormat(hDC, &pfd);
  TRACE("Pixelformat %d\n", pixelformat);
  if(SetPixelFormat(hDC, pixelformat, &pfd) == FALSE) {
    BGame::MyAfxMessageBox("SetPixelFormat failed");
  }
  m_hGLRC = wglCreateContext(hDC);
  wglMakeCurrent(hDC, m_hGLRC);*/

  // Check Player.State file integrity
  BGame::GetPlayer()->LoadStateFile();
  if ((BGame::GetPlayer()->m_dCash <= 0.01) &&
      (BGame::GetPlayer()->m_dFuel <= 0.01)) {
    // Assist player
    BGame::MyAfxMessageBox(
        "Assisting player with 50 units of cash, 1/4 tank of fuel.");
    BGame::GetPlayer()->m_dCash = 50.0;
    BGame::GetPlayer()->m_dFuel = 25.0;
    BGame::GetPlayer()->SaveStateFile();
  }
  if ((BGame::GetPlayer()->m_dCash > 0.0) &&
      (BGame::GetPlayer()->m_dFuel <= 0.01)) {
    // Assist player
    BGame::MyAfxMessageBox("Assisting player with 1/4 tank of fuel.");
    BGame::GetPlayer()->m_dCash -= 25.0;
    BGame::GetPlayer()->m_dFuel = 25.0;
    BGame::GetPlayer()->SaveStateFile();
  }

  // Set view for BGame
  BGame::SetView(this);

  // Read settings for graphics etc.
  Settings::ReadSettings(m_game.GetSimulation());
  switch (BGame::m_nTerrainResolution) {
  case 0:
    m_game.GetSimulation()->GetTerrain()->SetRenderResolution(
        BTerrain::RENDER_MINIMUM);
    break;
  case 1:
    m_game.GetSimulation()->GetTerrain()->SetRenderResolution(
        BTerrain::RENDER_LOW);
    break;
  case 2:
    m_game.GetSimulation()->GetTerrain()->SetRenderResolution(
        BTerrain::RENDER_MEDIUM);
    break;
  case 3:
    m_game.GetSimulation()->GetTerrain()->SetRenderResolution(
        BTerrain::RENDER_HIGH);
    break;
  case 4:
    m_game.GetSimulation()->GetTerrain()->SetRenderResolution(
        BTerrain::RENDER_MAXIMUM);
    break;
  case 5:
    m_game.GetSimulation()->GetTerrain()->SetRenderResolution(
        BTerrain::RENDER_SLOW_MACHINE);
    break;
  }
  BGame::m_bShowDust = (BGame::m_nDustAndClouds == 0);

  // Initialize common OpenGL features
  InitializeOpenGL();

  // Initialize multitexturing support
  OpenGLHelpers::Init();
  BTextures::Init();

  m_game.GetSimulation()->GetGround()->LoadTextures();
  m_game.GetSimulation()->GetGround()->PreProcessVisualization();
  m_game.GetSimulation()->GetTerrain()->PreProcessVisualization();
  m_game.GetSimulation()->PreProcessVisualization();
  BTerrain::GetGasStationObject()->PreProcessVisualizationGasStation();

  // Check for multiprosessor support
  // FIXME
  /*SYSTEM_INFO si;
  GetSystemInfo(&si);
  BGame::m_bMultiProcessor = si.dwNumberOfProcessors > 1;*/

  // Change cursor to Vera! cursor
  // FIXME
  /*m_hCursor = AfxGetApp()->LoadCursor(IDC_POINTER);
  if(m_hCursor) {
    ::SetClassLong(GetSafeHwnd(), GCL_HCURSOR, 0L);
    ::SetCursor(m_hCursor);
  }*/

  //::ReleaseDC(GetSafeHwnd(), hDC);

  BGame::MyAfxMessageBox("----------------------");
  BGame::MyAfxMessageBox("- OpenGL Info        -");
  BGame::MyAfxMessageBox("----------------------");
  string sInfo, sInfo2;
  sInfo2 = (char *)glGetString(GL_VENDOR);
  sInfo = "Vendor: " + sInfo2;
  BGame::MyAfxMessageBox(sInfo);
  sInfo2 = (char *)glGetString(GL_RENDERER);
  sInfo = "Renderer: " + sInfo2;
  BGame::MyAfxMessageBox(sInfo);
  sInfo2 = (char *)glGetString(GL_VERSION);
  sInfo = "Version: " + sInfo2;
  BGame::MyAfxMessageBox(sInfo);
  sInfo2 = (char *)glGetString(GL_EXTENSIONS);
  sInfo = "Extensions: " + sInfo2;
  BGame::MyAfxMessageBox(sInfo);
  BGame::MyAfxMessageBox("----------------------");

  // omat
  BGame::m_nDispWidth = window_width;
  BGame::m_nDispHeight = window_height;

  // return retval;
  return true;
}

bool CVeraView::OnEraseBkgnd() {
  // return TRUE;
  return true;
}

void CVeraView::InitializeOpenGL() {
  // glClearColor(225.0f/255.0f, 234.0f/255.0f, 1, 0); /* For RGB-mode */
  glClearColor(0, 0, 0, 0); /* For RGB-mode */

  glClearDepth(1);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  glShadeModel(GL_SMOOTH);
  glEnable(GL_LIGHTING);
  glEnable(GL_NORMALIZE);

  glEnable(GL_LIGHT0);
  OpenGLHelpers::SetDefaultLighting();

  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  switch (m_game.m_nTextureSmoothness) {
  case 0: // Boxy
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    break;
  case 1: // Gritty
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    break;
  case 2: // Silky
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    break;
  }

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // glClearColor(225.0f/255.0f, 234.0f/255.0f, 1, 0); /* For RGB-mode */
  glClearColor(0, 0, 0, 0);
  if (g_cbBlackAndWhite) {
    // glClearColor(0.7f, 0.7f, 0.7f, 0.0f);
    glClearColor(0, 0, 0, 0);
  }
  // glClearColor(225.0f/255.0f, 234.0f/255.0f, 1, 0); /* For RGB-mode */
  glClearColor(0, 0, 0, 0);

  GLfloat vFogColor[4];
  vFogColor[0] = 225.0f / 255.0f;
  vFogColor[1] = 234.0f / 255.0f;
  vFogColor[2] = 1.0;
  vFogColor[3] = 0.0;
  glEnable(GL_FOG);
  glFogi(GL_FOG_MODE, GL_LINEAR);
  glFogf(GL_FOG_START, 1000.0f);
  glFogf(GL_FOG_END, float(cdWorldHemisphereRadius));
  glFogfv(GL_FOG_COLOR, vFogColor);

  // Prepare for environment mapping
  // glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
  // glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);

  return;
}

int TerrainLoadThread(void *) {
  BGame::SetProgressPos(0);

  // Setup Terrain module for this scene
  BGame::GetSimulation()->GetTerrain()->StartUsingScene(
      BGame::GetSimulation()->GetScene()->m_sName,
      BGame::GetSimulation()->GetScene()->m_vOrigin,
      BGame::GetSimulation()->GetScene()->m_dGroundTextureScaler1,
      BGame::GetSimulation()->GetScene()->m_dGroundTextureScaler2);

  // EnterCriticalSection(&BGame::m_csMutex);
  SDL_LockMutex(BGame::m_csMutex);
  BGame::m_bGameReadyToStart = true;
  // LeaveCriticalSection(&BGame::m_csMutex);
  SDL_UnlockMutex(BGame::m_csMutex);
  // AfxEndThread(0);
  return 0;
}

void CVeraView::OnDraw() {
  if (m_pDrawFunction) {
    (this->*m_pDrawFunction)();
  }
}

void CVeraView::setExit() {
  if (m_pDrawFunction == &CVeraView::OnDrawGame) {
    BGame::GetPlayer()->m_dFuel = BGame::GetSimulation()->GetVehicle()->m_dFuel;
    BGame::GetPlayer()->m_dKerosine =
        BGame::GetSimulation()->GetVehicle()->m_dKerosine;
    BGame::GetPlayer()->SaveStateFile();
    BGame::GetPlayer()->SaveCurrentSceneInfo();
  }

  // Write settings for graphics etc.
  BGame::GetPlayer()->SaveStateFile();
  Settings::WriteSettings(m_game.GetSimulation());
  exit = true;
}

