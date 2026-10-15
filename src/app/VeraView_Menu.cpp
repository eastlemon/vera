// VeraView_Menu.cpp - menu rendering & menu input handlers.

#include <cstdlib>
#include <iostream>
#include <sstream>

#include <SDL2/SDL.h>

// GLEW must be included before any other GL header. On macOS we use the
// system OpenGL framework instead (Renderer2D.h falls back to gl.h + glext.h).
#if !defined(__APPLE__) || !defined(__MACH__)
#include <GL/glew.h>
#endif

using namespace std;

#include "VeraView.h"

#include "BTextures.h"
#include "OpenGLExtFunctions.h"
#include "OpenGLHelpers.h"
#include "Renderer2D.h"
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

void CVeraView::OnDrawCurrentMenu() {
  if (BGame::m_bQuitPending) {
    setExit();
  }

  if (!BGame::m_bMenusCreated) {
    BGame::SetupMenus();

    // Setup for first menu
    BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), &CVeraView::OnKeyDownCurrentMenu);

    // Start menu music
    SoundModule::StartMenuMusic();
    SoundModule::SetMenuMusicVolume(int(double(BGame::m_nMusicVolume) / 100.0 * 255.0));

    return;
  }

  SDL_LockMutex(BGame::m_csMutex);

  // Cater for Precaching Terrain menu special needs
  if (BGame::m_pMenuCurrent->m_type == BMenu::PRECACHING_TERRAIN) {
    if (!BGame::m_bGameLoading) {
      // Initiate game load (start separate thread)
      BGame::m_bGameReadyToStart = false;
      BGame::m_bGameLoading = true;

      BGame::GetSimulation()->GetScene()->LoadSceneFromFile(BGame::m_sScene);
      BGame::SetupScene();

      m_pThreadLoading = SDL_CreateThread(TerrainLoadThread, "TerrainLoad", (void *)this);
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

        for (int o = 0; o < BGame::GetSimulation()->GetScene()->m_nObjects; ++o) {
          BGame::GetSimulation()->GetScene()->m_pObjects[o].RecreateShadow();
        }

        BMessages::RemoveAll();

        // Init delivery
        BGame::m_state = BGame::PICKUP;
        BGame::m_dPizzaTemp = 0.0;

        // Move vehicle to start location
        BVector vStartLocation = BGame::GetSimulation()->GetScene()->m_vStartLocation;
        BVector vSceneLoc;
        if (BGame::GetPlayer()->LoadCurrentSceneInfo(vSceneLoc)) {
          vStartLocation = vSceneLoc;
        }
        BGame::GetSimulation()->UpdateCarLocation();
        BVector vLoc = BGame::GetSimulation()->GetVehicle()->m_vLocation;
        BVector val_bvector = vStartLocation - vLoc;
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
        BGame::m_bEarthquakeNextStart = (double)SDL_GetTicks() + 1000 * (60.0 + Random(60.0));

        // Init wind particles
        if (BGame::m_bWindActive) {
          BGame::UpdateWindParticles(true);
        }

        // Switch to game mode
        SoundModule::StopMenuMusic();
        SoundModule::StartEngineSound();
        SoundModule::SetVehicleSoundsVolume(int(double(BGame::m_nVehicleVolume) / 100.0 * 255.0));
        SoundModule::SetEngineSoundVolume(255.0);
        ReturnPressedOnCurrentMenu();
        m_bInitClock = true;
      }
    }
  }

  // Preparations stuff

  glClearColor(0, 0, 0, 0);

  SDL_ShowCursor(0);

  // Init OpenGL
  glDrawBuffer(GL_BACK);

  // Reset OpenGL
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

  // Draw background - sunset-sky texture stretched to fill the window.
  //
  // Stage B of the GL modernisation: this is the first piece of 2D
  // rendering to use the modern shader-based pipeline (Renderer2D ->
  // Shader + Mesh2D + GLM) instead of glBegin/glEnd. The rest of the
  // menu (title, items, scroll animation) still uses the legacy
  // fixed-function path; it will be migrated in subsequent commits.
  if (Renderer2D::get().init(window_width, window_height)) {
    Renderer2D::get().useShader();
    // Bind the menu background texture (loaded by BTextures at startup).
    BTextures::Use(BTextures::MENU_BACKGROUND);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Draw the fullscreen background quad.
    Renderer2D::get().drawTexturedQuad(
        0.0f, 0.0f, float(window_width), float(window_height));

    // Subtle dark overlay at the bottom to improve menu text contrast.
    // Two quads: from 40% height to 100% height, alpha 0 -> 0.45.
    // Implemented as a single solid quad with alpha 0.3 for simplicity -
    // the gradient version requires a separate vertex-color setup that
    // would complicate Mesh2D. The visual difference is minimal.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    Renderer2D::get().drawSolidQuad(
        0.0f, float(window_height) * 0.4f,
        float(window_width), float(window_height) * 0.6f,
        glm::vec4(0.0f, 0.0f, 0.0f, 0.35f));
    glDisable(GL_BLEND);

    Renderer2D::get().unuseShader();
  } else {
    // Fallback: legacy fixed-function background (only used if the shader
    // failed to compile, e.g. on a very old GPU without GLSL 1.30).
    OpenGLHelpers::SetColorFull(1, 1, 1, 1);
    OpenGLHelpers::SwitchToTexture(0);
    BTextures::Use(BTextures::MENU_BACKGROUND);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBegin(GL_TRIANGLE_STRIP);
    OpenGLHelpers::SetTexCoord(0, 0);
    glVertex3f(0, 0, 0);
    OpenGLHelpers::SetTexCoord(0, 1);
    glVertex3f(0, window_height, 0);
    OpenGLHelpers::SetTexCoord(1, 0);
    glVertex3f(window_width, 0, 0);
    OpenGLHelpers::SetTexCoord(1, 1);
    glVertex3f(window_width, window_height, 0);
    glEnd();
  }

  if (bScrolling) {
    // Draw previous and new menu scrolling
    glPushMatrix();
    if (m_scrollDir == SCROLL_RIGHT) {
      glTranslated(-int(double(window_width * dScrollPhase)), 0, 0);
    } else if (m_scrollDir == SCROLL_LEFT) {
      glTranslated(+int(double(window_width * dScrollPhase)), 0, 0);
    } else if (m_scrollDir == SCROLL_UP) {
      glTranslated(0, -int(double(window_height * dScrollPhase)), 0);
    } else if (m_scrollDir == SCROLL_DOWN) {
      glTranslated(0, +int(double(window_height * dScrollPhase)), 0);
    }
    DrawMenu(BGame::m_pMenuPrevious);
    glPopMatrix();

    glPushMatrix();
    if (m_scrollDir == SCROLL_RIGHT) {
      glTranslated(int(double(window_width * (1.0 - dScrollPhase))), 0, 0);
    } else if (m_scrollDir == SCROLL_LEFT) {
      glTranslated(-int(double(window_width * (1.0 - dScrollPhase))), 0, 0);
    } else if (m_scrollDir == SCROLL_UP) {
      glTranslated(0, +int(double(window_height * (1.0 - dScrollPhase))), 0);
    } else if (m_scrollDir == SCROLL_DOWN) {
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

  SDL_UnlockMutex(BGame::m_csMutex);

  // And again and again and...
  if ((BGame::m_pMenuCurrent == &(BGame::m_menuPrecachingTerrain)) && !BGame::m_bMultiProcessor) {
    SDL_Delay(1);
  }
}

void CVeraView::DrawMenu(BMenu *pMenu) {
  if (pMenu) {
    double dCharHeight = BUI::TextRenderer()->GetCharHeight();
    double dCharWidth = BUI::TextRenderer()->GetCharWidth();

    // Draw menu title
    // glPushMatrix();
    // int nThird = window_height / 6;
    // glTranslated((window_width - pMenu->m_nTitleWidth) / 2,
    //              window_height - nThird - pMenu->m_nTitleHeight / 2, 0);

    // if (pMenu->m_nTitleWidth) {
    //   OpenGLHelpers::SwitchToTexture(0);
    //   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
    //                   GL_LINEAR); // not mipmapped

    //   OpenGLHelpers::SetColorFull(1, 1, 1, 1);
    //   glBegin(GL_TRIANGLE_STRIP);
    //   OpenGLHelpers::SetTexCoord(pMenu->m_dTitleX, pMenu->m_dTitleY);
    //   glVertex3f(0, 0, 0);
    //   OpenGLHelpers::SetTexCoord(pMenu->m_dTitleX,
    //                              pMenu->m_dTitleY +
    //                                  (double(pMenu->m_nTitleHeight) / 512.0));
    //   glVertex3f(0, pMenu->m_nTitleHeight, 0);
    //   OpenGLHelpers::SetTexCoord(pMenu->m_dTitleX +
    //                                  (double(pMenu->m_nTitleWidth) / 512.0),
    //                              pMenu->m_dTitleY);
    //   glVertex3f(pMenu->m_nTitleWidth, 0, 0);
    //   OpenGLHelpers::SetTexCoord(
    //       pMenu->m_dTitleX + (double(pMenu->m_nTitleWidth) / 512.0),
    //       pMenu->m_dTitleY + (double(pMenu->m_nTitleHeight) / 512.0));
    //   glVertex3f(pMenu->m_nTitleWidth, pMenu->m_nTitleHeight, 0);
    //   glEnd();
    // }

    // Draw separator
    // if (pMenu->m_bDrawLine) {
    //   glEnable(GL_COLOR_MATERIAL);
    //   glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    //   glDisable(GL_TEXTURE_2D);
    //   glBegin(GL_LINE_STRIP);
    //   glColor3d(0, 0, 0);
    //   glVertex3f(-pMenu->m_nTitleWidth / 2 - 10, 0, 0);
    //   glColor3d(1, 1, 1);
    //   glVertex3f(-pMenu->m_nTitleWidth / 2, 0, 0);
    //   glColor3d(0.25, 0.35, 0.4);
    //   glVertex3f(pMenu->m_nTitleWidth + pMenu->m_nTitleWidth / 2, 0, 0);
    //   glColor3d(0, 0, 0);
    //   glVertex3f(pMenu->m_nTitleWidth + pMenu->m_nTitleWidth / 2 + 10, 0, 0);
    //   glEnd();
    //   glTranslated(0, 4, 0);
    //   glBegin(GL_LINE_STRIP);
    //   glColor3d(0, 0, 0);
    //   glVertex3f(-pMenu->m_nTitleWidth / 2 - 10, 0, 0);
    //   glColor3d(1, 1, 1);
    //   glVertex3f(-pMenu->m_nTitleWidth / 2, 0, 0);
    //   glColor3d(0.25, 0.35, 0.4);
    //   glVertex3f(pMenu->m_nTitleWidth + pMenu->m_nTitleWidth / 2, 0, 0);
    //   glColor3d(0, 0, 0);
    //   glVertex3f(pMenu->m_nTitleWidth + pMenu->m_nTitleWidth / 2 + 10, 0, 0);
    //   glEnd();
    //   glColor3d(1, 1, 1);
    //   glDisable(GL_COLOR_MATERIAL);
    // }

    glPopMatrix();

    // Draw menu contents
    if (pMenu->m_listMenu.GetNofItems()) {
      int i;

      if (pMenu->m_type == BMenu::CHOOSE_VEHICLE) {
        for (i = 0; i < pMenu->m_nItems; ++i) {
          // Check for owned vehicles
          string sVehicle;
          sVehicle = ">" + pMenu->m_items[i].m_sText + "<";
          if (BGame::GetPlayer()->m_sValidVehicles.find(sVehicle) == -1) {
            pMenu->m_items[i].m_bDisabled = true;
          } else {
            pMenu->m_items[i].m_bDisabled = false;
          }
        }
      }

      pMenu->m_listMenu.DrawAt(window_width / 2, window_height / 2, pMenu->m_align, 1, 1, 1, false);

      // If there are menu items that have an associated list value,
      // draw them also
      for (i = 0; i < pMenu->m_nItems; ++i) {
        if (pMenu->m_items[i].m_type == BMenuItem::STRING_FROM_LIST) {
          // Draw list string
          DrawMenuItemTextAtRelPos(window_width / 2,
                                   window_height / 2, pMenu->m_nItems, i,
                                   &(pMenu->m_items[i]));
        } else if (pMenu->m_items[i].m_type == BMenuItem::SLIDER) {
          // Draw slider
          DrawMenuItemSliderAtRelPos(window_width / 2,
                                     window_height / 2, pMenu->m_nItems, i,
                                     &(pMenu->m_items[i]));
        }
      }

      // Draw open subitem
      for (i = 0; i < pMenu->m_nItems; ++i) {
        if (pMenu->m_items[i].m_bOpen) {
          if (pMenu->m_items[i].m_type == BMenuItem::STRING_FROM_LIST) {
            // Draw submenu
            pMenu->m_items[i].m_listMenu.DrawAt(
                window_width / 2 + 35,
                window_height / 2 + double(i) * -dCharHeight +
                    (dCharHeight * double(pMenu->m_nItems)) / 2.0,
                BTextRenderer::ALIGN_LEFT, 1, 1, 1, true, true);
          } else if (pMenu->m_items[i].m_type == BMenuItem::SLIDER) {
            // Draw open slider
            DrawMenuItemSliderAtRelPos(window_width / 2,
                                       window_height / 2, pMenu->m_nItems, i,
                                       &(pMenu->m_items[i]));
          }
        }
      }

      // Draw associated image, if one is available.
      //
      // Stage B: migrated from glBegin/glEnd to Renderer2D (shader-based).
      // The image is a 256x256 quad placed at the right side of the menu,
      // centred vertically. If the item is disabled (e.g. locked vehicle),
      // a hatched overlay is drawn on top using legacy GL_LINES (kept as-is
      // since it's a visual effect, not a textured quad).
      string sTmp;
      int nSelected = pMenu->m_listMenu.GetSelectedItem(sTmp);
      if ((nSelected != -1) && (pMenu->m_items[nSelected].m_nAssocImage != -1)) {
        // Compute absolute screen coordinates.
        // Legacy code: glTranslated(w/2 + w/40, h/2, 0) then drew quad
        // from (0, -128) to (256, 128) relative to that origin.
        float imgX = float(window_width) / 2.0f + float(window_width) / 40.0f;
        float imgY = float(window_height) / 2.0f - 128.0f;
        float imgW = 256.0f;
        float imgH = 256.0f;

        if (Renderer2D::get().init(window_width, window_height)) {
          Renderer2D::get().useShader();
          OpenGLHelpers::SwitchToTexture(0);
          BTextures::Use(pMenu->m_items[nSelected].m_nAssocImage);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
          Renderer2D::get().drawTexturedQuad(imgX, imgY, imgW, imgH);
          Renderer2D::get().unuseShader();
        }

        // If disabled, draw veil over image (legacy GL_LINES hatch pattern).
        // Kept on the fixed-function path because it's a line-based effect,
        // not a textured quad - migrating lines is Stage C territory.
        if (pMenu->m_items[nSelected].m_bDisabled) {
          glPushMatrix();
          glTranslated(imgX, imgY + 128, 0);  // match legacy origin
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
          glPopMatrix();
        }
      }
    }

    // Draw progress indicator, if loading terrain
    if (BGame::m_pMenuCurrent->m_type == BMenu::PRECACHING_TERRAIN) {
      OpenGLHelpers::SetColorFull(1, 1, 1, 1);
      glEnable(GL_COLOR_MATERIAL);
      glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
      glDisable(GL_TEXTURE_2D);

      double dSmall = double(window_height) / 8.0;
      double dBig = double(window_height) / 5.3;

      glPushMatrix();
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
    if (pMenu->m_type == BMenu::CHOOSE_VEHICLE) {
      glPushMatrix();
      glTranslated(window_width / 2, window_height / 2, 0);
      if (BGame::m_bBuyingVehicle) {
        DrawPanel(dCharWidth * 32, dCharHeight * 8);
        BUI::TextRenderer()->StartRenderingText();
        BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 2,
                                        "Do you want to buy",
                                        BTextRenderer::ALIGN_CENTER);
        string sTmp;
        string sVehicle;
        pMenu->m_listMenu.GetSelectedItem(sVehicle);
        sTmp = sVehicle + "?";
        BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 1, sTmp,
                                        BTextRenderer::ALIGN_CENTER);
        stringstream format;
        format << "(It costs " << (int)BGame::m_dPurchasePrice << ". You have "
               << (int)BGame::GetPlayer()->m_dCash;
        sTmp = format.str();
        BUI::TextRenderer()->DrawTextAt(0, 0, sTmp,
                                        BTextRenderer::ALIGN_CENTER);
        BUI::TextRenderer()->StopRenderingText();
        BGame::m_listYesNo.DrawAt(0, -dCharHeight * 2.5,
                                  BTextRenderer::ALIGN_CENTER, 1, 1, 1, false);
      } else if (BGame::m_bCannotBuyVehicle) {
        DrawPanel(dCharWidth * 32, dCharHeight * 8);
        BUI::TextRenderer()->StartRenderingText();
        BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 2, "You can not buy",
                                        BTextRenderer::ALIGN_CENTER);
        string sTmp;
        string sVehicle;
        pMenu->m_listMenu.GetSelectedItem(sVehicle);
        sTmp = sVehicle;
        BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 1, sTmp,
                                        BTextRenderer::ALIGN_CENTER);
        stringstream format;
        format << "(It costs " << (int)BGame::m_dPurchasePrice << ". You have "
               << (int)BGame::GetPlayer()->m_dCash;
        sTmp = format.str();
        BUI::TextRenderer()->DrawTextAt(0, 0, sTmp,
                                        BTextRenderer::ALIGN_CENTER);
        BUI::TextRenderer()->StopRenderingText();
        BGame::m_listOK.DrawAt(0, -dCharHeight * 2.5,
                               BTextRenderer::ALIGN_CENTER, 1, 1, 1, false);
      }
      glPopMatrix();
    }
  }
}

void CVeraView::DrawMenuItemTextAtRelPos(int nX, int nY, int nItems, int nIndex, BMenuItem *pMenuItem) {
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
        BTextRenderer::ALIGN_LEFT, 0.7, 0.7, 0.7);
    BUI::TextRenderer()->StopRenderingText();
    glPopMatrix();
  }
}

void CVeraView::DrawMenuItemSliderAtRelPos(int nX, int nY, int nItems, int nIndex, BMenuItem *pMenuItem) {
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
  if (BGame::m_pMenuCurrent->m_type == BMenu::SETTINGS) {
    // See if we need to open/close sublist/slider
    string sTmp;
    int nSelected = BGame::m_pMenuCurrent->m_listMenu.GetSelectedItem(sTmp);
    if (nSelected != -1) {
      BMenuItem *pMenuItem = &(BGame::m_pMenuCurrent->m_items[nSelected]);
      if (!pMenuItem->m_bOpen) {
        if (pMenuItem->m_type == BMenuItem::STRING_FROM_LIST) {
          // Open menu
          pMenuItem->m_bOpen = true;
          pMenuItem->m_listMenu.SetItems(pMenuItem->m_sAssocListItems, pMenuItem->m_nAssocListItems);
          pMenuItem->m_listMenu.SelectItem(pMenuItem->m_sAssocListItems[pMenuItem->m_nValue]);
          BUI::StartUsingSelectionList(&(pMenuItem->m_listMenu), &CVeraView::OnKeyDownCurrentMenu);
        } else if (pMenuItem->m_type == BMenuItem::SLIDER) {
          // Open slider
          pMenuItem->m_bOpen = true;
          BUI::StartUsingSlider(&(pMenuItem->m_nValue), &CVeraView::OnKeyDownCurrentMenu);
        }
      } else {
        if (pMenuItem->m_type == BMenuItem::STRING_FROM_LIST) {
          // Update value
          pMenuItem->m_nValue = pMenuItem->m_listMenu.GetSelectedItem(sTmp);
        } else if (pMenuItem->m_type == BMenuItem::SLIDER) {
          // Set the selected volume
          if (pMenuItem->m_sText.compare("Music Volume:") == 0) {
            SoundModule::SetMenuMusicVolume(int(double(pMenuItem->m_nValue) / 100.0 * 255.0));
          } else if (pMenuItem->m_sText.compare("Vehicle Volume:") == 0) {
            SoundModule::SetVehicleSoundsVolume(int(double(pMenuItem->m_nValue) / 100.0 * 255.0));
          }
        }
        // Close menu
        pMenuItem->m_bOpen = false;
        BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), &CVeraView::OnKeyDownCurrentMenu);
      }
    }
  } else if (BGame::m_pMenuCurrent->m_type == BMenu::MAIN) {
    string sTmp;
    int nSelected = BGame::m_pMenuCurrent->m_listMenu.GetSelectedItem(sTmp);

    switch (nSelected) {
    case 0: // START GAME
      BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
      BGame::m_pMenuCurrent = &(BGame::m_menuChooseScene);
      BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), &CVeraView::OnKeyDownCurrentMenu);
      StartMenuScroll(SCROLL_RIGHT);
      break;
    case 1: // SETTINGS
      BGame::m_bSettingsFromGame = false;
      BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
      BGame::m_pMenuCurrent = &(BGame::m_menuSettings);
      BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), &CVeraView::OnKeyDownCurrentMenu);
      StartMenuScroll(SCROLL_LEFT);
      break;
    case 3:
      SoundModule::StopMenuMusic();
      setExit();
      break;
    }
  } else if (BGame::m_pMenuCurrent->m_type == BMenu::CHOOSE_SCENE) {
    // Load Scene
    string sTmp;
    int nSelected = BGame::m_pMenuCurrent->m_listMenu.GetSelectedItem(sTmp);
    BGame::m_sScene = BGame::m_pMenuCurrent->m_items[nSelected].m_sAssocFile;

    BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
    BGame::m_pMenuCurrent = &(BGame::m_menuChooseVehicle);
    BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), &CVeraView::OnKeyDownCurrentMenu);
    StartMenuScroll(SCROLL_RIGHT);
  } else if (BGame::m_pMenuCurrent->m_type == BMenu::CHOOSE_VEHICLE) {
    string sTmp;
    int nSelected = BGame::m_pMenuCurrent->m_listMenu.GetSelectedItem(sTmp);
    BGame::m_sVehicle = BGame::m_pMenuCurrent->m_items[nSelected].m_sAssocFile;

    if (BGame::m_bBuyingVehicle) {
      // Check whether user wants to buy the selected vehicle
      int nYesNo = BGame::m_listYesNo.GetSelectedItem(sTmp);
      if (nYesNo == 0) {
        // Buy
        BGame::GetPlayer()->m_sValidVehicles += (">" + BGame::m_pMenuCurrent->m_items[nSelected].m_sText + "<");
        BGame::GetPlayer()->m_dCash -= BGame::m_dPurchasePrice;
        BGame::GetPlayer()->SaveStateFile();
      }
      BGame::m_bBuyingVehicle = false;
      BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), &CVeraView::OnKeyDownCurrentMenu);
    } else if (BGame::m_bCannotBuyVehicle) {
      // exit cannot buy message
      BGame::m_bCannotBuyVehicle = false;
      BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), &CVeraView::OnKeyDownCurrentMenu);
    } else if (BGame::m_pMenuCurrent->m_items[nSelected].m_bDisabled) {
      double dPrice = 100.0;
      if (BGame::m_pMenuCurrent->m_items[nSelected].m_sText.compare("Spirit") == 0) {
        dPrice = 500.0;
      }
      BGame::m_dPurchasePrice = dPrice;
      if (BGame::GetPlayer()->m_dCash >= dPrice) {
        BUI::StartUsingSelectionList(&(BGame::m_listYesNo), &CVeraView::OnKeyDownCurrentMenu);
        BGame::m_bBuyingVehicle = true;
      } else {
        BUI::StartUsingSelectionList(&(BGame::m_listOK), &CVeraView::OnKeyDownCurrentMenu);
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

  } else if (BGame::m_pMenuCurrent->m_type == BMenu::PRECACHING_TERRAIN) {
    // START GAME
    BGame::m_bMenuMode = false;

    m_pDrawFunction = &CVeraView::OnDrawGame;
    m_pKeyDownFunction = &CVeraView::OnKeyDownGame;
    m_game.m_bFadingIn = true;
    m_game.m_clockFadeStart = SDL_GetTicks();
    SDL_ShowCursor(0);

    if (touch_seen && !isDialogOpen<DlgOnScreenKbd *>()) {
      dialogs.push_back(new DlgOnScreenKbd(this));
    }
  }
}

void CVeraView::CancelPressedOnCurrentMenu() {
  if (BGame::m_pMenuCurrent->m_type == BMenu::MAIN) {
    // NOT YET DONE, JUST RETURN TO MAIN MENU!!!
    // Setup for first menu
    BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), &CVeraView::OnKeyDownCurrentMenu);
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
        BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), &CVeraView::OnKeyDownCurrentMenu);
      } else {
        // Save settings
        string sTmp;
        sscanf(pMenu->m_items[1].m_sAssocListItems[pMenu->m_items[1].m_nValue].c_str(), "%d", &(BGame::m_nDispBits));
        sscanf(pMenu->m_items[2].m_sAssocListItems[pMenu->m_items[2].m_nValue].c_str(), "%d", &(BGame::m_nDispHz));
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

        // Go back to main menu or game menu
        if (BGame::m_bSettingsFromGame) {
          BGame::m_bMenuMode = false;
          m_pKeyDownFunction = &CVeraView::OnKeyDownGameMenu;
          BGame::m_bShowGameMenu = true;
          m_pDrawFunction = &CVeraView::OnDrawGame;
          m_game.m_bFadingIn = true;
          m_game.m_clockFadeStart = SDL_GetTicks();
          SDL_ShowCursor(0);
        } else {
          StartMenuScroll(SCROLL_RIGHT);
          BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
          BGame::m_pMenuCurrent = &(BGame::m_menuMain);
          BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), &CVeraView::OnKeyDownCurrentMenu);
        }
      }
    }
  } else if (BGame::m_pMenuCurrent->m_type == BMenu::CHOOSE_SCENE) {
    StartMenuScroll(SCROLL_LEFT);
    BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
    BGame::m_pMenuCurrent = &(BGame::m_menuMain);
    BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), &CVeraView::OnKeyDownCurrentMenu);
  } else if (BGame::m_pMenuCurrent->m_type == BMenu::CHOOSE_VEHICLE) {
    if (BGame::m_bBuyingVehicle || BGame::m_bCannotBuyVehicle) {
      BGame::m_bBuyingVehicle = false;
      BGame::m_bCannotBuyVehicle = false;
      BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), &CVeraView::OnKeyDownCurrentMenu);
    } else {
      StartMenuScroll(SCROLL_LEFT);
      BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
      BGame::m_pMenuCurrent = &(BGame::m_menuChooseScene);
      BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), &CVeraView::OnKeyDownCurrentMenu);
    }
  }
}

void CVeraView::StartMenuScroll(TMenuScroll scroll) {
  m_clockMenuScroll = SDL_GetTicks();
  m_scrollDir = scroll;
}

void CVeraView::OnKeyDownCurrentMenu(int nChar, int nRepCnt, int nFlags) {
  switch (nChar) {
  case SDLK_ESCAPE:
    break;
  case SDLK_RETURN:
    break;
  }
}

void CVeraView::OnKeyDownCancelQuestion(int nChar, int nRepCnt, int nFlags) {
  switch (nChar) {
  case SDLK_ESCAPE:
    // act as Nope was selected
    m_game.m_bShowCancelQuestion = false;
    m_nMenuTime += BGame::ContinueSimulation();
    m_pKeyDownFunction = &CVeraView::OnKeyDownGame;
    break;
  case SDLK_RETURN:
    switch (m_game.m_nYesNoSelection) {
    case 1:
      // Cancel Order (i.e. jump to home)
      if (!BGame::m_bJumpToHome) {
        m_game.m_bShowCancelQuestion = false;
        BGame::GetPlayer()->m_dCash -= 50;
        BGame::m_bJumpToHome = true;
        BGame::GetSimulation()->GetCamera()->m_locMode = BCamera::OVERVIEW;
        BGame::FreezeSimulation();
        // freeze so that the vehicle doesn't jump up
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
      break;
    case 2:
      // Don't cancel order
      m_game.m_bShowCancelQuestion = false;
      // Return to game
      m_nMenuTime += BGame::ContinueSimulation();
      m_pKeyDownFunction = &CVeraView::OnKeyDownGame;
      break;
    }
    break;
  case SDLK_DOWN:
  case SDLK_RIGHT:
    m_game.m_nYesNoSelection -= 1;
    if (m_game.m_nYesNoSelection == 0) {
      m_game.m_nYesNoSelection = 2;
    }
    break;
  case SDLK_UP:
  case SDLK_LEFT:
    m_game.m_nYesNoSelection += 1;
    if (m_game.m_nYesNoSelection == 3) {
      m_game.m_nYesNoSelection = 1;
    }
    break;
  }
}
