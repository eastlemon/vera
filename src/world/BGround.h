// Ground

#pragma once

#ifdef _WIN32
#endif

#include "BCamera.h"
#include "BObject.h"
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

const double g_cdGroundScale = 20.0;
const double g_cdGroundHeightScale = 0.5;
const int g_cnGroundXSize = 96; // must be multiple of g_cnAreaSize
const int g_cnGroundYSize = 48; // must be multiple of g_cnAreaSize
const int g_cnAreaSize = 16;    // must be power of two
const int g_cnAreas =
    (g_cnGroundXSize / g_cnAreaSize) * (g_cnGroundYSize / g_cnAreaSize);
const double g_cdWaterLevel = -2.499;

class BGroundArea {
public:
  int m_nDLFullRes1;
  int m_nDLFullRes2;
  int m_nDLFullRes3;
  int m_nDLCoarseRes1;
  int m_nDLCoarseRes2;
  int m_nDLCoarseRes3;
  BVector m_vCenter;
  BGroundArea() {
    m_nDLFullRes1 = m_nDLFullRes2 = m_nDLFullRes3 = m_nDLCoarseRes1 =
        m_nDLCoarseRes2 = m_nDLCoarseRes3 = 0;
  }
};

class BGroundSquare {
public:
  double m_dFriction1;
  double m_dFriction2;
  double m_dSurfaceDepth1;
  double m_dSurfaceDepth2;
  bool m_bWater1;
  bool m_bWater2;
  BVector m_vNormal1;
  BVector m_vNormal2;
  BVector m_vNormalVis;
  BGroundSquare() { m_bWater1 = m_bWater2 = false; };
};

class BGround {
  int m_nDLSkyCoarse;
  int m_nDLSkyDetailed;
  int m_nDLSeaBed;
  int m_nDLWaterSurface;
  int m_nDLGroundWater;
  int m_nDLObjects;
  int m_nDLGroundSand;
  int m_nDLGroundRock;

public:
  enum TGroundType { GT_GROUND, GT_SAND, GT_ROCK, GT_WATER };

  BGroundArea m_areas[g_cnAreas];
  double m_dHeightMap[g_cnGroundYSize * g_cnGroundXSize];
  BGroundSquare m_sqrHeightMapNormals[g_cnGroundYSize * g_cnGroundXSize];
  double m_dCoarseDistance;

  int m_nObjects;
  BObject *m_pObjects;

  BGround();
  virtual ~BGround();

  void PreProcessVisualization();
  void CreateGroundAreas();
  bool GroundTriangleIsOfType(int x, int y, int nSize, int nTriangle,
                              TGroundType gt);
  double SurfaceHeightAt(int x, int y, TGroundType gt);
  void RenderAreaFull(int nX, int nY, int nSize, TGroundType gt);
  void RenderAreaCoarse(int nX, int nY, int nSize, TGroundType gt);
  void RenderSky(int nDispList);
  bool AreaFlatEnough(int nX, int nY, int nSize);
  void PaintGroundAreas(BCamera &rCamera, TGroundType gt);
  void Paint(int nSkyDetail, int nWaterDetail, BCamera &rCamera,
             TGroundType gt);
  void PaintSky(int nSkyDetail);
  void PaintObjects(BCamera &rCamera);
  void PaintWaterSurface(double dBrightness, double dAlpha);
  void LoadGroundFromFile(string sFilename);
  void LoadObjectsFromFile(string sFilename);
  void LoadTextures();
};
