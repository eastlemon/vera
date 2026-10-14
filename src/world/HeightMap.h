
const int cnMapSize = 128;
const double cdPercistence = 0.6;
const int cnOctaves = 8;

#pragma once

#include "BaseClasses.h"
#include <math.h>

namespace HeightMap {
enum TTerrain {
  MONUMENT_VALLEY,
  LAKE_POWELL,
  PAINTED_DESERT,
  CONTINENTS,
  SCOTLAND,
  MIXER,
  OVERALL,
  NOISE
};
void GenerateMap(double dXOffset, double dYOffset, double dScale);
double HeightAt(double dX, double dY);
double CalcHeightAt(double dX, double dY, double &rdArea,
                    TTerrain nTerrain = OVERALL, int nOctaves = cnOctaves);
BVector NormalAt(double dX, double dY);
}; // namespace HeightMap
