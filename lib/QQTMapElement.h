#pragma once

#include <cstdint>
#include <vector>
#include "EQQTCity.h"

struct QQFDIMG;

struct QQTMapElement
{
  int32_t id;
  int16_t w;
  int16_t h;
  int16_t xOffset;
  int16_t yOffset;
  int32_t life;
  int32_t level;
  uint32_t special;
  std::vector<uint32_t> attributes;

  static EQQTCity getCity(int id) { return static_cast<EQQTCity>(id / 1000); }
  EQQTCity getCity() const { return getCity(id); }
  static int getIdInCity(int id) { return id % 1000; }
  int getIdInCity() const { return getIdInCity(id); }
  bool isGround() const;
  bool canBeHidden() const;
  bool canBeDestroyed() const { return special & 1; }
};
