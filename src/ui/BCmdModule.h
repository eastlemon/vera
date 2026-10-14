//
// BCmdModule: Internal command interface to control gameplay

#pragma once

#include "BSimulation.h"
#include "BaseClasses.h"
#include <string>

using namespace std;

class BCmdModule;

class BCmdHandler {
public:
  BCmdHandler *m_pNext;
  string m_sCommand;
  // void       (*m_pfnHandler)(BSimulation *pSim, string sParams, bool bHelp =
  // false);
  void (*m_pfnHandler)(BSimulation *pSim, string sParams, bool bHelp);
};

void ListCommands(BSimulation *pSim, string sParams);

class BCmdModule {
  friend void ListCommands(BSimulation *pSim, string sParams, bool bHelp);
  BSimulation *m_pSim;
  BCmdHandler *m_pHandlers;
  void RegisterCmdFunction(string sCommand, int nParams,
                           void (*pfnHandler)(BSimulation *pSim, string sParams,
                                              bool bHelp));
  bool StringsExactlySame(string s1, string sGiven);
  bool StringsPartiallySame(string s1, string sGiven, int &rnParamStart);
  BCmdHandler *FindCommand(string sCommand, int &rnParamStart);
  void Help(BSimulation *pSim, string sParams);

public:
  BCmdModule();
  void SetSim(BSimulation *pSim) { m_pSim = pSim; };
  void Run(string sCommand);
};
