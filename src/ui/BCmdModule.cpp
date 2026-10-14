// BCmdModule: Internal command interface to control gameplay

#include "BCmdModule.h"
#include "BGame.h"
#include "BNavSatWnd.h"
#include "BServiceWnd.h"
#include "BVehicle.h"

#include "StringTools.h"

// COMMAND HANDLERS

void PureHelp(BSimulation *pSim, string sParams, bool bHelp) {
  if (bHelp) {
    BGame::GetServiceWnd()->Output("YOU DON'T NEED HELP FOR HELP.            ");
    BGame::GetServiceWnd()->Output("JUST TYPE HELP.                          ");
    return;
  }
}

void ListCommands(BSimulation *pSim, string sParams, bool bHelp) {
  if (bHelp) {
    BGame::GetServiceWnd()->Output("LISTS ALL AVAILABLE COMMANDS.            ");
    return;
  }
  BCmdHandler *p = BGame::Command()->m_pHandlers;
  while (p) {
    BGame::GetServiceWnd()->Output(p->m_sCommand);
    if (p->m_pNext) {
      BGame::GetServiceWnd()->Output(", ");
    }
    p = p->m_pNext;
  }
}

void ToggleJet(BSimulation *pSim, string sParams, bool bHelp) {
  if (bHelp) {
    BGame::GetServiceWnd()->Output("ACTIVATES/DEACTIVATES THE AUXILIARY      ");
    BGame::GetServiceWnd()->Output("JET BOOST. USE CAUTION WITH THE POWERFUL ");
    BGame::GetServiceWnd()->Output("JET. JET CONSUMES A LOT OF KEROSINE.");
    return;
  }
  // Turn jet mode on/off
  if (pSim->GetVehicle()->m_jet.m_nJetMode) {
    pSim->GetVehicle()->m_jet.m_bJetModeActivating =
        !pSim->GetVehicle()->m_jet.m_bJetModeActivating;
  } else {
    pSim->GetVehicle()->m_jet.m_nJetMode = 1;
    pSim->GetVehicle()->m_jet.m_bJetModeActivating = true;
  }
  if (pSim->GetVehicle()->m_jet.m_bJetModeActivating) {
    BGame::GetServiceWnd()->Output("JET MODE ACTIVATED");
  } else {
    BGame::GetServiceWnd()->Output("JET MODE DEACTIVATED");
  }
}

void ToggleDashboard(BSimulation *pSim, string sParams, bool bHelp) {
  if (bHelp) {
    BGame::GetServiceWnd()->Output("DISPLAYES/HIDES THE DASHBOARD.           ");
    return;
  }
  BGame::m_bDashboard = !BGame::m_bDashboard;
}

void ToggleNavSat(BSimulation *pSim, string sParams, bool bHelp) {
  if (bHelp) {
    BGame::GetServiceWnd()->Output("DISPLAYES/HIDES THE NAVIGATION SATELLITE ");
    BGame::GetServiceWnd()->Output("(NAVSAT) PANEL. USE THE HANDLE TO SET    ");
    BGame::GetServiceWnd()->Output("THE SIZE OF THE IMAGE.");
    return;
  }
  BGame::m_bNavSat = !BGame::m_bNavSat;
  if (BGame::m_bNavSat) {
    // Start navsat
    BGame::GetNavSat()->StartTracking(
        BGame::GetSimulation()->GetVehicle()->m_vLocation);
    BGame::GetServiceWnd()->Output("NAVSAT TRACKING ACTIVATED");
  } else {
    // End navsat
    BGame::GetNavSat()->EndTracking();
  }
}

void ToggleService(BSimulation *pSim, string sParams, bool bHelp) {
  if (bHelp) {
    BGame::GetServiceWnd()->Output("DISPLAYES/HIDES THIS PANEL.              ");
    return;
  }
  BGame::m_bService = !BGame::m_bService;
}

void Exit(BSimulation *pSim, string sParams, bool bHelp) {
  if (bHelp) {
    BGame::GetServiceWnd()->Output("HIDES THIS PANEL.");
    return;
  }
  ToggleService(pSim, sParams, bHelp);
}

void SetNavSatResolution(BSimulation *pSim, string sParams, bool bHelp) {
  if (bHelp) {
    BGame::GetServiceWnd()->Output(
        "\"SET NAVSAT RESOLUTION <RES>\"            ", 0, 1, 0);
    BGame::GetServiceWnd()->Output("SETS THE SIZE OF THE AREA SHOWN IN THE   ");
    BGame::GetServiceWnd()->Output("NAVSAT PANEL. PARAMETER <RES> IS GIVEN IN");
    BGame::GetServiceWnd()->Output("METERS.");
    return;
  }
  double dRes;
  // if(sscanf(LPCTSTR(sParams), "%lf", &dRes) == 1) {
  if (sscanf(sParams.c_str(), "%lf", &dRes) == 1) {
    BGame::GetNavSat()->SetResolution(dRes);
    BGame::GetServiceWnd()->Output("NAVSAT TRACKING RESOLUTION CHANGED");
  } else {
    BGame::GetServiceWnd()->Output("SYNTAX ERROR: ILLEGAL RESOLUTION", 0.75, 0,
                                   0);
  }
}

// FRAMEWORK

BCmdModule::BCmdModule() {
  m_pSim = 0;
  m_pHandlers = 0;

  // Register command handlers
  RegisterCmdFunction("TOGGLE JET", 0, ToggleJet);
  RegisterCmdFunction("TOGGLE DASHBOARD", 0, ToggleDashboard);
  RegisterCmdFunction("TOGGLE NAVSAT", 0, ToggleNavSat);
  RegisterCmdFunction("TOGGLE SERVICE", 0, ToggleService);
  RegisterCmdFunction("HELP", 0, PureHelp);
  RegisterCmdFunction("LIST COMMANDS", 0, ListCommands);
  RegisterCmdFunction("SET NAVSAT RESOLUTION", 0, SetNavSatResolution);
  RegisterCmdFunction("EXIT", 0, Exit);
}

void BCmdModule::RegisterCmdFunction(string sCommand, int nParams,
                                     void (*pfnHandler)(BSimulation *pSim,
                                                        string sParams,
                                                        bool bHelp)) {
  // Add a new command handler
  BCmdHandler *p = m_pHandlers;
  if (!p) {
    // Add first
    p = new BCmdHandler;
    m_pHandlers = p;
  } else {
    // Add to the tail of the list
    while (p->m_pNext) {
      p = p->m_pNext;
    }
    BCmdHandler *pNew = new BCmdHandler;
    p->m_pNext = pNew;
    p = p->m_pNext;
  }
  p->m_pNext = 0;
  p->m_pfnHandler = pfnHandler;
  p->m_sCommand = sCommand;
}

bool BCmdModule::StringsExactlySame(string s1, string sGiven) {
  // return(s1.CompareNoCase(sGiven) == 0);
  return (s1.compare(sGiven) == 0);
}

bool BCmdModule::StringsPartiallySame(string s1, string sGiven,
                                      int &rnParamStart) {
  // Check if identical
  if (StringsExactlySame(s1, sGiven)) {
    return true;
  }
  // Check if correctly abbreviated
  int i1 = 0, i2 = 0;

  // Eat away leading spaces
  // while((i2 < sGiven.GetLength()) &&
  while ((i2 < sGiven.length()) &&
         //(sGiven.GetAt(i2) == ' ')) {
         (sGiven.at(i2) == ' ')) {
    ++i2;
  }
  // if(i2 >= sGiven.GetLength()) {
  if (i2 >= sGiven.length()) {
    return false;
  }
  // while((i1 < s1.GetLength()) &&
  while ((i1 < s1.length()) &&
         //(i2 < sGiven.GetLength())) {
         (i2 < sGiven.length())) {
    // if(sGiven.GetAt(i2) != s1.GetAt(i1)) {
    if (sGiven.at(i2) != s1.at(i1)) {
      // if(sGiven.GetAt(i2) == ' ') {
      if (sGiven.at(i2) == ' ') {
        // Skip to next words
        // while((i2 < sGiven.GetLength()) &&
        while ((i2 < sGiven.length()) &&
               //(sGiven.GetAt(i2) == ' ')) {
               (sGiven.at(i2) == ' ')) {
          ++i2;
        }
        // while((i1 < s1.GetLength()) &&
        while ((i1 < s1.length()) &&
               //(s1.GetAt(i1) != ' ')) {
               (s1.at(i1) != ' ')) {
          ++i1;
        }
        // while((i1 < s1.GetLength()) &&
        while ((i1 < s1.length()) &&
               //(s1.GetAt(i1) == ' ')) {
               (s1.at(i1) == ' ')) {
          ++i1;
        }
      } else {
        return false;
      }
    } else {
      ++i2;
      ++i1;
    }
  }
  // while((i2 < sGiven.GetLength()) &&
  while ((i2 < sGiven.length()) &&
         //(sGiven.GetAt(i2) == ' ')) {
         (sGiven.at(i2) == ' ')) {
    ++i2;
  }
  rnParamStart = i2;
  return true;
}

BCmdHandler *BCmdModule::FindCommand(string sCommand, int &rnParamStart) {
  BCmdHandler *p = m_pHandlers;
  // First look for an exact match
  while (p && !StringsExactlySame(p->m_sCommand, sCommand)) {
    p = p->m_pNext;
  }
  if (!p) {
    // Then look for an abbreviated match
    p = m_pHandlers;
    while (p && !StringsPartiallySame(p->m_sCommand, sCommand, rnParamStart)) {
      p = p->m_pNext;
    }
  } else {
    // rnParamStart = p->m_sCommand.GetLength() + 1;
    rnParamStart = p->m_sCommand.length() + 1;
  }
  return p;
}

void BCmdModule::Help(BSimulation *pSim, string sParams) {
  // BGame::GetServiceWnd()->Output("12345678901234567890123456789012345678901");
  int nTmp;
  // if(!sParams.IsEmpty()) {
  if (!sParams.empty()) {
    BCmdHandler *pCommand = FindCommand(sParams, nTmp);
    if (pCommand) {
      (*(pCommand->m_pfnHandler))(m_pSim, sParams, true);
      return;
    } else {
      string sError;
      // sError.Format("THERE'S NO COMMAND \"%s\". TO SEE ALL COMMANDS, TYPE
      // \"LIST COMMANDS\"", sParams);
      sError.assign("THERE'S NO COMMAND \"");
      sError.append(sParams);
      sError.append("\". TO SEE ALL COMMANDS, TYPE \"LIST COMMANDS\"");
      BGame::GetServiceWnd()->Output(sError, 0.75, 0, 0);
      return;
    }
  }
  BGame::GetServiceWnd()->Output("TO SEE ALL COMMANDS, TYPE \"LIST COMMANDS\"");
  BGame::GetServiceWnd()->Output("TO GET HELP ON INDIVIDUAL COMMAND, TYPE  ");
  BGame::GetServiceWnd()->Output("\"HELP XYZ\" (WHERE XYZ IS THE COMMAND)    ");
  BGame::GetServiceWnd()->Output("ALL COMMANDS CAN BE ABBREVIATED AS LONG  ");
  BGame::GetServiceWnd()->Output("AS THEY ARE UNAMBIGUOUS                  ");
  BGame::GetServiceWnd()->Output(
      "(E.G. \"LI CO\" =  \"LIST COMMANDS\")        ");
}

void BCmdModule::Run(string sCommand) {
  // sCommand.MakeUpper();
  StringTools::makeUpper(sCommand);
  int nParamStart = 0;
  BCmdHandler *pCommand = FindCommand(sCommand, nParamStart);
  if (pCommand) {
    // Run command
    string sParams = "";
    // if(nParamStart < sCommand.GetLength()) {
    if (nParamStart < sCommand.length()) {
      // sParams = LPCTSTR(sCommand) + nParamStart;
      sParams = sCommand.substr(nParamStart);
    }
    // if(pCommand->m_sCommand.Compare("HELP") == 0) {
    if (pCommand->m_sCommand.compare("HELP") == 0) {
      // Help is special command that needs access to other commands.
      // That's why it's handled within the class
      Help(m_pSim, sParams);
    } else {
      //(*(pCommand->m_pfnHandler))(m_pSim, sParams);
      (*(pCommand->m_pfnHandler))(m_pSim, sParams, false);
    }
  } else {
    // Report command not found
    // if(!sCommand.IsEmpty()) {
    if (!sCommand.empty()) {
      string sError;
      // sError.Format("UNKNOWN COMMAND: <%s>", sCommand);
      sError.assign("UNKNOWN COMMAND: <");
      sError.append(sCommand);
      sError.append(">");
      BGame::GetServiceWnd()->Output(sError, 0.75, 0, 0);
    }
  }
}
