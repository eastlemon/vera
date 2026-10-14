// All access settings module for all users of the game

#pragma once

#include "BaseClasses.h"
#include <string>

using namespace std;

class BSimulation;

class Settings {
  static string m_sFilename;

public:
  static string m_sBrowserPath;

  Settings() {};

  static void SetFilename(string sFilename) { m_sFilename = sFilename; }
  static void ReadSettings(BSimulation *pSimulation);
  static void WriteSettings(BSimulation *pSimulation);
};