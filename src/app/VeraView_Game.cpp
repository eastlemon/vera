// VeraView_Game.cpp - in-game frame: OnDrawGame + all Draw* HUD methods.

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
extern double g_dRate;
extern double g_dAveRate;
extern double g_d10LastFPS[];
extern double g_dExtraAlpha;
extern BVector g_vCenter;
extern int g_nVideoTexture;
extern bool g_bMouseButtonDown;
extern GLint g_nMatrixMode;

void CVeraView::OnDrawGame() {
  static int nFrameNo = 0;
  static unsigned clockLastCheckPoint = SDL_GetTicks();
  static unsigned clockLastCheckPoint2 = SDL_GetTicks();
  // static string sRate = _T("");
  static string sRate = "";

  BCamera *pCamera = m_game.GetSimulation()->GetCamera();
  pCamera->m_vSpeed = pCamera->m_vLocation - pCamera->m_vPrevLocation;
  pCamera->m_vPrevLocation = pCamera->m_vLocation;

  BGame::UpdateEarthquake();
  BGame::UpdateWindParticles();
  BGame::UpdateAnalyzer();

  if (m_bInitClock) {
    clockLastCheckPoint = SDL_GetTicks();
    m_bInitClock = false;
  }

  /*HDC hDC = pDC->GetSafeHdc();
  wglMakeCurrent(hDC, m_hGLRC);

  CRect rect;
  GetClientRect(&rect);*/

  // glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_FOG_BIT |
  // GL_DEPTH_BUFFER_BIT | GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT | GL_POLYGON_BIT
  // | GL_STENCIL_BUFFER_BIT | GL_TEXTURE_BIT);

  glDrawBuffer(GL_BACK);

  // Reset OpenGL
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glScaled(-1.0, 1.0, 1.0);

  double dScreenFormat = 1.0;
  if (m_game.m_nScreenFormat == 1) {
    dScreenFormat = 2.0 / 3.0;
  }
  double aspect = 1.0;
  // aspect = (double) rect.Width() / (double) (rect.Height() * dScreenFormat);
  aspect = (double)window_width / (double)(window_height * dScreenFormat);

  if (BGame::m_bJumpToHome) {
    double dJumpStep = 200.0;
    // Go towards home location. If close enough, return to normal simulation
    BVector vFrom = BGame::GetSimulation()->GetVehicle()->m_vLocation;
    BVector vTo = BGame::GetSimulation()->GetScene()->m_vStartLocation;
    if ((vFrom - vTo).Length() < dJumpStep) {
      // Return to normal simulation
      BGame::m_bJumpToHome = false;
      // BGame::GetSimulation()->GetVehicle()->Move(vTo - vFrom);
      BVector val_bvector(vTo - vFrom);
      BGame::GetSimulation()->GetVehicle()->Move(val_bvector);
      BGame::GetSimulation()->UpdateCar();
      BGame::GetSimulation()->EnsureVehicleIsOverGround();
      // BGame::GetSimulation()->GetCamera()->m_locMode =
      // BCamera::TCameraLoc::FOLLOW;
      BGame::GetSimulation()->GetCamera()->m_locMode = BCamera::FOLLOW;
      m_nMenuTime += BGame::ContinueSimulation();
    } else {
      // Jump towards home
      BVector vJump = vTo - vFrom;
      vJump.ToUnitLength();
      vJump = vJump * dJumpStep;
      // BGame::GetSimulation()->GetVehicle()->m_vLocation += vJump;
      BGame::GetSimulation()->GetVehicle()->Move(vJump);
      BGame::GetSimulation()->UpdateCar();
      vJump.ToUnitLength();
      BCamera *pCam = BGame::GetSimulation()->GetCamera();
      pCam->m_orientation.m_vForward = vJump;
      pCam->m_orientation.m_vUp = BVector(0, 0, -1);
      pCam->m_orientation.m_vRight = pCam->m_orientation.m_vUp.CrossProduct(
          pCam->m_orientation.m_vForward);
    }
  }

  if (!m_game.m_bFrozen) {
    // m_game.GetSimulation()->PrePaint(pDC);
    m_game.GetSimulation()->PrePaint();
  }

  static unsigned clockLastCrash = SDL_GetTicks();
  if (BGame::GetSimulation()->m_dMaxGForce > 1.0) {
    // if(BGame::GetState() == BGame::TState::DELIVERY) {
    if (BGame::GetState() == BGame::DELIVERY) {
      if ((double(SDL_GetTicks() - clockLastCrash) / 1000.0) > 1.0) {
        clockLastCrash = SDL_GetTicks();
        BMessages::Show(50, "pizzadamage", "Pizza damaged! profit penalty -10%",
                        3, false, 1, 0, 0);
        ++(BGame::m_nPizzaDamaged);
      }
    } else {
      BMessages::Show(40, "damage", "Relax, man!", 1, false, 1, 0, 0);
    }
  }

  // double dScaler = 10.0; // To get better z-buffer performance on a Geforce
  // MX2 card

  if (pCamera->m_locMode == BCamera::INCAR) {
    gluPerspective(pCamera->m_dAngleOfView, aspect, 1.0f,
                   float(cdWorldHemisphereRadius * 1.5));
  } else if (pCamera->m_locMode == BCamera::OVERVIEW) {
    gluPerspective(pCamera->m_dAngleOfView, aspect, 10.0f,
                   float(cdWorldHemisphereRadius * 1.5));
  } else if (pCamera->m_locMode == BCamera::ONSIDE) {
    gluPerspective(pCamera->m_dAngleOfView, aspect, 0.2f,
                   float(cdWorldHemisphereRadius * 1.5));
  } else {
    gluPerspective(pCamera->m_dAngleOfView, aspect, 1.0f,
                   float(cdWorldHemisphereRadius * 1.5));
  }

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Set look at -point for camera
  glViewport(0,
             //(GLint) ((rect.Height() - rect.Height() * dScreenFormat) / 2),
             (GLint)((window_height - window_height * dScreenFormat) / 2),
             //(GLint) rect.Width(),
             (GLint)window_width,
             //(GLint) (double(rect.Height()) * dScreenFormat));
             (GLint)(double(window_height) * dScreenFormat));

  static double dSortaClock = 0.0;
  // static clock_t clockPrev = clock();
  static unsigned clockPrev = SDL_GetTicks();
  // clock_t clockNow = clock();
  unsigned clockNow = SDL_GetTicks();
  dSortaClock += 0.001 * double(clockNow - clockPrev);
  clockPrev = clockNow;
  double dVert;
  double dHoriz;
  if (pCamera->m_locMode != BCamera::INCAR) {
    dVert = 0.125 * sin(dSortaClock) - 0.125 / 2.0;
    dHoriz = 0.125 * sin(dSortaClock / 3.0) - 0.125 / 2.0;
  } else {
    dVert = 0.0;
    dHoriz = 0.0;
  }

  if (!BGame::GetSceneEditor()->IsActive()) {
    // m_game.GetSimulation()->SetUpCamera(&rect);
    m_game.GetSimulation()->SetUpCamera();
  } else {
    BVector vVehicleTo =
        pCamera->m_vLocation + pCamera->m_orientation.m_vForward * 5.0;
    BVector vVehicleLoc = m_game.GetSimulation()->GetVehicle()->m_vLocation;
    // m_game.GetSimulation()->GetVehicle()->Move(vVehicleTo - vVehicleLoc);
    BVector val_bvector(vVehicleTo - vVehicleLoc);
    m_game.GetSimulation()->GetVehicle()->Move(val_bvector);
    m_game.GetSimulation()->UpdateCar();
    m_game.GetSimulation()->GetVehicle()->m_orientation =
        pCamera->m_orientation;

    // Ensure camera is above ground
    double dTmp;
    BVector vInternalOffset = BGame::GetSimulation()->GetTerrain()->m_vOffset;
    double dZ = -HeightMap::CalcHeightAt(
        vInternalOffset.m_dX + pCamera->m_vLocation.m_dX,
        vInternalOffset.m_dY + pCamera->m_vLocation.m_dY, dTmp,
        HeightMap::OVERALL);
    if (pCamera->m_vLocation.m_dZ > (dZ - 1.0)) {
      pCamera->m_vLocation.m_dZ = (dZ - 1.0);
    }
  }

  gluLookAt(pCamera->m_vLocation.m_dX + dHoriz, pCamera->m_vLocation.m_dY,
            pCamera->m_vLocation.m_dZ + dVert,
            pCamera->m_vLocation.m_dX + pCamera->m_orientation.m_vForward.m_dX +
                dHoriz,
            pCamera->m_vLocation.m_dY + pCamera->m_orientation.m_vForward.m_dY,
            pCamera->m_vLocation.m_dZ + pCamera->m_orientation.m_vForward.m_dZ +
                dVert,
            pCamera->m_orientation.m_vUp.m_dX,
            pCamera->m_orientation.m_vUp.m_dY,
            pCamera->m_orientation.m_vUp.m_dZ);

  GLfloat fLight1PositionG[4];
  fLight1PositionG[0] = (GLfloat)0.25;
  fLight1PositionG[1] = (GLfloat)0;
  fLight1PositionG[2] = (GLfloat)-1;
  fLight1PositionG[3] =
      (GLfloat)0; /* w=0 -> directional light (not positional) */
  glLightfv(GL_LIGHT0, GL_POSITION, fLight1PositionG);

  // Draw the world

  glEnable(GL_DEPTH_TEST);
  // m_nMenuTime += m_game.GetSimulation()->Paint(pDC, m_bCreateDLs,
  // m_bWireframe, m_bNormals, rect);
  m_nMenuTime +=
      m_game.GetSimulation()->Paint(m_bCreateDLs, m_bWireframe, m_bNormals);
  m_bCreateDLs = false;

  // Draw video on top of car
  if (m_bShowVideo) {
    DrawVideo();
  }

  // Scene editor stuff (active object highlight)
  if (m_game.GetSceneEditor()->IsActive()) {
    m_game.GetSceneEditor()->HighlightActiveObject();
  }

  // Find out the screen position for each tracking target
  if (BGame::m_bDrawOnScreenTracking) {
    BTrackingTarget *pTarget = m_game.GetSimulation()->m_targets;
    while (pTarget) {
      // UpdateTrackingTargetScreenPos(pTarget, m_rectWnd);
      UpdateTrackingTargetScreenPos(pTarget);
      pTarget = pTarget->m_pNext;
    }
  }

  // Update tracking of closest gas station
  BGame::GetSimulation()->UpdateGasStationTracking();

  // Check if we need to start fueling
  CheckForFueling();

  // Check if we need to start pickup or delivery
  // if(BGame::GetState() == BGame::TState::PICKUP) {
  if (BGame::GetState() == BGame::PICKUP) {
    CheckForPickup();
    //} else if(BGame::GetState() == BGame::TState::DELIVERY) {
  } else if (BGame::GetState() == BGame::DELIVERY) {
    CheckForDelivery();
    // Update pizza temp
    unsigned clockNow = SDL_GetTicks();
    double dElapsed =
        double(clockNow - BGame::m_clockLastPizzaTempCheck) / 1000.0 * 0.21;
    double dScaler = (BGame::m_dPizzaTemp - 26.2) / (60.0 - 26.2);
    BGame::m_clockLastPizzaTempCheck = clockNow;
    BGame::m_dPizzaTemp -= (dElapsed * dScaler);
  }

  // Draw 2D graphics
  // if(BGame::m_nVisualize & BGame::TAnalyzerVis::GASSTATIONS) {
  if (BGame::m_nVisualize & BGame::GASSTATIONS) {
    Setup2DRendering();

    // If under water, draw faint water veil
    if (m_game.GetSimulation()->GetCamera()->m_vLocation.m_dZ > 0.0) {
      glDisable(GL_TEXTURE_2D);
      OpenGLHelpers::SetColorFull(0.25, 0.35, 0.5, 0.8);
      glBegin(GL_TRIANGLE_STRIP);
      glVertex3f(0, 0, 0);
      // glVertex3f(0, m_rectWnd.Height(), 0);
      glVertex3f(0, window_height, 0);
      // glVertex3f(m_rectWnd.Width(), 0, 0);
      glVertex3f(window_width, 0, 0);
      // glVertex3f(m_rectWnd.Width(), m_rectWnd.Height(), 0);
      glVertex3f(window_width, window_height, 0);
      glEnd();
    }

    // Draw tracking targets on screen.
    if (BGame::m_bDrawOnScreenTracking) {
      BTrackingTarget *pTarget = m_game.GetSimulation()->m_targets;
      while (pTarget) {
        // DrawTrackingTargetOnScreen(pTarget, m_rectWnd);
        DrawTrackingTargetOnScreen(pTarget);
        pTarget = pTarget->m_pNext;
      }
    }

    if ((BGame::m_bShowDust) && (g_dExtraAlpha > 0.0)) {
      // If inside cloud, draw veil
      OpenGLHelpers::SetColorFull(1, 1, 1, 0.7 * g_dExtraAlpha);
      glDisable(GL_TEXTURE_2D);

      glBegin(GL_TRIANGLE_STRIP);
      glVertex3f(0, 0, 0);
      // glVertex3f(0, rect.Height(), 0);
      glVertex3f(0, window_height, 0);
      // glVertex3f(rect.Width(), 0, 0);
      glVertex3f(window_width, 0, 0);
      // glVertex3f(rect.Width(), rect.Height(), 0);
      glVertex3f(window_width, window_height, 0);
      glEnd();
    }

    // Draw panels if they're on
    if (BGame::m_bDashboard) {
      // DrawDashboard(pDC);
      DrawDashboard();
    }
    if (BGame::m_bService) {
      // DrawServiceWnd(pDC);
      DrawServiceWnd();
    }
    if (BGame::m_bNavSat) {
      // DrawNavSat(pDC);
      DrawNavSat();
      if (!BGame::m_bMultiProcessor) {
        // Sleep(1);
        SDL_Delay(1);
      }
    }

    // Scene editor stuff
    if (m_game.GetSceneEditor()->IsActive()) {
      // m_game.GetSceneEditor()->Draw(m_rectWnd);
      SDL_Rect rectWnd;
      rectWnd.w = window_width;
      rectWnd.h = window_height;
      m_game.GetSceneEditor()->Draw(rectWnd);
    }

    // If something requires attention, draw a faint veil
    if (m_game.m_bDeliveryStartInProgress || m_game.m_bPickupStartInProgress ||
        m_game.m_bShowGameMenu || m_game.m_bShowQuickHelp ||
        m_game.m_bShowCancelQuestion || m_game.m_bFueling) {
      // OpenGLHelpers::DrawVeil(0.6, 0.6, 1, 0.5, m_rectWnd);
      SDL_Rect rectWnd;
      rectWnd.w = window_width;
      rectWnd.h = window_height;
      OpenGLHelpers::DrawVeil(0.6, 0.6, 1, 0.5, rectWnd);
    }

    if (BGame::m_bPickupStartInProgress) {
      // DrawPickupInfo(pDC, rect);
      DrawPickupInfo();
      if (((SDL_GetTicks() - BGame::m_clockPickupStart) / 1000.0) > 5) {
        m_nMenuTime += BGame::ContinueSimulation();
        BGame::m_bPickupStartInProgress = false;
        BMessages::Show(50, "deliver", "DELIVER PIZZA TO CLIENT", 8, true);
        BGame::m_nPizzaDamaged = 0;
      }
    }

    if (BGame::m_bDeliveryStartInProgress) {
      // DrawDeliveryInfo(pDC, rect);
      DrawDeliveryInfo();
      if (((SDL_GetTicks() - BGame::m_clockDeliveryStart) / 1000.0) > 5) {
        m_nMenuTime += BGame::ContinueSimulation();
        BGame::m_bDeliveryStartInProgress = false;
        BMessages::Show(50, "pickup", "RETURN TO BASE", 8, true);
      }
    }

    // If fueling, draw gas station buttons
    if (BGame::m_bFueling) {
      // DrawFuelingButtons(pDC, rect);
      DrawFuelingButtons();
    }

    if (BGame::m_bShowQuickHelp) {
      // DrawQuickHelp(pDC);
      DrawQuickHelp();
    }

    if (m_game.m_bShowCancelQuestion) {
      // Cancel order question
      glEnable(GL_TEXTURE_2D);
      OpenGLHelpers::SwitchToTexture(0);
      // BTextures::Use(BTextures::Texture::PANEL);
      BTextures::Use(BTextures::PANEL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

      glPushMatrix();
      // glTranslated(m_rectWnd.Width() / 2.0 - 235.0 / 2.0, m_rectWnd.Height()
      // / 2.0 - 50, 0);
      glTranslated(window_width / 2.0 - 235.0 / 2.0, window_height / 2.0 - 50,
                   0);

      OpenGLHelpers::SetColorFull(1, 0.25, 0.25, 1);

      glBegin(GL_TRIANGLE_STRIP);
      OpenGLHelpers::SetTexCoord(138.0 / 512.0, (512.0 - 259.0) / 512.0);
      glVertex3f(0, 0, 0);
      OpenGLHelpers::SetTexCoord(138.0 / 512.0, (512.0 - 149.0) / 512.0);
      glVertex3f(0, 110.0, 0);
      OpenGLHelpers::SetTexCoord(373.0 / 512.0, (512.0 - 259.0) / 512.0);
      glVertex3f(235.0, 0, 0);
      OpenGLHelpers::SetTexCoord(373.0 / 512.0, (512.0 - 149.0) / 512.0);
      glVertex3f(235.0, 110.0, 0);
      glEnd();

      glTranslated(70.0, -50.0, 0);

      // Draw Yeah and Nope (highlight selected)
      if (m_game.m_nYesNoSelection == 1) {
        OpenGLHelpers::SetColorFull(0, 0.5, 0, 1);
      } else {
        OpenGLHelpers::SetColorFull(0, 0.5, 0, 0.25);
      }

      glBegin(GL_TRIANGLE_STRIP);
      OpenGLHelpers::SetTexCoord(166.0 / 512.0, (512.0 - 308.0) / 512.0);
      glVertex3f(0 - 40.0, 0, 0);
      OpenGLHelpers::SetTexCoord(166.0 / 512.0, (512.0 - 271.0) / 512.0);
      glVertex3f(0 - 40.0, 37.0, 0);
      OpenGLHelpers::SetTexCoord(246.0 / 512.0, (512.0 - 308.0) / 512.0);
      glVertex3f(80.0 - 40.0, 0, 0);
      OpenGLHelpers::SetTexCoord(246.0 / 512.0, (512.0 - 271.0) / 512.0);
      glVertex3f(80.0 - 40.0, 37.0, 0);
      glEnd();

      if (m_game.m_nYesNoSelection == 2) {
        OpenGLHelpers::SetColorFull(1, 0.25, 0.25, 1);
      } else {
        OpenGLHelpers::SetColorFull(1, 0.25, 0.25, 0.25);
      }

      glBegin(GL_TRIANGLE_STRIP);
      OpenGLHelpers::SetTexCoord(252.0 / 512.0, (512.0 - 315.0) / 512.0);
      glVertex3f(0 + 40.0, -7, 0);
      OpenGLHelpers::SetTexCoord(252.0 / 512.0, (512.0 - 271.0) / 512.0);
      glVertex3f(0 + 40.0, 37.0, 0);
      OpenGLHelpers::SetTexCoord(338.0 / 512.0, (512.0 - 315.0) / 512.0);
      glVertex3f(86.0 + 40.0, -7, 0);
      OpenGLHelpers::SetTexCoord(338.0 / 512.0, (512.0 - 271.0) / 512.0);
      glVertex3f(86.0 + 40.0, 37.0, 0);
      glEnd();

      glPopMatrix();
    }

    if (m_game.m_bShowGameMenu) {
      // Main menu
      glEnable(GL_TEXTURE_2D);
      OpenGLHelpers::SwitchToTexture(0);
      // BTextures::Use(BTextures::Texture::MAIN_GAME_MENU);
      BTextures::Use(BTextures::MAIN_GAME_MENU);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

      glPushMatrix();
      // glTranslated(m_rectWnd.Width() / 2.0 - 128, m_rectWnd.Height() / 2.0 -
      // 128, 0);
      glTranslated(window_width / 2.0 - 128, window_height / 2.0 - 128, 0);

      double dHeight = 256.0 / 5.0;

      if (BGame::m_nGameMenuSelection == 1) {
        OpenGLHelpers::SetColorFull(1, 1, 1, 1);
      } else {
        OpenGLHelpers::SetColorFull(1, 1, 1, 0.25);
      }

      glBegin(GL_TRIANGLE_STRIP);
      OpenGLHelpers::SetTexCoord(0, 0);
      glVertex3f(0, 0, 0);
      OpenGLHelpers::SetTexCoord(0, 0.2);
      glVertex3f(0, dHeight, 0);
      OpenGLHelpers::SetTexCoord(1, 0);
      glVertex3f(256, 0, 0);
      OpenGLHelpers::SetTexCoord(1, 0.2);
      glVertex3f(256, dHeight, 0);
      glEnd();

      if (BGame::m_nGameMenuSelection == 2) {
        OpenGLHelpers::SetColorFull(1, 1, 1, 1);
      } else {
        OpenGLHelpers::SetColorFull(1, 1, 1, 0.25);
      }

      glBegin(GL_TRIANGLE_STRIP);
      OpenGLHelpers::SetTexCoord(0, 0.2);
      glVertex3f(0, dHeight, 0);
      OpenGLHelpers::SetTexCoord(0, 0.4);
      glVertex3f(0, dHeight * 2.0, 0);
      OpenGLHelpers::SetTexCoord(1, 0.2);
      glVertex3f(256, dHeight, 0);
      OpenGLHelpers::SetTexCoord(1, 0.4);
      glVertex3f(256, dHeight * 2.0, 0);
      glEnd();

      if (BGame::m_nGameMenuSelection == 3) {
        OpenGLHelpers::SetColorFull(1, 1, 1, 1);
      } else {
        OpenGLHelpers::SetColorFull(1, 1, 1, 0.25);
      }

      glBegin(GL_TRIANGLE_STRIP);
      OpenGLHelpers::SetTexCoord(0, 0.4);
      glVertex3f(0, dHeight * 2.0, 0);
      OpenGLHelpers::SetTexCoord(0, 0.6);
      glVertex3f(0, dHeight * 3.0, 0);
      OpenGLHelpers::SetTexCoord(1, 0.4);
      glVertex3f(256, dHeight * 2.0, 0);
      OpenGLHelpers::SetTexCoord(1, 0.6);
      glVertex3f(256, dHeight * 3.0, 0);
      glEnd();

      if (BGame::m_nGameMenuSelection == 4) {
        OpenGLHelpers::SetColorFull(1, 1, 1, 1);
      } else {
        OpenGLHelpers::SetColorFull(1, 1, 1, 0.25);
      }

      glBegin(GL_TRIANGLE_STRIP);
      OpenGLHelpers::SetTexCoord(0, 0.6);
      glVertex3f(0, dHeight * 3.0, 0);
      OpenGLHelpers::SetTexCoord(0, 0.8);
      glVertex3f(0, dHeight * 4.0, 0);
      OpenGLHelpers::SetTexCoord(1, 0.6);
      glVertex3f(256, dHeight * 3.0, 0);
      OpenGLHelpers::SetTexCoord(1, 0.8);
      glVertex3f(256, dHeight * 4.0, 0);
      glEnd();

      if (BGame::m_nGameMenuSelection == 5) {
        OpenGLHelpers::SetColorFull(1, 1, 1, 1);
      } else {
        OpenGLHelpers::SetColorFull(1, 1, 1, 0.25);
      }

      glBegin(GL_TRIANGLE_STRIP);
      OpenGLHelpers::SetTexCoord(0, 0.8);
      glVertex3f(0, dHeight * 4.0, 0);
      OpenGLHelpers::SetTexCoord(0, 1.0);
      glVertex3f(0, dHeight * 5.0, 0);
      OpenGLHelpers::SetTexCoord(1, 0.8);
      glVertex3f(256, dHeight * 4.0, 0);
      OpenGLHelpers::SetTexCoord(1, 1.0);
      glVertex3f(256, dHeight * 5.0, 0);
      glEnd();

      glPopMatrix();
    }

    // Draw hint text if it's on
    if (m_game.m_bShowHint) {
      unsigned clockNow = SDL_GetTicks();
      if ((clockNow - m_game.m_clockHintStart) > 2000) {
        m_game.m_bShowHint = false;
      } else {
        double dAlpha =
            1.0 - double(clockNow - m_game.m_clockHintStart) / 2000.0;

        // OpenGLHelpers::SetColorFull(0.3, 0.6, 0.8, dAlpha);
        OpenGLHelpers::SetColorFull(0.9, 0.2, 0.1, dAlpha);
        OpenGLHelpers::SwitchToTexture(0);
        BTextures::Use(BTextures::PANEL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glPushMatrix();
        // glTranslated(m_rectWnd.Width() / 2.0, m_rectWnd.Height() / 2.0, 0);
        glTranslated(window_width / 2.0, window_height / 2.0, 0);

        glBegin(GL_TRIANGLE_STRIP);
        OpenGLHelpers::SetTexCoord(0, 0);
        glVertex3f(-321.0 / 2.0, -57.0 / 2.0, 0);
        OpenGLHelpers::SetTexCoord(0, (512.0 - 455.0) / 512.0);
        glVertex3f(-321.0 / 2.0, 57.0 / 2.0, 0);
        OpenGLHelpers::SetTexCoord(321.0 / 512.0, 0);
        glVertex3f(321.0 / 2.0, -57.0 / 2.0, 0);
        OpenGLHelpers::SetTexCoord(321.0 / 512.0, (512.0 - 455.0) / 512.0);
        glVertex3f(321.0 / 2.0, 57.0 / 2.0, 0);
        glEnd();

        glPopMatrix();
      }
    }

    if (!m_game.m_bShowGameMenu && !m_game.m_bShowQuickHelp &&
        !m_game.m_bShowCancelQuestion) {
      glPushMatrix();
      // glTranslated(m_rectWnd.Width() / 2.0, m_rectWnd.Height() * 0.35, 0);
      glTranslated(window_width / 2.0, window_height * 0.35, 0);
      m_messages.Render();
      glPopMatrix();
    }

    // Draw mouse cursor, if needed
    if (((BGame::m_bService) || (BGame::m_bNavSat)) &&
        !(m_game.m_bShowGameMenu || m_game.m_bShowQuickHelp ||
          m_game.m_bShowCancelQuestion)) {
      // DrawMouseCursor(m_rectWnd);
      DrawMouseCursor();
    }

    // On top of everything, draw fade in if we are in the first second
    if (m_game.m_bFadingIn) {
      double dFade = double(SDL_GetTicks() - m_game.m_clockFadeStart) / 1000.0;
      if (dFade > 1.0) {
        m_game.m_bFadingIn = false;
        BMessages::Show(50, "start game", "GO TO BASE FOR FIRST PICKUP", 5,
                        true);
      } else {
        glDisable(GL_TEXTURE_2D);
        OpenGLHelpers::SetColorFull(0, 0, 0, 1.0 - dFade);
        glBegin(GL_TRIANGLE_STRIP);
        glVertex3f(0, 0, 0);
        // glVertex3f(0, m_rectWnd.Height(), 0);
        glVertex3f(0, window_height, 0);
        // glVertex3f(m_rectWnd.Width(), 0, 0);
        glVertex3f(window_width, 0, 0);
        // glVertex3f(m_rectWnd.Width(), m_rectWnd.Height(), 0);
        glVertex3f(window_width, window_height, 0);
        glEnd();
      }
    }

    End2DRendering();

    // Calculate framerates
    clockNow = SDL_GetTicks();
    g_d10LastFPS[nFrameNo] = double(clockNow - clockLastCheckPoint2) / 1000.0;
    if (g_d10LastFPS[nFrameNo] < 0.00001) {
      g_d10LastFPS[nFrameNo] = 999.9;
    } else {
      g_d10LastFPS[nFrameNo] = 1.0 / g_d10LastFPS[nFrameNo];
    }
    clockLastCheckPoint2 = clockNow;

    if (++nFrameNo == 10) {
      g_dRate = 10.0 / (double(clockNow - (clockLastCheckPoint + m_nMenuTime)) /
                        1000.0);
      g_dAveRate += g_dRate;
      if (m_nMenuTime) {
        m_nMenuTime = 0;
      }

      m_game.GetSimulation()->GetVehicle()->m_dSpeedKmh =
          m_game.GetSimulation()->GetVehicle()->m_dSpeed * g_dRate *
          m_game.GetSimulation()->m_nPhysicsStepsBetweenRender * 3.6;

      /*sRate.Format("FPS:%.1lf, Speed:%.0lf km/h (%.2lf), SimSteps:%d,
         CamLoc:(%.1lf, %.1lf, %.1lf)", g_dRate,
                   m_game.GetSimulation()->GetVehicle()->m_dSpeedKmh,
                   m_game.GetSimulation()->GetVehicle()->m_dSpeed,
                   m_game.GetSimulation()->m_nPhysicsStepsBetweenRender,
                   m_game.GetSimulation()->GetCamera()->m_vLocation.m_dX,
                   m_game.GetSimulation()->GetCamera()->m_vLocation.m_dY,
                   m_game.GetSimulation()->GetCamera()->m_vLocation.m_dZ);*/
      stringstream format;
      format << "FPS:" << g_dRate
             << ", Speed:" << m_game.GetSimulation()->GetVehicle()->m_dSpeedKmh
             << " km/h (" << m_game.GetSimulation()->GetVehicle()->m_dSpeed
             << "), SimSteps:"
             << m_game.GetSimulation()->m_nPhysicsStepsBetweenRender
             << ", CamLoc:("
             << m_game.GetSimulation()->GetCamera()->m_vLocation.m_dX << ", "
             << m_game.GetSimulation()->GetCamera()->m_vLocation.m_dY << ", "
             << m_game.GetSimulation()->GetCamera()->m_vLocation.m_dZ << ")";
      sRate = format.str();

      // Calculate the absolute time sync ratio (ATSR)
      if (!m_game.m_bFrozen) {
        m_game.GetSimulation()->m_dMaxGForce = 0.0;
        static int nSkipAmount = 1;
        static int nSkipper = 0;
        if (++nSkipper >= nSkipAmount) {
          if (nSkipAmount < 1) {
            ++nSkipAmount;
          }
          nSkipper = 0;
          g_dAveRate /= double(nSkipAmount);
          m_game.GetSimulation()->m_nPhysicsStepsBetweenRender =
              int(g_dPhysicsStepsInSecond / g_dAveRate);
          double dFraction = (g_dPhysicsStepsInSecond / g_dRate) -
                             double(int(g_dPhysicsStepsInSecond / g_dRate));
          m_game.GetSimulation()->m_dPhysicsFraction = dFraction;
          m_game.GetSimulation()->m_bCalibrateSimulationSpeed = false;
          g_dAveRate = 0;
        }
      }

      nFrameNo = 0;
      clockLastCheckPoint = clockNow;
    }

    if (BGame::m_bFrozen) {
      GLint nDepth;
      glGetIntegerv(GL_MODELVIEW_STACK_DEPTH, &nDepth);

      /*sRate.Format("FPS:%.1lf, CamLoc:(%.1lf, %.1lf, %.1lf), STACK: %d",
                   g_dRate,
                   m_game.GetSimulation()->GetCamera()->m_vLocation.m_dX,
                   m_game.GetSimulation()->GetCamera()->m_vLocation.m_dY,
                   m_game.GetSimulation()->GetCamera()->m_vLocation.m_dZ,
                   nDepth);*/
      stringstream format;
      format << "FPS:" << g_dRate << ", CamLoc:("
             << m_game.GetSimulation()->GetCamera()->m_vLocation.m_dX << ", "
             << m_game.GetSimulation()->GetCamera()->m_vLocation.m_dY << ", "
             << m_game.GetSimulation()->GetCamera()->m_vLocation.m_dZ
             << "), STACK: " << nDepth;
      sRate = format.str();
    }

    // Override realistic physics time when in slow motion
    if (BGame::m_bSlowMotion) {
      m_game.GetSimulation()->m_nPhysicsStepsBetweenRender = 1;
    }

    if (m_game.m_bDisplayInfo) {
      BUI::TextRenderer()->StartRenderingText();
      // BUI::TextRenderer()->DrawSmallTextAt(58, rect.Height() - 20, sRate,
      // sRate.GetLength(), BTextRenderer::TTextAlign::ALIGN_LEFT, 0, 0, 0, 1);
      BUI::TextRenderer()->DrawSmallTextAt(
          58, window_height - 20, sRate, sRate.length(),
          BTextRenderer::ALIGN_LEFT, 0, 0, 0, 1);
      string sText = "BETA VERSION! (build 6)";
      // BUI::TextRenderer()->DrawSmallTextAt(rect.Width() / 2, rect.Height() -
      // 40, sText, sText.GetLength(), BTextRenderer::TTextAlign::ALIGN_CENTER,
      // 0.75, 0, 0, 1);
      BUI::TextRenderer()->DrawSmallTextAt(
          window_width / 2, window_height - 40, sText, sText.length(),
          BTextRenderer::ALIGN_CENTER, 0.75, 0, 0, 1);
      BUI::TextRenderer()->StopRenderingText();
    }
  }

  drawDialogs();

  SDL_GL_SwapWindow(window);

  //::ReleaseDC(GetSafeHwnd(), hDC);

  // glPopAttrib();

  if (!m_game.GetSimulation()->m_bPaused) {
    // Invalidate();
  }
}

void CVeraView::DrawVideo() {
  static int nFrame = 0;
  // Setup 256x256 texture
  OpenGLHelpers::SetColorFull(0, 1, 0.5, 1);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);

  glActiveTextureARB(GL_TEXTURE0_ARB);
  glEnable(GL_TEXTURE_2D);

  // OpenGLHelpers::SwitchToTexture(0);
  BTextures::Use(g_nVideoTexture);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // GrabAVIFrame(nFrame);
  // nFrame = (nFrame + 1) % g_nFramesInVideo; //FIXME

  // Draw quad on top of car
  BVector vCar =
      g_vCenter + BGame::GetSimulation()->GetVehicle()->m_orientation.m_vUp;

  BVector v1 =
      BGame::GetSimulation()->GetVehicle()->m_orientation.m_vRight * -1.5;
  BVector v2 =
      BGame::GetSimulation()->GetVehicle()->m_orientation.m_vRight * -1.5 +
      BGame::GetSimulation()->GetVehicle()->m_orientation.m_vUp * 2.0;
  BVector v3 =
      BGame::GetSimulation()->GetVehicle()->m_orientation.m_vRight * 1.5 +
      BGame::GetSimulation()->GetVehicle()->m_orientation.m_vUp * 2.0;
  BVector v4 =
      BGame::GetSimulation()->GetVehicle()->m_orientation.m_vRight * 1.5;

  glPushMatrix();
  glTranslatef(vCar.m_dX, vCar.m_dY, vCar.m_dZ);

  glBegin(GL_QUAD_STRIP);
  for (int i = 0; i < 100; ++i) {
    if (i % 2) {
      OpenGLHelpers::SetColorFull(1, 1, 1, 1);
    } else {
      OpenGLHelpers::SetColorFull(0.5, 0.5, 0.5, 1);
    }
    OpenGLHelpers::SetTexCoord(0, double(i) * 0.01);
    BVector vNew1 =
        v1 + BGame::GetSimulation()->GetVehicle()->m_orientation.m_vUp * 2.0 *
                 (double(i) * 0.01);
    glVertex3f(vNew1.m_dX, vNew1.m_dY, vNew1.m_dZ);
    OpenGLHelpers::SetTexCoord(1, double(i) * 0.01);
    vNew1 = v4 + BGame::GetSimulation()->GetVehicle()->m_orientation.m_vUp *
                     2.0 * (double(i) * 0.01);
    glVertex3f(vNew1.m_dX, vNew1.m_dY, vNew1.m_dZ);
  }
  glEnd();
  glPopMatrix();

  /*
  OpenGLHelpers::SetTexCoord(0, 0);
  glVertex3f(v1.m_dX, v1.m_dY, v1.m_dZ);
  OpenGLHelpers::SetTexCoord(0, 1);
  glVertex3f(v2.m_dX, v2.m_dY, v2.m_dZ);
  OpenGLHelpers::SetTexCoord(1, 1);
  glVertex3f(v3.m_dX, v3.m_dY, v3.m_dZ);
  OpenGLHelpers::SetTexCoord(1, 0);
  glVertex3f(v4.m_dX, v4.m_dY, v4.m_dZ);
  */

  // cleanup
  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);
}

void CVeraView::CheckForFueling() {
  // Check if we need to start fueling
  BVehicle *pVehicle = BGame::GetSimulation()->GetVehicle();
  BVector vFuel = pVehicle->ToWorldCoord(pVehicle->m_vFuelLocation);
  if (((vFuel - BGame::m_vGasStationClosest).Length() <
       pVehicle->m_dFuelDistance) &&
      !m_game.m_bFueling && !BGame::m_bShowCancelQuestion &&
      !BGame::m_bShowGameMenu && !BGame::m_bShowQuickHelp) {
    if ((SDL_GetTicks() - BGame::m_clockLastFuelExit) > 3000) {
      // Start fueling
      BGame::FreezeSimulation();
      m_pKeyDownFunction = &CVeraView::OnKeyDownFueling;
      m_game.m_bFueling = true;
    }
  }
}

void CVeraView::CheckForPickup() {
  // Check if we need to pickup
  BVehicle *pVehicle = BGame::GetSimulation()->GetVehicle();
  BVector v = pVehicle->ToWorldCoord(pVehicle->m_vFuelLocation);

  BObject *pBase = BGame::GetBase();
  if (pBase && (((v - pBase->m_vCenter).Length() < pBase->m_dActiveRadius) &&
                !BGame::m_bShowCancelQuestion && !BGame::m_bShowGameMenu &&
                !BGame::m_bShowQuickHelp)) {

    // When in base, always fix rotors (for 10$ a part)
    BVehicle *pVehicle = BGame::GetSimulation()->GetVehicle();
    if (pVehicle->m_bHasRotor) {
      if (!(pVehicle->m_rotor.m_bHeliOK)) {
        pVehicle->m_rotor.m_bHeliOK = true;
        BVehicle *pVehicle = BGame::GetSimulation()->GetVehicle();
        pVehicle->m_rotor.m_dHeliBladePower = 1.0;
        double dCost = 0.0;
        if (!(pVehicle->m_rotor.m_bHeliCoverOK)) {
          pVehicle->m_rotor.m_bHeliCoverOK = true;
          dCost += 10.0;
        }
        for (int i = 0; i < 3; ++i) {
          if (!(pVehicle->m_rotor.m_bHeliBladeOK[i])) {
            pVehicle->m_rotor.m_bHeliBladeOK[i] = true;
            dCost += 10.0;
          }
        }
        BGame::GetPlayer()->m_dCash -= dCost;
        string sTmp;
        // sTmp.Format("Rotor fixed. Cost %.0lf$", dCost);
        stringstream format;
        format << "Rotor fixed. Cost " << (int)dCost << "$";
        sTmp = format.str();
        BMessages::Show(40, "rotorfixed", sTmp, 3, false, 0.2, 0.5, 0.2);
      }
    }

    // Start pickup
    BGame::m_nPizzaDamaged = 0.0;
    BMessages::Remove("start game");
    BGame::FreezeSimulation();
    BGame::m_bPickupStartInProgress = true;
    BGame::m_clockPickupStart = SDL_GetTicks();
    // BGame::SetState(BGame::TState::DELIVERY);
    BGame::SetState(BGame::DELIVERY);
    BGame::m_dPizzaTemp = 60.0 + Random(5.0);
    BGame::m_nPizza = rand() % BGame::m_nPizzas;

    // Select client
    bool bFound = false;
    BScene *pScene = BGame::GetSimulation()->GetScene();
    if (!pScene->m_bRandomDeliveryInUse) {
      if (pScene->m_nCurrentDeliveryEntry < pScene->m_nDeliveryOrderEntries) {
        if ((pScene->m_nDeliveryOrder[pScene->m_nCurrentDeliveryEntry] <
             pScene->m_nObjects) &&
            // pScene->m_pObjects[pScene->m_nDeliveryOrder[pScene->m_nCurrentDeliveryEntry]].m_type
            // == BObject::TType::CLIENT) {
            pScene->m_pObjects
                    [pScene->m_nDeliveryOrder[pScene->m_nCurrentDeliveryEntry]]
                        .m_type == BObject::CLIENT) {
          bFound = true;
          BGame::SetCurrentClient(
              &pScene->m_pObjects
                   [pScene->m_nDeliveryOrder[pScene->m_nCurrentDeliveryEntry]]);
          ++(pScene->m_nCurrentDeliveryEntry);
        }
      }
    }
    if (!bFound) {
      pScene->m_bRandomDeliveryInUse = true;
      // Select random client

      // First count clients
      int nClients = 0;
      int nCandidate = 0;
      while (nCandidate < pScene->m_nObjects) {
        // if(pScene->m_pObjects[nCandidate].m_type == BObject::TType::CLIENT) {
        if (pScene->m_pObjects[nCandidate].m_type == BObject::CLIENT) {
          ++nClients;
        }
        ++nCandidate;
      }

      if (nClients) {
        int nClient = rand() % nClients;
        int nCandidate = 0;
        int nClientCandidate = 0;
        while (nCandidate < pScene->m_nObjects) {
          // if(pScene->m_pObjects[nCandidate].m_type == BObject::TType::CLIENT)
          // {
          if (pScene->m_pObjects[nCandidate].m_type == BObject::CLIENT) {
            if (nClientCandidate == nClient) {
              bFound = true;
              BGame::SetCurrentClient(&pScene->m_pObjects[nCandidate]);
              break;
            }
            ++nClientCandidate;
          }
          ++nCandidate;
        }
      }
    }

    if (BGame::GetCurrentClient()) {
      BGame::m_dLastDeliveryDistance =
          (BGame::GetCurrentClient()->m_vCenterOnGround -
           BGame::GetSimulation()->GetVehicle()->m_vLocation)
              .Length();
    } else {
      BGame::m_dLastDeliveryDistance = 800.0;
    }

    BGame::m_clockLastPizzaTempCheck = SDL_GetTicks();
  }
}

void CVeraView::CheckForDelivery() {
  // Check if we need to deliver
  BVehicle *pVehicle = BGame::GetSimulation()->GetVehicle();
  BVector v = pVehicle->ToWorldCoord(pVehicle->m_vFuelLocation);

  BObject *pClient = BGame::GetCurrentClient();
  if (pClient &&
      (((v - pClient->m_vCenter).Length() < pClient->m_dActiveRadius) &&
       !BGame::m_bShowCancelQuestion && !BGame::m_bShowGameMenu &&
       !BGame::m_bShowQuickHelp)) {
    // Start delivery
    BGame::FreezeSimulation();
    BGame::m_bDeliveryStartInProgress = true;
    BGame::m_clockDeliveryStart = SDL_GetTicks();
    // BGame::SetState(BGame::TState::PICKUP);
    BGame::SetState(BGame::PICKUP);
    BGame::GetSimulation()->RemoveTrackingTarget("CLIENT");

    // Calculate profit and add that to player's pocket
    BGame::m_dLastProfit = BGame::GetMaxProfitFor(
        BGame::m_dLastDeliveryDistance, BGame::m_dPizzaTemp);
    BGame::GetPlayer()->m_dCash += BGame::m_dLastProfit;

    BGame::m_dPizzaTemp = 0.0;
  }
}

void CVeraView::Setup2DRendering() {
  // Setup 2D projection and draw dashboard.

  glGetIntegerv(GL_MATRIX_MODE, &g_nMatrixMode);

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


  // Set up projection geometry so that we can use screen coordinates
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  // gluOrtho2D(0, (GLfloat) m_rectWnd.Width(), 0, (GLfloat)
  // m_rectWnd.Height());
  gluOrtho2D(0, (GLfloat)window_width, 0, (GLfloat)window_height);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  OpenGLHelpers::SetColorFull(1, 1, 1, 1);

  GLfloat fLight1AmbientG[4];
  fLight1AmbientG[0] = 1;
  fLight1AmbientG[1] = 1;
  fLight1AmbientG[2] = 1;
  fLight1AmbientG[3] = 1;
  glLightfv(GL_LIGHT0, GL_AMBIENT, fLight1AmbientG);
}

void CVeraView::End2DRendering() {
  glEnable(GL_CULL_FACE);

  GLfloat fLight1AmbientG[4];
  fLight1AmbientG[0] = 0.4f;
  fLight1AmbientG[1] = 0.4f;
  fLight1AmbientG[2] = 0.4f;
  fLight1AmbientG[3] = 0.0f;
  glLightfv(GL_LIGHT0, GL_AMBIENT, fLight1AmbientG);

  glMatrixMode(g_nMatrixMode);
}

void CVeraView::DrawFuelingButtons() {

  BVehicle *pVehicle = BGame::GetSimulation()->GetVehicle();

  // Check for fuel intake amount
  if (BGame::m_bFuelingInProgress) {
    unsigned clockNow = SDL_GetTicks();
    double dLitresInserted =
        double(clockNow - BGame::m_clockFuelingStarted) / 1000.0 * 5.0;
    BGame::m_clockFuelingStarted = clockNow;
    double dCostOfFuel = dLitresInserted;

    if ((BGame::GetPlayer()->m_dCash - dCostOfFuel) < 0.0) {
      if (BGame::GetPlayer()->m_dCash > 0.0) {
        dCostOfFuel = BGame::GetPlayer()->m_dCash;
        dLitresInserted = dCostOfFuel;
      } else {
        dCostOfFuel = 0.0;
        dLitresInserted = 0.0;
      }
      BGame::m_bFuelingInProgress = false;
    }

    if (BGame::m_nFuelSelect == 0) {
      // Insert gasoline
      if (pVehicle->m_dFuel + dLitresInserted > 100.0) {
        dLitresInserted = 100.0 - pVehicle->m_dFuel;
        BGame::m_bFuelingInProgress = false;
      }
      pVehicle->m_dFuel += dLitresInserted;
    }

    if (BGame::m_nFuelSelect == 1) {
      // Insert kerosine
      if (pVehicle->m_dKerosine + dLitresInserted > 100.0) {
        dLitresInserted = 100.0 - pVehicle->m_dKerosine;
        BGame::m_bFuelingInProgress = false;
      }
      pVehicle->m_dKerosine += dLitresInserted;
    }

    // Pay the meter
    BGame::GetPlayer()->m_dCash -= dCostOfFuel;
  }

  static string sFueling[3];
  // sFueling[0].Format("GASOLINE: %d %%",
  // int(BGame::GetSimulation()->GetVehicle()->m_dFuel));
  stringstream format;
  format << "GASOLINE: " << (int)BGame::GetSimulation()->GetVehicle()->m_dFuel
         << " %";
  sFueling[0] = format.str();
  // sFueling[1].Format("KEROSINE: %d %%",
  // int(BGame::GetSimulation()->GetVehicle()->m_dKerosine));
  format.str("");
  format << "KEROSINE: "
         << (int)BGame::GetSimulation()->GetVehicle()->m_dKerosine << " %";
  sFueling[1] = format.str();
  // sFueling[2].Format("    CASH: %d $", int(BGame::GetPlayer()->m_dCash));
  format.str("");
  format << "    CASH: " << (int)BGame::GetPlayer()->m_dCash << "  $";
  sFueling[2] = format.str();

  double dCharHeight = BUI::TextRenderer()->GetCharHeight();
  double dCharWidth = BUI::TextRenderer()->GetCharWidth();

  // Draw panel around
  glPushMatrix();
  // glTranslated(rectWnd.Width() / 2.0, rectWnd.Height() / 2.0 + 40, 0);
  glTranslated(window_width / 2.0, window_height / 2.0 + 40, 0);
  DrawPanel(dCharWidth * 30, dCharHeight * 5 + 256);
  glPopMatrix();

  // Draw buttons
  OpenGLHelpers::SwitchToTexture(0);
  // BTextures::Use(BTextures::Texture::FUELING);
  BTextures::Use(BTextures::FUELING);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR); // not mipmapped

  double dBlink = fabs(double(SDL_GetTicks() % 500 - 250.0)) / 250.0;
  double dBlink2 = fabs(double(SDL_GetTicks() % 250 - 125.0)) / 125.0;

  // Draw Exit button
  double dColor = 1.0;
  if ((BGame::m_nFuelSelect == 2) && (!BGame::m_bFuelingInProgress)) {
    dColor = 0.5 + dBlink / 2.0;
  }

  OpenGLHelpers::SetColorFull(dColor, dColor, dColor, dColor);
  glPushMatrix();
  // glTranslated(rectWnd.Width() / 2.0, rectWnd.Height() / 2.0 - dCharHeight
  // * 1.5 - 54.0 / 2.0, 0);
  glTranslated(window_width / 2.0,
               window_height / 2.0 - dCharHeight * 1.5 - 54.0 / 2.0, 0);
  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(0, 0);
  glVertex3f(-120.0 / 2.0, -54.0 / 2.0, 0);
  OpenGLHelpers::SetTexCoord(0, 54.0 / 256);
  glVertex3f(-120.0 / 2.0, 54.0 / 2.0, 0);
  OpenGLHelpers::SetTexCoord(120.0 / 256.0, 0);
  glVertex3f(120.0 / 2.0, -54.0 / 2.0, 0);
  OpenGLHelpers::SetTexCoord(120.0 / 256.0, 54.0 / 256);
  glVertex3f(120.0 / 2.0, 54.0 / 2.0, 0);
  glEnd();
  glPopMatrix();

  // Draw Gaso button
  dColor = 1.0;
  if ((BGame::m_nFuelSelect == 0) && (!BGame::m_bFuelingInProgress)) {
    dColor = 0.5 + dBlink / 2.0;
  }

  OpenGLHelpers::SetColorFull(dColor, dColor, dColor, dColor);
  glPushMatrix();
  // glTranslated(rectWnd.Width() / 2.0 - 70.0, rectWnd.Height() / 2.0 +
  // dCharHeight * 2.5 + 128.0 / 2.0, 0);
  glTranslated(window_width / 2.0 - 70.0,
               window_height / 2.0 + dCharHeight * 2.5 + 128.0 / 2.0, 0);
  if ((BGame::m_nFuelSelect == 0) && (BGame::m_bFuelingInProgress)) {
    glScaled(0.9 + dBlink2 * 0.2, 0.9 + dBlink2 * 0.2, 0.9 + dBlink2 * 0.2);
  }
  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(0, 0.5);
  glVertex3f(-128.0 / 2.0, -128.0 / 2.0, 0);
  OpenGLHelpers::SetTexCoord(0, 1.0);
  glVertex3f(-128.0 / 2.0, 128.0 / 2.0, 0);
  OpenGLHelpers::SetTexCoord(0.5, 0.5);
  glVertex3f(128.0 / 2.0, -128.0 / 2.0, 0);
  OpenGLHelpers::SetTexCoord(0.5, 1.0);
  glVertex3f(128.0 / 2.0, 128.0 / 2.0, 0);
  glEnd();

  glPopMatrix();

  // Draw Kero button
  dColor = 1.0;
  if ((BGame::m_nFuelSelect == 1) && (!BGame::m_bFuelingInProgress)) {
    dColor = 0.5 + dBlink / 2.0;
  }

  OpenGLHelpers::SetColorFull(dColor, dColor, dColor, dColor);
  glPushMatrix();
  // glTranslated(rectWnd.Width() / 2.0 + 70.0, rectWnd.Height() / 2.0 +
  // dCharHeight * 2.5 + 128.0 / 2.0, 0);
  glTranslated(window_width / 2.0 + 70.0,
               window_height / 2.0 + dCharHeight * 2.5 + 128.0 / 2.0, 0);
  if ((BGame::m_nFuelSelect == 1) && (BGame::m_bFuelingInProgress)) {
    glScaled(0.9 + dBlink2 * 0.2, 0.9 + dBlink2 * 0.2, 0.9 + dBlink2 * 0.2);
  }
  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(0.5, 0.5);
  glVertex3f(-128.0 / 2.0, -128.0 / 2.0, 0);
  OpenGLHelpers::SetTexCoord(0.5, 1.0);
  glVertex3f(-128.0 / 2.0, 128.0 / 2.0, 0);
  OpenGLHelpers::SetTexCoord(1.0, 0.5);
  glVertex3f(128.0 / 2.0, -128.0 / 2.0, 0);
  OpenGLHelpers::SetTexCoord(1.0, 1.0);
  glVertex3f(128.0 / 2.0, 128.0 / 2.0, 0);
  glEnd();
  glPopMatrix();

  // Draw texts
  glPushMatrix();
  // glTranslated(rectWnd.Width() / 2.0, rectWnd.Height() / 2.0, 0);
  glTranslated(window_width / 2.0, window_height / 2.0, 0);

  BUISelectionList selList;
  selList.SetItems(sFueling, 3);
  // selList.DrawAt(-dCharWidth * 8.0, 0,
  // BTextRenderer::TTextAlign::ALIGN_LEFT);
  selList.DrawAt(-dCharWidth * 8.0, 0, BTextRenderer::ALIGN_LEFT);
  glPopMatrix();
}

void CVeraView::DrawDashboard() {
  // if short on space, draw tight
  glPushMatrix();
  if ((BGame::m_nDispWidth < 1024) && (BGame::m_bNavSat)) {
    glTranslated(128, 0, 0);
  }

  // Draw dashboard as a texture
  OpenGLHelpers::SetColorFull(1, 1, 1, 1);

  // Dashboard
  OpenGLHelpers::SwitchToTexture(0);
  BTextures::Use(BTextures::DASHBOARD);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(0, 1);
  // glVertex3f((m_rectWnd.Width() - 512) / 2, 128, 0);
  glVertex3f((window_width - 512) / 2, 128, 0);
  OpenGLHelpers::SetTexCoord(0, 0);
  // glVertex3f((m_rectWnd.Width() - 512) / 2, 0, 0);
  glVertex3f((window_width - 512) / 2, 0, 0);
  OpenGLHelpers::SetTexCoord(1, 1);
  // glVertex3f((m_rectWnd.Width() - 512) / 2 + 512, 128, 0);
  glVertex3f((window_width - 512) / 2 + 512, 128, 0);
  OpenGLHelpers::SetTexCoord(1, 0);
  // glVertex3f((m_rectWnd.Width() - 512) / 2 + 512, 0, 0);
  glVertex3f((window_width - 512) / 2 + 512, 0, 0);
  glEnd();

  // Draw compass in correct orientation
  double dAngle = 0.0;

  BVector vNorth(0, 1, 0);
  BVector vEast(1, 0, 0);
  BVector vForward =
      m_game.GetSimulation()->GetVehicle()->m_orientation.m_vForward;
  vForward.m_dZ = 0.0;
  vForward.ToUnitLength();

  double dCos = vNorth.ScalarProduct(vForward);
  double dTmp = vEast.ScalarProduct(vForward);
  if (dTmp > 0.0) {
    dAngle = acos(dCos);
  } else {
    dAngle = -acos(dCos);
  }
  dAngle = dAngle / 3.141592654 * 180.0;

  glPushMatrix();
  // glTranslated((m_rectWnd.Width() - 512) / 2 + 256 - 40, 66, 0);
  glTranslated((window_width - 512) / 2 + 256 - 40, 66, 0);
  glRotated(dAngle, 0, 0, 1);

  BTextures::Use(BTextures::COMPASS);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(0, 1);
  glVertex3f(-44, -44, 0);
  OpenGLHelpers::SetTexCoord(0, 0);
  glVertex3f(-44, 44, 0);
  OpenGLHelpers::SetTexCoord(1, 1);
  glVertex3f(44, -44, 0);
  OpenGLHelpers::SetTexCoord(1, 0);
  glVertex3f(44, 44, 0);
  glEnd();
  glPopMatrix();

  // Draw fuel indicators (30� full, 155� empty)
  // Draw car fuel indicator
  dAngle =
      155.0 - 125.0 * (m_game.GetSimulation()->GetVehicle()->m_dFuel / 100.0);
  glPushMatrix();
  // glTranslated((m_rectWnd.Width() - 512) / 2 + 117.5, 55.0, 0);
  glTranslated((window_width - 512) / 2 + 117.5, 55.0, 0);
  glRotated(dAngle, 0, 0, 1);
  glTranslated(0, 7, 0);
  glDisable(GL_TEXTURE_2D);
  OpenGLHelpers::SetColorFull(0.1, 0.1, 0.1, 1);
  glBegin(GL_POLYGON);
  glVertex3f(-2.0, 0, 0);
  glVertex3f(-1.0, 20.0, 0);
  glVertex3f(1.0, 20.0, 0);
  glVertex3f(2.0, 0, 0);
  glEnd();
  glPopMatrix();

  // Draw kerosine indicator
  dAngle = -155.0 +
           125.0 * (m_game.GetSimulation()->GetVehicle()->m_dKerosine / 100.0);
  glPushMatrix();
  // glTranslated((m_rectWnd.Width() - 512) / 2 + 117.5, 55.0, 0);
  glTranslated((window_width - 512) / 2 + 117.5, 55.0, 0);
  glRotated(dAngle, 0, 0, 1);
  glTranslated(0, 7, 0);
  OpenGLHelpers::SetColorFull(0.1, 0.1, 0.1, 1);
  glBegin(GL_POLYGON);
  glVertex3f(-2.0, 0, 0);
  glVertex3f(-1.0, 20.0, 0);
  glVertex3f(1.0, 20.0, 0);
  glVertex3f(2.0, 0, 0);
  glEnd();
  glPopMatrix();

  // Draw beacon indicators
  BTrackingTarget *pTarget = m_game.GetSimulation()->m_targets;
  while (pTarget) {
    // DrawTrackingTarget(pTarget, m_rectWnd);
    DrawTrackingTarget(pTarget);
    pTarget = pTarget->m_pNext;
  }

  // Draw Temp and Cash on the auxiliary panel
  if (!BGame::m_bSceneEditorMode) {
    glPushMatrix();
    // glTranslated((m_rectWnd.Width() - 512) / 2 + 363.0, -8.0, 0);
    glTranslated((window_width - 512) / 2 + 363.0, -8.0, 0);
    BUI::TextRenderer()->StartRenderingText();
    string sTmp;
    // sTmp.Format("%.0lf$", BGame::GetPlayer()->m_dCash);
    stringstream format;
    format << (int)BGame::GetPlayer()->m_dCash << "$";
    sTmp = format.str();
    // BUI::TextRenderer()->DrawSmallTextAt(1.5, 128.0 - 66.0 - 1.5, sTmp,
    // sTmp.GetLength(), BTextRenderer::TTextAlign::ALIGN_LEFT, 0, 0, 0, 0.3);
    BUI::TextRenderer()->DrawSmallTextAt(
        1.5, 128.0 - 66.0 - 1.5, sTmp, sTmp.length(), BTextRenderer::ALIGN_LEFT,
        0, 0, 0, 0.3);
    // BUI::TextRenderer()->DrawSmallTextAt(0, 128.0 - 66.0, sTmp,
    // sTmp.GetLength(), BTextRenderer::TTextAlign::ALIGN_LEFT, 0, 0, 0, 1);
    BUI::TextRenderer()->DrawSmallTextAt(0, 128.0 - 66.0, sTmp, sTmp.length(),
                                         BTextRenderer::ALIGN_LEFT, 0, 0, 0, 1);
    if (BGame::m_dPizzaTemp > 10.0) {
      // sTmp.Format("%.1lf", BGame::m_dPizzaTemp);
      format.str("");
      format.precision(3);
      format << BGame::m_dPizzaTemp;
      sTmp = format.str();
      // BUI::TextRenderer()->DrawSmallTextAt(1.5, 128.0 - 81.0 - 1.5, sTmp,
      // sTmp.GetLength(), BTextRenderer::TTextAlign::ALIGN_LEFT, 0, 0, 0, 0.3);
      BUI::TextRenderer()->DrawSmallTextAt(
          1.5, 128.0 - 81.0 - 1.5, sTmp, sTmp.length(),
          BTextRenderer::ALIGN_LEFT, 0, 0, 0, 0.3);
      // BUI::TextRenderer()->DrawSmallTextAt(0, 128.0 - 81.0, sTmp,
      // sTmp.GetLength(), BTextRenderer::TTextAlign::ALIGN_LEFT, 0, 0, 0, 1);
      BUI::TextRenderer()->DrawSmallTextAt(0, 128.0 - 81.0, sTmp, sTmp.length(),
                                           BTextRenderer::ALIGN_LEFT, 0, 0, 0,
                                           1);
    }
    BUI::TextRenderer()->StopRenderingText();
    glPopMatrix();
    OpenGLHelpers::SetColorFull(1, 1, 1, 1);
  }

  glPopMatrix();

  // G-Force meter

  // First level
  double dGForce = BGame::GetSimulation()->m_dMaxGForce;
  if (dGForce > 1.3) {
    dGForce = 1.3;
  }
  BVector vColor = ColorForGForce(dGForce);
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glDisable(GL_TEXTURE_2D);
  glBegin(GL_TRIANGLE_STRIP);
  glColor4f(vColor.m_dX, vColor.m_dY, vColor.m_dZ, 1);
  // glVertex3f(10, m_rectWnd.Height() - 220.0 + 140.0 * dGForce, 0);
  glVertex3f(10, window_height - 220.0 + 140.0 * dGForce, 0);
  glColor4f(0, 0.5, 0, 1);
  // glVertex3f(10, m_rectWnd.Height() - 220.0, 0);
  glVertex3f(10, window_height - 220.0, 0);
  glColor4f(vColor.m_dX, vColor.m_dY, vColor.m_dZ, 1);
  // glVertex3f(5 + 40, m_rectWnd.Height() - 220.0 + 140.0 * dGForce, 0);
  glVertex3f(5 + 40, window_height - 220.0 + 140.0 * dGForce, 0);
  glColor4f(0, 0.5, 0, 1);
  // glVertex3f(5 + 40, m_rectWnd.Height() - 220.0, 0);
  glVertex3f(5 + 40, window_height - 220.0, 0);
  glEnd();
  glDisable(GL_COLOR_MATERIAL);

  // Then frames
  OpenGLHelpers::SetColorFull(1, 1, 1, 1);
  OpenGLHelpers::SwitchToTexture(0);
  BTextures::Use(BTextures::PANEL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(454.0 / 512.0, (512.0 - 266.0) / 512.0);
  // glVertex3f(5, m_rectWnd.Height() - 220.0 - 5, 0);
  glVertex3f(5, window_height - 220.0 - 5, 0);
  OpenGLHelpers::SetTexCoord(454.0 / 512.0, (512.0 - 46.0) / 512.0);
  // glVertex3f(5, m_rectWnd.Height() - 5, 0);
  glVertex3f(5, window_height - 5, 0);
  OpenGLHelpers::SetTexCoord(502.0 / 512.0, (512.0 - 266.0) / 512.0);
  // glVertex3f(5 + 48, m_rectWnd.Height() - 220.0 - 5, 0);
  glVertex3f(5 + 48, window_height - 220.0 - 5, 0);
  OpenGLHelpers::SetTexCoord(502.0 / 512.0, (512.0 - 46.0) / 512.0);
  // glVertex3f(5 + 48, m_rectWnd.Height() - 5, 0);
  glVertex3f(5 + 48, window_height - 5, 0);
  glEnd();
}

BVector CVeraView::ColorForGForce(double dGForce) {
  if (dGForce > 1.0) {
    dGForce = 1.0;
  }
  if (dGForce < 0.0) {
    dGForce = 0.0;
  }
  return BVector(dGForce, (1.0 - dGForce) * 0.5, 0.0);
}

void CVeraView::DrawTrackingTarget(BTrackingTarget *pTarget) {
  double dAngle;

  BVector vToTarget(pTarget->m_vLoc.m_dX -
                        m_game.GetSimulation()->GetVehicle()->m_vLocation.m_dX,
                    pTarget->m_vLoc.m_dY -
                        m_game.GetSimulation()->GetVehicle()->m_vLocation.m_dY,
                    0);
  double dDist = vToTarget.Length();
  vToTarget.ToUnitLength();
  BVector vUp(0, 0, -1);
  BVector vRight = vUp.CrossProduct(vToTarget);
  BVector vForward =
      m_game.GetSimulation()->GetVehicle()->m_orientation.m_vForward;
  vForward.m_dZ = 0.0;
  vForward.ToUnitLength();
  double dCos = vToTarget.ScalarProduct(vForward);
  double dTmp = vRight.ScalarProduct(vForward);
  if (dTmp > 0.0) {
    dAngle = acos(dCos);
  } else {
    dAngle = -acos(dCos);
  }
  dAngle = dAngle / 3.141592654 * 180.0;

  glPushMatrix();
  // glTranslated((rectWnd.Width() - 512) / 2 + 256 - 41, 66.5, 0);
  glTranslated((window_width - 512) / 2 + 256 - 41, 66.5, 0);
  glRotated(dAngle, 0, 0, 1);
  glTranslated(0, 21, 0);
  glDisable(GL_TEXTURE_2D);
  OpenGLHelpers::SetColorFull(pTarget->m_dRed, pTarget->m_dGreen,
                              pTarget->m_dBlue, 0.5);
  glBegin(GL_TRIANGLES);
  glVertex3f(-5, 0, 0);
  glVertex3f(0, 10, 0);
  glVertex3f(5, 0, 0);
  glEnd();
  glPopMatrix();

  // Draw distance text
  glEnable(GL_TEXTURE_2D);
  BTextures::Use(BTextures::LED_NUMBERS);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  // OpenGLHelpers::SetColorFull(1, 1, 1, 1);
  // if(pTarget->m_sId.CompareNoCase("BASE") == 0) {
  if (pTarget->m_sId.compare("BASE") == 0) {
    OpenGLHelpers::SetColorFull((pTarget->m_dRed + 1) / 2,
                                (pTarget->m_dGreen + 1) / 2,
                                (pTarget->m_dBlue + 1) / 2, 1);
  } else {
    OpenGLHelpers::SetColorFull(pTarget->m_dRed, pTarget->m_dGreen,
                                pTarget->m_dBlue, 1);
  }
  string sDist;
  // sDist.Format("%.0lf", dDist);
  stringstream format;
  format << (int)dDist;
  sDist = format.str();
  // double dX = (rectWnd.Width() - 512) / 2 + 224;
  double dX = (window_width - 512) / 2 + 224;
  double dY = 63;
  // if(pTarget->m_sId.CompareNoCase("BASE") == 0) {
  if (pTarget->m_sId.compare("BASE") == 0) {
    // Distance to base next to blue dot
    dY -= 10;
    //} else if(pTarget->m_sId.CompareNoCase("CLIENT") == 0) {
  } else if (pTarget->m_sId.compare("CLIENT") == 0) {
    // Distance to client next to green dot
    dY += 10;
    //} else if(pTarget->m_sId.CompareNoCase("FUEL") == 0) {
  } else if (pTarget->m_sId.compare("FUEL") == 0) {
    // Distance to fuel next to red dot
  }

  // Draw text number by number
  // int nDigit = sDist.GetLength() - 1;
  int nDigit = sDist.length() - 1;
  while (nDigit >= 0) {
    // TCHAR c = sDist.GetAt(nDigit);
    char c = sDist.at(nDigit);
    c -= '0';
    glBegin(GL_POLYGON);
    OpenGLHelpers::SetTexCoord((c * 5) / 64.0, 8.0 / 16.0);
    glVertex3f(dX, dY, 0);
    OpenGLHelpers::SetTexCoord((c * 5) / 64.0, 16.0 / 16.0);
    glVertex3f(dX, dY + 8, 0);
    OpenGLHelpers::SetTexCoord((c * 5 + 4) / 64.0, 16.0 / 16.0);
    glVertex3f(dX + 4, dY + 8, 0);
    OpenGLHelpers::SetTexCoord((c * 5 + 4) / 64.0, 8.0 / 16.0);
    glVertex3f(dX + 4, dY, 0);
    glEnd();
    dX -= 5;
    --nDigit;
  }
}

void CVeraView::UpdateTrackingTargetScreenPos(BTrackingTarget *pTarget) {
  GLdouble modelMatrix[16];
  GLdouble projMatrix[16];
  GLint viewport[4];

  glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
  glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
  glGetIntegerv(GL_VIEWPORT, viewport);

  if (gluProject(pTarget->m_vLoc.m_dX, pTarget->m_vLoc.m_dY,
                 pTarget->m_vLoc.m_dZ, modelMatrix, projMatrix, viewport,
                 &(pTarget->m_vScreenPos.m_dX), &(pTarget->m_vScreenPos.m_dY),
                 &(pTarget->m_vScreenPos.m_dZ))) {

    // Check for behindness and offscreen

    BVector vToTarget =
        pTarget->m_vLoc - BGame::GetSimulation()->GetCamera()->m_vLocation;
    double dBehind = vToTarget.ScalarProduct(
        BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward);

    if (dBehind < 0.0) {
      // if(pTarget->m_vScreenPos.m_dX < rectWnd.Width() / 2) {
      if (pTarget->m_vScreenPos.m_dX < window_width / 2) {
        // pTarget->m_vScreenPos.m_dX = rectWnd.Width() - 30.0;
        pTarget->m_vScreenPos.m_dX = window_width - 30.0;
      } else {
        pTarget->m_vScreenPos.m_dX = 30.0;
      }
      // if(pTarget->m_vScreenPos.m_dY < rectWnd.Height() / 2) {
      if (pTarget->m_vScreenPos.m_dY < window_height / 2) {
        // pTarget->m_vScreenPos.m_dY = rectWnd.Height() - 30.0;
        pTarget->m_vScreenPos.m_dY = window_height - 30.0;
      } else {
        pTarget->m_vScreenPos.m_dY = 30.0;
      }
    } else {
      if (pTarget->m_vScreenPos.m_dX < 30.0) {
        pTarget->m_vScreenPos.m_dX = 30.0;
      }
      if (pTarget->m_vScreenPos.m_dY < 30.0) {
        pTarget->m_vScreenPos.m_dY = 30.0;
      }
      // if(pTarget->m_vScreenPos.m_dX > rectWnd.Width() - 30.0) {
      if (pTarget->m_vScreenPos.m_dX > window_width - 30.0) {
        // pTarget->m_vScreenPos.m_dX = rectWnd.Width() - 30.0;
        pTarget->m_vScreenPos.m_dX = window_width - 30.0;
      }
      // if(pTarget->m_vScreenPos.m_dY > rectWnd.Height() - 30.0) {
      if (pTarget->m_vScreenPos.m_dY > window_height - 30.0) {
        // pTarget->m_vScreenPos.m_dY = rectWnd.Height() - 30.0;
        pTarget->m_vScreenPos.m_dY = window_height - 30.0;
      }
    }

  } else {
    pTarget->m_vScreenPos.Set(0, 0, 0);
  }
}

void CVeraView::DrawTrackingTargetOnScreen(BTrackingTarget *pTarget) {
  unsigned clockNow = SDL_GetTicks();
  double dAlpha = fabs(double(clockNow % 1000) - 500.0) / 500.0;
  double dAlphaBase, dAlphaFluct;
  // double dDist = sqrt((rectWnd.Width() / 2.0 - pTarget->m_vScreenPos.m_dX) *
  // (rectWnd.Width() / 2.0 - pTarget->m_vScreenPos.m_dX) +
  double dDist = sqrt((window_width / 2.0 - pTarget->m_vScreenPos.m_dX) *
                          (window_width / 2.0 - pTarget->m_vScreenPos.m_dX) +
                      //(rectWnd.Height() / 2.0 - pTarget->m_vScreenPos.m_dY) *
                      //(rectWnd.Height() / 2.0 - pTarget->m_vScreenPos.m_dY)) /
                      //(rectWnd.Width() / 2.0);
                      (window_height / 2.0 - pTarget->m_vScreenPos.m_dY) *
                          (window_height / 2.0 - pTarget->m_vScreenPos.m_dY)) /
                 (window_width / 2.0);
  if (dDist > 1.0) {
    dDist = 1.0;
  }
  dAlphaBase = 0.025 + dDist * 0.475;
  dAlphaFluct = dAlphaBase;
  OpenGLHelpers::SetColorFull(pTarget->m_dRed, pTarget->m_dGreen,
                              pTarget->m_dBlue,
                              dAlphaBase + dAlpha * dAlphaFluct);

  OpenGLHelpers::SwitchToTexture(0);
  BTextures::Use(BTextures::PANEL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glPushMatrix();
  glTranslated(pTarget->m_vScreenPos.m_dX, pTarget->m_vScreenPos.m_dY, 0.0);

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(128.0 / 512.0, (512.0 - 68.0) / 512.0);
  glVertex3f(-70.0 / 2.0, -37.0 / 2.0, 0.0);
  OpenGLHelpers::SetTexCoord(128.0 / 512.0, (512.0 - 32.0) / 512.0);
  glVertex3f(-70.0 / 2.0, 37.0 / 2.0, 0.0);
  OpenGLHelpers::SetTexCoord(197.0 / 512.0, (512.0 - 68.0) / 512.0);
  glVertex3f(70.0 / 2.0, -37.0 / 2.0, 0.0);
  OpenGLHelpers::SetTexCoord(197.0 / 512.0, (512.0 - 32.0) / 512.0);
  glVertex3f(70.0 / 2.0, 37.0 / 2.0, 0.0);
  glEnd();

  // Draw text

  OpenGLHelpers::SetColorFull(pTarget->m_dRed * 0.5, pTarget->m_dGreen * 0.5,
                              pTarget->m_dBlue * 0.5, dAlphaBase + 0.25);

  double dTexY = 16.0;
  // if(pTarget->m_sId.CompareNoCase("BASE") == 0) {
  if (pTarget->m_sId.compare("BASE") == 0) {
    dTexY = 16.0;
    //} else if(pTarget->m_sId.CompareNoCase("CLIENT") == 0) {
  } else if (pTarget->m_sId.compare("CLIENT") == 0) {
    dTexY = 16.0 + 22.0;
    //} else if(pTarget->m_sId.CompareNoCase("FUEL") == 0) {
  } else if (pTarget->m_sId.compare("FUEL") == 0) {
    dTexY = 16.0 + 44.0;
  }

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(196.0 / 512.0, (512.0 - (dTexY + 22.0)) / 512.0);
  glVertex3f(-53.0 / 2.0, -22.0 / 2.0 - 17.0, 0.0);
  OpenGLHelpers::SetTexCoord(196.0 / 512.0, (512.0 - dTexY) / 512.0);
  glVertex3f(-53.0 / 2.0, 22.0 / 2.0 - 17.0, 0.0);
  OpenGLHelpers::SetTexCoord(249.0 / 512.0, (512.0 - (dTexY + 22.0)) / 512.0);
  glVertex3f(53.0 / 2.0, -22.0 / 2.0 - 17.0, 0.0);
  OpenGLHelpers::SetTexCoord(249.0 / 512.0, (512.0 - dTexY) / 512.0);
  glVertex3f(53.0 / 2.0, 22.0 / 2.0 - 17.0, 0.0);
  glEnd();

  glPopMatrix();
}

void CVeraView::DrawQuickHelp() {
  // Draw quick help as a texture
  OpenGLHelpers::SetColorFull(1, 1, 1, 1);

  OpenGLHelpers::SwitchToTexture(0);
  if (BGame::GetSceneEditor()->IsActive()) {
    BTextures::Use(BTextures::QUICK_HELP_SCENE_EDITOR);
  } else {
    BTextures::Use(BTextures::QUICK_HELP);
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(0, 1);
  // glVertex3f((m_rectWnd.Width() - 512) / 2, m_rectWnd.Height() -
  // (m_rectWnd.Height() - 512) / 2, 0);
  glVertex3f((window_width - 512) / 2,
             window_height - (window_height - 512) / 2, 0);
  OpenGLHelpers::SetTexCoord(0, 0);
  // glVertex3f((m_rectWnd.Width() - 512) / 2, (m_rectWnd.Height() - 512) / 2,
  // 0);
  glVertex3f((window_width - 512) / 2, (window_height - 512) / 2, 0);
  OpenGLHelpers::SetTexCoord(1, 1);
  // glVertex3f((m_rectWnd.Width() - 512) / 2 + 512, m_rectWnd.Height() -
  // (m_rectWnd.Height() - 512) / 2, 0);
  glVertex3f((window_width - 512) / 2 + 512,
             window_height - (window_height - 512) / 2, 0);
  OpenGLHelpers::SetTexCoord(1, 0);
  // glVertex3f((m_rectWnd.Width() - 512) / 2 + 512, (m_rectWnd.Height() - 512)
  // / 2, 0);
  glVertex3f((window_width - 512) / 2 + 512, (window_height - 512) / 2, 0);
  glEnd();
}

void CVeraView::DrawNavSat() {
  // Draw navsat as a texture
  OpenGLHelpers::SetColorFull(1, 1, 1, 1);

  // NavSat window
  glPushMatrix();
  glTranslated(7, 7, 0);
  OpenGLHelpers::SwitchToTexture(0);
  int nRes = BGame::GetNavSat()->ActivateCurrentMapTexture();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  double dMaxTex = (double(nRes) - 1.0) / double(nRes);
  if (nRes == 256) {
    dMaxTex = 1.0;
  }
  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(0, dMaxTex);
  glVertex3f(0, 256, 0);
  OpenGLHelpers::SetTexCoord(0, 0);
  glVertex3f(0, 0, 0);
  OpenGLHelpers::SetTexCoord(dMaxTex, dMaxTex);
  glVertex3f(256, 256, 0);
  OpenGLHelpers::SetTexCoord(dMaxTex, 0);
  glVertex3f(256, 0, 0);
  glEnd();
  glDisable(GL_TEXTURE_2D);

  // Draw crosses on "ground glass"
  OpenGLHelpers::SetColorFull(0, 0, 0, 0.3);
  for (double y = 0; y < 128.0; y += 53.0) {
    for (double x = 0; x < 128.0; x += 53.0) {
      glBegin(GL_LINES);
      glVertex3f(128 + x - 10.0, 128 + y, 0);
      glVertex3f(128 + x + 10.0, 128 + y, 0);
      glVertex3f(128 + x, 128 + y - 10.0, 0);
      glVertex3f(128 + x, 128 + y + 10.0, 0);

      if (x != 0) {
        glVertex3f(128 - x - 10.0, 128 + y, 0);
        glVertex3f(128 - x + 10.0, 128 + y, 0);
        glVertex3f(128 - x, 128 + y - 10.0, 0);
        glVertex3f(128 - x, 128 + y + 10.0, 0);
      }

      if (y != 0) {
        glVertex3f(128 + x - 10.0, 128 - y, 0);
        glVertex3f(128 + x + 10.0, 128 - y, 0);
        glVertex3f(128 + x, 128 - y - 10.0, 0);
        glVertex3f(128 + x, 128 - y + 10.0, 0);
      }

      if ((x != 0) && (y != 0)) {
        glVertex3f(128 - x - 10.0, 128 - y, 0);
        glVertex3f(128 - x + 10.0, 128 - y, 0);
        glVertex3f(128 - x, 128 - y - 10.0, 0);
        glVertex3f(128 - x, 128 - y + 10.0, 0);
      }

      glEnd();
    }
  }

  // Draw panel over image and indicators
  OpenGLHelpers::SetColorFull(1, 1, 1, 1);
  glEnable(GL_TEXTURE_2D);
  OpenGLHelpers::SwitchToTexture(0);
  BTextures::Use(BTextures::PANEL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  // Draw tracking beacons
  unsigned clockNow = SDL_GetTicks();
  BTrackingTarget *pTarget = m_game.GetSimulation()->m_targets;
  while (pTarget) {
    BVector vTmp(pTarget->m_vLoc.m_dX, pTarget->m_vLoc.m_dY, 0.0);
    /*DrawNavSatIndicator(false,
                        0,
                        pTarget->m_dRed,
                        pTarget->m_dGreen,
                        pTarget->m_dBlue,
                        fabs(double(clockNow % 1000) - 500.0) / 500.0,
                        vTmp,
                        m_rectWnd);*/
    DrawNavSatIndicator(false, 0, pTarget->m_dRed, pTarget->m_dGreen,
                        pTarget->m_dBlue,
                        fabs(double(clockNow % 1000) - 500.0) / 500.0, vTmp);
    pTarget = pTarget->m_pNext;
  }

  // Draw car indicator
  double dAngle;
  BVector vNorth(0, 1, 0);
  BVector vEast(1, 0, 0);
  double dCos = vNorth.ScalarProduct(
      m_game.GetSimulation()->GetVehicle()->m_orientation.m_vForward);
  double dTmp = vEast.ScalarProduct(
      m_game.GetSimulation()->GetVehicle()->m_orientation.m_vForward);
  if (dTmp > 0.0) {
    dAngle = -acos(dCos);
  } else {
    dAngle = acos(dCos);
  }
  dAngle = dAngle / 3.141592654 * 180.0;

  (void)BGame::GetNavSat()->Track(
      BGame::GetSimulation()->GetVehicle()->m_vLocation);
  // DrawNavSatIndicator(true, dAngle, 1, 1, 1, 1,
  // BGame::GetSimulation()->GetVehicle()->m_vLocation, m_rectWnd);
  DrawNavSatIndicator(true, dAngle, 1, 1, 1, 1,
                      BGame::GetSimulation()->GetVehicle()->m_vLocation);
  glPopMatrix();

  // Panel (in 4 parts to exclude the center text)
  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(120.0 / 512.0, (511.0 - 94.0) / 512.0);
  glVertex3f(0, 298, 0);
  OpenGLHelpers::SetTexCoord(120.0 / 512.0, (511.0 - 145.0) / 512.0);
  glVertex3f(0, 298 - 51, 0);
  OpenGLHelpers::SetTexCoord(392.0 / 512.0, (511.0 - 94.0) / 512.0);
  glVertex3f(272, 298, 0);
  OpenGLHelpers::SetTexCoord(392.0 / 512.0, (511.0 - 145.0) / 512.0);
  glVertex3f(272, 298 - 51, 0);
  glEnd();

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(120.0 / 512.0, (511.0 - 145.0) / 512.0);
  glVertex3f(0, 298 - 51, 0);
  OpenGLHelpers::SetTexCoord(120.0 / 512.0, (511.0 - 352.0) / 512.0);
  glVertex3f(0, 298 - 278, 0);
  OpenGLHelpers::SetTexCoord(138.0 / 512.0, (511.0 - 145.0) / 512.0);
  glVertex3f(18, 298 - 51, 0);
  OpenGLHelpers::SetTexCoord(138.0 / 512.0, (511.0 - 352.0) / 512.0);
  glVertex3f(18, 298 - 278, 0);
  glEnd();

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(375.0 / 512.0, (511.0 - 145.0) / 512.0);
  glVertex3f(255, 298 - 51, 0);
  OpenGLHelpers::SetTexCoord(375.0 / 512.0, (511.0 - 372.0) / 512.0);
  glVertex3f(255, 298 - 278, 0);
  OpenGLHelpers::SetTexCoord(393.0 / 512.0, (511.0 - 145.0) / 512.0);
  glVertex3f(255 + 18, 298 - 51, 0);
  OpenGLHelpers::SetTexCoord(393.0 / 512.0, (511.0 - 372.0) / 512.0);
  glVertex3f(255 + 18, 298 - 278, 0);
  glEnd();

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(120.0 / 512.0, (511.0 - 372.0) / 512.0);
  glVertex3f(0, 20, 0);
  OpenGLHelpers::SetTexCoord(120.0 / 512.0, (511.0 - 392.0) / 512.0);
  glVertex3f(0, 0, 0);
  OpenGLHelpers::SetTexCoord(392.0 / 512.0, (511.0 - 372.0) / 512.0);
  glVertex3f(272, 20, 0);
  OpenGLHelpers::SetTexCoord(392.0 / 512.0, (511.0 - 392.0) / 512.0);
  glVertex3f(272, 0, 0);
  glEnd();

  // Draw handle
  glPushMatrix();
  glTranslated(135, 187, 0);
  glRotated(BGame::m_dNavSatHandleAngle, 0, 0, 1);
  glTranslated(0, 90.5, 0);
  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(423.0 / 512.0, (511.0 - 176.0) / 512.0);
  glVertex3f(-12, 64, 0);
  OpenGLHelpers::SetTexCoord(423.0 / 512.0, (511.0 - 240.0) / 512.0);
  glVertex3f(-12, 0, 0);
  OpenGLHelpers::SetTexCoord(447.0 / 512.0, (511.0 - 176.0) / 512.0);
  glVertex3f(12, 64, 0);
  OpenGLHelpers::SetTexCoord(447.0 / 512.0, (511.0 - 240.0) / 512.0);
  glVertex3f(12, 0, 0);
  glEnd();
  glPopMatrix();

  glDisable(GL_TEXTURE_2D);
}

void CVeraView::DrawNavSatIndicator(bool bCar, double dAngle, double dR,
                                    double dG, double dB, double dAlpha,
                                    // BVector vLoc,
                                    BVector vLoc) {
  BVector vLocOnWnd = BGame::GetNavSat()->GetRelLoc(vLoc);
  if (vLocOnWnd.m_dX > 0.0 && vLocOnWnd.m_dX < 255 && vLocOnWnd.m_dY > 0.0 &&
      vLocOnWnd.m_dY < 255) {
    OpenGLHelpers::SetColorFull(dR, dG, dB, dAlpha);
    glPushMatrix();
    glTranslated(vLocOnWnd.m_dX, vLocOnWnd.m_dY, 0);
    glRotated(dAngle, 0, 0, 1);
    glBegin(GL_TRIANGLE_STRIP);
    if (bCar) {
      OpenGLHelpers::SetTexCoord(95.0 / 512.0, (511.0 - 247.0) / 512.0);
      glVertex3f(-7.5, 7.5, 0);
      OpenGLHelpers::SetTexCoord(95.0 / 512.0, (511.0 - 262.0) / 512.0);
      glVertex3f(-7.5, -7.5, 0);
      OpenGLHelpers::SetTexCoord(110.0 / 512.0, (511.0 - 247.0) / 512.0);
      glVertex3f(7.5, 7.5, 0);
      OpenGLHelpers::SetTexCoord(110.0 / 512.0, (511.0 - 262.0) / 512.0);
      glVertex3f(7.5, -7.5, 0);
    } else {
      OpenGLHelpers::SetTexCoord(95.0 / 512.0, (511.0 - 195.0) / 512.0);
      glVertex3f(-7.5, 7.5, 0);
      OpenGLHelpers::SetTexCoord(95.0 / 512.0, (511.0 - 212.0) / 512.0);
      glVertex3f(-7.5, -7.5, 0);
      OpenGLHelpers::SetTexCoord(111.0 / 512.0, (511.0 - 195.0) / 512.0);
      glVertex3f(7.5, 7.5, 0);
      OpenGLHelpers::SetTexCoord(111.0 / 512.0, (511.0 - 212.0) / 512.0);
      glVertex3f(7.5, -7.5, 0);
    }
    glEnd();
    glPopMatrix();
  }
}

void CVeraView::DrawServiceWnd() {
  // Draw service window
  // Draw background and texts
  glDisable(GL_TEXTURE_2D);
  double dAlpha = (-BGame::m_dServiceHandleAngle + 20.0) / 40.0;
  OpenGLHelpers::SetColorFull(0, 0, 0, dAlpha);
  glBegin(GL_TRIANGLE_STRIP);
  // glVertex3f(m_rectWnd.Width() - 7 - 256 - 100, m_rectWnd.Height() - 256 - 7,
  // 0);
  glVertex3f(window_width - 7 - 256 - 100, window_height - 256 - 7, 0);
  // glVertex3f(m_rectWnd.Width() - 7, m_rectWnd.Height() - 256 - 7, 0);
  glVertex3f(window_width - 7, window_height - 256 - 7, 0);
  // glVertex3f(m_rectWnd.Width() - 7 - 256 - 100, m_rectWnd.Height() - 7, 0);
  glVertex3f(window_width - 7 - 256 - 100, window_height - 7, 0);
  // glVertex3f(m_rectWnd.Width() - 7, m_rectWnd.Height() - 7, 0);
  glVertex3f(window_width - 7, window_height - 7, 0);
  glEnd();

  OpenGLHelpers::SwitchToTexture(0);
  BTextures::Use(BTextures::PANEL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  // DrawServiceWndTexts(m_rectWnd);
  DrawServiceWndTexts();

  // Panel (draw in four parts to make it wider and to exclude the "It'll cost
  // ya..." text)
  OpenGLHelpers::SetColorFull(1, 1, 1, 1);
  OpenGLHelpers::SwitchToTexture(0);
  BTextures::Use(BTextures::PANEL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(392.0 / 512.0, (511.0 - (391.0 - 27.0)) / 512.0);
  // glVertex3f(m_rectWnd.Width() - 27, m_rectWnd.Height() - 272, 0);
  glVertex3f(window_width - 27, window_height - 272, 0);
  OpenGLHelpers::SetTexCoord(392.0 / 512.0, (511.0 - 391.0) / 512.0);
  // glVertex3f(m_rectWnd.Width(), m_rectWnd.Height() - 272, 0);
  glVertex3f(window_width, window_height - 272, 0);
  OpenGLHelpers::SetTexCoord(120.0 / 512.0, (511.0 - (391.0 - 27.0)) / 512.0);
  // glVertex3f(m_rectWnd.Width() - 27, m_rectWnd.Height(), 0);
  glVertex3f(window_width - 27, window_height, 0);
  OpenGLHelpers::SetTexCoord(120.0 / 512.0, (511.0 - 391.0) / 512.0);
  // glVertex3f(m_rectWnd.Width(), m_rectWnd.Height(), 0);
  glVertex3f(window_width, window_height, 0);
  glEnd();

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(392.0 / 512.0, (511.0 - 144.0) / 512.0);
  // glVertex3f(m_rectWnd.Width() - 347, m_rectWnd.Height() - 272, 0);
  glVertex3f(window_width - 347, window_height - 272, 0);
  OpenGLHelpers::SetTexCoord(392.0 / 512.0, (511.0 - (391.0 - 27.0)) / 512.0);
  // glVertex3f(m_rectWnd.Width() - 27, m_rectWnd.Height() - 272, 0);
  glVertex3f(window_width - 27, window_height - 272, 0);
  OpenGLHelpers::SetTexCoord((392.0 - 20.0) / 512.0, (511.0 - 144.0) / 512.0);
  // glVertex3f(m_rectWnd.Width() - 347, m_rectWnd.Height() - 272 + 20.0, 0);
  glVertex3f(window_width - 347, window_height - 272 + 20.0, 0);
  OpenGLHelpers::SetTexCoord((392.0 - 20.0) / 512.0,
                             (511.0 - (391.0 - 27.0)) / 512.0);
  // glVertex3f(m_rectWnd.Width() - 27, m_rectWnd.Height() - 272 + 20.0, 0);
  glVertex3f(window_width - 27, window_height - 272 + 20.0, 0);
  glEnd();

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(120.0 / 512.0, (511.0 - 144.0) / 512.0);
  // glVertex3f(m_rectWnd.Width() - 347, m_rectWnd.Height(), 0);
  glVertex3f(window_width - 347, window_height, 0);
  OpenGLHelpers::SetTexCoord(120.0 / 512.0, (511.0 - (391.0 - 27.0)) / 512.0);
  // glVertex3f(m_rectWnd.Width() - 27, m_rectWnd.Height(), 0);
  glVertex3f(window_width - 27, window_height, 0);
  OpenGLHelpers::SetTexCoord((120.0 + 20.0) / 512.0, (511.0 - 144.0) / 512.0);
  // glVertex3f(m_rectWnd.Width() - 347, m_rectWnd.Height() - 20.0, 0);
  glVertex3f(window_width - 347, window_height - 20.0, 0);
  OpenGLHelpers::SetTexCoord((120.0 + 20.0) / 512.0,
                             (511.0 - (391.0 - 27.0)) / 512.0);
  // glVertex3f(m_rectWnd.Width() - 27, m_rectWnd.Height() - 20.0, 0);
  glVertex3f(window_width - 27, window_height - 20.0, 0);
  glEnd();

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(392.0 / 512.0, (511.0 - 94.0) / 512.0);
  // glVertex3f(m_rectWnd.Width() - 397, m_rectWnd.Height() - 272, 0);
  glVertex3f(window_width - 397, window_height - 272, 0);
  OpenGLHelpers::SetTexCoord(392.0 / 512.0, (511.0 - 144.0) / 512.0);
  // glVertex3f(m_rectWnd.Width() - 347, m_rectWnd.Height() - 272, 0);
  glVertex3f(window_width - 347, window_height - 272, 0);
  OpenGLHelpers::SetTexCoord(120.0 / 512.0, (511.0 - 94.0) / 512.0);
  // glVertex3f(m_rectWnd.Width() - 397, m_rectWnd.Height(), 0);
  glVertex3f(window_width - 397, window_height, 0);
  OpenGLHelpers::SetTexCoord(120.0 / 512.0, (511.0 - 144.0) / 512.0);
  // glVertex3f(m_rectWnd.Width() - 347, m_rectWnd.Height(), 0);
  glVertex3f(window_width - 347, window_height, 0);
  glEnd();

  // Draw overlay to overwrite Near/Far texts with Clear/Black
  glPushMatrix();
  // glTranslated(m_rectWnd.Width() - 394, m_rectWnd.Height() - 246, 0);
  glTranslated(window_width - 394, window_height - 246, 0);
  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(366.0 / 512.0, (511.0 - 404.0) / 512.0);
  glVertex3f(0, 0, 0);
  OpenGLHelpers::SetTexCoord(366.0 / 512.0, (511.0 - 433.0) / 512.0);
  glVertex3f(29, 0, 0);
  OpenGLHelpers::SetTexCoord(149.0 / 512.0, (511.0 - 404.0) / 512.0);
  glVertex3f(0, 217, 0);
  OpenGLHelpers::SetTexCoord(149.0 / 512.0, (511.0 - 433.0) / 512.0);
  glVertex3f(29, 217, 0);
  glEnd();
  glPopMatrix();

  // Draw handle
  glPushMatrix();
  // glTranslated(m_rectWnd.Width() - 287, m_rectWnd.Height() - 137, 0);
  glTranslated(window_width - 287, window_height - 137, 0);
  glRotated(90 + BGame::m_dServiceHandleAngle, 0, 0, 1);
  glTranslated(0, 90.5, 0);
  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(423.0 / 512.0, (511.0 - 176.0) / 512.0);
  glVertex3f(-12, 64, 0);
  OpenGLHelpers::SetTexCoord(423.0 / 512.0, (511.0 - 240.0) / 512.0);
  glVertex3f(-12, 0, 0);
  OpenGLHelpers::SetTexCoord(447.0 / 512.0, (511.0 - 176.0) / 512.0);
  glVertex3f(12, 64, 0);
  OpenGLHelpers::SetTexCoord(447.0 / 512.0, (511.0 - 240.0) / 512.0);
  glVertex3f(12, 0, 0);
  glEnd();
  glPopMatrix();

  glDisable(GL_TEXTURE_2D);
}

void CVeraView::DrawServiceWndTexts() {
  glPushMatrix();
  // glTranslated(rectWnd.Width() - 256 - 100, rectWnd.Height() - 14 - 18, 0);
  glTranslated(window_width - 256 - 100, window_height - 14 - 18, 0);
  BGame::GetServiceWnd()->DrawTexts();
  glPopMatrix();
}

void CVeraView::DrawPickupInfo() {
  double dCharWidth = BUI::TextRenderer()->GetCharWidth();
  double dCharHeight = BUI::TextRenderer()->GetCharHeight();
  glPushMatrix();
  // glTranslated(rectWnd.Width() / 2, rectWnd.Height() / 2, 0);
  glTranslated(window_width / 2, window_height / 2, 0);

  glPushMatrix();
  glTranslated(0, dCharHeight / 2, 0);
  DrawPanel(dCharWidth * 34, dCharHeight * 9);
  glPopMatrix();

  BUI::TextRenderer()->StartRenderingText();
  // BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 3.0,  "* * * DELIVERY INFO
  // * * *", BTextRenderer::TTextAlign::ALIGN_CENTER, 0.55, 0.6, 0.85, 1);
  BUI::TextRenderer()->DrawTextAt(
      0, dCharHeight * 3.0, "* * * DELIVERY INFO * * *",
      BTextRenderer::ALIGN_CENTER, 0.55, 0.6, 0.85, 1);
  // BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 1.0,  "CLIENT:",
  // BTextRenderer::TTextAlign::ALIGN_RIGHT, 1, 0.6, 0.6, 1);
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 1.0,
                                  "CLIENT:", BTextRenderer::ALIGN_RIGHT, 1, 0.6,
                                  0.6, 1);
  // BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 0,    "DISTANCE:",
  // BTextRenderer::TTextAlign::ALIGN_RIGHT, 1, 0.6, 0.6, 1);
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 0,
                                  "DISTANCE:", BTextRenderer::ALIGN_RIGHT, 1,
                                  0.6, 0.6, 1);
  // BUI::TextRenderer()->DrawTextAt(0, dCharHeight * -1.0, "MAX PROFIT:",
  // BTextRenderer::TTextAlign::ALIGN_RIGHT, 1, 0.6, 0.6, 1);
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * -1.0,
                                  "MAX PROFIT:", BTextRenderer::ALIGN_RIGHT, 1,
                                  0.6, 0.6, 1);
  // BUI::TextRenderer()->DrawTextAt(0, dCharHeight * -2.0, "PIZZA:",
  // BTextRenderer::TTextAlign::ALIGN_RIGHT, 1, 0.6, 0.6, 1);
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * -2.0,
                                  "PIZZA:", BTextRenderer::ALIGN_RIGHT, 1, 0.6,
                                  0.6, 1);

  string sTmp;
  // sTmp.Format("%s", BGame::GetCurrentClient()->m_sName);
  sTmp.assign(BGame::GetCurrentClient()->m_sName);
  // BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 1.0,  sTmp,
  // BTextRenderer::TTextAlign::ALIGN_LEFT, 0.6, 1, 0.6, 1);
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 1.0, sTmp,
                                  BTextRenderer::ALIGN_LEFT, 0.6, 1, 0.6, 1);
  // sTmp.Format("%.1lf NU", BGame::m_dLastDeliveryDistance);
  stringstream format;
  format.precision(5);
  format.unsetf(ios::floatfield);
  format << BGame::m_dLastDeliveryDistance << " NU";
  sTmp = format.str();
  // BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 0,    sTmp,
  // BTextRenderer::TTextAlign::ALIGN_LEFT, 0.6, 1, 0.6, 1);
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 0, sTmp,
                                  BTextRenderer::ALIGN_LEFT, 0.6, 1, 0.6, 1);
  // sTmp.Format("%.0lf $",
  // BGame::GetMaxProfitFor(BGame::m_dLastDeliveryDistance,
  // BGame::m_dPizzaTemp));
  format.str("");
  format << (int)BGame::GetMaxProfitFor(BGame::m_dLastDeliveryDistance,
                                        BGame::m_dPizzaTemp)
         << " $";
  sTmp = format.str();
  // BUI::TextRenderer()->DrawTextAt(0, dCharHeight * -1.0, sTmp,
  // BTextRenderer::TTextAlign::ALIGN_LEFT, 0.6, 1, 0.6, 1);
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * -1.0, sTmp,
                                  BTextRenderer::ALIGN_LEFT, 0.6, 1, 0.6, 1);
  // BUI::TextRenderer()->DrawTextAt(0, dCharHeight * -2.0,
  // BGame::m_sPizzas[BGame::m_nPizza], BTextRenderer::TTextAlign::ALIGN_LEFT,
  // 0.6, 1, 0.6, 1);
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * -2.0,
                                  BGame::m_sPizzas[BGame::m_nPizza],
                                  BTextRenderer::ALIGN_LEFT, 0.6, 1, 0.6, 1);
  BUI::TextRenderer()->StopRenderingText();
  glPopMatrix();
}

void CVeraView::DrawDeliveryInfo() {
  double dCharWidth = BUI::TextRenderer()->GetCharWidth();
  double dCharHeight = BUI::TextRenderer()->GetCharHeight();
  glPushMatrix();
  // glTranslated(rectWnd.Width() / 2, rectWnd.Height() / 2, 0);
  glTranslated(window_width / 2, window_height / 2, 0);

  glPushMatrix();
  glTranslated(0, dCharHeight / 2, 0);
  DrawPanel(dCharWidth * 34, dCharHeight * 9);
  glPopMatrix();

  BUI::TextRenderer()->StartRenderingText();
  // BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 3.0,  "> > > DELIVERY
  // SUMMARY < < <", BTextRenderer::TTextAlign::ALIGN_CENTER, 0.55, 0.6, 0.85,
  // 1);
  BUI::TextRenderer()->DrawTextAt(
      0, dCharHeight * 3.0, "> > > DELIVERY SUMMARY < < <",
      BTextRenderer::ALIGN_CENTER, 0.55, 0.6, 0.85, 1);
  // BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 1.0,  "CLIENT:",
  // BTextRenderer::TTextAlign::ALIGN_RIGHT, 1 * 0.5, 0.6 * 0.5, 0.6 * 0.5, 1);
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 1.0,
                                  "CLIENT:", BTextRenderer::ALIGN_RIGHT,
                                  1 * 0.5, 0.6 * 0.5, 0.6 * 0.5, 1);
  // BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 0,    "DISTANCE:",
  // BTextRenderer::TTextAlign::ALIGN_RIGHT, 1 * 0.5, 0.6 * 0.5, 0.6 * 0.5, 1);
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 0,
                                  "DISTANCE:", BTextRenderer::ALIGN_RIGHT,
                                  1 * 0.5, 0.6 * 0.5, 0.6 * 0.5, 1);
  // BUI::TextRenderer()->DrawTextAt(0, dCharHeight * -1.0, "PROFIT:",
  // BTextRenderer::TTextAlign::ALIGN_RIGHT, 1, 0.6, 0.6, 1);
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * -1.0,
                                  "PROFIT:", BTextRenderer::ALIGN_RIGHT, 1, 0.6,
                                  0.6, 1);
  // BUI::TextRenderer()->DrawTextAt(0, dCharHeight * -2.0, "PIZZA:",
  // BTextRenderer::TTextAlign::ALIGN_RIGHT, 1 * 0.5, 0.6 * 0.5, 0.6 * 0.5, 1);
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * -2.0,
                                  "PIZZA:", BTextRenderer::ALIGN_RIGHT, 1 * 0.5,
                                  0.6 * 0.5, 0.6 * 0.5, 1);

  string sTmp;
  // sTmp.Format("%s", BGame::GetCurrentClient()->m_sName);
  sTmp.assign(BGame::GetCurrentClient()->m_sName);
  // BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 1.0,  sTmp,
  // BTextRenderer::TTextAlign::ALIGN_LEFT, 0.6 * 0.5, 1 * 0.5, 0.6 * 0.5, 1);
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 1.0, sTmp,
                                  BTextRenderer::ALIGN_LEFT, 0.6 * 0.5, 1 * 0.5,
                                  0.6 * 0.5, 1);
  // sTmp.Format("%.1lf NU", BGame::m_dLastDeliveryDistance);
  stringstream format;
  format.precision(5);
  format.unsetf(ios::floatfield);
  format << BGame::m_dLastDeliveryDistance << " NU";
  sTmp = format.str();
  // BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 0,    sTmp,
  // BTextRenderer::TTextAlign::ALIGN_LEFT, 0.6 * 0.5, 1 * 0.5, 0.6 * 0.5, 1);
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 0, sTmp,
                                  BTextRenderer::ALIGN_LEFT, 0.6 * 0.5, 1 * 0.5,
                                  0.6 * 0.5, 1);
  // sTmp.Format("%.0lf $", BGame::m_dLastProfit);
  format.str("");
  format << (int)BGame::m_dLastProfit << " $";
  sTmp = format.str();
  // BUI::TextRenderer()->DrawTextAt(0, dCharHeight * -1.0, sTmp,
  // BTextRenderer::TTextAlign::ALIGN_LEFT, 0.6, 1, 0.6, 1);
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * -1.0, sTmp,
                                  BTextRenderer::ALIGN_LEFT, 0.6, 1, 0.6, 1);
  // BUI::TextRenderer()->DrawTextAt(0, dCharHeight * -2.0,
  // BGame::m_sPizzas[BGame::m_nPizza], BTextRenderer::TTextAlign::ALIGN_LEFT,
  // 0.6 * 0.5, 1 * 0.5, 0.6 * 0.5, 1);
  BUI::TextRenderer()->DrawTextAt(
      0, dCharHeight * -2.0, BGame::m_sPizzas[BGame::m_nPizza],
      BTextRenderer::ALIGN_LEFT, 0.6 * 0.5, 1 * 0.5, 0.6 * 0.5, 1);
  BUI::TextRenderer()->StopRenderingText();
  glPopMatrix();
}

void CVeraView::DrawPanel(double dWidth, double dHeight, double dRed,
                          double dGreen, double dBlue, double dAlpha) {
  // Draw an empty panel centered about the current origin

  // Draw background
  glDisable(GL_TEXTURE_2D);
  OpenGLHelpers::SetColorFull(dRed, dGreen, dBlue, dAlpha);
  glBegin(GL_TRIANGLE_STRIP);
  glVertex3f(-dWidth / 2, -dHeight / 2, 0);
  glVertex3f(-dWidth / 2, dHeight / 2, 0);
  glVertex3f(dWidth / 2, -dHeight / 2, 0);
  glVertex3f(dWidth / 2, dHeight / 2, 0);
  glEnd();

  // Draw borders
  OpenGLHelpers::SetColorFull(1, 1, 1, 1);
  OpenGLHelpers::SwitchToTexture(0);
  // BTextures::Use(BTextures::Texture::PANEL);
  BTextures::Use(BTextures::PANEL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR); // not mipmapped

  glBegin(GL_QUADS);

  // Corners
  OpenGLHelpers::SetTexCoord(395.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(-dWidth / 2 - 14, dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(-dWidth / 2 - 14, dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(-dWidth / 2 + 14, dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(395.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(-dWidth / 2 + 14, dHeight / 2 + 14, 0);

  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(-dWidth / 2 - 14, -dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(115.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(-dWidth / 2 - 14, -dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(115.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(-dWidth / 2 + 14, -dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(-dWidth / 2 + 14, -dHeight / 2 + 14, 0);

  OpenGLHelpers::SetTexCoord(395.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(dWidth / 2 - 14, dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(dWidth / 2 - 14, dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(dWidth / 2 + 14, dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(395.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(dWidth / 2 + 14, dHeight / 2 + 14, 0);

  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(dWidth / 2 - 14, -dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(115.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(dWidth / 2 - 14, -dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(115.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(dWidth / 2 + 14, -dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(dWidth / 2 + 14, -dHeight / 2 + 14, 0);

  // Left and right border

  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(-dWidth / 2 - 14, dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(-dWidth / 2 - 14, -dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(-dWidth / 2 + 14, -dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(-dWidth / 2 + 14, dHeight / 2 - 14, 0);

  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(dWidth / 2 - 14, dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(dWidth / 2 - 14, -dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(dWidth / 2 + 14, -dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(dWidth / 2 + 14, dHeight / 2 - 14, 0);

  // Top and bottom borders
  OpenGLHelpers::SetTexCoord(395.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(-dWidth / 2 + 14, dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(-dWidth / 2 + 14, dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 254.0) / 512.0);
  glVertex3f(0, dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(395.0 / 512.0, (512.0 - 254.0) / 512.0);
  glVertex3f(0, dHeight / 2 + 14, 0);

  OpenGLHelpers::SetTexCoord(395.0 / 512.0, (512.0 - 254.0) / 512.0);
  glVertex3f(0, dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 254.0) / 512.0);
  glVertex3f(0, dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(dWidth / 2 - 14, dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(395.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(dWidth / 2 - 14, dHeight / 2 + 14, 0);

  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(-dWidth / 2 + 14, -dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(115.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(-dWidth / 2 + 14, -dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(115.0 / 512.0, (512.0 - 254.0) / 512.0);
  glVertex3f(0, -dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 254.0) / 512.0);
  glVertex3f(0, -dHeight / 2 + 14, 0);

  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 254.0) / 512.0);
  glVertex3f(0, -dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(115.0 / 512.0, (512.0 - 254.0) / 512.0);
  glVertex3f(0, -dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(115.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(dWidth / 2 - 14, -dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(dWidth / 2 - 14, -dHeight / 2 + 14, 0);

  glEnd();
}

void CVeraView::DrawMouseCursor() {
  // POINT pntMouse;
  SDL_Rect pntMouse;
  // GetCursorPos(&pntMouse);
  pntMouse.x = mouse_x;
  pntMouse.y = mouse_y;
  // pntMouse.y = rectWnd.Height() - pntMouse.y;
  pntMouse.y = window_height - pntMouse.y;

  double dX = 458.0 / 512.0;
  if (g_bMouseButtonDown) {
    dX = 407.0 / 512.0;
  }

  glPushMatrix();
  glTranslatef(pntMouse.x - 10, pntMouse.y + 10, 0);
  OpenGLHelpers::SetColor(1, 1, 1, 1);
  OpenGLHelpers::SwitchToTexture(0);
  // BTextures::Use(BTextures::Texture::PANEL);
  BTextures::Use(BTextures::PANEL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR); // not mipmapped
  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(dX, 54.0 / 512);
  glVertex3f(0, 0, 0);
  OpenGLHelpers::SetTexCoord(dX, 0.0 / 512);
  glVertex3f(0, -54, 0);
  OpenGLHelpers::SetTexCoord(dX + (54.0 / 512.0), 54.0 / 512);
  glVertex3f(54, 0, 0);
  OpenGLHelpers::SetTexCoord(dX + (54.0 / 512.0), 0.0 / 512);
  glVertex3f(54, -54, 0);
  glEnd();
  glPopMatrix();
}

