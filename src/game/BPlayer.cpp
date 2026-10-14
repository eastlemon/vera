// BPlayer: Player
#include <sstream>

using namespace std;

#include "BGame.h"
#include "BPlayer.h"
#include "FileIOHelpers.h"

BPlayer::BPlayer() {
  m_dCash = 50.0;
  m_dFuel = 25.0;
  m_dKerosine = 25.0;
  m_sValidVehicles = ">Bogian<";
  m_sSceneInfo = "";
}

void BPlayer::SaveStateFile() {
  // Write in INI file format
  string s;
  // s.Format("%.1lf", m_dCash);
  stringstream format;
  format << m_dCash;
  s = format.str();
  FileHelpers::WriteKeyStringToINIFile("State", "Cash", s, "./Player.state");
  // s.Format("%.1lf", m_dFuel);
  format.str("");
  format << m_dFuel;
  s = format.str();
  FileHelpers::WriteKeyStringToINIFile("State", "Fuel", s, "./Player.state");
  // s.Format("%.1lf", m_dKerosine);
  format.str("");
  format << m_dKerosine;
  s = format.str();
  FileHelpers::WriteKeyStringToINIFile("State", "Kerosine", s,
                                       "./Player.state");
  FileHelpers::WriteKeyStringToINIFile("State", "Vehicles", m_sValidVehicles,
                                       "./Player.state");
  FileHelpers::WriteKeyStringToINIFile(
      "State", "Checksum", BGame::GetScrambleChecksum(), "./Player.state");
}

void BPlayer::LoadStateFile() {
  FileHelpers::GetKeyDoubleFromINIFile("State", "Cash", 0.0, m_dCash,
                                       "./Player.state");
  FileHelpers::GetKeyDoubleFromINIFile("State", "Fuel", 0.0, m_dFuel,
                                       "./Player.state");
  FileHelpers::GetKeyDoubleFromINIFile("State", "Kerosine", 0.0, m_dKerosine,
                                       "./Player.state");
  FileHelpers::GetKeyStringFromINIFile("State", "Vehicles", "",
                                       m_sValidVehicles, "./Player.state");
  string sChecksum;
  FileHelpers::GetKeyStringFromINIFile("State", "Checksum", "", sChecksum,
                                       "./Player.state");

  // if(sChecksum.Compare(BGame::GetScrambleChecksum()) != 0) {
  if (sChecksum.compare(BGame::GetScrambleChecksum()) != 0) {
    // File has been tampered with, ask for info
    BGame::MyAfxMessageBox("Player.state file checksum doesn't match!");
    // FIXME
    /*if(AfxMessageBox("The Player.State file checksum doesn't match.\n"
                     "Note that you are not allowed to edit the state file.\n"
                     "DO YOU WANT TO EXIT THE GAME?\n"
                     "(If you answer No, player state will be reset to default
    values.)" , MB_YESNO) == IDYES) { BGame::MyAfxMessageBox("Exiting game.");
      BGame::m_bQuitPending = true;
    } else {*/
    BGame::MyAfxMessageBox("Resetting the player state to default values.");
    m_dCash = 1000.0;
    // m_dCash = 50.0;
    m_dFuel = 25.0;
    m_dKerosine = 25.0;
    m_sValidVehicles = ">Bogian<";
    SaveStateFile();
    //}
  }
}

void BPlayer::SaveCurrentSceneInfo() {
  BScene *pScene = BGame::GetSimulation()->GetScene();
  BVehicle *pVehicle = BGame::GetSimulation()->GetVehicle();
  string sSceneInfo;
  /*sSceneInfo.Format("%c %03d: %.2lf %.2lf %.2lf",
                    pScene->m_bRandomDeliveryInUse ? 'R' : 'D',
                    pScene->m_nCurrentDeliveryEntry,
                    pVehicle->m_vLocation.m_dX,
                    pVehicle->m_vLocation.m_dY,
                    pVehicle->m_vLocation.m_dZ); */
  stringstream format;
  format << (char)(pScene->m_bRandomDeliveryInUse ? 'R' : 'D') << " ";
  format.width(3);
  format.fill('0');
  format << pScene->m_nCurrentDeliveryEntry << ": "
         << pVehicle->m_vLocation.m_dX << " " << pVehicle->m_vLocation.m_dY
         << " " << pVehicle->m_vLocation.m_dZ;
  sSceneInfo = format.str();
  FileHelpers::WriteKeyStringToINIFile("State", pScene->m_sName, sSceneInfo,
                                       "./Player.state");
}

bool BPlayer::LoadCurrentSceneInfo(BVector &rvVehicleLoc) {
  bool bRet = false;
  BScene *pScene = BGame::GetSimulation()->GetScene();
  string sSceneInfo;
  FileHelpers::GetKeyStringFromINIFile("State", pScene->m_sName, "default",
                                       sSceneInfo, "./Player.state");
  // if(sSceneInfo.CompareNoCase("default") != 0) {
  if (sSceneInfo.compare("default") != 0) {
    // Fetch scene info from string. The string is in format
    // "d xxx: x y z", where d = R or D for delivery type, xxx is delivery order
    // counter, and x y z is vehicle location
    // if(sSceneInfo.GetLength() >= 12) {
    if (sSceneInfo.length() >= 12) {
      // Fetch delivery entry
      if (sSceneInfo[0] == 'R') {
        pScene->m_bRandomDeliveryInUse = true;
      } else {
        pScene->m_bRandomDeliveryInUse = false;
        int nClientCounter = -1;
        // sscanf(LPCTSTR(sSceneInfo) + 2, "%d", &nClientCounter);
        sscanf(sSceneInfo.c_str() + 2, "%d", &nClientCounter);
        if ((nClientCounter < 0) ||
            (nClientCounter > (pScene->m_nDeliveryOrderEntries - 1))) {
          pScene->m_bRandomDeliveryInUse = true;
        } else {
          pScene->m_nCurrentDeliveryEntry = nClientCounter;
        }
      }

      // Fetch vehicle location
      BVector vVehicleLoc(-9999.9, -9999.9, -9999.9);

      /*sscanf(LPCTSTR(sSceneInfo) + 7,
             "%lf %lf %lf",
             &(vVehicleLoc.m_dX),
             &(vVehicleLoc.m_dY),
             &(vVehicleLoc.m_dZ));*/
      sscanf(sSceneInfo.c_str() + 7, "%lf %lf %lf", &(vVehicleLoc.m_dX),
             &(vVehicleLoc.m_dY), &(vVehicleLoc.m_dZ));
      if ((vVehicleLoc - BVector(-9999.9, -9999.9, -9999.9)).Length() > 0.1) {
        bRet = true;
        rvVehicleLoc = vVehicleLoc;
      }
    }
  }
  return bRet;
}
