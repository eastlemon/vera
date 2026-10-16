// All textures for Vera

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
#if defined(__APPLE__) && defined(__MACH__)
#include <OpenGL/glext.h>
#else
#include <GL/glext.h>
#endif
#include "Settings.h"
#include <string>

using namespace std;

class BTexture {
public:
  bool m_bValid;
  string m_sFilename;
  GLuint m_nGLTexName;
  int m_nXSize;
  int m_nYSize;
  int m_nComponents;
  bool m_bMipmapped;
  GLubyte *m_pPixels;

  BTexture() {
    m_bValid = false;
    m_pPixels = 0;
  }
};

class BTextures {

  static bool LoadTextureFromTGA(const string sFilename, int nTextureIndex,
                                 bool bMipmapped);
  static void LoadAllInternalTextures();

public:
  static int m_nTextures;
  static BTexture m_textures[100]; // We allow at most 100 textures
  enum Texture {
    NONE = 0,
    MAIN_GAME_MENU,
    SKY,
    GROUND_COLOR_MAP,
    GROUND_BASE,
    CAR,
    DUSTCLOUD,
    SHADOW,
    WHEEL,
    DASHBOARD,
    COMPASS,
    LED_NUMBERS,
    PANEL,
    SATELLITEIMAGE,
    ENVMAP,
    ENVMAP_SHINY,
    BRICKS,
    QUICK_HELP,
    QUICK_HELP_SCENE_EDITOR,
    CLOUD,
    WATER,
    GASSTATION,
    MENU_TITLES,
    MENU_BACKGROUND,
    FUELING,
    USER_BASE
  }; // Always make sure USER_BASE is the last texture!
  static void Init();
  static void Exit();
  static void Use(int nTextureIndex);
  static int LoadTexture(string sTextureName, bool bMipmapped = true);
  static int ReloadTexture(int nTextureIndex, string sTextureName);
  static void StopUsingTexture(int nTextureIndex);
};
