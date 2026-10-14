// Object

#pragma once

#ifdef _WIN32
#endif

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

class BFace {
public:
  int m_nPoints;
  BVector m_vPoint[5];
  BVector m_vNormal;

  BFace() { m_nPoints = 0; }
  void SetPoints(int nPoints, BVector *pvPoints);
  void CalculateNormal();
};

class BObjectPart : public BPart {
public:
  double m_dFriction;
  int m_nFaces;
  BFace *m_pFace;

  BObjectPart() {
    m_nFaces = 0;
    m_pFace = 0;
    m_dFriction = 0.5;
  }
  ~BObjectPart();
};

class BObject {

  BOBJData *m_pOBJData;

public:
  enum TCollDet { ACCURATE, BOUNDING_SPHERE, BOUNDING_BOX };
  enum TType { CLIENT, BASE, USER_DEF };

  int m_nDL;
  int m_nDLShadow;
  bool m_bDLIsValid;

  int m_nParts;   // visualization parts
  BPart *m_pPart; // visualization parts

  int m_nCollDetParts;         // collision detection parts
  BObjectPart *m_pCollDetPart; // collision detection parts

  string m_sObjectFilename;

  string m_sName;
  TType m_type;
  TCollDet m_collisionDetection;

  BVector m_vLocation;
  BVector m_vCenter;
  double m_dScale;
  double m_dScale2;
  double m_dZRotation;
  double m_dRadius;
  double m_dRed;
  double m_dGreen;
  double m_dBlue;
  double m_dFriction;
  BVector m_vCenterOnGround; // Same as m_vCenter except that the Z-coordinate
                             // is brought to the ground level
  double m_dActiveRadius;    // Max distance (from m_vCenterOnGround) at which
                             // pickup or delivery is successful

  bool m_bVisible;
  bool m_bHasShadow;
  BBoundary m_boundary;
  BBoundary m_shadow;
  bool m_bShadowPolygonOK;

  BObject();
  ~BObject();

  void SetOBJData(BOBJData *pOBJData) { m_pOBJData = pOBJData; }

  void LoadObjectFromFile(string sFilename, string sSection,
                          bool bOnlyShape = false);
  void LoadCollisionPartFromFile(string sFilename, string sSection,
                                 BPart &rVisPart, BObjectPart &rPart);

  void Setup();
  double PointIsInsideObject(BVector &rvPoint, BVector &rvNormal,
                             double &rdGroundFriction, double &rdBaseDepth);
  void CreateXYBoundary();
  void CreateShadowPolygon();
  void RecreateShadow();

  void PreProcessVisualization(bool bOnlyShadow = false);
  void PreProcessVisualizationGasStation();
  void DrawObject(bool bShadow);
  // Draw a GasoKeroStation
};
