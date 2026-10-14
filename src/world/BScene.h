// Scene
#pragma once

#ifdef _WIN32
#endif

#include "BObject.h"
#include "BTerrain.h"
#include "BUI.h"
#include "BaseClasses.h"
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
#include <string>

using namespace std;

class BScene {

public:
  // General properties

  string m_sFilename;
  string m_sName;
  string m_sImageFilename;
  double m_dFriction;
  double m_dGroundTextureScaler1;
  double m_dGroundTextureScaler2;
  string m_sSkyTexture;
  string m_sGround1Texture;
  string m_sGround2Texture;
  string m_sEnvMapTexture;
  double m_dWindFactor;
  bool m_bHasEarthquakes;
  bool m_bSceneInUse;

  // Shape/Geometry related properties

  BVector m_vOrigin;
  BVector m_vStartLocation;
  BOBJData m_OBJData;

  // Object related properties

  int m_nCurrentDeliveryEntry;
  int m_nDeliveryOrderEntries;
  int m_nDeliveryOrder[10];
  bool m_bRandomDeliveryInUse;

  // Objects

  int m_nObjects;
  BObject m_pObjects[100];

  string m_sSceneObjectNames[100];
  BUISelectionList m_sellistSceneObjects;

  BScene();
  ~BScene();
  void CleanUp();
  void Save();
  void LoadSceneFromFile(string sFilename);
  void PlaceTerrainObjects(BTerrainBlock *pBlockSingle = 0);
  void PlaceObjectsOnTerrain(BTerrainBlock *pBlockSingle = 0);
  void UpdateObjectList();
};
