// OpenGL helpers
#pragma once

#ifdef _WIN32
#endif

#include <SDL2/SDL.h>

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
#if defined(__APPLE__) && defined(__MACH__)
#include <OpenGL/glext.h>
#else
#include <GL/glext.h>
#endif
class OpenGLHelpers {
  static GLuint m_nDLTex;

public:
  static bool m_bMultiTexturing;

  static void Init();

  static bool LoadExtensionFunctions();

  static void SetColorFullSafe(double dRed, double dGreen, double dBlue,
                               double dAlpha);

  static void SetColorFull(double dRed, double dGreen, double dBlue,
                           double dAlpha);

  static void SetColor(double dRed, double dGreen, double dBlue, double dAlpha);

  static void SwitchToTexture(int nTexture = 0, bool bDisable = false);

  static void SetDefaultLighting();

  static void Triangle(BVector &p1, BVector &p2, BVector &p3);

  static void TxtrTriangle(BVector &p1, double dTxtrX1, double dTxtrY1,
                           BVector &p2, double dTxtrX2, double dTxtrY2,
                           BVector &p3, double dTxtrX3, double dTxtrY3);

  static void TxtrTriangleBare(BVector &p1, double dTxtrX1, double dTxtrY1,
                               BVector &p2, double dTxtrX2, double dTxtrY2,
                               BVector &p3, double dTxtrX3, double dTxtrY3);

  static void TriangleStrip(BVector *pvPoints, int nPoints);

  static void TriangleStripWithNormals(BVector *pvPoints, BVector *pvNormals,
                                       int nPoints);

  static void TriangleStripTexturedX(BVector *pvPoints, int nPoints, int nOrig1,
                                     int nOrig2, double dTXTRMin,
                                     double dTXTRMax);

  static void TriangleFan(BVector *pvPoints, int nPoints);

  static void TriangleFanWithNormals(BVector *pvPoints, BVector *pvNormals,
                                     int nPoints);

  static void Line(BVector &p1, BVector &p2);

  static void Lines(BVector *pvPoints, int nPoints);

  static void CreateTexName(int nTexIndex, GLuint *pnTexName);

  static void FreeTexName(int nTexIndex, GLuint *pnTexName);

  static void CreateMipMaps(GLubyte *pStart, int nWidth, int nHeight,
                            int nComponents);

  static void BindMipMapTexture(int nWidth, int nHeight, int nComponents,
                                GLenum format, GLubyte *pStart, int nTexIndex,
                                GLuint nTexNam);

  static void BindTexture(int nWidth, int nHeight, int nComponents,
                          GLenum format, GLubyte *pStart, int nTexIndex,
                          GLuint nTexName, bool bForceCreate = false);

  static void SetTexCoord(double x, double y);

  static void DrawVeil(double dRed, double dGreen, double dBlue, double dAlpha,
                       SDL_Rect &rectWnd);
};