// Settings - reads/writes game settings via the SQLite database.
//
// Previously used a flat settings.dat file with line-by-line parsing.
// Now uses Database::getSetting/setSetting which stores key-value pairs
// in the SQLite database (vera.db). On first launch, the Database
// migration system imports any existing settings.dat automatically.
#include "Settings.h"
#include "BGame.h"
#include "BGround.h"
#include "Database.h"
#include "SoundModule.h"

#if defined(__APPLE__) && defined(__MACH__)
string Settings::m_sBrowserPath = "open";
#elif defined(__linux__)
string Settings::m_sBrowserPath = "xdg-open";
#else
string Settings::m_sBrowserPath = "c:\\program files\\internet explorer\\iexplore.exe";
#endif

string Settings::m_sFilename = "vera.db";  // kept for compatibility, unused

extern bool g_cbBlackAndWhite;
extern bool g_cbMipMap;

void Settings::ReadSettings(BSimulation *pSimulation) {
  // Read all settings from the SQLite database.
  // If the database isn't ready (init failed), we fall through and
  // the game uses the defaults set in BGame's constructor.

  if (Database::isReady()) {
    BGame::m_nTerrainResolution = Database::getSettingInt("TerrainResolution", BGame::m_nTerrainResolution);
    BGame::m_nDustAndClouds = Database::getSettingInt("DustAndClouds", BGame::m_nDustAndClouds);
    BGame::m_nMusicVolume = Database::getSettingInt("MusicVolume", BGame::m_nMusicVolume);
    BGame::m_nVehicleVolume = Database::getSettingInt("VehicleVolume", BGame::m_nVehicleVolume);
    BGame::m_nSoundscape = Database::getSettingInt("Soundscape", BGame::m_nSoundscape);
    SoundModule::SetSoundSpace(1 - BGame::m_nSoundscape);
    BGame::m_nSkyDetail = Database::getSettingInt("SkyDetail", BGame::m_nSkyDetail);
    BGame::m_nDistantDetail = Database::getSettingInt("DistantDetail", BGame::m_nDistantDetail);
    BGame::m_nWaterDetail = Database::getSettingInt("WaterDetail", BGame::m_nWaterDetail);
    BGame::m_nColorMode = Database::getSettingInt("ColorMode", BGame::m_nColorMode);
    BGame::m_nScreenFormat = Database::getSettingInt("ScreenFormat", BGame::m_nScreenFormat);
    BGame::m_nTextureSmoothness = Database::getSettingInt("TextureSmoothness", BGame::m_nTextureSmoothness);
    g_cbMipMap = (BGame::m_nTextureSmoothness == 2);

    SoundModule::SetMenuMusicVolume(Database::getSettingInt("MenuMusicVolume", 128));
    SoundModule::SetVehicleSoundsVolume(Database::getSettingInt("VehicleSoundsVolume", 128));
    SoundModule::SetMessageSoundVolume(Database::getSettingInt("MessageSoundVolume", 128));

    ControllerModule::m_keymap.m_unLeft = Database::getSettingInt("KEYMAP-Left", ControllerModule::m_keymap.m_unLeft);
    ControllerModule::m_keymap.m_unRight = Database::getSettingInt("KEYMAP-Right", ControllerModule::m_keymap.m_unRight);
    ControllerModule::m_keymap.m_unAccelerate = Database::getSettingInt("KEYMAP-Accelerate", ControllerModule::m_keymap.m_unAccelerate);
    ControllerModule::m_keymap.m_unReverse = Database::getSettingInt("KEYMAP-Reverse", ControllerModule::m_keymap.m_unReverse);
    ControllerModule::m_keymap.m_unBreak = Database::getSettingInt("KEYMAP-Break", ControllerModule::m_keymap.m_unBreak);
    ControllerModule::m_keymap.m_unJet = Database::getSettingInt("KEYMAP-Jet", ControllerModule::m_keymap.m_unJet);
    ControllerModule::m_keymap.m_unHeli = Database::getSettingInt("KEYMAP-Heli", ControllerModule::m_keymap.m_unHeli);
    ControllerModule::m_keymap.m_unLift = Database::getSettingInt("KEYMAP-Lift", ControllerModule::m_keymap.m_unLift);
    ControllerModule::m_keymap.m_unCamera = Database::getSettingInt("KEYMAP-Camera", ControllerModule::m_keymap.m_unCamera);

    BGame::m_nWaterSurface = Database::getSettingInt("WaterSurface", BGame::m_nWaterSurface);
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
}

void Settings::WriteSettings(BSimulation *pSimulation) {
  if (!Database::isReady()) return;

  Database::setSettingInt("TerrainResolution", BGame::m_nTerrainResolution);
  Database::setSettingInt("DustAndClouds", BGame::m_nDustAndClouds);
  Database::setSettingInt("MusicVolume", BGame::m_nMusicVolume);
  Database::setSettingInt("VehicleVolume", BGame::m_nVehicleVolume);
  Database::setSettingInt("Soundscape", BGame::m_nSoundscape);
  Database::setSettingInt("SkyDetail", BGame::m_nSkyDetail);
  Database::setSettingInt("DistantDetail", BGame::m_nDistantDetail);
  Database::setSettingInt("WaterDetail", BGame::m_nWaterDetail);
  Database::setSettingInt("ColorMode", BGame::m_nColorMode);
  Database::setSettingInt("ScreenFormat", BGame::m_nScreenFormat);
  Database::setSettingInt("TextureSmoothness", BGame::m_nTextureSmoothness);

  Database::setSettingInt("MenuMusicVolume", SoundModule::GetMenuMusicVolume());
  Database::setSettingInt("VehicleSoundsVolume", SoundModule::GetVehicleSoundsVolume());
  Database::setSettingInt("MessageSoundVolume", SoundModule::GetMessageSoundVolume());

  Database::setSettingInt("KEYMAP-Left", ControllerModule::m_keymap.m_unLeft);
  Database::setSettingInt("KEYMAP-Right", ControllerModule::m_keymap.m_unRight);
  Database::setSettingInt("KEYMAP-Accelerate", ControllerModule::m_keymap.m_unAccelerate);
  Database::setSettingInt("KEYMAP-Reverse", ControllerModule::m_keymap.m_unReverse);
  Database::setSettingInt("KEYMAP-Break", ControllerModule::m_keymap.m_unBreak);
  Database::setSettingInt("KEYMAP-Jet", ControllerModule::m_keymap.m_unJet);
  Database::setSettingInt("KEYMAP-Heli", ControllerModule::m_keymap.m_unHeli);
  Database::setSettingInt("KEYMAP-Lift", ControllerModule::m_keymap.m_unLift);
  Database::setSettingInt("KEYMAP-Camera", ControllerModule::m_keymap.m_unCamera);

  Database::setSettingInt("WaterSurface", BGame::m_nWaterSurface);
}
