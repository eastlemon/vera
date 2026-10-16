// Simulation
#pragma once
#include "BCamera.h"
#include "BGround.h"
#include "BScene.h"
#include "BTerrain.h"
#include "BVehicle.h"
#include "BaseClasses.h"
#include "ControllerModule.h"
#include "OpenGLHelpers.h"
#include "VeraPhysicsEngine.h"
#include <string>

using namespace std;

class BTrackingTarget {
public:
  BTrackingTarget *m_pNext;
  string m_sId;
  BVector m_vLoc;
  BVector m_vScreenPos;
  double m_dRed;
  double m_dGreen;
  double m_dBlue;
};

const int g_cnDustClouds1 = 300;
const int g_cnDustClouds2 = 300;
const int g_cnDustClouds3 = 150;

class BCloudPuff {
public:
  BVector m_vLocation;
  double m_dRadius;
};

class BCloud {
public:
  BVector m_vCenter;
  int m_nPuffs;
  BCloudPuff m_puff[10];
};

class BSimulation {

  BVehicle m_vehicle;
  BGround m_ground;
  BCamera m_camera;
  BScene m_scene;
  BTerrain m_terrain;

  OpenGLHelpers m_tmpHelpers;

  int m_nDustCloudsHead1;
  int m_nDustClouds1;
  BDustCloud m_dustClouds1[g_cnDustClouds1];

  int m_nDustCloudsHead2;
  int m_nDustClouds2;
  BDustCloud m_dustClouds2[g_cnDustClouds2];

  int m_nDustCloudsHead3;
  int m_nDustClouds3;
  BDustCloud m_dustClouds3[g_cnDustClouds3];

  int m_nClouds;
  BCloud m_cloud[2];
  void CreateTestCloud();

public:
  BTrackingTarget *m_targets;

private:
  FILE *m_fp;

  void ApplyGroundEffectToAPoint(int i, BVector &vNormal, double depth,
                                 double dGroundFriction, double dBaseDepth);

public:
  double m_dMaxGForce;

  static const double g_cdBrakesFriction;
  static const double g_cdMaxSpeed;
  static const double g_cdTurnFactor;
  static const double g_cdAirTurnFactor;
  static const double g_cdPropThrustFactor;

  bool m_bLiftingUp; // Is the car being lifted up by a virtual crane
  double m_dLiftZ;   // Z coordinate of the car when the lifting was started
  bool m_bRaining;   // Debug code to the ground depth
  bool m_bPaused;
  bool m_bSteeringAidOn;
  bool m_bCalibrateSimulationSpeed;

  bool m_bRecordTrail;

  // double m_dAccelerationFactor;
  double m_dTurnFactor;
  double m_dAirTurnFactor;
  double m_dPropThrustFactor;
  int m_nPhysicsStepsBetweenRender;
  double m_dPhysicsFraction;
  int m_nPhysicsSteps;

  enum THeliPart { LEFTHATCH, RIGHTHATCH, COVER, BLADE };
  double m_dRotorVolFactor;

  BTerrainBlock *m_pCenterBlock;

  BSimulation();
  ~BSimulation();

  BVehicle *GetVehicle() { return &m_vehicle; }
  BGround *GetGround() { return &m_ground; }
  BCamera *GetCamera() { return &m_camera; }
  BTerrain *GetTerrain() { return &m_terrain; }
  BScene *GetScene() { return &m_scene; }

  void StartRecording();
  void StopRecording();

  // void PrePaint(CDC *pDC);
  void PrePaint();
  void PreProcessVisualization();
  void PaintSceneObjects();
  void PaintSky(float fBrightness, bool bFog);
  void PaintWaterSurface();
  void PaintFaintWaterSurface();
  // int  Paint(CDC *pDC, bool bCreateDLs, bool bWireframe, bool bNormals, CRect
  // &rectWnd);
  int Paint(bool bCreateDLs, bool bWireframe, bool bNormals);
  void DrawShadowAndTrails();
  void DrawTrails();
  void DrawDustClouds();
  void DrawClouds();
  void DrawSmokeTrails();
  void DrawRotor();
  void DrawRetractedBlades(BVector &rvCenter, double dRad, double dPhase,
                           double dAngle);
  void CheckHeliPartForDamage(THeliPart part, BVector vPoint, int nInfo = 0);
  void DamageHeliPart(THeliPart part, int nInfo = 0);
  void DrawJet();

  // void SetUpCamera(CRect *pRect = 0);
  void SetUpCamera();
  void UpdateCar();
  void UpdateCarLocation();
  void EnsureVehicleIsOverGround();
  void UpdateHeliControls();
  void ToggleHeli();
  void SwitchCameraMode();
  void UpdateAirplaneControls();
  void UpdateJetControls();
  void UpdateTrails();
  void UpdateDustClouds();
  void CreateDustCloudAt(BVector vHitPoint, int nSlot,
                         double dInitialAlpha = -1.0,
                         BVector vInitial = BVector(0, 0, 0),
                         double dOffset = 0.0);
  void CreateSmokeTrails();

  void UpdateEngineSound();
  void MoveCarPoints();
  void ApplySteering();
  void UpdateGasStationLocation();
  void UpdateGasStationTracking();

  BTerrainBlock *FindTerrainBlock(BVector &rvPoint);
  double PointUnderGround(BVector vPoint, BVector &rvNormal, double &rdFriction,
                          double &rdBaseDepth, double &rdThermoLoss);
  double PointInsideObject(BVector &rvPoint, BVector &rvNormal,
                           double &rdFriction, double &rdBaseDepth);
  double PointUnderGroundShadow(BVector vPoint, BVector &rvNormal);
  double PointUnderObjectsShadow(double &rdCandidate, BVector &rvPoint,
                                 BVector &rvNormal, bool &rbChanged);
  double Friction(BBodyPoint &rPoint);
  double Friction(BVector &rPoint);
  double GroundHardnessAt(BVector &rvLocation);

  void AddTrackingTarget(string sId, BVector vLoc, double dRed, double dGreen,
                         double dBlue);
  void RemoveTrackingTarget(string sId);
};
