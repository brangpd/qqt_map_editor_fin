#ifndef QQT_MAP_EDITOR_FIN_LIB_QQTMAPELEMENT_H
#define QQT_MAP_EDITOR_FIN_LIB_QQTMAPELEMENT_H

#include "EQQTCity.h"
#include <cstdint>
#include <vector>
#include <memory>

struct QQFDIMG;
struct QQTMapElement {
  int32_t id;
  int16_t w;
  int16_t h;
  int16_t offsetW;
  int16_t offsetH;
  int32_t life;
  int32_t level;
  uint32_t special;
  std::vector<unsigned> attributes;
  std::shared_ptr<QQFDIMG> qqfdimg;

  EQQTCity getCity() const;
  bool isGround() const;
};

#endif // QQT_MAP_EDITOR_FIN_LIB_QQTMAPELEMENT_H
