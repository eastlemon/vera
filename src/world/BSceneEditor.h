//
// BSceneEditor: Simple scene editor

#pragma once

#include <SDL2/SDL.h>

#include "BUI.h"
#include "BaseClasses.h"
#include <string>

using namespace std;

class BObject;

class BSceneEditor {

  bool m_bSceneModified;

  string m_sAllObjects[1000];
  int m_nAllObjects;
  string m_sObjectTypes[3];
  string m_sObjectShadows[2];
  string m_sOverwrite[2];

  void RecurseObjectFiles(string sPath, string sSubDir);
  void FetchAllObjects();

public:
  string m_sActiveObject;
  string m_sObjectType;
  string m_sObjectShadow;
  BUIEdit m_edit;
  BUISelectionList m_sellistAllObjects;
  BUISelectionList m_sellistObjectType;
  BUISelectionList m_sellistObjectShadow;
  BUISelectionList m_sellistOverwrite;

  enum TPhase {
    BASIC,
    ASKING_OBJECT_NAME,
    SELECTING_OBJECT_TYPE,
    SELECTING_OBJECT_SHADOW,
    SELECTING_OBJECT_FILE,
    MOVING_OBJECT,
    SELECTING_SCENE_OBJECT,
    SELECTING_SCENE_OBJECT_TO_DELETE,
    ASKING_SCENE_DISPLAY_NAME,
    ASKING_SCENE_FILENAME,
    OVERWRITE_SCENE_FILE_QUESTION
  };

  TPhase m_phase;

  BSceneEditor();
  void Activate();
  void Deactivate();
  bool IsActive();
  // void Draw(CRect &rectWnd);
  void Draw(SDL_Rect &rectWnd);
  void HighlightActiveObject();
  void AdvancePhase();
  void CancelPhase();
  BObject *GetActiveObject();
};
