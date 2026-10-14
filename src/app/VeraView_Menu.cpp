// VeraView_Menu.cpp - menu rendering & menu input handlers.

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
int TerrainLoadThread(void *);

void CVeraView::OnDrawIntro() {
  // FIXME
  /*HDC hDC = pDC->GetSafeHdc();
  wglMakeCurrent(hDC, m_hGLRC);
  ::ReleaseDC(GetSafeHwnd(), hDC);*/
}

void CVeraView::OnDrawCurrentMenu() {

  if (BGame::m_bQuitPending) {
    // AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_FILE_CLOSE, 0);
    setExit();
  }

  if (!BGame::m_bMenusCreated) {
    BGame::SetupMenus();

    // Setup for first menu
    BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu),
                                 &CVeraView::OnKeyDownCurrentMenu);
    // Invalidate();

    // Start menu music
    SoundModule::StartMenuMusic();
    SoundModule::SetMenuMusicVolume(
        int(double(BGame::m_nMusicVolume) / 100.0 * 255.0));

    return;
  }

  // EnterCriticalSection(&(BGame::m_csMutex));
  SDL_LockMutex(BGame::m_csMutex);

  // Check if we need to change display settings
  // static bResolutionChanged = false;
  static bool bResolutionChanged = false;
  // FIXME
  /*if(!bResolutionChanged) {
    bResolutionChanged = true;
    DEVMODE devmode;
    EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);
    if((devmode.dmPelsWidth  != (DWORD)BGame::m_nDispWidth) ||
       (devmode.dmPelsHeight != (DWORD)BGame::m_nDispHeight) ||
       (devmode.dmBitsPerPel != (DWORD)BGame::m_nDispBits) ||
       (devmode.dmDisplayFrequency != (DWORD)BGame::m_nDispHz)) {
      devmode.dmPelsWidth = (DWORD)BGame::m_nDispWidth;
      devmode.dmPelsHeight = (DWORD)BGame::m_nDispHeight;
      devmode.dmBitsPerPel = (DWORD)BGame::m_nDispBits;
      devmode.dmDisplayFrequency = (DWORD)BGame::m_nDispHz;
      ChangeDisplaySettings(&devmode, CDS_FULLSCREEN);
      AfxGetMainWnd()->SetWindowPos(NULL, -2, -2, BGame::m_nDispWidth + 4,
  BGame::m_nDispHeight + 4, 0);
    }
  }*/

  // Cater for Precaching Terrain menu special needs
  // if(BGame::m_pMenuCurrent->m_type == BMenu::TType::PRECACHING_TERRAIN) {
  if (BGame::m_pMenuCurrent->m_type == BMenu::PRECACHING_TERRAIN) {
    if (!BGame::m_bGameLoading) {
      // Initiate game load (start separate thread)
      BGame::m_bGameReadyToStart = false;
      BGame::m_bGameLoading = true;

      BGame::GetSimulation()->GetScene()->LoadSceneFromFile(BGame::m_sScene);
      BGame::SetupScene();

      // m_pThreadLoading = AfxBeginThread(TerrainLoadThread, 0,
      // THREAD_PRIORITY_LOWEST);
      // m_pThreadLoading = AfxBeginThread(TerrainLoadThread, 0); // XP fix?
      m_pThreadLoading =
          SDL_CreateThread(TerrainLoadThread, "TerrainLoad", (void *)this);
    } else {
      if (BGame::m_bGameReadyToStart) {

        // Prepare to enter game
        BGame::GetPlayer()->LoadStateFile();
        BGame::GetSimulation()->GetScene()->PlaceTerrainObjects();
        BGame::GetSimulation()->GetTerrain()->CreateTerrainDisplayLists();
        BGame::GetSimulation()->GetVehicle()->LoadVehicleFromFile(
            BGame::m_sVehicle);
        BGame::GetSimulation()->GetVehicle()->m_dFuel =
            BGame::GetPlayer()->m_dFuel;
        BGame::GetSimulation()->GetVehicle()->m_dKerosine =
            BGame::GetPlayer()->m_dKerosine;
        BGame::GetSimulation()->GetVehicle()->PreProcessVisualization();
        // (Dunno why, but this needs to be done again here)
        for (int o = 0; o < BGame::GetSimulation()->GetScene()->m_nObjects;
             ++o) {
          BGame::GetSimulation()->GetScene()->m_pObjects[o].RecreateShadow();
        }

        BMessages::RemoveAll();

        // Init delivery
        // BGame::m_state = BGame::TState::PICKUP;
        BGame::m_state = BGame::PICKUP;
        BGame::m_dPizzaTemp = 0.0;

        // Move vehicle to start location
        BVector vStartLocation =
            BGame::GetSimulation()->GetScene()->m_vStartLocation;
        BVector vSceneLoc;
        if (BGame::GetPlayer()->LoadCurrentSceneInfo(vSceneLoc)) {
          vStartLocation = vSceneLoc;
        }
        BGame::GetSimulation()->UpdateCarLocation();
        BVector vLoc = BGame::GetSimulation()->GetVehicle()->m_vLocation;
        BVector val_bvector = vStartLocation - vLoc;
        // BGame::GetSimulation()->GetVehicle()->Move(vStartLocation - vLoc);
        BGame::GetSimulation()->GetVehicle()->Move(val_bvector);
        BGame::GetSimulation()->UpdateCar();

        // Make terrain valid so that the first simulation will work
        BGame::GetSimulation()->GetTerrain()->MakeTerrainValid(
            BGame::GetSimulation()->GetVehicle()->m_vLocation,
            BGame::GetSimulation()->GetCamera()->m_vLocation,
            BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward, true,
            false, false);
        BVector vOnGround, vNormal;
        BGame::GetSimulation()->EnsureVehicleIsOverGround();
        // BGame::GetSimulation()->GetCamera()->m_locMode =
        // BCamera::TCameraLoc::FOLLOW;
        BGame::GetSimulation()->GetCamera()->m_locMode = BCamera::FOLLOW;
        BGame::GetSimulation()->GetCamera()->m_vLocation =
            BGame::GetSimulation()->GetVehicle()->m_vLocation +
            BVector(-20, -10, 0);
        BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward =
            BGame::GetSimulation()->GetVehicle()->m_vLocation -
            BGame::GetSimulation()->GetCamera()->m_vLocation;
        BGame::GetSimulation()->GetCamera()->m_orientation.m_vUp =
            BVector(0, 0, -1);
        BGame::GetSimulation()->GetCamera()->m_orientation.m_vRight =
            BGame::GetSimulation()
                ->GetCamera()
                ->m_orientation.m_vUp.CrossProduct(
                    BGame::GetSimulation()
                        ->GetCamera()
                        ->m_orientation.m_vForward);
        // Make terrain valid so that the first simulation will work
        BGame::GetSimulation()->GetTerrain()->MakeTerrainValid(
            BGame::GetSimulation()->GetVehicle()->m_vLocation,
            BGame::GetSimulation()->GetCamera()->m_vLocation,
            BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward, true,
            false, false);

        // Init earthquake
        BGame::m_bEarthquakeFactor = 0.0;
        BGame::m_bEarthquakeActive = false;
        BGame::m_bEarthquakeNextStart =
            (double)SDL_GetTicks() + 1000 * (60.0 + Random(60.0));

        // Init wind particles
        if (BGame::m_bWindActive) {
          BGame::UpdateWindParticles(true);
        }

        // Switch to game mode
        SoundModule::StopMenuMusic();
        SoundModule::StartEngineSound();
        SoundModule::SetVehicleSoundsVolume(
            int(double(BGame::m_nVehicleVolume) / 100.0 * 255.0));
        SoundModule::SetEngineSoundVolume(255.0);
        ReturnPressedOnCurrentMenu();
        m_bInitClock = true;
      }
    }
  }

  // Preparations stuff

  glClearColor(0, 0, 0, 0);

  /*if(m_hCursor) {
    ::SetCursor(m_hCursor);
  }
  ShowCursor(FALSE);

  HDC hDC = pDC->GetSafeHdc();
  wglMakeCurrent(hDC, m_hGLRC); // Setup as opengl dc to get the black and white
  stuff work

  CRect rectWnd;
  GetClientRect(&rectWnd);*/
  SDL_ShowCursor(0);

  // Init OpenGL
  glDrawBuffer(GL_BACK);

  // Reset OpenGL
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // glViewport(0, 0, (GLint) rectWnd.Width(), (GLint) rectWnd.Height());
  glViewport(0, 0, (GLint)window_width, (GLint)window_height);
  gluLookAt(0, -5, 0, 0, 0, 0, 0, 0, -1);

  GLfloat fLight1PositionG[4];
  fLight1PositionG[0] = (GLfloat)0.25;
  fLight1PositionG[1] = (GLfloat)0;
  fLight1PositionG[2] = (GLfloat)-1;
  fLight1PositionG[3] =
      (GLfloat)0; /* w=0 -> directional light (not positional) */
  glLightfv(GL_LIGHT0, GL_POSITION, fLight1PositionG);

  bool bScrolling = false;
  double dScrollPhase = 0;
  unsigned clockNow = SDL_GetTicks();
  if ((clockNow - m_clockMenuScroll) < 500.0) {
    bScrolling = true;
    dScrollPhase = double(clockNow - m_clockMenuScroll) / 500.0;
    dScrollPhase =
        (1.0 + sin(((dScrollPhase * 2.0) - 1.0) * 3.1415926 * 0.5)) * 0.5;
  }

  // Draw menu

  // Setup 2D rendering
  Setup2DRendering();

  // Draw background
  OpenGLHelpers::SetColorFull(0, 0, 0.5, 1);
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glDisable(GL_TEXTURE_2D);

  // double dPhase1 = sin(2.0 * 3.141592654 * double(clock() % 11031) /
  // 11030.0);
  double dPhase1 =
      sin(2.0 * 3.141592654 * double(SDL_GetTicks() % 11031) / 11030.0);
  // double dPhase2 = sin(2.0 * 3.141592654 * double(clock() % 17131) /
  // 17130.0);
  double dPhase2 =
      sin(2.0 * 3.141592654 * double(SDL_GetTicks() % 17131) / 17130.0);
  // double dPhase3 = sin(2.0 * 3.141592654 * double(clock() % 15131) /
  // 15130.0);
  double dPhase3 =
      sin(2.0 * 3.141592654 * double(SDL_GetTicks() % 15131) / 15130.0);
  // double dPhase4 = sin(2.0 * 3.141592654 * double(clock() % 12131) /
  // 12130.0);
  double dPhase4 =
      sin(2.0 * 3.141592654 * double(SDL_GetTicks() % 12131) / 12130.0);

  glBegin(GL_TRIANGLE_STRIP);
  glColor4d(dPhase1 * 0.3, 0, 0, 1);
  glVertex3f(0, 0, 0);
  glColor4d(0, dPhase2 * 0.2, 0, 1);
  // glVertex3f(0, rectWnd.Height(), 0);
  glVertex3f(0, window_height, 0);
  glColor4d(0, 0, dPhase3 * 0.2, 1);
  // glVertex3f(rectWnd.Width(), 0, 0);
  glVertex3f(window_width, 0, 0);
  glColor4d(dPhase4 * 0.35, dPhase4 * 0.2, 0, 1);
  // glVertex3f(rectWnd.Width(), rectWnd.Height(), 0);
  glVertex3f(window_width, window_height, 0);
  glEnd();

  glColor4d(1, 1, 1, 1);
  glDisable(GL_COLOR_MATERIAL);

  if (BGame::m_pMenuCurrent != &(BGame::m_menuCredits)) {
    { //  Draw Alpha/Beta text (REMOVE WHEN GAME READY!) *

      BUI::TextRenderer()->StartRenderingText();
      string sText = "BETA VERSION! (build 6)";
      // BUI::TextRenderer()->DrawSmallTextAt(rectWnd.Width() / 2,
      // rectWnd.Height() - 40, sText, sText.GetLength(),
      // BTextRenderer::TTextAlign::ALIGN_CENTER, 0.75, 0, 0, 1);
      BUI::TextRenderer()->DrawSmallTextAt(
          window_width / 2, window_height - 40, sText, sText.length(),
          BTextRenderer::ALIGN_CENTER, 0.75, 0, 0, 1);
      BUI::TextRenderer()->StopRenderingText();
    }

    // Always draw the copyright on bottom
    OpenGLHelpers::SetColorFull(0.65, 0.7, 0.75, 1);
    OpenGLHelpers::SwitchToTexture(0);
    // BTextures::Use(BTextures::Texture::MENU_TITLES);
    BTextures::Use(BTextures::MENU_TITLES);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR); // not mipmapped

    glPushMatrix();
    // glTranslated((rectWnd.Width() - 512) / 2, 0, 0);
    glTranslated((window_width - 512) / 2, 0, 0);
    glBegin(GL_TRIANGLE_STRIP);
    OpenGLHelpers::SetTexCoord(0, 0);
    glVertex3f(0, 0, 0);
    OpenGLHelpers::SetTexCoord(0, 30.0 / 512.0);
    glVertex3f(0, 30, 0);
    OpenGLHelpers::SetTexCoord(1, 0);
    glVertex3f(512, 0, 0);
    OpenGLHelpers::SetTexCoord(1, 30.0 / 512.0);
    glVertex3f(512, 30, 0);
    glEnd();
    glPopMatrix();
  }

  if (bScrolling) {
    // Draw previous and new menu scrolling
    glPushMatrix();
    if (m_scrollDir == SCROLL_RIGHT) {
      // glTranslated(-int(double(rectWnd.Width() * dScrollPhase)), 0, 0);
      glTranslated(-int(double(window_width * dScrollPhase)), 0, 0);
    } else if (m_scrollDir == SCROLL_LEFT) {
      // glTranslated(+int(double(rectWnd.Width() * dScrollPhase)), 0, 0);
      glTranslated(+int(double(window_width * dScrollPhase)), 0, 0);
    } else if (m_scrollDir == SCROLL_UP) {
      // glTranslated(0, -int(double(rectWnd.Height() * dScrollPhase)), 0);
      glTranslated(0, -int(double(window_height * dScrollPhase)), 0);
    } else if (m_scrollDir == SCROLL_DOWN) {
      // glTranslated(0, +int(double(rectWnd.Height() * dScrollPhase)), 0);
      glTranslated(0, +int(double(window_height * dScrollPhase)), 0);
    }
    DrawMenu(BGame::m_pMenuPrevious);
    glPopMatrix();

    glPushMatrix();
    if (m_scrollDir == SCROLL_RIGHT) {
      // glTranslated(int(double(rectWnd.Width() * (1.0 - dScrollPhase))), 0,
      // 0);
      glTranslated(int(double(window_width * (1.0 - dScrollPhase))), 0, 0);
    } else if (m_scrollDir == SCROLL_LEFT) {
      // glTranslated(-int(double(rectWnd.Width() * (1.0 - dScrollPhase))), 0,
      // 0);
      glTranslated(-int(double(window_width * (1.0 - dScrollPhase))), 0, 0);
    } else if (m_scrollDir == SCROLL_UP) {
      // glTranslated(0, +int(double(rectWnd.Height() * (1.0 - dScrollPhase))),
      // 0);
      glTranslated(0, +int(double(window_height * (1.0 - dScrollPhase))), 0);
    } else if (m_scrollDir == SCROLL_DOWN) {
      // glTranslated(0, -int(double(rectWnd.Height() * (1.0 - dScrollPhase))),
      // 0);
      glTranslated(0, -int(double(window_height * (1.0 - dScrollPhase))), 0);
    }
    DrawMenu(BGame::m_pMenuCurrent);
    glPopMatrix();
  } else {
    DrawMenu(BGame::m_pMenuCurrent);
  }

  // Finish draw

  glFinish();

  SDL_GL_SwapWindow(window);

  // LeaveCriticalSection(&(BGame::m_csMutex));
  SDL_UnlockMutex(BGame::m_csMutex);

  // And again and again and...
  if ((BGame::m_pMenuCurrent == &(BGame::m_menuPrecachingTerrain)) &&
      !BGame::m_bMultiProcessor) {
    // Sleep(1);
    SDL_Delay(1);
  }
  // Invalidate();
}

void CVeraView::DrawMenu(BMenu *pMenu) {

  if (pMenu) {

    double dCharHeight = BUI::TextRenderer()->GetCharHeight();
    double dCharWidth = BUI::TextRenderer()->GetCharWidth();

    // Draw menu title

    glPushMatrix();
    // int nThird = rectWnd.Height() / 6;
    int nThird = window_height / 6;
    // glTranslated((rectWnd.Width() - pMenu->m_nTitleWidth) / 2,
    glTranslated((window_width - pMenu->m_nTitleWidth) / 2,
                 // rectWnd.Height() - nThird - pMenu->m_nTitleHeight / 2,
                 window_height - nThird - pMenu->m_nTitleHeight / 2, 0);

    if (pMenu->m_nTitleWidth) {
      OpenGLHelpers::SwitchToTexture(0);
      // BTextures::Use(BTextures::Texture::MENU_TITLES);
      BTextures::Use(BTextures::MENU_TITLES);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                      GL_LINEAR); // not mipmapped

      OpenGLHelpers::SetColorFull(1, 1, 1, 1);
      glBegin(GL_TRIANGLE_STRIP);
      OpenGLHelpers::SetTexCoord(pMenu->m_dTitleX, pMenu->m_dTitleY);
      glVertex3f(0, 0, 0);
      OpenGLHelpers::SetTexCoord(pMenu->m_dTitleX,
                                 pMenu->m_dTitleY +
                                     (double(pMenu->m_nTitleHeight) / 512.0));
      glVertex3f(0, pMenu->m_nTitleHeight, 0);
      OpenGLHelpers::SetTexCoord(pMenu->m_dTitleX +
                                     (double(pMenu->m_nTitleWidth) / 512.0),
                                 pMenu->m_dTitleY);
      glVertex3f(pMenu->m_nTitleWidth, 0, 0);
      OpenGLHelpers::SetTexCoord(
          pMenu->m_dTitleX + (double(pMenu->m_nTitleWidth) / 512.0),
          pMenu->m_dTitleY + (double(pMenu->m_nTitleHeight) / 512.0));
      glVertex3f(pMenu->m_nTitleWidth, pMenu->m_nTitleHeight, 0);
      glEnd();
    }

    // Draw separator
    if (pMenu->m_bDrawLine) {
      glEnable(GL_COLOR_MATERIAL);
      glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
      glDisable(GL_TEXTURE_2D);
      glBegin(GL_LINE_STRIP);
      glColor3d(0, 0, 0);
      glVertex3f(-pMenu->m_nTitleWidth / 2 - 10, 0, 0);
      glColor3d(1, 1, 1);
      glVertex3f(-pMenu->m_nTitleWidth / 2, 0, 0);
      glColor3d(0.25, 0.35, 0.4);
      glVertex3f(pMenu->m_nTitleWidth + pMenu->m_nTitleWidth / 2, 0, 0);
      glColor3d(0, 0, 0);
      glVertex3f(pMenu->m_nTitleWidth + pMenu->m_nTitleWidth / 2 + 10, 0, 0);
      glEnd();
      glTranslated(0, 4, 0);
      glBegin(GL_LINE_STRIP);
      glColor3d(0, 0, 0);
      glVertex3f(-pMenu->m_nTitleWidth / 2 - 10, 0, 0);
      glColor3d(1, 1, 1);
      glVertex3f(-pMenu->m_nTitleWidth / 2, 0, 0);
      glColor3d(0.25, 0.35, 0.4);
      glVertex3f(pMenu->m_nTitleWidth + pMenu->m_nTitleWidth / 2, 0, 0);
      glColor3d(0, 0, 0);
      glVertex3f(pMenu->m_nTitleWidth + pMenu->m_nTitleWidth / 2 + 10, 0, 0);
      glEnd();
      glColor3d(1, 1, 1);
      glDisable(GL_COLOR_MATERIAL);
    }

    glPopMatrix();

    // Draw menu contents
    if (pMenu->m_listMenu.GetNofItems()) {
      int i;

      // if(pMenu->m_type == BMenu::TType::CHOOSE_VEHICLE) {
      if (pMenu->m_type == BMenu::CHOOSE_VEHICLE) {
        for (i = 0; i < pMenu->m_nItems; ++i) {
          // Check for owned vehicles
          string sVehicle;
          // sVehicle.Format(">%s<", pMenu->m_items[i].m_sText);
          sVehicle = ">" + pMenu->m_items[i].m_sText + "<";
          // if(BGame::GetPlayer()->m_sValidVehicles.Find(sVehicle) == -1) {
          if (BGame::GetPlayer()->m_sValidVehicles.find(sVehicle) == -1) {
            pMenu->m_items[i].m_bDisabled = true;
          } else {
            pMenu->m_items[i].m_bDisabled = false;
          }
        }
      }

      // pMenu->m_listMenu.DrawAt(rectWnd.Width() / 2, rectWnd.Height() / 2,
      // pMenu->m_align, 1, 1, 1, false);
      pMenu->m_listMenu.DrawAt(window_width / 2, window_height / 2,
                               pMenu->m_align, 1, 1, 1, false);

      // If there are menu items that have an associated list value,
      // draw them also
      for (i = 0; i < pMenu->m_nItems; ++i) {
        // if(pMenu->m_items[i].m_type == BMenuItem::TType::STRING_FROM_LIST) {
        if (pMenu->m_items[i].m_type == BMenuItem::STRING_FROM_LIST) {
          // Draw list string
          // DrawMenuItemTextAtRelPos(rectWnd.Width() / 2,
          DrawMenuItemTextAtRelPos(window_width / 2,
                                   // rectWnd.Height() / 2,
                                   window_height / 2, pMenu->m_nItems, i,
                                   &(pMenu->m_items[i]));
          //} else if(pMenu->m_items[i].m_type == BMenuItem::TType::SLIDER) {
        } else if (pMenu->m_items[i].m_type == BMenuItem::SLIDER) {
          // Draw slider
          // DrawMenuItemSliderAtRelPos(rectWnd.Width() / 2,
          DrawMenuItemSliderAtRelPos(window_width / 2,
                                     // rectWnd.Height() / 2,
                                     window_height / 2, pMenu->m_nItems, i,
                                     &(pMenu->m_items[i]));
        }
      }

      // Draw open subitem
      for (i = 0; i < pMenu->m_nItems; ++i) {
        if (pMenu->m_items[i].m_bOpen) {
          // if(pMenu->m_items[i].m_type == BMenuItem::TType::STRING_FROM_LIST)
          // {
          if (pMenu->m_items[i].m_type == BMenuItem::STRING_FROM_LIST) {
            // Draw submenu
            // pMenu->m_items[i].m_listMenu.DrawAt(rectWnd.Width() / 2 + 35,
            pMenu->m_items[i].m_listMenu.DrawAt(
                window_width / 2 + 35,
                // rectWnd.Height() / 2 + double(i) * -dCharHeight +
                // (dCharHeight * double(pMenu->m_nItems)) / 2.0,
                window_height / 2 + double(i) * -dCharHeight +
                    (dCharHeight * double(pMenu->m_nItems)) / 2.0,
                // BTextRenderer::TTextAlign::ALIGN_LEFT,
                BTextRenderer::ALIGN_LEFT, 1, 1, 1, true, true);
            //} else if(pMenu->m_items[i].m_type == BMenuItem::TType::SLIDER) {
          } else if (pMenu->m_items[i].m_type == BMenuItem::SLIDER) {
            // Draw open slider
            // DrawMenuItemSliderAtRelPos(rectWnd.Width() / 2,
            DrawMenuItemSliderAtRelPos(window_width / 2,
                                       // rectWnd.Height() / 2,
                                       window_height / 2, pMenu->m_nItems, i,
                                       &(pMenu->m_items[i]));
          }
        }
      }

      // Draw associated image, if one is available
      string sTmp;
      int nSelected = pMenu->m_listMenu.GetSelectedItem(sTmp);
      if ((nSelected != -1) &&
          (pMenu->m_items[nSelected].m_nAssocImage != -1)) {
        OpenGLHelpers::SetColorFull(1, 1, 1, 1);
        OpenGLHelpers::SwitchToTexture(0);
        BTextures::Use(pMenu->m_items[nSelected].m_nAssocImage);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR); // not mipmapped
        glPushMatrix();
        // glTranslated(rectWnd.Width() / 2 + rectWnd.Width() / 40,
        glTranslated(window_width / 2 + window_width / 40,
                     // rectWnd.Height() / 2,
                     window_height / 2, 0);
        glBegin(GL_TRIANGLE_STRIP);
        OpenGLHelpers::SetTexCoord(0, 0);
        glVertex3f(0, -128, 0);
        OpenGLHelpers::SetTexCoord(0, 1);
        glVertex3f(0, 128, 0);
        OpenGLHelpers::SetTexCoord(1, 0);
        glVertex3f(256, -128, 0);
        OpenGLHelpers::SetTexCoord(1, 1);
        glVertex3f(256, 128, 0);
        glEnd();

        // If disabled, draw veil over image
        if (pMenu->m_items[nSelected].m_bDisabled) {
          OpenGLHelpers::SetColorFull(0, 0, 0, 0.5);
          glDisable(GL_TEXTURE_2D);
          glBegin(GL_LINES);
          for (i = 1; i < 256; i += 2) {
            glVertex3f(0, -128 + i, 0);
            glVertex3f(256, -128 + i, 0);
          }
          OpenGLHelpers::SetColorFull(0, 0, 0, 0.25);
          for (i = 0; i < 256; i += 2) {
            glVertex3f(0, -128 + i, 0);
            glVertex3f(256, -128 + i, 0);
          }
          glEnd();
          OpenGLHelpers::SetColorFull(1, 1, 1, 1);
        }

        glPopMatrix();
      }
    }

    // Draw credits images with fade, if showing credits
    // if(pMenu->m_type == BMenu::TType::CREDITS) {
    if (pMenu->m_type == BMenu::CREDITS) {
      // Find the active item(s)
      int i;
      int nItems = 0;
      int nItemsToDraw[2];
      double dItemsAlpha[2];
      nItemsToDraw[0] = -1;
      nItemsToDraw[1] = -1;
      unsigned clockNow = SDL_GetTicks();
      double dCurSecond = double(clockNow - pMenu->m_clockStarted) / 1000.0;

      if ((BGame::m_pMenuCurrent == &(BGame::m_menuCredits)) &&
          (dCurSecond > (1 + pMenu->m_items[pMenu->m_nItems - 1].m_nValue +
                         pMenu->m_items[pMenu->m_nItems - 1].m_nValue2))) {
        // Return to main menu
        BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
        // BGame::m_pMenuPrevious = &(BGame::m_menuCredits);
        BGame::m_pMenuCurrent = &(BGame::m_menuMain);
        BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu),
                                     &CVeraView::OnKeyDownCurrentMenu);
        StartMenuScroll(SCROLL_UP);
        // Invalidate();
        return;
      } else {
        for (i = 0; i < pMenu->m_nItems; ++i) {
          double dFrom = double(pMenu->m_items[i].m_nValue);
          double dTo = dFrom + double(pMenu->m_items[i].m_nValue2);
          if ((dCurSecond >= dFrom) && (dCurSecond <= (dTo - 2.0))) {
            nItemsToDraw[nItems] = i;
            dItemsAlpha[nItems] = 1.0;
            ++nItems;
          } else if ((dCurSecond >= (dFrom - 2.0)) && (dCurSecond <= (dFrom))) {
            // Fading in
            nItemsToDraw[nItems] = i;
            dItemsAlpha[nItems] = 0.5 * (dCurSecond - (dFrom - 2.0));
            ++nItems;
          } else if ((dCurSecond >= (dTo - 2.0)) && (dCurSecond <= (dTo))) {
            // Fading out
            nItemsToDraw[nItems] = i;
            dItemsAlpha[nItems] = 1.0 - (0.5 * (dCurSecond - (dTo - 2.0)));
            ++nItems;
          }
          if (nItems == 2) {
            break;
          }
        }

        // Draw the two menu items (their images)
        glPushMatrix();
        // glTranslated(rectWnd.Width() / 2 - 256.0,
        glTranslated(window_width / 2 - 256.0,
                     // rectWnd.Height() / 2,
                     window_height / 2, 0);
        for (i = 0; i < nItems; ++i) {
          if (pMenu->m_items[nItemsToDraw[i]].m_nAssocImage >= 0) {
            OpenGLHelpers::SetColorFull(1, 1, 1, dItemsAlpha[i]);
            OpenGLHelpers::SwitchToTexture(0);
            BTextures::Use(pMenu->m_items[nItemsToDraw[i]].m_nAssocImage);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                            GL_LINEAR); // not mipmapped
            glBegin(GL_TRIANGLE_STRIP);
            OpenGLHelpers::SetTexCoord(0, 0);
            glVertex3f(0, -128, 0);
            OpenGLHelpers::SetTexCoord(0, 1);
            glVertex3f(0, 128, 0);
            OpenGLHelpers::SetTexCoord(1, 0);
            glVertex3f(512, -128, 0);
            OpenGLHelpers::SetTexCoord(1, 1);
            glVertex3f(512, 128, 0);
            glEnd();
          }
        }

        // Draw frames
        float dAlpha = 0.5;
        if ((dCurSecond < (pMenu->m_items[1].m_nValue - 2.0)) ||
            (dCurSecond > (pMenu->m_items[pMenu->m_nItems - 1].m_nValue))) {
          dAlpha = 0;
        }
        if ((dCurSecond >= (pMenu->m_items[1].m_nValue - 2.0)) &&
            (dCurSecond <= (pMenu->m_items[1].m_nValue))) {
          dAlpha =
              0.5 * 0.5 * (dCurSecond - (pMenu->m_items[1].m_nValue - 2.0));
        }
        if ((dCurSecond >=
             (pMenu->m_items[pMenu->m_nItems - 2].m_nValue +
              pMenu->m_items[pMenu->m_nItems - 2].m_nValue2 - 2.0)) &&
            (dCurSecond <= (pMenu->m_items[pMenu->m_nItems - 2].m_nValue +
                            pMenu->m_items[pMenu->m_nItems - 2].m_nValue2))) {
          dAlpha =
              0.5 *
              (1.0 -
               0.5 * (dCurSecond -
                      (pMenu->m_items[pMenu->m_nItems - 2].m_nValue +
                       pMenu->m_items[pMenu->m_nItems - 2].m_nValue2 - 2.0)));
        }

        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
        glDisable(GL_TEXTURE_2D);

        glBegin(GL_LINES);
        glColor4f(1, 1, 1, dAlpha);
        glVertex3f(0, -128, 0);
        glVertex3f(0, 127, 0);
        glVertex3f(511, -128, 0);
        glVertex3f(511, 127, 0);
        glVertex3f(0, -128, 0);
        glVertex3f(511, -128, 0);
        glVertex3f(0, 127, 0);
        glVertex3f(511, 127, 0);

        glColor4f(0, 0, 0, 0);
        glVertex3f(0, -128 - 40, 0);
        glColor4f(1, 1, 1, dAlpha);
        glVertex3f(0, -128, 0);

        glColor4f(0, 0, 0, 0);
        glVertex3f(511, -128 - 40, 0);
        glColor4f(1, 1, 1, dAlpha);
        glVertex3f(511, -128, 0);

        glColor4f(0, 0, 0, 0);
        glVertex3f(0, 127 + 40, 0);
        glColor4f(1, 1, 1, dAlpha);
        glVertex3f(0, 127, 0);

        glColor4f(0, 0, 0, 0);
        glVertex3f(511, 127 + 40, 0);
        glColor4f(1, 1, 1, dAlpha);
        glVertex3f(511, 126, 0);

        glColor4f(0, 0, 0, 0);
        glVertex3f(-40, -128, 0);
        glColor4f(1, 1, 1, dAlpha);
        glVertex3f(0, -128, 0);

        glColor4f(0, 0, 0, 0);
        glVertex3f(-40, 127, 0);
        glColor4f(1, 1, 1, dAlpha);
        glVertex3f(0, 127, 0);

        glColor4f(0, 0, 0, 0);
        glVertex3f(511 + 40, -128, 0);
        glColor4f(1, 1, 1, dAlpha);
        glVertex3f(511, -128, 0);

        glColor4f(0, 0, 0, 0);
        glVertex3f(511 + 40, 127, 0);
        glColor4f(1, 1, 1, dAlpha);
        glVertex3f(511, 127, 0);

        glEnd();

        glDisable(GL_COLOR_MATERIAL);

        glPopMatrix();
      }
    }

    // Draw progress indicator, if loading terrain
    // if(BGame::m_pMenuCurrent->m_type == BMenu::TType::PRECACHING_TERRAIN) {
    if (BGame::m_pMenuCurrent->m_type == BMenu::PRECACHING_TERRAIN) {
      OpenGLHelpers::SetColorFull(1, 1, 1, 1);
      glEnable(GL_COLOR_MATERIAL);
      glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
      glDisable(GL_TEXTURE_2D);

      // double dSmall = double(rectWnd.Height()) / 8.0;
      double dSmall = double(window_height) / 8.0;
      // double dBig = double(rectWnd.Height()) / 5.3;
      double dBig = double(window_height) / 5.3;

      glPushMatrix();
      // glTranslated(rectWnd.Width() / 2, rectWnd.Height() / 2, 0);
      glTranslated(window_width / 2, window_height / 2, 0);
      glBegin(GL_TRIANGLE_STRIP);
      int nLastAngle = int(BGame::GetRelativeProgress() * 360.0);
      if (nLastAngle > 360) {
        nLastAngle = 360;
      }
      int i;
      for (i = 0; i <= nLastAngle; ++i) {
        double dAngle = (double(i) / 360.0) * (2.0 * 3.141592654);
        glColor4d(0.8, 0.9, 1.0, 1);
        glVertex3f(sin(dAngle) * dSmall, cos(dAngle) * dSmall, 0);
        glColor4d(0, 0, 0, 0);
        glVertex3f(sin(dAngle) * dBig, cos(dAngle) * dBig, 0);
      }
      glEnd();
      glBegin(GL_TRIANGLE_STRIP);
      for (i = 0; i <= nLastAngle; ++i) {
        double dAngle = (double(i) / 360.0) * (2.0 * 3.141592654);
        glColor4d(0, 0, 0, 0);
        glVertex3f(sin(dAngle) * (dSmall - 2), cos(dAngle) * (dSmall - 2), 0);
        glColor4d(0.8, 0.9, 1.0, 1);
        glVertex3f(sin(dAngle) * dSmall, cos(dAngle) * dSmall, 0);
      }
      glEnd();
      glPopMatrix();
      glColor4d(1, 1, 1, 1);
      glDisable(GL_COLOR_MATERIAL);
    }

    // Finally, draw message box over menu, if needed
    // if(pMenu->m_type == BMenu::TType::CHOOSE_VEHICLE) {
    if (pMenu->m_type == BMenu::CHOOSE_VEHICLE) {
      glPushMatrix();
      // glTranslated(rectWnd.Width() / 2, rectWnd.Height() / 2, 0);
      glTranslated(window_width / 2, window_height / 2, 0);
      if (BGame::m_bBuyingVehicle) {
        DrawPanel(dCharWidth * 32, dCharHeight * 8);
        BUI::TextRenderer()->StartRenderingText();
        // BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 2, "Do you want to
        // buy", BTextRenderer::TTextAlign::ALIGN_CENTER);
        BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 2,
                                        "Do you want to buy",
                                        BTextRenderer::ALIGN_CENTER);
        string sTmp;
        string sVehicle;
        pMenu->m_listMenu.GetSelectedItem(sVehicle);
        // sTmp.Format("%s?", sVehicle);
        sTmp = sVehicle + "?";
        // BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 1, sTmp,
        // BTextRenderer::TTextAlign::ALIGN_CENTER);
        BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 1, sTmp,
                                        BTextRenderer::ALIGN_CENTER);
        // sTmp.Format("(It costs %.0lf. You have %.0lf)",
        // BGame::m_dPurchasePrice, BGame::GetPlayer()->m_dCash);
        stringstream format;
        format << "(It costs " << (int)BGame::m_dPurchasePrice << ". You have "
               << (int)BGame::GetPlayer()->m_dCash;
        sTmp = format.str();
        // BUI::TextRenderer()->DrawTextAt(0, 0, sTmp,
        // BTextRenderer::TTextAlign::ALIGN_CENTER);
        BUI::TextRenderer()->DrawTextAt(0, 0, sTmp,
                                        BTextRenderer::ALIGN_CENTER);
        BUI::TextRenderer()->StopRenderingText();
        // BGame::m_listYesNo.DrawAt(0, -dCharHeight * 2.5,
        // BTextRenderer::TTextAlign::ALIGN_CENTER, 1, 1, 1, false);
        BGame::m_listYesNo.DrawAt(0, -dCharHeight * 2.5,
                                  BTextRenderer::ALIGN_CENTER, 1, 1, 1, false);
      } else if (BGame::m_bCannotBuyVehicle) {
        DrawPanel(dCharWidth * 32, dCharHeight * 8);
        BUI::TextRenderer()->StartRenderingText();
        // BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 2, "You can not
        // buy", BTextRenderer::TTextAlign::ALIGN_CENTER);
        BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 2, "You can not buy",
                                        BTextRenderer::ALIGN_CENTER);
        string sTmp;
        string sVehicle;
        pMenu->m_listMenu.GetSelectedItem(sVehicle);
        // sTmp.Format("%s", sVehicle);
        sTmp = sVehicle;
        // BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 1, sTmp,
        // BTextRenderer::TTextAlign::ALIGN_CENTER);
        BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 1, sTmp,
                                        BTextRenderer::ALIGN_CENTER);
        // sTmp.Format("(It costs %.0lf. You have %.0lf)",
        // BGame::m_dPurchasePrice, BGame::GetPlayer()->m_dCash);
        stringstream format;
        format << "(It costs " << (int)BGame::m_dPurchasePrice << ". You have "
               << (int)BGame::GetPlayer()->m_dCash;
        sTmp = format.str();
        // BUI::TextRenderer()->DrawTextAt(0, 0, sTmp,
        // BTextRenderer::TTextAlign::ALIGN_CENTER);
        BUI::TextRenderer()->DrawTextAt(0, 0, sTmp,
                                        BTextRenderer::ALIGN_CENTER);
        BUI::TextRenderer()->StopRenderingText();
        // BGame::m_listOK.DrawAt(0, -dCharHeight * 2.5,
        // BTextRenderer::TTextAlign::ALIGN_CENTER, 1, 1, 1, false);
        BGame::m_listOK.DrawAt(0, -dCharHeight * 2.5,
                               BTextRenderer::ALIGN_CENTER, 1, 1, 1, false);
      }
      glPopMatrix();
    }
  }
}

void CVeraView::DrawMenuItemTextAtRelPos(int nX, int nY, int nItems, int nIndex,
                                         BMenuItem *pMenuItem) {
  double dCharHeight = BUI::TextRenderer()->GetCharHeight();
  double dCharWidth = BUI::TextRenderer()->GetCharWidth();

  if ((pMenuItem->m_nValue >= 0) &&
      (pMenuItem->m_nValue < pMenuItem->m_nAssocListItems)) {

    glPushMatrix();
    glTranslated(10, (dCharHeight * double(nItems)) / 2.0, 0);

    BUI::TextRenderer()->StartRenderingText();
    BUI::TextRenderer()->DrawTextAt(
        nX, nY + double(nIndex) * -dCharHeight,
        pMenuItem->m_sAssocListItems[pMenuItem->m_nValue],
        // BTextRenderer::TTextAlign::ALIGN_LEFT,
        BTextRenderer::ALIGN_LEFT, 0.7, 0.7, 0.7);
    BUI::TextRenderer()->StopRenderingText();
    glPopMatrix();
  }
}

void CVeraView::DrawMenuItemSliderAtRelPos(int nX, int nY, int nItems,
                                           int nIndex, BMenuItem *pMenuItem) {
  double dCharHeight = BUI::TextRenderer()->GetCharHeight();
  double dCharWidth = BUI::TextRenderer()->GetCharWidth();

  OpenGLHelpers::SwitchToTexture(1, true);
  OpenGLHelpers::SwitchToTexture(0, true);
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glDisable(GL_TEXTURE_2D);

  if ((pMenuItem->m_nValue >= 0) && (pMenuItem->m_nValue <= 100)) {

    glPushMatrix();
    glTranslated(nX + 10,
                 -1 + nY + double(nIndex) * -dCharHeight +
                     (dCharHeight * double(nItems)) / 2.0,
                 0);

    double dDimmer = 0.75;
    if (pMenuItem->m_bOpen) {
      double dAlpha = fabs(double(SDL_GetTicks() % 1000) - 500.0) / 500.0;
      dDimmer = 0.6 + 0.4 * dAlpha;
    }

    glBegin(GL_TRIANGLE_STRIP);

    glColor4d(dDimmer * 1, dDimmer * 1, dDimmer * 1, 1);
    glVertex3f(0, -dCharHeight / 6, 0);
    glColor4d(0, 0, 0, 0);
    glVertex3f(0, -dCharHeight / 6 - dCharHeight / 3, 0);
    glColor4d(dDimmer * 1, dDimmer * 1, dDimmer * 1, 1);
    glVertex3f(100, -dCharHeight / 6, 0);
    glColor4d(0, 0, 0, 0);
    glVertex3f(100, -dCharHeight / 6 - dCharHeight / 3, 0);

    glColor4d(dDimmer * 1, dDimmer * 1, dDimmer * 1, 1);
    glVertex3f(100, -dCharHeight / 6, 0);
    glColor4d(0, 0, 0, 0);
    glVertex3f(100 + dCharHeight / 3, -dCharHeight / 6, 0);
    glColor4d(dDimmer * 1, dDimmer * 1, dDimmer * 1, 1);
    glVertex3f(100, -dCharHeight / 6 + dCharHeight / 3, 0);
    glColor4d(0, 0, 0, 0);
    glVertex3f(100 + dCharHeight / 3, -dCharHeight / 6 + dCharHeight / 3, 0);

    glColor4d(dDimmer * 1, dDimmer * 1, dDimmer * 1, 1);
    glVertex3f(100, dCharHeight / 6, 0);
    glColor4d(0, 0, 0, 0);
    glVertex3f(100, dCharHeight / 6 + dCharHeight / 3, 0);
    glColor4d(dDimmer * 1, dDimmer * 1, dDimmer * 1, 1);
    glVertex3f(0, dCharHeight / 6, 0);
    glColor4d(0, 0, 0, 0);
    glVertex3f(0, dCharHeight / 6 + dCharHeight / 3, 0);

    glColor4d(dDimmer * 1, dDimmer * 1, dDimmer * 1, 1);
    glVertex3f(0, -dCharHeight / 6 + dCharHeight / 3, 0);
    glColor4d(0, 0, 0, 0);
    glVertex3f(0 - dCharHeight / 3, -dCharHeight / 6 + dCharHeight / 3, 0);
    glColor4d(dDimmer * 1, dDimmer * 1, dDimmer * 1, 1);
    glVertex3f(0, -dCharHeight / 6, 0);
    glColor4d(0, 0, 0, 0);
    glVertex3f(0 - dCharHeight / 3, -dCharHeight / 6, 0);

    glColor4d(dDimmer * 1, dDimmer * 1, dDimmer * 1, 1);
    glVertex3f(0, -dCharHeight / 6, 0);
    glColor4d(0, 0, 0, 0);
    glVertex3f(0, -dCharHeight / 6 - dCharHeight / 3, 0);

    glEnd();

    glBegin(GL_QUADS);
    glColor4d(dDimmer * 0.5, dDimmer * 0.65, dDimmer * 0.9, 1);
    glVertex3f(0, -dCharHeight / 6, 0);
    glColor4d(dDimmer * 0.3, dDimmer * 0.45, dDimmer * 0.7, 1);
    glVertex3f(pMenuItem->m_nValue, -dCharHeight / 6, 0);
    glColor4d(dDimmer * 0.4, dDimmer * 0.5, dDimmer * 0.8, 1);
    glVertex3f(pMenuItem->m_nValue, dCharHeight / 6, 0);
    glColor4d(dDimmer * 0.8, dDimmer * 0.9, dDimmer * 1, 1);
    glVertex3f(0, dCharHeight / 6, 0);
    glEnd();

    /*
    glBegin(GL_QUADS);
    glColor4d(dDimmer * 0.8, dDimmer * 0.9, dDimmer * 1, 1);
    glVertex3f(0, -dCharHeight / 6, 0);
    glColor4d(dDimmer * 0.4, dDimmer * 0.5, dDimmer * 0.8, 1);
    glVertex3f(pMenuItem->m_nValue, -dCharHeight / 6, 0);
    glColor4d(dDimmer * 0.3, dDimmer * 0.45, dDimmer * 0.7, 1);
    glVertex3f(pMenuItem->m_nValue, dCharHeight / 6, 0);
    glColor4d(dDimmer * 0.5, dDimmer * 0.65, dDimmer * 0.9, 1);
    glVertex3f(0, dCharHeight / 6, 0);
    glEnd();
    */
    // glColor4d(dDimmer * 1, dDimmer * 1, dDimmer * 1, 1);
    glColor4d(1, 1, 1, 1);
    glBegin(GL_LINE_STRIP);
    glVertex3f(0, -dCharHeight / 6, 0);
    glVertex3f(100, -dCharHeight / 6, 0);
    glVertex3f(100, dCharHeight / 6, 0);
    glVertex3f(0, dCharHeight / 6, 0);
    glVertex3f(0, -dCharHeight / 6, 0);
    glEnd();

    glPopMatrix();
  }

  glColor4d(1, 1, 1, 1);
  glDisable(GL_COLOR_MATERIAL);
}

void CVeraView::ReturnPressedOnCurrentMenu() {
  // if(BGame::m_pMenuCurrent->m_type == BMenu::TType::SETTINGS) {
  if (BGame::m_pMenuCurrent->m_type == BMenu::SETTINGS) {
    // See if we need to open/close sublist/slider
    string sTmp;
    int nSelected = BGame::m_pMenuCurrent->m_listMenu.GetSelectedItem(sTmp);
    if (nSelected != -1) {
      BMenuItem *pMenuItem = &(BGame::m_pMenuCurrent->m_items[nSelected]);
      if (!pMenuItem->m_bOpen) {
        // if(pMenuItem->m_type == BMenuItem::TType::STRING_FROM_LIST) {
        if (pMenuItem->m_type == BMenuItem::STRING_FROM_LIST) {
          // Open menu
          pMenuItem->m_bOpen = true;
          pMenuItem->m_listMenu.SetItems(pMenuItem->m_sAssocListItems,
                                         pMenuItem->m_nAssocListItems);
          pMenuItem->m_listMenu.SelectItem(
              pMenuItem->m_sAssocListItems[pMenuItem->m_nValue]);
          BUI::StartUsingSelectionList(&(pMenuItem->m_listMenu),
                                       &CVeraView::OnKeyDownCurrentMenu);
          //} else if(pMenuItem->m_type == BMenuItem::TType::SLIDER) {
        } else if (pMenuItem->m_type == BMenuItem::SLIDER) {
          // Open slider
          pMenuItem->m_bOpen = true;
          BUI::StartUsingSlider(&(pMenuItem->m_nValue),
                                &CVeraView::OnKeyDownCurrentMenu);
        }
        // Invalidate();
      } else {
        // if(pMenuItem->m_type == BMenuItem::TType::STRING_FROM_LIST) {
        if (pMenuItem->m_type == BMenuItem::STRING_FROM_LIST) {
          // Update value
          pMenuItem->m_nValue = pMenuItem->m_listMenu.GetSelectedItem(sTmp);
          //} else if(pMenuItem->m_type == BMenuItem::TType::SLIDER) {
        } else if (pMenuItem->m_type == BMenuItem::SLIDER) {
          // Set the selected volume
          // if(pMenuItem->m_sText.CompareNoCase("Music Volume:") == 0) {
          if (pMenuItem->m_sText.compare("Music Volume:") == 0) {
            SoundModule::SetMenuMusicVolume(
                int(double(pMenuItem->m_nValue) / 100.0 * 255.0));
            //} else if(pMenuItem->m_sText.CompareNoCase("Vehicle Volume:") ==
            //0) {
          } else if (pMenuItem->m_sText.compare("Vehicle Volume:") == 0) {
            SoundModule::SetVehicleSoundsVolume(
                int(double(pMenuItem->m_nValue) / 100.0 * 255.0));
          }
        }
        // Close menu
        pMenuItem->m_bOpen = false;
        BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu),
                                     &CVeraView::OnKeyDownCurrentMenu);
        // Invalidate();
      }
    }
    //} else if(BGame::m_pMenuCurrent->m_type == BMenu::TType::MAIN) {
  } else if (BGame::m_pMenuCurrent->m_type == BMenu::MAIN) {

    string sTmp;
    int nSelected = BGame::m_pMenuCurrent->m_listMenu.GetSelectedItem(sTmp);

    switch (nSelected) {
    case 0: // START GAME
      BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
      BGame::m_pMenuCurrent = &(BGame::m_menuChooseScene);
      BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu),
                                   &CVeraView::OnKeyDownCurrentMenu);
      StartMenuScroll(SCROLL_RIGHT);
      // Invalidate();
      break;
    case 1: // SETTINGS
      BGame::m_bSettingsFromGame = false;
      BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
      BGame::m_pMenuCurrent = &(BGame::m_menuSettings);
      BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu),
                                   &CVeraView::OnKeyDownCurrentMenu);
      StartMenuScroll(SCROLL_LEFT);
      // Invalidate();
      break;
    case 2: // CREDITS
      BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
      BGame::m_pMenuCurrent = &(BGame::m_menuCredits);
      BGame::m_menuCredits.m_clockStarted = SDL_GetTicks();
      BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu),
                                   &CVeraView::OnKeyDownCurrentMenu);
      StartMenuScroll(SCROLL_DOWN);
      // Invalidate();
      break;
    case 3: // EXIT
      // Exit
      SoundModule::StopMenuMusic();

      // Write settings for graphics etc.
      // BGame::GetPlayer()->SaveStateFile();
      // Settings::WriteSettings(m_game.GetSimulation());

      // AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_FILE_CLOSE, 0);
      setExit();
      break;
    }
    //} else if(BGame::m_pMenuCurrent->m_type == BMenu::TType::CHOOSE_SCENE) {
  } else if (BGame::m_pMenuCurrent->m_type == BMenu::CHOOSE_SCENE) {
    // Load Scene
    string sTmp;
    int nSelected = BGame::m_pMenuCurrent->m_listMenu.GetSelectedItem(sTmp);
    BGame::m_sScene = BGame::m_pMenuCurrent->m_items[nSelected].m_sAssocFile;

    BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
    BGame::m_pMenuCurrent = &(BGame::m_menuChooseVehicle);
    BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu),
                                 &CVeraView::OnKeyDownCurrentMenu);
    StartMenuScroll(SCROLL_RIGHT);
    // Invalidate();
    //} else if(BGame::m_pMenuCurrent->m_type == BMenu::TType::CHOOSE_VEHICLE) {
  } else if (BGame::m_pMenuCurrent->m_type == BMenu::CHOOSE_VEHICLE) {

    string sTmp;
    int nSelected = BGame::m_pMenuCurrent->m_listMenu.GetSelectedItem(sTmp);
    BGame::m_sVehicle = BGame::m_pMenuCurrent->m_items[nSelected].m_sAssocFile;

    if (BGame::m_bBuyingVehicle) {
      // Check whether user wants to buy the selected vehicle
      int nYesNo = BGame::m_listYesNo.GetSelectedItem(sTmp);
      if (nYesNo == 0) {
        // Buy
        BGame::GetPlayer()->m_sValidVehicles +=
            (">" + BGame::m_pMenuCurrent->m_items[nSelected].m_sText + "<");
        BGame::GetPlayer()->m_dCash -= BGame::m_dPurchasePrice;
        BGame::GetPlayer()->SaveStateFile();
      }
      BGame::m_bBuyingVehicle = false;
      BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu),
                                   &CVeraView::OnKeyDownCurrentMenu);
    } else if (BGame::m_bCannotBuyVehicle) {
      // exit cannot buy message
      BGame::m_bCannotBuyVehicle = false;
      BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu),
                                   &CVeraView::OnKeyDownCurrentMenu);
    } else if (BGame::m_pMenuCurrent->m_items[nSelected].m_bDisabled) {
      double dPrice = 100.0;
      if (BGame::m_pMenuCurrent->m_items[nSelected].m_sText.compare("Spirit") ==
          0) {
        dPrice = 500.0;
      }
      BGame::m_dPurchasePrice = dPrice;
      if (BGame::GetPlayer()->m_dCash >= dPrice) {
        BUI::StartUsingSelectionList(&(BGame::m_listYesNo),
                                     &CVeraView::OnKeyDownCurrentMenu);
        BGame::m_bBuyingVehicle = true;
      } else {
        BUI::StartUsingSelectionList(&(BGame::m_listOK),
                                     &CVeraView::OnKeyDownCurrentMenu);
        BGame::m_bCannotBuyVehicle = true;
      }
    } else {
      // Load Vehicle
      // Proceed to PreCaching Terrain menu
      BGame::SetProgressPos(0);
      BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
      BGame::m_pMenuCurrent = &(BGame::m_menuPrecachingTerrain);
      StartMenuScroll(SCROLL_RIGHT);
    }
    // Invalidate();
  } else if (BGame::m_pMenuCurrent->m_type == BMenu::PRECACHING_TERRAIN) {
    // START GAME

    BGame::m_bMenuMode = false;

    m_pDrawFunction = &CVeraView::OnDrawGame;
    m_pKeyDownFunction = &CVeraView::OnKeyDownGame;
    m_game.m_bFadingIn = true;
    m_game.m_clockFadeStart = SDL_GetTicks();
    // ShowCursor(FALSE);
    SDL_ShowCursor(0);
    // Invalidate();
    if (touch_seen && !isDialogOpen<DlgOnScreenKbd *>()) {
      dialogs.push_back(new DlgOnScreenKbd(this));
    }
  }
}

void CVeraView::CancelPressedOnCurrentMenu() {
  // if(BGame::m_pMenuCurrent->m_type == BMenu::TType::MAIN) {
  if (BGame::m_pMenuCurrent->m_type == BMenu::MAIN) {
    // NOT YET DONE, JUST RETURN TO MAIN MENU!!!
    // Setup for first menu
    BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu),
                                 &CVeraView::OnKeyDownCurrentMenu);
    // Exit
    // SoundModule::StopMenuMusic();

    // Write settings for graphics etc.
    // Settings::WriteSettings(m_game.GetSimulation());

    // AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_FILE_CLOSE, 0);
    //} else if(BGame::m_pMenuCurrent->m_type == BMenu::TType::SETTINGS) {
  } else if (BGame::m_pMenuCurrent->m_type == BMenu::SETTINGS) {
    // See if we need to open/close sublist/slider
    BMenu *pMenu = BGame::m_pMenuCurrent;
    string sTmp;
    int nSelected = pMenu->m_listMenu.GetSelectedItem(sTmp);
    if (nSelected != -1) {
      BMenuItem *pMenuItem = &(pMenu->m_items[nSelected]);
      if (pMenuItem->m_bOpen) {
        // Close menu
        pMenuItem->m_bOpen = false;
        BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu),
                                     &CVeraView::OnKeyDownCurrentMenu);
        // Invalidate();
      } else {

        // Save settings
        string sTmp;
        // sscanf(LPCTSTR(pMenu->m_items[0].m_sAssocListItems[pMenu->m_items[0].m_nValue]),
        // "%d*%d", &(BGame::m_nDispWidth), &(BGame::m_nDispHeight));
        // sscanf(pMenu->m_items[0].m_sAssocListItems[pMenu->m_items[0].m_nValue].c_str(),
        // "%d*%d", &(BGame::m_nDispWidth), &(BGame::m_nDispHeight)); //we use
        // only desktop resolution now
        // sscanf(LPCTSTR(pMenu->m_items[1].m_sAssocListItems[pMenu->m_items[1].m_nValue]),
        // "%d", &(BGame::m_nDispBits));
        sscanf(pMenu->m_items[1]
                   .m_sAssocListItems[pMenu->m_items[1].m_nValue]
                   .c_str(),
               "%d", &(BGame::m_nDispBits));
        // sscanf(LPCTSTR(pMenu->m_items[2].m_sAssocListItems[pMenu->m_items[2].m_nValue]),
        // "%d", &(BGame::m_nDispHz));
        sscanf(pMenu->m_items[2]
                   .m_sAssocListItems[pMenu->m_items[2].m_nValue]
                   .c_str(),
               "%d", &(BGame::m_nDispHz));
        BGame::m_nTerrainResolution = pMenu->m_items[3].m_nValue;
        BGame::m_nDustAndClouds = pMenu->m_items[4].m_nValue;
        BGame::m_nWaterSurface = pMenu->m_items[5].m_nValue;
        BGame::m_nMusicVolume = pMenu->m_items[6].m_nValue;
        BGame::m_nVehicleVolume = pMenu->m_items[7].m_nValue;
        BGame::m_nSoundscape = pMenu->m_items[8].m_nValue;

        Settings::WriteSettings(BGame::GetSimulation());

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

        // See if resolution needs to be changed
        // FIXME
        /*DEVMODE devmode;
        EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);
        if((devmode.dmPelsWidth  != (DWORD)BGame::m_nDispWidth) ||
           (devmode.dmPelsHeight != (DWORD)BGame::m_nDispHeight) ||
           (devmode.dmBitsPerPel != (DWORD)BGame::m_nDispBits) ||
           (devmode.dmDisplayFrequency != (DWORD)BGame::m_nDispHz)) {
          devmode.dmPelsWidth = (DWORD)BGame::m_nDispWidth;
          devmode.dmPelsHeight = (DWORD)BGame::m_nDispHeight;
          devmode.dmBitsPerPel = (DWORD)BGame::m_nDispBits;
          devmode.dmDisplayFrequency = (DWORD)BGame::m_nDispHz;
          ChangeDisplaySettings(&devmode, CDS_FULLSCREEN);
          AfxGetMainWnd()->SetWindowPos(NULL, -2, -2, BGame::m_nDispWidth + 4,
        BGame::m_nDispHeight + 4, 0);
        }*/

        // Go back to main menu or game menu

        if (BGame::m_bSettingsFromGame) {
          BGame::m_bMenuMode = false;
          m_pKeyDownFunction = &CVeraView::OnKeyDownGameMenu;
          BGame::m_bShowGameMenu = true;
          m_pDrawFunction = &CVeraView::OnDrawGame;
          m_game.m_bFadingIn = true;
          m_game.m_clockFadeStart = SDL_GetTicks();
          // ShowCursor(FALSE);
          SDL_ShowCursor(0);
        } else {
          StartMenuScroll(SCROLL_RIGHT);
          BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
          BGame::m_pMenuCurrent = &(BGame::m_menuMain);
          BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu),
                                       &CVeraView::OnKeyDownCurrentMenu);
        }
        // Invalidate();
      }
    }
    //} else if(BGame::m_pMenuCurrent->m_type == BMenu::TType::CHOOSE_SCENE) {
  } else if (BGame::m_pMenuCurrent->m_type == BMenu::CHOOSE_SCENE) {
    StartMenuScroll(SCROLL_LEFT);
    BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
    BGame::m_pMenuCurrent = &(BGame::m_menuMain);
    BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu),
                                 &CVeraView::OnKeyDownCurrentMenu);
    // Invalidate();
    //} else if(BGame::m_pMenuCurrent->m_type == BMenu::TType::CHOOSE_VEHICLE) {
  } else if (BGame::m_pMenuCurrent->m_type == BMenu::CHOOSE_VEHICLE) {
    if (BGame::m_bBuyingVehicle || BGame::m_bCannotBuyVehicle) {
      BGame::m_bBuyingVehicle = false;
      BGame::m_bCannotBuyVehicle = false;
      BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu),
                                   &CVeraView::OnKeyDownCurrentMenu);
    } else {
      StartMenuScroll(SCROLL_LEFT);
      BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
      BGame::m_pMenuCurrent = &(BGame::m_menuChooseScene);
      BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu),
                                   &CVeraView::OnKeyDownCurrentMenu);
    }
    // Invalidate();
    //} else if(BGame::m_pMenuCurrent->m_type == BMenu::TType::CREDITS) {
  } else if (BGame::m_pMenuCurrent->m_type == BMenu::CREDITS) {
    StartMenuScroll(SCROLL_UP);
    BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
    BGame::m_pMenuCurrent = &(BGame::m_menuMain);
    BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu),
                                 &CVeraView::OnKeyDownCurrentMenu);
    // Invalidate();
  }
}

void CVeraView::StartMenuScroll(TMenuScroll scroll) {
  m_clockMenuScroll = SDL_GetTicks();
  m_scrollDir = scroll;
}

void CVeraView::OnKeyDownCurrentMenu(int nChar, int nRepCnt, int nFlags) {
  switch (nChar) {
  // case VK_ESCAPE:
  case SDLK_ESCAPE:
    break;
  // case VK_RETURN:
  case SDLK_RETURN:
    break;
  }
}

void CVeraView::OnKeyDownCancelQuestion(int nChar, int nRepCnt, int nFlags) {
  switch (nChar) {
  // case VK_ESCAPE:
  case SDLK_ESCAPE:
    // act as Nope was selected
    m_game.m_bShowCancelQuestion = false;
    m_nMenuTime += BGame::ContinueSimulation();
    m_pKeyDownFunction = &CVeraView::OnKeyDownGame;
    // Invalidate();
    break;
  // case VK_RETURN:
  case SDLK_RETURN:
    switch (m_game.m_nYesNoSelection) {
    case 1:
      // Cancel Order (i.e. jump to home)
      if (!BGame::m_bJumpToHome) {
        m_game.m_bShowCancelQuestion = false;
        BGame::GetPlayer()->m_dCash -= 50;
        BGame::m_bJumpToHome = true;
        // BGame::GetSimulation()->GetCamera()->m_locMode =
        // BCamera::TCameraLoc::OVERVIEW;
        BGame::GetSimulation()->GetCamera()->m_locMode = BCamera::OVERVIEW;
        BGame::FreezeSimulation(); // freeze so that the vehicle doesn't jump up
                                   // if it happens to go underground

        // Stop vehicle
        BVehicle *pVehicle = BGame::GetSimulation()->GetVehicle();
        for (int i = 0; i < pVehicle->m_nBodyPoints; ++i) {
          pVehicle->m_pBodyPoint[i].m_vector = BVector(0, 0, 0);
        }

        // Return to game
        m_nMenuTime += BGame::ContinueSimulation();
        m_pKeyDownFunction = &CVeraView::OnKeyDownGame;
      }
      // Invalidate();
      break;
    case 2:
      // Don't cancel order
      m_game.m_bShowCancelQuestion = false;
      // Return to game
      m_nMenuTime += BGame::ContinueSimulation();
      m_pKeyDownFunction = &CVeraView::OnKeyDownGame;
      // Invalidate();
      break;
    }
    break;
  // case VK_DOWN:
  case SDLK_DOWN:
  // case VK_RIGHT:
  case SDLK_RIGHT:
    m_game.m_nYesNoSelection -= 1;
    if (m_game.m_nYesNoSelection == 0) {
      m_game.m_nYesNoSelection = 2;
    }
    // Invalidate();
    break;
  // case VK_UP:
  case SDLK_UP:
  // case VK_LEFT:
  case SDLK_LEFT:
    m_game.m_nYesNoSelection += 1;
    if (m_game.m_nYesNoSelection == 3) {
      m_game.m_nYesNoSelection = 1;
    }
    // Invalidate();
    break;
  }
}

