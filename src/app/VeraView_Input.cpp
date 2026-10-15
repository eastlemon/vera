// VeraView_Input.cpp - keyboard/mouse/touch dispatch + dialog helpers.

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
extern bool g_bMouseButtonDown;
extern double g_dRate;
extern double g_d10LastFPS[];

void CVeraView::OnChar(int nChar, int nRepCnt, int nFlags) {
  if (m_pDrawFunction != &CVeraView::OnDrawGame) {
    return;
  }

  if (m_bIgnoreNextChar) {
    m_bIgnoreNextChar = false;
    return;
  }

  // Check if Scene Editor processes key input
  switch (m_game.GetSceneEditor()->m_phase) {
  // case BSceneEditor::TPhase::ASKING_OBJECT_NAME:
  case BSceneEditor::ASKING_OBJECT_NAME:
  // case BSceneEditor::TPhase::ASKING_SCENE_DISPLAY_NAME:
  case BSceneEditor::ASKING_SCENE_DISPLAY_NAME:
  // case BSceneEditor::TPhase::ASKING_SCENE_FILENAME:
  case BSceneEditor::ASKING_SCENE_FILENAME: {
    
    m_game.GetSceneEditor()->m_edit.ProcessChar(nChar);
    BUIEdit::TStatus statusEdit;
    string sObjectName = m_game.GetSceneEditor()->m_edit.GetValue(statusEdit);
    // if(statusEdit == BUIEdit::TStatus::READY) {
    if (statusEdit == BUIEdit::READY) {
      m_game.GetSceneEditor()->AdvancePhase();
      // if(m_game.GetSceneEditor()->m_phase ==
      // BSceneEditor::TPhase::SELECTING_OBJECT_TYPE) {
      if (m_game.GetSceneEditor()->m_phase ==
          BSceneEditor::SELECTING_OBJECT_TYPE) {
        BUI::StartUsingSelectionList(
            &(m_game.GetSceneEditor()->m_sellistObjectType),
            &CVeraView::OnKeyDownSceneEditor);
      }
      //} else if(statusEdit == BUIEdit::TStatus::CANCELED) {
    } else if (statusEdit == BUIEdit::CANCELED) {
      m_game.GetSceneEditor()->CancelPhase();
    }
    return;
  } break;
  }

  // Check if service window processes key input
  if ((BGame::m_bService) &&
      ((toupper(nChar) >= 32) && (toupper(nChar) <= 96) ||
       // nChar == VK_BACK ||
       nChar == SDLK_BACKSPACE ||
       // nChar == VK_RETURN)) {
       nChar == SDLK_RETURN)) {
    BGame::GetServiceWnd()->AddChar(toupper(nChar));
    return;
  }
}

void CVeraView::OnKeyDownGame(int nChar, int nRepCnt, int nFlags) {
  if (m_pDrawFunction != &CVeraView::OnDrawGame) {
    return;
  }

  static bool bOpen = false;
  // Check for user input

  if (m_game.GetSimulation()->GetCamera()->m_bNeedsToBeInitialized) {
    m_game.GetSimulation()->GetCamera()->m_locMode = BCamera::FOLLOW;
    m_game.GetSimulation()->GetCamera()->m_dAngleOfView = 75.0;
    m_game.GetSimulation()->GetCamera()->m_bNeedsToBeInitialized = false;
  }

  bool bProcessed = false;
  BVector vToHome;

  // Process non-writing key commands always
  switch (nChar) {
  // case VK_CONTROL:
  case SDLK_LCTRL:
    g_bControl = true;
    break;
  // case VK_SHIFT:
  case SDLK_LSHIFT:
    g_bShift = true;
    break;
  // case VK_ESCAPE:
  case SDLK_ESCAPE:
    bProcessed = true;

    if (m_game.m_bShowQuickHelp) {
      m_game.m_bShowQuickHelp = false;
      m_nMenuTime += BGame::ContinueSimulation();
    } else if (BGame::m_bDeliveryStartInProgress) {
      m_nMenuTime += BGame::ContinueSimulation();
      BGame::m_bDeliveryStartInProgress = false;
      BMessages::Show(50, "pickup", "RETURN TO BASE", 8, true);
    } else if (BGame::m_bPickupStartInProgress) {
      m_nMenuTime += BGame::ContinueSimulation();
      BGame::m_bPickupStartInProgress = false;
      BMessages::Show(50, "deliver", "DELIVER PIZZA TO CLIENT", 8, true);
      BGame::m_nPizzaDamaged = 0;
    } else {
      BGame::FreezeSimulation();
      m_pKeyDownFunction = &CVeraView::OnKeyDownGameMenu;
      m_game.m_bShowGameMenu = !m_game.m_bShowGameMenu;
    }
    break;
  // case VK_F1:
  case SDLK_F1:
    bProcessed = true;
    if (!m_game.m_bShowQuickHelp) {
      BGame::FreezeSimulation(false);
      // show quick help
      m_game.m_bShowQuickHelp = true;
    }
    break;
  // case VK_HOME:
  //   bProcessed = true;
  //   m_game.GetSimulation()->GetCamera()->m_dFollowHeight = -3.0;
  //   vToHome = m_game.GetSimulation()->GetVehicle()->GetHomeLocation() -
  //   m_game.GetSimulation()->GetVehicle()->m_vLocation + BVector(35, 0, -20);
  //   m_game.GetSimulation()->GetVehicle()->Move(vToHome);
  //   m_game.GetSimulation()->UpdateCar();
  //   break;
  // case VK_DELETE:
  //   bProcessed = true;
  //   m_game.GetSimulation()->GetVehicle()->Move(BVector(0, 0, -1000.0));
  //   m_game.GetSimulation()->UpdateCar();
  //   break;
  // case VK_F2:
  case SDLK_F2:
    bProcessed = true;
    m_game.GetSimulation()->GetCamera()->m_locMode = BCamera::FOLLOW;
    m_game.GetSimulation()->GetCamera()->m_dAngleOfView = 75.0;
    m_messages.Remove("camera");
    m_messages.Show(60, "camera", "camera: chase", 1);
    break;
  // case VK_F3:
  case SDLK_F3:
    bProcessed = true;
    m_game.GetSimulation()->GetCamera()->m_locMode = BCamera::OVERVIEW;
    m_game.GetSimulation()->GetCamera()->m_bInitLoc = true;
    m_game.GetSimulation()->GetCamera()->m_dAngleOfView = 80.0;
    m_messages.Remove("camera");
    m_messages.Show(60, "camera", "camera: overview", 1);
    break;
  // case VK_F4:
  case SDLK_F4:
    bProcessed = true;
    m_game.GetSimulation()->GetCamera()->m_locMode = BCamera::INCAR;
    m_game.GetSimulation()->GetCamera()->m_dAngleOfView = 75.0;
    m_messages.Remove("camera");
    m_messages.Show(60, "camera", "camera: 1st person", 1);
    break;
  // case VK_F5:
  case SDLK_F5:
    bProcessed = true;
    m_game.GetSimulation()->GetCamera()->m_locMode = BCamera::FIXED;
    m_game.GetSimulation()->GetCamera()->m_vFixLocation =
        m_game.GetSimulation()->GetCamera()->m_vLocation;
    m_game.GetSimulation()->GetCamera()->m_dAngleOfView = 70.0;
    m_messages.Remove("camera");
    m_messages.Show(60, "camera", "camera: stationary", 1);
    break;
  // case VK_F7:
  case SDLK_F7:
    bProcessed = true;
    m_game.Command()->Run("toggle dashboard");
    break;
  // case VK_F8:
  case SDLK_F8:
    bProcessed = true;
    m_game.Command()->Run("toggle navsat");
    break;
  // case VK_F9:
  case SDLK_F9:
    bProcessed = true;
    m_game.Command()->Run("toggle service");
    break;
  // case VK_F11:
  case SDLK_F11:
    bProcessed = true;
    m_game.GetSimulation()->m_bSteeringAidOn =
        !m_game.GetSimulation()->m_bSteeringAidOn;
    if (m_game.GetSimulation()->m_bSteeringAidOn) {
      m_messages.Remove("steeringaid");
      m_messages.Show(60, "steeringaid", "Steering aid on", 1);
    } else {
      m_messages.Remove("steeringaid");
      m_messages.Show(60, "steeringaid", "Steering aid off", 1);
    }
    break;
  // case VK_ADD:
  //   bProcessed = true;
  //   m_game.GetSimulation()->m_dAccelerationFactor *= 2.0;
  //   break;
  // case VK_SUBTRACT:
  //   bProcessed = true;
  //   m_game.GetSimulation()->m_dAccelerationFactor /= 2.0;
  //   break;
  // case VK_PRIOR:
  case SDLK_PAGEUP:
    bProcessed = true;
    m_game.GetSimulation()->GetCamera()->m_dFollowHeight -= 1.0;
    break;
  // case VK_NEXT:
  case SDLK_PAGEDOWN:
    bProcessed = true;
    m_game.GetSimulation()->GetCamera()->m_dFollowHeight += 1.0;
    if (m_game.GetSimulation()->GetCamera()->m_dFollowHeight > 0.0) {
      m_game.GetSimulation()->GetCamera()->m_dFollowHeight = 0.0;
    }
    break;
  }

  if (!bProcessed) {
    // Check if service window processes key input
    if (!BGame::m_bService) {
      if (nChar == ControllerModule::m_keymap.m_unAccelerate) {
        bProcessed = true;
        m_game.GetSimulation()->GetVehicle()->m_bAccelerating = true;
        m_game.GetSimulation()->GetVehicle()->m_dAccelerationFactor =
            1.0 * m_game.GetSimulation()->GetVehicle()->m_dFuelFactor;
        if (m_game.GetSimulation()->GetVehicle()->m_rotor.m_nHeliMode > 399) {
          m_game.GetSimulation()->GetVehicle()->m_rotor.m_bHeliForwarding =
              true;
          m_game.GetSimulation()->GetVehicle()->m_dAccelerationFactor =
              1.0 * m_game.GetSimulation()->GetVehicle()->m_dKerosineFactor;
        }
        if (m_game.GetSimulation()->GetVehicle()->m_jet.m_nJetMode > 100) {
          m_game.GetSimulation()->GetVehicle()->m_dAccelerationFactor =
              1.0 * m_game.GetSimulation()->GetVehicle()->m_dKerosineFactor;
        }
      } else if (nChar == ControllerModule::m_keymap.m_unReverse) {
        bProcessed = true;
        m_game.GetSimulation()->GetVehicle()->m_bReversing = true;
        m_game.GetSimulation()->GetVehicle()->m_dReversingFactor =
            1.0 * m_game.GetSimulation()->GetVehicle()->m_dFuelFactor;
        if (m_game.GetSimulation()->GetVehicle()->m_rotor.m_nHeliMode > 399) {
          m_game.GetSimulation()->GetVehicle()->m_rotor.m_bHeliBacking = true;
        }
      } else if (nChar == ControllerModule::m_keymap.m_unPropeller) {
        bProcessed = true;
        if (m_game.GetSimulation()->GetVehicle()->m_rotor.m_nHeliMode > 0) {
          m_game.GetSimulation()->GetVehicle()->m_rotor.m_bHeliLifting = true;
        } else {
          m_game.GetSimulation()->GetVehicle()->m_bPropeller = true;
          // m_game.GetSimulation()->GetVehicle()->m_dPropellerFactor = 1.0 *
          // m_game.GetSimulation()->GetVehicle()->m_dKerosineFactor;
        }
      } else if (nChar == ControllerModule::m_keymap.m_unPropellerReverse) {
        bProcessed = true;
        if (m_game.GetSimulation()->GetVehicle()->m_rotor.m_nHeliMode > 0) {
          m_game.GetSimulation()->GetVehicle()->m_rotor.m_bHeliDescending =
              true;
        } else {
          m_game.GetSimulation()->GetVehicle()->m_bPropReverse = true;
          // m_game.GetSimulation()->GetVehicle()->m_dPropellerFactor = -0.5 *
          // m_game.GetSimulation()->GetVehicle()->m_dKerosineFactor;
        }
      } else if (nChar == ControllerModule::m_keymap.m_unBreak) {
        bProcessed = true;
        m_game.GetSimulation()->GetVehicle()->m_bBreaking = true;
      } else if (nChar == ControllerModule::m_keymap.m_unLeft) {
        bProcessed = true;
        m_game.GetSimulation()->GetVehicle()->m_bTurningLeft = true;
        if ((m_game.GetSimulation()->GetVehicle()->m_rotor.m_nHeliMode > 399) &&
            (m_game.GetSimulation()->GetVehicle()->m_dKerosineFactor > 0.0)) {
          m_game.GetSimulation()->GetVehicle()->m_rotor.m_bHeliLefting = true;
        }
      } else if (nChar == ControllerModule::m_keymap.m_unRight) {
        bProcessed = true;
        m_game.GetSimulation()->GetVehicle()->m_bTurningRight = true;
        if ((m_game.GetSimulation()->GetVehicle()->m_rotor.m_nHeliMode > 399) &&
            (m_game.GetSimulation()->GetVehicle()->m_dKerosineFactor > 0.0)) {
          m_game.GetSimulation()->GetVehicle()->m_rotor.m_bHeliRighting = true;
        }
      } else if (nChar == ControllerModule::m_keymap.m_unCamera) {
        bProcessed = true;
        m_game.GetSimulation()->SwitchCameraMode();
      } else if (nChar == ControllerModule::m_keymap.m_unLift) {
        bProcessed = true;
        if (!m_game.GetSimulation()->m_bLiftingUp) {
          if ((m_game.GetSimulation()->GetVehicle()->m_orientation.m_vUp.m_dZ >
               0.0) ||
              (((SDL_GetTicks() - m_game.m_clockLastLift) / 1000.0) < 6)) {
            m_game.m_clockLastLift = SDL_GetTicks();
            m_game.GetSimulation()->m_bLiftingUp = true;
            m_game.GetSimulation()->m_dLiftZ = m_game.GetSimulation()
                                                   ->GetVehicle()
                                                   ->m_pBodyPoint[0]
                                                   .m_vLocation.m_dZ;
          }
        }
      } else if ((nChar == ControllerModule::m_keymap.m_unHeli) &&
                 (m_game.GetSimulation()->GetVehicle()->m_dKerosineFactor >
                  0.0)) {
        bProcessed = true;
        m_game.GetSimulation()->ToggleHeli();
      } else if ((nChar == ControllerModule::m_keymap.m_unJet) &&
                 (m_game.GetSimulation()->GetVehicle()->m_dKerosineFactor >
                  0.0)) {
        bProcessed = true;
        m_game.Command()->Run("toggle jet");
      }

      switch (nChar) {
      case '0':
        bProcessed = true;
        m_game.FreezeSimulation();
        m_game.GetSimulation()->GetTerrain()->SetRenderResolution(
            BTerrain::RENDER_SLOW_MACHINE);
        BGame::UpdateSettings();
        BGame::GetSimulation()->GetTerrain()->MakeTerrainValid(
            BGame::GetSimulation()->GetVehicle()->m_vLocation,
            BGame::GetSimulation()->GetCamera()->m_vLocation,
            BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward, true,
            false, false);
        BGame::GetSimulation()->EnsureVehicleIsOverGround();
        m_nMenuTime += BGame::ContinueSimulation();
        break;
      case '1':
        bProcessed = true;
        if (BGame::m_bNavSat && g_bControl) {
          BGame::Command()->Run("set navsat resolution 10400");
          BGame::m_dNavSatHandleAngle = 20.0;
        } else {
          m_game.FreezeSimulation();
          m_game.GetSimulation()->GetTerrain()->SetRenderResolution(
              BTerrain::RENDER_MINIMUM);
          BGame::UpdateSettings();
          BGame::GetSimulation()->GetTerrain()->MakeTerrainValid(
              BGame::GetSimulation()->GetVehicle()->m_vLocation,
              BGame::GetSimulation()->GetCamera()->m_vLocation,
              BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward,
              true, false, false);
          BGame::GetSimulation()->EnsureVehicleIsOverGround();
          m_nMenuTime += BGame::ContinueSimulation();
        }
        break;
      case '2':
        bProcessed = true;
        if (BGame::m_bNavSat && g_bControl) {
          BGame::Command()->Run("set navsat resolution 5200");
          BGame::m_dNavSatHandleAngle = 10.0;
        } else {
          m_game.FreezeSimulation();
          m_game.GetSimulation()->GetTerrain()->SetRenderResolution(
              BTerrain::RENDER_LOW);
          BGame::UpdateSettings();
          BGame::GetSimulation()->GetTerrain()->MakeTerrainValid(
              BGame::GetSimulation()->GetVehicle()->m_vLocation,
              BGame::GetSimulation()->GetCamera()->m_vLocation,
              BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward,
              true, false, false);
          BGame::GetSimulation()->EnsureVehicleIsOverGround();
          m_nMenuTime += BGame::ContinueSimulation();
        }
        break;
      case '3':
        bProcessed = true;
        if (BGame::m_bNavSat && g_bControl) {
          BGame::Command()->Run("set navsat resolution 2600");
          BGame::m_dNavSatHandleAngle = 0.0;
        } else {
          m_game.FreezeSimulation();
          m_game.GetSimulation()->GetTerrain()->SetRenderResolution(
              BTerrain::RENDER_MEDIUM);
          BGame::UpdateSettings();
          BGame::GetSimulation()->GetTerrain()->MakeTerrainValid(
              BGame::GetSimulation()->GetVehicle()->m_vLocation,
              BGame::GetSimulation()->GetCamera()->m_vLocation,
              BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward,
              true, false, false);
          BGame::GetSimulation()->EnsureVehicleIsOverGround();
          m_nMenuTime += BGame::ContinueSimulation();
        }
        break;
      case '4':
        bProcessed = true;
        if (BGame::m_bNavSat && g_bControl) {
          BGame::Command()->Run("set navsat resolution 1300");
          BGame::m_dNavSatHandleAngle = -10.0;
        } else {
          m_game.FreezeSimulation();
          m_game.GetSimulation()->GetTerrain()->SetRenderResolution(
              BTerrain::RENDER_HIGH);
          BGame::UpdateSettings();
          BGame::GetSimulation()->GetTerrain()->MakeTerrainValid(
              BGame::GetSimulation()->GetVehicle()->m_vLocation,
              BGame::GetSimulation()->GetCamera()->m_vLocation,
              BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward,
              true, false, false);
          BGame::GetSimulation()->EnsureVehicleIsOverGround();
          m_nMenuTime += BGame::ContinueSimulation();
        }
        break;
      case '5':
        bProcessed = true;
        if (BGame::m_bNavSat && g_bControl) {
          BGame::Command()->Run("set navsat resolution 650");
          BGame::m_dNavSatHandleAngle = -20.0;
        } else {
          m_game.FreezeSimulation();
          m_game.GetSimulation()->GetTerrain()->SetRenderResolution(
              BTerrain::RENDER_MAXIMUM);
          BGame::UpdateSettings();
          BGame::GetSimulation()->GetTerrain()->MakeTerrainValid(
              BGame::GetSimulation()->GetVehicle()->m_vLocation,
              BGame::GetSimulation()->GetCamera()->m_vLocation,
              BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward,
              true, false, false);
          BGame::GetSimulation()->EnsureVehicleIsOverGround();
          m_nMenuTime += BGame::ContinueSimulation();
        }
        break;
      case 'w':
      case 'W': {
        bProcessed = true;
        // Check whether this is the second click
        static unsigned clockPrev = 0;
        unsigned clockNow = SDL_GetTicks();
        if ((clockNow - clockPrev) < (1000.0 / 3)) {
          m_game.GetSimulation()->GetVehicle()->m_bWireframe =
              !m_game.GetSimulation()->GetVehicle()->m_bWireframe;
          // m_bWireframe = !m_bWireframe;
          // m_bCreateDLs = true;
        }
        clockPrev = clockNow;
      } break;
      // case 'n':
      // case 'N':
      //   bProcessed = true;
      //   m_bNormals = !m_bNormals;
      //   m_bCreateDLs = true;
      //   break;
      case 'v':
      case 'V':
        bProcessed = true;
        m_bShowVideo = !m_bShowVideo;
        break;
      case 'd':
      case 'D':
        bProcessed = true;
        m_game.m_bShowDust = !m_game.m_bShowDust;
        if (m_game.m_bShowDust) {
          m_game.m_nDustAndClouds = 0;
          m_messages.Remove("dust");
          m_messages.Show(60, "dust", "dust on", 1);
        } else {
          m_game.m_nDustAndClouds = 1;
          m_messages.Remove("dust");
          m_messages.Show(60, "dust", "dust off", 1);
        }
        BGame::UpdateSettings();
        break;
      case 'a':
      case 'A': {
        bProcessed = true;

        // Check whether this is the second click
        static unsigned clockPrev = 0;
        unsigned clockNow = SDL_GetTicks();
        if ((clockNow - clockPrev) < (1000.0 / 3)) {
          BGame::FreezeSimulation(false);
          // Start Analyzer
          BGame::m_bAnalyzerMode = true;
          BGame::m_clockAnalyzerStarted = clockNow;

          {
            // FIXME
            /*BGame::MyAfxMessageBox("--------------------------");
            BGame::MyAfxMessageBox("ANALYZER STARTED!");
            BGame::MyAfxMessageBox("--------------------------");
            string sLogInfo;
            sLogInfo.Format("Vehicle: %s",
            BGame::GetSimulation()->GetVehicle()->m_sName);
            BGame::MyAfxMessageBox(sLogInfo);
            sLogInfo.Format("Scene: %s",
            BGame::GetSimulation()->GetScene()->m_sName);
            BGame::MyAfxMessageBox(sLogInfo);
            sLogInfo.Format("Screen: %d*%d*%d @ %dHz", BGame::m_nDispWidth,
            BGame::m_nDispHeight, BGame::m_nDispBits, BGame::m_nDispHz);
            BGame::MyAfxMessageBox(sLogInfo);
            sLogInfo.Format("Terrain: %d", BGame::m_nTerrainResolution);
            BGame::MyAfxMessageBox(sLogInfo);
            sLogInfo.Format("Effects: dust=%d water=%d",
            BGame::m_nDustAndClouds, BGame::m_nWaterSurface);
            BGame::MyAfxMessageBox(sLogInfo);
            sLogInfo.Format("FPS: AVE=%.2lf, Last10=%.1lf %.1lf %.1lf %.1lf
            %.1lf %.1lf %.1lf %.1lf %.1lf %.1lf ", g_dRate, g_d10LastFPS[0],
                            g_d10LastFPS[1],
                            g_d10LastFPS[2],
                            g_d10LastFPS[3],
                            g_d10LastFPS[4],
                            g_d10LastFPS[5],
                            g_d10LastFPS[6],
                            g_d10LastFPS[7],
                            g_d10LastFPS[8],
                            g_d10LastFPS[9]);
            BGame::MyAfxMessageBox(sLogInfo);*/
          }
        }
        clockPrev = clockNow;
      } break;
      case 'e':
      case 'E': {
        bProcessed = true;

        // Check whether this is the second click
        static unsigned clockPrev = 0;
        unsigned clockNow = SDL_GetTicks();
        if ((clockNow - clockPrev) < (1000.0 / 3)) {
          BGame::FreezeSimulation(false);
          m_game.GetSceneEditor()->Activate();
          m_messages.Remove("sceneeditor");
          m_messages.Show(50, "sceneeditor", "scene editor mode", 1);
          m_pKeyDownFunction = &CVeraView::OnKeyDownSceneEditor;
        }
        clockPrev = clockNow;
      } break;
      case 'i':
      case 'I':
        bProcessed = true;
        m_game.m_bDisplayInfo = !m_game.m_bDisplayInfo;
        break;
      case 'b':
      case 'B':
        bProcessed = true;
        m_game.m_bSlowMotion = !m_game.m_bSlowMotion;
        if (m_game.m_bSlowMotion) {
          m_messages.Remove("slow motion");
          m_messages.Show(40, "slow motion", "slow motion", 1);
        } else {
          m_messages.Remove("slow motion");
          m_messages.Show(40, "slow motion", "slow motion off", 1);
        }
        break;
      case 'f':
      case 'F':
        bProcessed = true;
        {
          if (!m_game.m_bFrozen) {
            m_messages.Show(40, "frozen", "Frozen (f)", 999);
            BGame::FreezeSimulation();
          } else {
            m_messages.Remove("frozen");
            m_nMenuTime += BGame::ContinueSimulation();
          }
        }
        break;
      }

      switch (nChar) {
      // case VK_SPACE:
      case SDLK_SPACE:
        bProcessed = true;
        m_game.GetSimulation()->GetVehicle()->m_bHandBreaking = true;
        break;
      }
    }
  }

  if (!bProcessed) {
    m_game.m_bShowHint = true;
    m_game.m_clockHintStart = SDL_GetTicks();
  }
}

void CVeraView::OnKeyDownFueling(int nChar, int nRepCnt, int nFlags) {
  switch (nChar) {
  // case VK_ESCAPE:
  case SDLK_ESCAPE:
    m_nMenuTime += BGame::ContinueSimulation();
    m_pKeyDownFunction = &CVeraView::OnKeyDownGame;
    m_game.m_bFueling = false;
    m_game.m_clockLastFuelExit = SDL_GetTicks();
    BMessages::Show(50, "fuelexit", "Exit gas station (within 3 seconds)", 3);
    break;
  // case VK_RETURN:
  case SDLK_RETURN:
    if (m_game.m_nFuelSelect == 2) {
      m_nMenuTime += BGame::ContinueSimulation();
      m_pKeyDownFunction = &CVeraView::OnKeyDownGame;
      m_game.m_bFueling = false;
      m_game.m_clockLastFuelExit = SDL_GetTicks();
      BMessages::Show(50, "fuelexit", "Exit gas station (within 3 seconds)", 3);

      BVehicle *pVehicle = BGame::GetSimulation()->GetVehicle();

      if (pVehicle->m_dFuel > 0.001) {
        pVehicle->m_dFuelFactor = 1.0;
        pVehicle->m_dAccelerationFactor = 1.0;
        pVehicle->m_dReversingFactor = 1.0;
      }
      if (pVehicle->m_dKerosine > 0.001) {
        pVehicle->m_dKerosineFactor = 1.0;
      }

    } else {
      m_game.m_bFuelingInProgress = !m_game.m_bFuelingInProgress;
      if (m_game.m_bFuelingInProgress) {
        m_game.m_clockFuelingStarted = SDL_GetTicks();
      }
    }
    break;
  // case VK_DOWN:
  case SDLK_DOWN:
    if (!BGame::m_bFuelingInProgress) {
      m_game.m_nFuelSelect = 2;
    }
    break;
  // case VK_RIGHT:
  case SDLK_RIGHT:
    if (!BGame::m_bFuelingInProgress) {
      m_game.m_nFuelSelect = 1;
    }
    break;
  // case VK_UP:
  case SDLK_UP:
  // case VK_LEFT:
  case SDLK_LEFT:
    if (!BGame::m_bFuelingInProgress) {
      m_game.m_nFuelSelect = 0;
    }
    break;
  }
  
}

void CVeraView::OnKeyDownGameMenu(int nChar, int nRepCnt, int nFlags) {
  switch (nChar) {
  // case VK_ESCAPE:
  case SDLK_ESCAPE:
    if (m_game.m_bShowQuickHelp) {
      m_game.m_bShowQuickHelp = false;
      m_nMenuTime += BGame::ContinueSimulation();
    } else {
      m_nMenuTime += BGame::ContinueSimulation();
      m_pKeyDownFunction = &CVeraView::OnKeyDownGame;
      m_game.m_bShowGameMenu = !m_game.m_bShowGameMenu;
    }
    break;
  // case VK_RETURN:
  case SDLK_RETURN:
    switch (m_game.m_nGameMenuSelection) {

      // NOTE! main menu selection is numbered from bottom up (i.e. 1 is
      // bottommost menu item)

    case 1:
      // Quit (Return to main menu)
      BGame::GetPlayer()->m_dFuel =
          BGame::GetSimulation()->GetVehicle()->m_dFuel;
      BGame::GetPlayer()->m_dKerosine =
          BGame::GetSimulation()->GetVehicle()->m_dKerosine;
      BGame::GetPlayer()->SaveStateFile();
      BGame::GetPlayer()->SaveCurrentSceneInfo();
      // ShowCursor(FALSE);
      SDL_ShowCursor(0);
      m_nMenuTime += BGame::ContinueSimulation();
      m_game.m_bShowGameMenu = false;
      BGame::m_bMenuMode = true;
      BGame::m_pMenuCurrent = &(BGame::m_menuMain);
      m_pKeyDownFunction = &CVeraView::OnKeyDownCurrentMenu;
      BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu),
                                   &CVeraView::OnKeyDownCurrentMenu);
      m_pDrawFunction = &CVeraView::OnDrawCurrentMenu;
      BGame::m_bGameLoading = false;
      m_bDrawOnlyMenu = false;
      m_bFullRedraw = true;
      // InvalidateRect(NULL);

      // Save log info
      {
        stringstream outputStream;
        BGame::MyAfxMessageBox("------------------------------------");
        BGame::MyAfxMessageBox("EXITING SCENE");
        string sLogInfo;
        // sLogInfo.Format("Vehicle: %s",
        // BGame::GetSimulation()->GetVehicle()->m_sName);
        sLogInfo = "Vehicle: " + BGame::GetSimulation()->GetVehicle()->m_sName;
        BGame::MyAfxMessageBox(sLogInfo);
        // sLogInfo.Format("Scene: %s",
        // BGame::GetSimulation()->GetScene()->m_sName);
        sLogInfo = "Scene: " + BGame::GetSimulation()->GetScene()->m_sName;
        BGame::MyAfxMessageBox(sLogInfo);
        // sLogInfo.Format("Screen: %d*%d*%d @ %dHz", BGame::m_nDispWidth,
        // BGame::m_nDispHeight, BGame::m_nDispBits, BGame::m_nDispHz);
        outputStream << "Screen: " << BGame::m_nDispWidth << "*"
                     << BGame::m_nDispHeight << "*" << BGame::m_nDispBits
                     << " @ " << BGame::m_nDispHz << "Hz";
        sLogInfo = outputStream.str();
        BGame::MyAfxMessageBox(sLogInfo);
        // sLogInfo.Format("Terrain: %d", BGame::m_nTerrainResolution);
        outputStream.str("");
        outputStream << "Terrain: " << BGame::m_nTerrainResolution;
        sLogInfo = outputStream.str();
        BGame::MyAfxMessageBox(sLogInfo);
        // sLogInfo.Format("Effects: dust=%d water=%d", BGame::m_nDustAndClouds,
        // BGame::m_nWaterSurface);
        outputStream.str("");
        outputStream << "Effects: dust=" << BGame::m_nDustAndClouds
                     << " water=" << BGame::m_nWaterSurface;
        sLogInfo = outputStream.str();
        BGame::MyAfxMessageBox(sLogInfo);
        /*sLogInfo.Format("FPS: AVE=%.2lf, Last10=%.1lf %.1lf %.1lf %.1lf %.1lf
           %.1lf %.1lf %.1lf %.1lf %.1lf ", g_dRate, g_d10LastFPS[0],
                        g_d10LastFPS[1],
                        g_d10LastFPS[2],
                        g_d10LastFPS[3],
                        g_d10LastFPS[4],
                        g_d10LastFPS[5],
                        g_d10LastFPS[6],
                        g_d10LastFPS[7],
                        g_d10LastFPS[8],
                        g_d10LastFPS[9]);*/
        outputStream.str("");
        outputStream << "FPS: AVE=" << g_dRate << ", Last10=" << g_d10LastFPS[0]
                     << " " << g_d10LastFPS[1] << " " << g_d10LastFPS[2] << " "
                     << g_d10LastFPS[3] << " " << g_d10LastFPS[4] << " "
                     << g_d10LastFPS[5] << " " << g_d10LastFPS[6] << " "
                     << g_d10LastFPS[7] << " " << g_d10LastFPS[8] << " "
                     << g_d10LastFPS[9];
        sLogInfo = outputStream.str();
        BGame::MyAfxMessageBox(sLogInfo);
      }

      // Start menu music
      SoundModule::StopEngineSound();
      SoundModule::StartMenuMusic();
      SoundModule::SetMenuMusicVolume(
          int(double(BGame::m_nMusicVolume) / 100.0 * 255.0));

      break;
    case 2:
      // Show Help
      m_nMenuTime += BGame::ContinueSimulation();
      m_pKeyDownFunction = &CVeraView::OnKeyDownGame;
      m_game.m_bShowGameMenu = false;
      m_game.FreezeSimulation(false);
      // show quick help
      m_game.m_bShowQuickHelp = true;
      break;
    case 3:
      // Settings
      BGame::m_pMenuPrevious = 0;
      BGame::m_bMenuMode = true;
      BGame::m_bSettingsFromGame = true;
      BGame::m_pMenuCurrent = &(BGame::m_menuSettings);
      m_pDrawFunction = &CVeraView::OnDrawCurrentMenu;
      BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu),
                                   &CVeraView::OnKeyDownCurrentMenu);
      StartMenuScroll(SCROLL_LEFT);
      
      break;
    case 4:
      // Cancel Order
      m_game.m_nYesNoSelection = 1; // preselect "Yeah"
      m_pKeyDownFunction = &CVeraView::OnKeyDownCancelQuestion;
      m_game.m_bShowGameMenu = !m_game.m_bShowGameMenu;
      m_game.m_bShowCancelQuestion = true;
      
      break;
    case 5:
      // Return to game
      m_nMenuTime += BGame::ContinueSimulation();
      m_pKeyDownFunction = &CVeraView::OnKeyDownGame;
      m_game.m_bShowGameMenu = false;
      break;
    }
    break;
  // case VK_DOWN:
  case SDLK_DOWN:
    m_game.m_nGameMenuSelection -= 1;
    if (m_game.m_nGameMenuSelection == 0) {
      m_game.m_nGameMenuSelection = 5;
    }
    
    break;
  // case VK_UP:
  case SDLK_UP:
    m_game.m_nGameMenuSelection += 1;
    if (m_game.m_nGameMenuSelection == 6) {
      m_game.m_nGameMenuSelection = 1;
    }
    
    break;
  }
}

void CVeraView::OnKeyDownSceneEditor(int nChar, int nRepCnt, int nFlags) {
  BCamera *pCamera = m_game.GetSimulation()->GetCamera();
  BScene *pScene = m_game.GetSimulation()->GetScene();

  switch (m_game.GetSceneEditor()->m_phase) {
  // case BSceneEditor::TPhase::ASKING_OBJECT_NAME:
  case BSceneEditor::ASKING_OBJECT_NAME:
  // case BSceneEditor::TPhase::ASKING_SCENE_DISPLAY_NAME:
  case BSceneEditor::ASKING_SCENE_DISPLAY_NAME:
  // case BSceneEditor::TPhase::ASKING_SCENE_FILENAME:
  case BSceneEditor::ASKING_SCENE_FILENAME:
    // OnChar() processes these
    return;
    break;
  }

  double dModeScaler = 20.0;
  if (g_bShift) {
    dModeScaler = 1.0;
  }
  // if((m_game.GetSceneEditor()->m_phase == BSceneEditor::TPhase::BASIC) &&
  if ((m_game.GetSceneEditor()->m_phase == BSceneEditor::BASIC) && g_bControl) {
    dModeScaler = 150.0;
  }

  switch (nChar) {
  // case VK_CONTROL:
  case SDLK_LCTRL:
    g_bControl = true;
    break;
  // case VK_SHIFT:
  case SDLK_LSHIFT:
    g_bShift = true;
    break;
  // case VK_RETURN:
  case SDLK_RETURN:
    // If moving object, return to camera mode
    // if(BGame::GetSceneEditor()->m_phase ==
    // BSceneEditor::TPhase::MOVING_OBJECT) {
    if (BGame::GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) {
      BGame::GetSceneEditor()->AdvancePhase();
    }
    break;
  case 'e':
  case 'E':
    // Return to game mode (NOTE: SHOULD WE WARN ABOUT AN UNSAVED SCENE?)
    m_game.GetSceneEditor()->Deactivate();
    m_pKeyDownFunction = &CVeraView::OnKeyDownGame;
    m_messages.Remove("sceneeditor");
    m_messages.Show(50, "sceneeditor", "game mode", 1);
    m_nMenuTime += BGame::ContinueSimulation();
    break;
  // case VK_RIGHT:
  case SDLK_RIGHT:
    // if((m_game.GetSceneEditor()->m_phase ==
    // BSceneEditor::TPhase::MOVING_OBJECT) &&
    if ((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
        (!g_bControl)) {
      // Move object to right
      BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
      if (pObject) {
        pObject->m_vLocation +=
            pCamera->m_orientation.m_vRight * 0.1 * dModeScaler;
        pObject->m_vCenter +=
            pCamera->m_orientation.m_vRight * 0.1 * dModeScaler;
        pObject->RecreateShadow();
      }
    } else {
      if (!g_bShift) {
        pCamera->m_orientation.m_vForward =
            pCamera->m_orientation.m_vForward +
            pCamera->m_orientation.m_vRight * 0.02 * dModeScaler;
      } else {
        pCamera->m_orientation.m_vForward =
            pCamera->m_orientation.m_vForward +
            pCamera->m_orientation.m_vRight * 0.05 * dModeScaler;
      }
      pCamera->m_orientation.m_vForward.ToUnitLength();
      pCamera->m_orientation.m_vRight =
          pCamera->m_orientation.m_vUp.CrossProduct(
              pCamera->m_orientation.m_vForward);
    }
    break;
  // case VK_LEFT:
  case SDLK_LEFT:
    // if((m_game.GetSceneEditor()->m_phase ==
    // BSceneEditor::TPhase::MOVING_OBJECT) &&
    if ((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
        (!g_bControl)) {
      // Move object to west
      BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
      if (pObject) {
        pObject->m_vLocation -=
            pCamera->m_orientation.m_vRight * 0.1 * dModeScaler;
        pObject->m_vCenter -=
            pCamera->m_orientation.m_vRight * 0.1 * dModeScaler;
        pObject->RecreateShadow();
      }
    } else {
      if (!g_bShift) {
        pCamera->m_orientation.m_vForward =
            pCamera->m_orientation.m_vForward +
            pCamera->m_orientation.m_vRight * -0.02 * dModeScaler;
      } else {
        pCamera->m_orientation.m_vForward =
            pCamera->m_orientation.m_vForward +
            pCamera->m_orientation.m_vRight * -0.05 * dModeScaler;
      }
      pCamera->m_orientation.m_vForward.ToUnitLength();
      pCamera->m_orientation.m_vRight =
          pCamera->m_orientation.m_vUp.CrossProduct(
              pCamera->m_orientation.m_vForward);
    }
    break;
  // case VK_DOWN:
  case SDLK_DOWN:
    // if((m_game.GetSceneEditor()->m_phase ==
    // BSceneEditor::TPhase::MOVING_OBJECT) &&
    if ((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
        (!g_bControl)) {
      // Move object to south
      BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
      if (pObject) {
        pObject->m_vLocation -=
            pCamera->m_orientation.m_vForward * 0.1 * dModeScaler;
        pObject->m_vCenter -=
            pCamera->m_orientation.m_vForward * 0.1 * dModeScaler;
        pObject->RecreateShadow();
      }
    } else {
      pCamera->m_vLocation = pCamera->m_vLocation +
                             pCamera->m_orientation.m_vForward * -dModeScaler;
    }
    break;
  // case VK_UP:
  case SDLK_UP:
    // if((m_game.GetSceneEditor()->m_phase ==
    // BSceneEditor::TPhase::MOVING_OBJECT) &&
    if ((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
        (!g_bControl)) {
      // Move object to north
      BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
      if (pObject) {
        pObject->m_vLocation +=
            pCamera->m_orientation.m_vForward * 0.1 * dModeScaler;
        pObject->m_vCenter +=
            pCamera->m_orientation.m_vForward * 0.1 * dModeScaler;
        pObject->RecreateShadow();
      }
    } else {
      pCamera->m_vLocation = pCamera->m_vLocation +
                             pCamera->m_orientation.m_vForward * dModeScaler;
    }
    break;
  // case VK_NEXT:
  case SDLK_PAGEDOWN:
    // if((m_game.GetSceneEditor()->m_phase ==
    // BSceneEditor::TPhase::MOVING_OBJECT) &&
    if ((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
        (!g_bControl)) {
      // Move object down
      BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
      if (pObject) {
        pObject->m_vLocation.m_dZ += 0.1 * dModeScaler;
        pObject->m_vCenter.m_dZ += 0.1 * dModeScaler;
        pObject->RecreateShadow();
      }
    } else {
      pCamera->m_vLocation =
          pCamera->m_vLocation + pCamera->m_orientation.m_vUp * -dModeScaler;
    }
    break;
  // case VK_PRIOR:
  case SDLK_PAGEUP:
    // if((m_game.GetSceneEditor()->m_phase ==
    // BSceneEditor::TPhase::MOVING_OBJECT) &&
    if ((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
        (!g_bControl)) {
      // Move object up
      BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
      if (pObject) {
        pObject->m_vLocation.m_dZ -= 0.1 * dModeScaler;
        pObject->m_vCenter.m_dZ -= 0.1 * dModeScaler;
        pObject->RecreateShadow();
      }
    } else {
      pCamera->m_vLocation =
          pCamera->m_vLocation + pCamera->m_orientation.m_vUp * dModeScaler;
    }
    break;
  // case VK_HOME:
  case SDLK_HOME:
    // if((m_game.GetSceneEditor()->m_phase ==
    // BSceneEditor::TPhase::MOVING_OBJECT) &&
    if ((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
        (!g_bControl)) {
      // Rotate clockwise
      BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
      if (pObject) {
        pObject->m_dZRotation += 1 * dModeScaler;
        pObject->RecreateShadow();
      }
    }
    break;
  // case VK_END:
  case SDLK_END:
    // if((m_game.GetSceneEditor()->m_phase ==
    // BSceneEditor::TPhase::MOVING_OBJECT) &&
    if ((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
        (!g_bControl)) {
      // Rotate counter clockwise
      BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
      if (pObject) {
        pObject->m_dZRotation -= 1 * dModeScaler;
        pObject->RecreateShadow();
      }
    }
    break;
  case 'o':
  case 'O':
    // if((m_game.GetSceneEditor()->m_phase ==
    // BSceneEditor::TPhase::MOVING_OBJECT) &&
    if ((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
        (!g_bControl)) {
      // Scale object's active radius
      BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
      if (pObject) {
        double dScaler = 1.01;
        if (!g_bShift) {
          dScaler = 2.0;
        }
        pObject->m_dActiveRadius *= dScaler;
      }
    }
    break;
  case 'i':
  case 'I':
    // if((m_game.GetSceneEditor()->m_phase ==
    // BSceneEditor::TPhase::MOVING_OBJECT) &&
    if ((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
        (!g_bControl)) {
      // Shrink object's active radius
      BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
      if (pObject) {
        double dScaler = (1.0 / 1.01);
        if (!g_bShift) {
          dScaler = 0.5;
        }
        pObject->m_dActiveRadius *= dScaler;
      }
    }
    break;
  case 'l':
  case 'L':
    // if((m_game.GetSceneEditor()->m_phase ==
    // BSceneEditor::TPhase::MOVING_OBJECT) &&
    if ((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
        (!g_bControl)) {
      // Scale object bigger
      BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
      if (pObject) {
        double dScaler = 1.01;
        if (!g_bShift) {
          dScaler = 2.0;
        }
        pObject->m_dScale2 *= dScaler;
        pObject->m_dRadius *= dScaler;
        pObject->RecreateShadow();
      }
    }
    break;
  case 'k':
  case 'K':
    // if((m_game.GetSceneEditor()->m_phase ==
    // BSceneEditor::TPhase::MOVING_OBJECT) &&
    if ((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
        (!g_bControl)) {
      // Scale object bigger
      BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
      if (pObject) {
        double dScaler = (1.0 / 1.01);
        if (!g_bShift) {
          dScaler = 0.5;
        }
        pObject->m_dScale2 *= dScaler;
        pObject->m_dRadius *= dScaler;
        pObject->RecreateShadow();
      }
    }
    break;
  case 'm':
  case 'M':
    BUI::StartUsingSelectionList(
        &(m_game.GetSimulation()->GetScene()->m_sellistSceneObjects),
        &CVeraView::OnKeyDownSceneEditor);
    m_game.GetSimulation()->GetScene()->m_sellistSceneObjects.SelectItem(
        BGame::GetSceneEditor()->m_sActiveObject);
    // m_game.GetSceneEditor()->m_phase =
    // BSceneEditor::TPhase::SELECTING_SCENE_OBJECT;
    m_game.GetSceneEditor()->m_phase = BSceneEditor::SELECTING_SCENE_OBJECT;
    break;
  case 'a':
  case 'A':
    m_game.GetSceneEditor()->m_edit.Setup("Object name:", "", 32);
    // m_game.GetSceneEditor()->m_phase =
    // BSceneEditor::TPhase::ASKING_OBJECT_NAME;
    m_game.GetSceneEditor()->m_phase = BSceneEditor::ASKING_OBJECT_NAME;
    m_bIgnoreNextChar = true;
    break;
  case 'd':
  case 'D':
    BUI::StartUsingSelectionList(
        &(m_game.GetSimulation()->GetScene()->m_sellistSceneObjects),
        &CVeraView::OnKeyDownSceneEditor);
    m_game.GetSimulation()->GetScene()->m_sellistSceneObjects.SelectItem(
        BGame::GetSceneEditor()->m_sActiveObject);
    // m_game.GetSceneEditor()->m_phase =
    // BSceneEditor::TPhase::SELECTING_SCENE_OBJECT_TO_DELETE;
    m_game.GetSceneEditor()->m_phase =
        BSceneEditor::SELECTING_SCENE_OBJECT_TO_DELETE;
    break;
  case 's':
  case 'S':
    m_game.GetSceneEditor()->m_edit.Setup(
        "Scene display name:", pScene->m_sName, 32);
    // m_game.GetSceneEditor()->m_phase =
    // BSceneEditor::TPhase::ASKING_SCENE_DISPLAY_NAME;
    m_game.GetSceneEditor()->m_phase = BSceneEditor::ASKING_SCENE_DISPLAY_NAME;
    m_bIgnoreNextChar = true;
    break;
  // case VK_F1:
  case SDLK_F1:
    // show quick help
    m_game.m_bShowQuickHelp = true;
    break;
  // case VK_ESCAPE:
  case SDLK_ESCAPE:
    if (m_game.m_bShowQuickHelp) {
      m_game.m_bShowQuickHelp = false;
    }
    break;
  case '0':
    m_game.GetSimulation()->GetTerrain()->SetRenderResolution(
        BTerrain::RENDER_SLOW_MACHINE);
    BGame::UpdateSettings();
    break;
  case '1':
    if (BGame::m_bNavSat && g_bControl) {
      BGame::Command()->Run("set navsat resolution 10400");
      BGame::m_dNavSatHandleAngle = 20.0;
    } else {
      m_game.GetSimulation()->GetTerrain()->SetRenderResolution(
          BTerrain::RENDER_MINIMUM);
      BGame::UpdateSettings();
    }
    break;
  case '2':
    if (BGame::m_bNavSat && g_bControl) {
      BGame::Command()->Run("set navsat resolution 5200");
      BGame::m_dNavSatHandleAngle = 10.0;
    } else {
      m_game.GetSimulation()->GetTerrain()->SetRenderResolution(
          BTerrain::RENDER_LOW);
      BGame::UpdateSettings();
    }
    break;
  case '3':
    if (BGame::m_bNavSat && g_bControl) {
      BGame::Command()->Run("set navsat resolution 2600");
      BGame::m_dNavSatHandleAngle = 0.0;
    } else {
      m_game.GetSimulation()->GetTerrain()->SetRenderResolution(
          BTerrain::RENDER_MEDIUM);
      BGame::UpdateSettings();
    }
    break;
  case '4':
    if (BGame::m_bNavSat && g_bControl) {
      BGame::Command()->Run("set navsat resolution 1300");
      BGame::m_dNavSatHandleAngle = -10.0;
    } else {
      m_game.GetSimulation()->GetTerrain()->SetRenderResolution(
          BTerrain::RENDER_HIGH);
      BGame::UpdateSettings();
    }
    break;
  case '5':
    if (BGame::m_bNavSat && g_bControl) {
      BGame::Command()->Run("set navsat resolution 650");
      BGame::m_dNavSatHandleAngle = -20.0;
    } else {
      m_game.GetSimulation()->GetTerrain()->SetRenderResolution(
          BTerrain::RENDER_MAXIMUM);
      BGame::UpdateSettings();
    }
    break;
  // case VK_F7:
  case SDLK_F7:
    m_game.Command()->Run("toggle dashboard");
    break;
  // case VK_F8:
  case SDLK_F8:
    m_game.Command()->Run("toggle navsat");
    break;
  // case VK_F9:
  case SDLK_F9:
    m_game.Command()->Run("toggle service");
    break;
  }
  
}

void CVeraView::OnKeyDownSelectionList(int nChar, int nRepCnt, int nFlags) {
  BUISelectionList *pList = BUI::GetActiveSelectionList();
  if (!pList) {
    m_pKeyDownFunction = BUI::StopUsingSelectionList();
    
  }

  bool bAdvancePhase = false;

  switch (nChar) {
  // case VK_DOWN:
  case SDLK_DOWN:
    pList->AdvanceSelection(1);
    break;
  // case VK_UP:
  case SDLK_UP:
    pList->AdvanceSelection(-1);
    break;
  // case VK_HOME:
  case SDLK_HOME:
    pList->AdvanceSelection(-1000); // to get to the beginning
    break;
  // case VK_END:
  case SDLK_END:
    pList->AdvanceSelection(1000); // to get to the end
    break;
  // case VK_RETURN:
  case SDLK_RETURN:
    m_pKeyDownFunction = BUI::StopUsingSelectionList();
    bAdvancePhase = true;

    if (BGame::m_bMenuMode) {
      ReturnPressedOnCurrentMenu();
    }

    break;
  // case VK_ESCAPE:
  case SDLK_ESCAPE:
    pList->Cancel();
    m_pKeyDownFunction = BUI::StopUsingSelectionList();
    if (BGame::m_bSceneEditorMode) {
      m_game.GetSceneEditor()->CancelPhase();
    }
    bAdvancePhase = false;

    if (BGame::m_bMenuMode) {
      CancelPressedOnCurrentMenu();
    }

    break;
  }

  if (bAdvancePhase && BGame::m_bSceneEditorMode) {
    BGame::GetSceneEditor()->AdvancePhase();
  }

  
}

void CVeraView::OnKeyDownSlider(int nChar, int nRepCnt, int nFlags) {
  int *pnSliderValue = BUI::m_pnSliderValue;
  if (!pnSliderValue) {
    m_pKeyDownFunction = BUI::StopUsingSlider();
    
  }

  switch (nChar) {
  // case VK_DOWN:
  case SDLK_DOWN:
    *pnSliderValue -= 10;
    break;
  // case VK_UP:
  case SDLK_UP:
    *pnSliderValue += 10;
    break;
  // case VK_LEFT:
  case SDLK_LEFT:
    *pnSliderValue -= 1;
    break;
  // case VK_RIGHT:
  case SDLK_RIGHT:
    *pnSliderValue += 1;
    break;
  // case VK_HOME:
  case SDLK_HOME:
    *pnSliderValue = 0;
    break;
  // case VK_END:
  case SDLK_END:
    *pnSliderValue = 100;
    break;
  // case VK_RETURN:
  case SDLK_RETURN:
    m_pKeyDownFunction = BUI::StopUsingSlider();

    if (BGame::m_bMenuMode) {
      ReturnPressedOnCurrentMenu();
    }

    break;
  // case VK_ESCAPE:
  case SDLK_ESCAPE:
    *pnSliderValue = BUI::m_nPrevSliderValue;
    m_pKeyDownFunction = BUI::StopUsingSlider();

    if (BGame::m_bMenuMode) {
      CancelPressedOnCurrentMenu();
    }

    break;
  }

  if (*pnSliderValue < 0) {
    *pnSliderValue = 0;
  }
  if (*pnSliderValue > 100) {
    *pnSliderValue = 100;
  }

  
}

void CVeraView::OnKeyDown(int nChar, int nRepCnt, int nFlags) {
  if (m_pKeyDownFunction) {
    // Call active draw function
    (this->*m_pKeyDownFunction)(nChar, nRepCnt, nFlags);
  }
}

void CVeraView::OnKeyUp(int nChar, int nRepCnt, int nFlags) {

  // don't even ask about these...
  // if(nChar == VK_CONTROL) {
  if (nChar == SDLK_LCTRL) {
    g_bControl = false;
  }
  // if(nChar == VK_SHIFT) {
  if (nChar == SDLK_LSHIFT) {
    g_bShift = false;
  }

  if (nChar == ControllerModule::m_keymap.m_unAccelerate) {
    m_game.GetSimulation()->GetVehicle()->m_bAccelerating = false;
    m_game.GetSimulation()->GetVehicle()->m_dAccelerationFactor = 0.0;
    if (m_game.GetSimulation()->GetVehicle()->m_rotor.m_nHeliMode > 399) {
      m_game.GetSimulation()->GetVehicle()->m_rotor.m_bHeliForwarding = false;
    }
  } else if (nChar == ControllerModule::m_keymap.m_unReverse) {
    m_game.GetSimulation()->GetVehicle()->m_bReversing = false;
    if (m_game.GetSimulation()->GetVehicle()->m_rotor.m_nHeliMode > 399) {
      m_game.GetSimulation()->GetVehicle()->m_rotor.m_bHeliBacking = false;
    }
  } else if (nChar == ControllerModule::m_keymap.m_unPropeller) {
    m_game.GetSimulation()->GetVehicle()->m_bPropeller = false;
    m_game.GetSimulation()->GetVehicle()->m_rotor.m_bHeliLifting = false;
  } else if (nChar == ControllerModule::m_keymap.m_unPropellerReverse) {
    m_game.GetSimulation()->GetVehicle()->m_bPropReverse = false;
    m_game.GetSimulation()->GetVehicle()->m_rotor.m_bHeliDescending = false;
  } else if (nChar == ControllerModule::m_keymap.m_unBreak) {
    m_game.GetSimulation()->GetVehicle()->m_bBreaking = false;
  } else if (nChar == ControllerModule::m_keymap.m_unLeft) {
    m_game.GetSimulation()->GetVehicle()->m_bTurningLeft = false;
    if (m_game.GetSimulation()->GetVehicle()->m_rotor.m_nHeliMode > 399) {
      m_game.GetSimulation()->GetVehicle()->m_rotor.m_bHeliLefting = false;
    }
  } else if (nChar == ControllerModule::m_keymap.m_unRight) {
    m_game.GetSimulation()->GetVehicle()->m_bTurningRight = false;
    if (m_game.GetSimulation()->GetVehicle()->m_rotor.m_nHeliMode > 399) {
      m_game.GetSimulation()->GetVehicle()->m_rotor.m_bHeliRighting = false;
    }
  } else if (nChar == ControllerModule::m_keymap.m_unLift) {
    m_game.GetSimulation()->m_bLiftingUp = false;
  } else {
    switch (nChar) {
    // case VK_SPACE:
    case SDLK_SPACE:
      m_game.GetSimulation()->GetVehicle()->m_bHandBreaking = false;
      break;
    }
  }
}

void CVeraView::ProcessMouseInput(MouseFlags nFlags, SDL_Point point) {
  // if(nFlags & MK_LBUTTON) {
  if (nFlags == MOUSE_DOWN) {
    point.y = window_height - point.y;
    if (BGame::m_bNavSat) {
      // See if navsat resolution is to be changed
      if ((point.x > (135 - 60)) && (point.x < (135 + 60)) && (point.y > 260) &&
          (point.y < (260 + 100))) {
        double dCurAngle = BGame::m_dNavSatHandleAngle;
        double dNewAngle;
        // possibly change resolution
        BVector vUp(0, 1, 0);
        BVector vRight(1, 0, 0);
        BVector vToMouse(point.x - 135.0, point.y - 187, 0);
        vToMouse.ToUnitLength();
        double dCos = vUp.ScalarProduct(vToMouse);
        double dTmp = vRight.ScalarProduct(vToMouse);
        if (dTmp < 0.0) {
          dNewAngle = acos(dCos);
        } else {
          dNewAngle = -acos(dCos);
        }
        dNewAngle = dNewAngle / 3.141592654 * 180.0;
        if (dNewAngle > 0.0) {
          dNewAngle = double(int((dNewAngle + 5.0) / 10.0)) * 10.0;
        } else {
          dNewAngle = double(int((dNewAngle - 5.0) / 10.0)) * 10.0;
        }
        if (fabs(dNewAngle) > 20.0) {
          dNewAngle = fabs(dNewAngle) / dNewAngle * 20.0;
        }
        if (dNewAngle != dCurAngle) {
          // Change to new resolution
          BGame::m_dNavSatHandleAngle = dNewAngle;
          if (dNewAngle == -20.0) {
            BGame::Command()->Run("set navsat resolution 650");
          } else if (dNewAngle == -10.0) {
            BGame::Command()->Run("set navsat resolution 1300");
          } else if (dNewAngle == 0.0) {
            BGame::Command()->Run("set navsat resolution 2600");
          } else if (dNewAngle == 10.0) {
            BGame::Command()->Run("set navsat resolution 5200");
          } else if (dNewAngle == 20.0) {
            BGame::Command()->Run("set navsat resolution 10400");
          }
        }
      }
    }
    if (BGame::m_bService) {
      // See if navsat resolution is to be changed
      // if((point.y > rectWnd.Height() - (135 + 60)) &&
      if ((point.y > window_height - (135 + 60)) &&
          //(point.y < rectWnd.Height() - (135 - 60)) &&
          (point.y < window_height - (135 - 60)) &&
          //(point.x > rectWnd.Width() - (360 + 100)) &&
          (point.x > window_width - (360 + 100)) &&
          //(point.x < rectWnd.Width() - (360))) {
          (point.x < window_width - (360))) {
        double dCurAngle = BGame::m_dServiceHandleAngle;
        double dNewAngle;
        // possibly change resolution
        BVector vUp(-1, 0, 0);
        BVector vRight(0, 1, 0);
        // BVector vToMouse(point.x - (rectWnd.Width() - 287), point.y -
        // (rectWnd.Height() - 135.0), 0);
        BVector vToMouse(point.x - (window_width - 287),
                         point.y - (window_height - 135.0), 0);
        vToMouse.ToUnitLength();
        double dCos = vUp.ScalarProduct(vToMouse);
        double dTmp = vRight.ScalarProduct(vToMouse);
        if (dTmp < 0.0) {
          dNewAngle = acos(dCos);
        } else {
          dNewAngle = -acos(dCos);
        }
        dNewAngle = dNewAngle / 3.141592654 * 180.0;
        if (fabs(dNewAngle) > 20.0) {
          dNewAngle = fabs(dNewAngle) / dNewAngle * 20.0;
        }
        BGame::m_dServiceHandleAngle = dNewAngle;
      }
    }
  }
}

void CVeraView::OnMouseMove(SDL_Point point) {
  mouse_x = point.x;
  mouse_y = point.y;
  if ((m_pDrawFunction == &CVeraView::OnDrawGame) &&
      (BGame::m_bNavSat || BGame::m_bService)) {
    // ProcessMouseInput(nFlags, point);
    ProcessMouseInput(MOUSE_NONE, point);
    return;
  }
}

void CVeraView::OnLButtonDown(SDL_Point point) {
  g_bMouseButtonDown = true;
  if ((m_pDrawFunction == &CVeraView::OnDrawGame) &&
      (BGame::m_bNavSat || BGame::m_bService)) {
    // ProcessMouseInput(nFlags, point);
    ProcessMouseInput(MOUSE_DOWN, point);
    return;
  }
}

void CVeraView::OnLButtonUp(SDL_Point point) { g_bMouseButtonDown = false; }

void CVeraView::OnFingerDown(float x, float y, int finger_id) {
  if (dialogs.size() > 0) {
    vector<Dialog *>::iterator i = --dialogs.end();
    (*i)->onFingerDown(x * window_width - (*i)->getPos()->x,
                       y * window_height - (*i)->getPos()->y, finger_id);
  } else if (!isDialogOpen<DlgOnScreenKbd *>() &&
             m_pDrawFunction == &CVeraView::OnDrawGame) {
    dialogs.push_back(new DlgOnScreenKbd(this));
  }
  touch_seen = true;
}

void CVeraView::OnFingerUp(float x, float y, int finger_id) {
  if (dialogs.size() > 0) {
    vector<Dialog *>::iterator i = --dialogs.end();
    (*i)->onFingerUp(x * window_width - (*i)->getPos()->x,
                     y * window_height - (*i)->getPos()->y, finger_id);
  }
  SDL_Point point;
  point.x = x * window_width;
  point.y = y * window_height;
  HandleBUITouch(point);
}

void CVeraView::drawDialogs() {
  for (int x = 0; x < dialogs.size(); x++) {
    if (dialogs[x]->isExit()) {
      delete dialogs[x];
      dialogs[x] = NULL;
      dialogs.erase(dialogs.begin() + x);
      x--;
    } else {
      SDL_Surface *surface = dialogs[x]->getSurface();
      SDL_Rect *r = dialogs[x]->getPos();
      if (dialogs[x]->isDefaultPos()) {
        r->x = window_width / 2 - surface->w / 2;
        r->y = window_height / 2 - surface->h / 2;
      }
      double x1 = r->x;
      double y1 = r->y;
      double x2 = r->x + r->w;
      double y2 = r->y + r->h;

      drawSurface(x1, y1, x2, y2, GL_RGBA, surface);
    }
  }
}

void CVeraView::drawSurface(double x1, double y1, double x2, double y2,
                            GLenum format, SDL_Surface *surface) {
  glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_FOG_BIT |
               GL_DEPTH_BUFFER_BIT | GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT |
               GL_POLYGON_BIT | GL_STENCIL_BUFFER_BIT);
  glViewport(0, 0, (GLint)window_width, (GLint)window_height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, window_width, window_height, 0, 0, 1);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_FOG);
  GLint front_face;
  glGetIntegerv(GL_FRONT_FACE, &front_face);
  glFrontFace(GL_CW);
  glDisable(GL_LIGHTING);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glColor4f(1.0, 1.0, 1.0, 1);

  // glClear(GL_COLOR_BUFFER_BIT);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  OpenGLHelpers::SetColorFull(1, 1, 1, 1);

  unsigned short row_data_width =
      surface->pitch / surface->format->BytesPerPixel;
  GLuint texture[1];
  glGenTextures(1, &texture[0]);
  glBindTexture(GL_TEXTURE_2D, texture[0]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, row_data_width, surface->h, 0, format,
               GL_UNSIGNED_BYTE, surface->pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glBindTexture(GL_TEXTURE_2D, texture[0]);

  glBegin(GL_QUADS);
  // glColor3f(0.0, 0.0, 0.0);
  glTexCoord2f(0.0f, 0.0f);
  glVertex2f(x1, y1);
  glTexCoord2f((float)surface->w / row_data_width, 0.0f);
  glVertex2f(x2, y1);
  glTexCoord2f((float)surface->w / row_data_width, 1.0f);
  glVertex2f(x2, y2);
  glTexCoord2f(0.0f, 1.0f);
  glVertex2f(x1, y2);
  glEnd();

  glDeleteTextures(1, &texture[0]);

  glFrontFace(front_face);
  glPopAttrib();
}

void CVeraView::HandleBUITouch(SDL_Point point) {
  if (BGame::m_bMenuMode) {
    BUISelectionList *pList = BUI::GetActiveSelectionList();
    if (!pList) {
      pList = &BGame::m_pMenuCurrent->m_listMenu;
    }
    double dX = window_width / 2;
    double dY = window_height / 2;

    // Check if we have open submenu and need position of it
    bool bScrolling = false;
    string sTmp;
    int nSelected = BGame::m_pMenuCurrent->m_listMenu.GetSelectedItem(sTmp);
    if (nSelected != -1) {
      BMenuItem *pMenuItem = &(BGame::m_pMenuCurrent->m_items[nSelected]);
      if (pMenuItem->m_bOpen &&
          pMenuItem->m_type == BMenuItem::STRING_FROM_LIST) {
        double dCharHeight = BUI::TextRenderer()->GetCharHeight();
        // Values from CVeraView::DrawMenu
        dX += 35;
        dY -= double(nSelected) * -dCharHeight +
              (dCharHeight * double(BGame::m_pMenuCurrent->m_nItems)) / 2.0;
        bScrolling = true;
      }
    }

    bool menu_item_pressed =
        pList->OnFingerUp(point.x, point.y, dX, dY, bScrolling);
    if (menu_item_pressed) {
      ReturnPressedOnCurrentMenu();
    } else {
      CancelPressedOnCurrentMenu();
    }

    nSelected = BGame::m_pMenuCurrent->m_listMenu.GetSelectedItem(sTmp);
    if (nSelected != -1) {
      BMenuItem *pMenuItem = &(BGame::m_pMenuCurrent->m_items[nSelected]);
      if (pMenuItem->m_bOpen && pMenuItem->m_type == BMenuItem::SLIDER) {
        *BUI::m_pnSliderValue = point.x - dX - 10;
        if (*BUI::m_pnSliderValue < 0) {
          *BUI::m_pnSliderValue = 0;
        }
        if (*BUI::m_pnSliderValue > 100) {
          *BUI::m_pnSliderValue = 100;
        }

        // closing slider as no need to keep it open
        ReturnPressedOnCurrentMenu();
      }
    }
  } else {
    // Using values from OnDrawGame
    if (m_game.m_bShowGameMenu) {
      double dHeight = 256.0 / 5.0;
      int i = (point.y - (window_height / 2.0 - 128)) / dHeight;
      m_game.m_nGameMenuSelection = 5 - i;
      OnKeyDownGameMenu(SDLK_RETURN, 0, 0);
    } else if (m_game.m_bShowCancelQuestion) {
      int win_w_center = window_width / 2.0 - 235.0 / 2.0 + 70.0;
      int win_h_center = window_height / 2.0 + 50 + 50;
      if (win_h_center - 37.0 < point.y && win_h_center > point.y) {
        if (win_w_center + (0 - 40.0) < point.x &&
            win_w_center + (80.0 - 40.0) > point.x) {
          m_game.m_nYesNoSelection = 1;
          OnKeyDownCancelQuestion(SDLK_RETURN, 0, 0);
        }
        if (win_w_center + (0 + 40.0) < point.x &&
            win_w_center + (86.0 + 40.0) > point.x) {
          m_game.m_nYesNoSelection = 2;
          OnKeyDownCancelQuestion(SDLK_RETURN, 0, 0);
        }
      }
    } else if (m_game.m_bFueling) {
      double dCharHeight = BUI::TextRenderer()->GetCharHeight();
      double dCharWidth = BUI::TextRenderer()->GetCharWidth();
      int win_w_center = window_width / 2.0;
      int win_h_center = window_height / 2.0 + dCharHeight * 1.5 + 54.0 / 2.0;

      if (win_w_center - 120.0 / 2.0 < point.x &&
          win_w_center + 120.0 / 2.0 > point.x &&
          win_h_center - 54.0 / 2.0 < point.y &&
          win_h_center + 54.0 / 2.0 > point.y) {
        m_game.m_nFuelSelect = 2;
        OnKeyDownFueling(SDLK_RETURN, 0, 0);
      }

      win_h_center = window_height / 2.0 - dCharHeight * 2.5 - 128.0 / 2.0;
      if (win_h_center - 128.0 / 2.0 < point.y &&
          win_h_center + 128.0 / 2.0 > point.y) {
        win_w_center = window_width / 2.0 - 70.0;
        if (win_w_center - 128.0 / 2.0 < point.x &&
            win_w_center + 128.0 / 2.0 > point.x) {
          m_game.m_nFuelSelect = 0;
          OnKeyDownFueling(SDLK_RETURN, 0, 0);
        }
        win_w_center = window_width / 2.0 + 70.0;
        if (win_w_center - 128.0 / 2.0 < point.x &&
            win_w_center + 128.0 / 2.0 > point.x) {
          m_game.m_nFuelSelect = 1;
          OnKeyDownFueling(SDLK_RETURN, 0, 0);
        }
      }
    } else if (m_game.m_bShowQuickHelp || m_game.m_bPickupStartInProgress ||
               m_game.m_bDeliveryStartInProgress) {
      OnKeyDownGame(SDLK_ESCAPE, 0, 0);
    }
  }
}

