#pragma once

#ifdef _WIN32
#endif

#include "CoreClasses.h"
#include <math.h>
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

const int g_cnMaxTrailPoints = 100;

class BDustCloud {
public:
  BVector m_vLocation; // Where is it
  BVector m_vector;    // Where it's going
  bool m_bWater;       // Water or dust?
  double m_dAlpha;     // How dense it is
  double m_dSize;      // How big it is
  BVector m_color;
  BDustCloud() {}
};

class BBoundaryPoint : public BVector {
public:
  BBoundaryPoint *m_pNext;
  BBoundaryPoint() { m_pNext = 0; }
};

class BBoundary {
  BBoundaryPoint *AlreadyInserted(BVector &rP);

public:
  BBoundaryPoint *m_pHead;
  BBoundary() { m_pHead = 0; }
  void Empty();
  bool PointIsLeftOfLine(BBoundaryPoint *pStart, double dX, double dY);
  bool TurnsRight(BBoundaryPoint *p);
  BBoundaryPoint *Head() { return m_pHead; }
  BBoundaryPoint *AddAfter(BBoundaryPoint *pAfter, BVector &rNew2D);
  BBoundaryPoint *PointIsOutside(BVector &rP);
  BBoundaryPoint *PointIsOutside(double dX, double dY);
  BBoundaryPoint *Delete(BBoundaryPoint *p);
};

class BOBJVertex : public BVector {
public:
  BVector m_vNormal;
};

class BOBJFace {
public:
  int m_nV1;
  int m_nV2;
  int m_nV3;
  int m_nV4; // used only by the collision detection load
  int m_nVertices;
  BVector m_vNormal;
  int m_nTexCoords[3];     // used only for the FromFile texture mapping
  BVector m_vTexCoords[3]; // used only for the FromFile texture mapping
};

class BOBJData {
public:
  void Init();
  void Free();
  BOBJData();
  ~BOBJData();

  int m_nOBJPoints;
  int m_nOBJFaces;
  BOBJVertex *m_pvOBJPoints;
  BOBJFace *m_pOBJFaces;

  string m_sRightDir;
  string m_sForwardDir;
  string m_sDownDir;
  double m_dScale;

  BVector m_vMassCenter;

  void PrepareWaveFrontModel();
};

// One (visualization) part of the vehicle/object
class BPart {
public:
  BOBJData *m_pOBJData;

  BPart();

  void AppendWaveFrontOBJShape(string sShapeFilename, int &rnVertices,
                               int &rnFaces, bool bUseMassCenter,
                               bool bReadTexCoords, bool bSplitQuads);

  void LoadPartFromFile(string sFilename, string sSection, bool bOnlyShape,
                        bool bUseMassCenter);

  void SetPartColor(GLfloat fAlpha = 1.0);
  void RenderPart(int nReset);

  void SetOBJData(BOBJData *pObjData) { m_pOBJData = pObjData; }

  enum TTexMapping { FROMFILE, ENVMAP, WRAPAROUND };
  enum TShading { SMOOTH, FLAT, NONE };

  string m_sShapeFilename;
  int m_nVertices;
  int m_nVertexStart;
  int m_nFaces;
  int m_nFaceStart;
  int m_nTexVertices;
  int m_nTexVertexStart;
  bool m_bTextured;
  double m_dRed;
  double m_dGreen;
  double m_dBlue;
  double m_dAmbient;
  double m_dDiffuse;
  double m_dSpecular;
  double m_dBrilliance;
  bool m_bShiny;
  TShading m_shading;
  int m_nTexture;
  TTexMapping m_tmTextureMapping;
  double m_dWrapFactor;
};
