// All access settings module for all users of the game
#include "Settings.h"
#include "BGame.h"
#include "BGround.h"
#include "SoundModule.h"

// string Settings::m_sFilename = _T(".\\settings.dat");
string Settings::m_sFilename = "settings.dat";
// string Settings::m_sBrowserPath = _T("c:\\program files\\internet
// explorer\\iexplore.exe");
#if defined(__APPLE__) && defined(__MACH__)
string Settings::m_sBrowserPath =
    "open"; // macOS: use `open` to launch default browser
#elif defined(__linux__)
string Settings::m_sBrowserPath = "xdg-open"; // Linux: use `xdg-open`
#else
string Settings::m_sBrowserPath =
    "c:\\program files\\internet explorer\\iexplore.exe";
#endif

extern bool g_cbBlackAndWhite;
extern bool g_cbMipMap;

void Settings::ReadSettings(BSimulation *pSimulation) {
  /*char  sComputerName[MAX_COMPUTERNAME_LENGTH + 1];
  DWORD dwLen = MAX_COMPUTERNAME_LENGTH + 1;
  if(GetComputerName(sComputerName, &dwLen)) {
    m_sFilename = ".\\";
    m_sFilename += sComputerName;
    m_sFilename += "settings.dat";
  }*/

  FILE *fp;
  // fp = fopen(m_sFilename, "r");
  fp = fopen(m_sFilename.c_str(), "r");
  if (fp) {
    char sLine[1024];
    fgets(sLine, 1024, fp);
    /*sscanf(sLine, "ScreenRes = %d, %d, %d, %d",
      &(BGame::m_nDispWidth),
      &(BGame::m_nDispHeight),
      &(BGame::m_nDispBits),
      &(BGame::m_nDispHz));*/ //we use only desktop resolution now
    fgets(sLine, 1024, fp);
    sscanf(sLine, "TerrainResolution = %d", &(BGame::m_nTerrainResolution));
    fgets(sLine, 1024, fp);
    sscanf(sLine, "DustAndClouds = %d", &(BGame::m_nDustAndClouds));
    fgets(sLine, 1024, fp);
    sscanf(sLine, "MusicVolume = %d", &(BGame::m_nMusicVolume));
    fgets(sLine, 1024, fp);
    sscanf(sLine, "VehicleVolume = %d", &(BGame::m_nVehicleVolume));
    fgets(sLine, 1024, fp);
    sscanf(sLine, "Soundscape = %d", &(BGame::m_nSoundscape));
    SoundModule::SetSoundSpace(1 - BGame::m_nSoundscape);
    fgets(sLine, 1024, fp);
    sscanf(sLine, "SkyDetail = %d", &(BGame::m_nSkyDetail));
    fgets(sLine, 1024, fp);
    sscanf(sLine, "DistantDetail = %d", &(BGame::m_nDistantDetail));
    fgets(sLine, 1024, fp);
    sscanf(sLine, "WaterDetail = %d", &(BGame::m_nWaterDetail));
    fgets(sLine, 1024, fp);
    sscanf(sLine, "ColorMode = %d", &(BGame::m_nColorMode));
    fgets(sLine, 1024, fp);
    sscanf(sLine, "ScreenFormat = %d", &(BGame::m_nScreenFormat));
    fgets(sLine, 1024, fp);
    sscanf(sLine, "TextureSmoothness = %d", &(BGame::m_nTextureSmoothness));
    g_cbMipMap = (BGame::m_nTextureSmoothness == 2);

    int nTmp;
    fgets(sLine, 1024, fp);
    sscanf(sLine, "MenuMusicVolume = %d", &nTmp);
    SoundModule::SetMenuMusicVolume(nTmp);
    fgets(sLine, 1024, fp);
    sscanf(sLine, "VehicleSoundsVolume = %d", &nTmp);
    SoundModule::SetVehicleSoundsVolume(nTmp);
    fgets(sLine, 1024, fp);
    sscanf(sLine, "MessageSoundVolume = %d", &nTmp);
    SoundModule::SetMessageSoundVolume(nTmp);
    fgets(sLine, 1024, fp); // legacy "ControllerID = 0" line - ignored

    fgets(sLine, 1024, fp);
    sscanf(sLine, "KEYMAP-Left = %u", &ControllerModule::m_keymap.m_unLeft);
    fgets(sLine, 1024, fp);
    sscanf(sLine, "KEYMAP-Right = %u", &ControllerModule::m_keymap.m_unRight);
    fgets(sLine, 1024, fp);
    sscanf(sLine, "KEYMAP-Accelerate = %u",
           &ControllerModule::m_keymap.m_unAccelerate);
    fgets(sLine, 1024, fp);
    sscanf(sLine, "KEYMAP-Reverse = %u",
           &ControllerModule::m_keymap.m_unReverse);
    fgets(sLine, 1024, fp);
    sscanf(sLine, "KEYMAP-Break = %u", &ControllerModule::m_keymap.m_unBreak);
    fgets(sLine, 1024, fp);
    sscanf(sLine, "KEYMAP-Jet = %u", &ControllerModule::m_keymap.m_unJet);
    fgets(sLine, 1024, fp);
    sscanf(sLine, "KEYMAP-Heli = %u", &ControllerModule::m_keymap.m_unHeli);
    fgets(sLine, 1024, fp);
    sscanf(sLine, "KEYMAP-Lift = %u", &ControllerModule::m_keymap.m_unLift);
    fgets(sLine, 1024, fp);
    sscanf(sLine, "KEYMAP-Camera = %u", &ControllerModule::m_keymap.m_unCamera);

    // Legacy CONTROLLERMAP-* lines (joystick bindings, now removed) - skip
    // 9 lines: Left, Right, Accelerate, Reverse, Break, Jet, Heli, Lift, Camera
    for (int i = 0; i < 9; ++i) {
      fgets(sLine, 1024, fp);
    }

    g_cbBlackAndWhite = (BGame::m_nColorMode == 0);
    switch (BGame::m_nDistantDetail) {
    case 0:
      pSimulation->GetGround()->m_dCoarseDistance = 300;
      break;
    case 1:
      pSimulation->GetGround()->m_dCoarseDistance = 600;
      break;
    case 2:
      pSimulation->GetGround()->m_dCoarseDistance = 16000;
      break;
    }
    fgets(sLine, 1024, fp);
    sscanf(sLine, "WaterSurface = %d", &(BGame::m_nWaterSurface));
    fclose(fp);
  } else {
    string sTmp;
    // sTmp.Format("Cannot open %s. Using default settings.", m_sFilename);
    sTmp.assign("Cannot open ");
    sTmp.append(m_sFilename);
    sTmp.append(". Using default settings.");
    // BGame::MyAfxMessageBox(sTmp, MB_OK); //FIXME
  }
}

void Settings::WriteSettings(BSimulation *pSimulation) {
  /*char  sComputerName[MAX_COMPUTERNAME_LENGTH + 1];
  DWORD dwLen = MAX_COMPUTERNAME_LENGTH + 1;
  if(GetComputerName(sComputerName, &dwLen)) {
    m_sFilename = ".\\";
    m_sFilename += sComputerName;
    m_sFilename += "settings.dat";
  }*/
  FILE *fp;
  // fp = fopen(m_sFilename, "w");
  fp = fopen(m_sFilename.c_str(), "w");
  if (fp) {
    fprintf(fp, "ScreenRes = %d, %d, %d, %d\n", BGame::m_nDispWidth,
            BGame::m_nDispHeight, BGame::m_nDispBits, BGame::m_nDispHz);
    fprintf(fp, "TerrainResolution = %d\n", BGame::m_nTerrainResolution);
    fprintf(fp, "DustAndClouds = %d\n", BGame::m_nDustAndClouds);
    fprintf(fp, "MusicVolume = %d\n", BGame::m_nMusicVolume);
    fprintf(fp, "VehicleVolume = %d\n", BGame::m_nVehicleVolume);
    fprintf(fp, "SoundSpace = %d\n", BGame::m_nSoundscape);
    fprintf(fp, "SkyDetail = %d\n", BGame::m_nSkyDetail);
    fprintf(fp, "DistantDetail = %d\n", BGame::m_nDistantDetail);
    fprintf(fp, "WaterDetail = %d\n", BGame::m_nWaterDetail);
    fprintf(fp, "ColorMode = %d\n", BGame::m_nColorMode);
    fprintf(fp, "ScreenFormat = %d\n", BGame::m_nScreenFormat);
    fprintf(fp, "TextureSmoothness = %d\n", BGame::m_nTextureSmoothness);

    fprintf(fp, "MenuMusicVolume = %d\n", SoundModule::GetMenuMusicVolume());
    fprintf(fp, "VehicleSoundsVolume = %d\n",
            SoundModule::GetVehicleSoundsVolume());
    fprintf(fp, "MessageSoundVolume = %d\n",
            SoundModule::GetMessageSoundVolume());

    fprintf(fp, "ControllerID = 0\n"); // legacy - always 0 (keyboard) now

    fprintf(fp, "KEYMAP-Left = %u\n", ControllerModule::m_keymap.m_unLeft);
    fprintf(fp, "KEYMAP-Right = %u\n", ControllerModule::m_keymap.m_unRight);
    fprintf(fp, "KEYMAP-Accelerate = %u\n",
            ControllerModule::m_keymap.m_unAccelerate);
    fprintf(fp, "KEYMAP-Reverse = %u\n",
            ControllerModule::m_keymap.m_unReverse);
    fprintf(fp, "KEYMAP-Break = %u\n", ControllerModule::m_keymap.m_unBreak);
    fprintf(fp, "KEYMAP-Jet = %u\n", ControllerModule::m_keymap.m_unJet);
    fprintf(fp, "KEYMAP-Heli = %u\n", ControllerModule::m_keymap.m_unHeli);
    fprintf(fp, "KEYMAP-Lift = %u\n", ControllerModule::m_keymap.m_unLift);
    fprintf(fp, "KEYMAP-Camera = %u\n", ControllerModule::m_keymap.m_unCamera);

    // Legacy CONTROLLERMAP-* lines (joystick bindings, now removed).
    // Write empty placeholders so old settings.dat files keep their layout.
    for (int i = 0; i < 9; ++i) {
      fprintf(fp, "CONTROLLERMAP-legacy = \n");
    }
    fprintf(fp, "WaterSurface = %d\n", BGame::m_nWaterSurface);
    fclose(fp);
  } else {
    string sTmp;
    // sTmp.Format("Cannot open %s. Settings were not saved.", m_sFilename);
    sTmp.assign("Cannot open ");
    sTmp.append(m_sFilename);
    sTmp.append(". Using default settings.");
    // BGame::MyAfxMessageBox(sTmp);
  }
}
