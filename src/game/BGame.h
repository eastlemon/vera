//
// BGame: Center location for controlling the gameplay

#pragma once

#include <SDL2/SDL.h>

#include <string>

using namespace std;

#include "BCmdModule.h"
#include "BMenu.h"
#include "BNavSatWnd.h"
#include "BPlayer.h"
#include "BSceneEditor.h"
#include "BServiceWnd.h"
#include "BSimulation.h"
#include "BUI.h"
#include "BaseClasses.h"

class CVeraView;

const int g_cnWindParticles = 80;

class BGame {
  static BSimulation m_simulation;
  static BPlayer m_player;
  static BCmdModule m_cmdModule;
  static BNavSatWnd m_navsatWnd;
  static BServiceWnd m_serviceWnd;
  static BSceneEditor m_sceneEditor;
  static CVeraView *m_pView;

  static unsigned m_clockFrozenStart;
  static int m_nFreezeRefCount;

public:
  static int m_nDispWidth;
  static int m_nDispHeight;
  static int m_nDispBits;
  static int m_nDispHz;
  static int m_nTerrainResolution;
  static int m_nDustAndClouds;
  static int m_nWaterSurface;
  static int m_nMusicVolume;
  static int m_nVehicleVolume;
  static int m_nSoundscape;
  static int m_nSkyDetail;
  static int m_nDistantDetail;
  static int m_nWaterDetail;
  static int m_nColorMode;
  static int m_nScreenFormat;
  static int m_nTextureSmoothness;
  static int m_nPhysicsSteps;
  static bool m_bSlowMotion;  // "Bullet-time"
  static bool m_bDisplayInfo; // Show fps, speed, altitude etc.
  static bool m_bDashboard;   // Show dashboard?
  static bool m_bNavSat;      // Show Nav-Sat map?
  static bool m_bService;     // Show service window?
  static bool m_bShowDust;    // Show dust effects?
  static bool m_bShowQuickHelp;
  static bool m_bDrawOnScreenTracking;
  static bool m_bFrozen;
  static bool m_bShowHint;
  static unsigned m_clockHintStart;
  static unsigned m_clockLastLift;
  static double m_dNavSatHandleAngle;
  static double m_dServiceHandleAngle;
  static int m_nGameMenuSelection;
  static int m_nYesNoSelection;
  static bool m_bShowGameMenu;
  static bool m_bShowCancelQuestion;
  static bool m_bSceneEditorMode;
  static bool m_bFadingIn;
  static unsigned m_clockFadeStart;
  static BMenu *m_pMenuCurrent;
  static BMenu *m_pMenuPrevious;
  static bool m_bMenusCreated;
  static bool m_bMenuMode;

  static double m_dPizzaTemp;

  static string m_sScene;
  static string m_sVehicle;

  static BMenu m_menuMain;
  static BMenu m_menuChooseScene;
  static BMenu m_menuChooseVehicle;
  static BMenu m_menuSettings;
  static BMenu m_menuPrecachingTerrain;
  static bool m_bSettingsFromGame;

  static bool m_bGameLoading;
  static bool m_bGameReadyToStart;
  static bool m_bQuitPending;

  static BVector m_vGasStationClosest;
  static unsigned m_clockLastFuelExit;
  static unsigned m_clockFuelingStarted;
  static bool m_bFueling;
  static int m_nFuelSelect;
  static bool m_bFuelingInProgress;

  static BObject *m_pClient;
  static BObject *m_pBase;
  static unsigned m_clockLastPizzaTempCheck;
  static unsigned m_clockPickupStart;
  static unsigned m_clockDeliveryStart;
  static bool m_bDeliveryStartInProgress;
  static bool m_bPickupStartInProgress;

  static int m_nPizzaDamaged;
  static double m_dLastProfit;
  static double m_dLastDeliveryDistance;
  static int m_nPizza;
  static int m_nPizzas;
  static string m_sPizzas[11];

  // static CRITICAL_SECTION m_csMutex;
  static SDL_mutex *m_csMutex;
  static double m_dProgressMax;
  static double m_dProgressPos;

  static bool m_bJumpToHome;
  static bool m_bBuyingVehicle;
  static bool m_bCannotBuyVehicle;
  static double m_dPurchasePrice;

  static bool m_bHasEarthquakes;
  static bool m_bEarthquakeActive;
  static double m_bEarthquakeFactor;
  static unsigned m_bEarthquakeStarted;
  static unsigned m_bEarthquakeWillEnd;
  static unsigned m_bEarthquakeNextStart;

  static bool m_bWindActive;
  static BVector m_vWindDirection; // unit vector
  static BVector m_vWindParticles[g_cnWindParticles];
  static double m_dWindStrength; // i.e. how far particle flies in one SimStep
  static double
      m_dBaseWindStrength; // i.e. how far particle flies in one SimStep

  static bool m_bMultiProcessor;

  static bool m_bAnalyzerMode;
  static unsigned m_clockAnalyzerStarted;
  static int m_nVisualize;

  static BUISelectionList m_listYesNo;
  static BUISelectionList m_listOK;

  enum TState { PICKUP, DELIVERY };
  static TState m_state;

  BGame();
  ~BGame();
  static BSimulation *GetSimulation() { return &m_simulation; }
  static BPlayer *GetPlayer() { return &m_player; }
  static BCmdModule *Command() { return &m_cmdModule; }
  static BNavSatWnd *GetNavSat() { return &m_navsatWnd; }
  static BServiceWnd *GetServiceWnd() { return &m_serviceWnd; }
  static BSceneEditor *GetSceneEditor() { return &m_sceneEditor; }
  static CVeraView *GetView() { return m_pView; }
  static void SetView(CVeraView *pView) { m_pView = pView; }

  static TState GetState() { return m_state; }
  static void SetState(TState state) { m_state = state; }

  static BObject *GetCurrentClient() { return m_pClient; }
  static void SetCurrentClient(BObject *pClient);
  static BObject *GetBase() { return m_pBase; }

  static void SetupScene();
  static double GetMaxProfitFor(double dDist, double dPizzaTemp);

  static void SetProgressRange(double dMax);
  static void SetProgressPos(double dPos);
  static double GetRelativeProgress();

  static string GetScrambleChecksum();

  static void FreezeSimulation(bool bPause = false);
  static unsigned ContinueSimulation();
  static void SetupMenus();
  static void UpdateSettings();

  static void UpdateEarthquake();
  static void UpdateWindParticles(bool bInit = false);

  enum TAnalyzerVis {
    SKY = 1,
    WATER = 2,
    TERRAIN = 4,
    VEHICLE = 8,
    GASSTATIONS = 16,
    OBJECTS = 32,
    DUSTANDCLOUDS = 64,
    GRAPHICS2D = 128,
    ALL = 255
  };

  static void UpdateAnalyzer();

  static void MyAfxMessageBox(string sText, int nTmp = 0);

private:
  static void SetBase(BObject *pBase) { m_pBase = pBase; }
  static void EnumerateScreenResolutions();
  static bool FindStringFromArray(string s, string *psArray, int nItems,
                                  int &rnIndex);
};
