// BPlayer: Player

#pragma once

#include "BaseClasses.h"
#include <string>

using namespace std;

class BPlayer {
public:
  BPlayer();
  double m_dCash;          // Amount of money the player has
  double m_dFuel;          // Amount of fuel the player has (in percentage)
  double m_dKerosine;      // Amount of kerosine the player has (in percentage)
  string m_sValidVehicles; // List of vehicles the player owns
  string m_sSceneInfo;     // Info about each scene where to deliver next

  void SaveStateFile();
  void LoadStateFile();
  void SaveCurrentSceneInfo();
  bool LoadCurrentSceneInfo(BVector &rvVehicleLoc); // returns true if vehicle
                                                    // location could be loaded
};
