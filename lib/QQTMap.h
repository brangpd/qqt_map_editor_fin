#ifndef QQT_MAP_EDITOR_FIN_LIB_QQTMAP_H
#define QQT_MAP_EDITOR_FIN_LIB_QQTMAP_H

#include "EQQTGameMode.h"
#include <cstdint>
#include <vector>
#include <utility>

struct QQTMap {
  int32_t version;
  EQQTGameMode gameMode;
  int8_t nMaxPlayers;
  int16_t w;
  int16_t h;
  std::vector<std::vector<int32_t>> elementIds[2 /*layers*/];
  std::vector<std::pair<int32_t, int32_t>> specialPoints;
  std::vector<std::pair<int32_t, int32_t>> spawnPoints[2 /*groups*/];
};

#endif // QQT_MAP_EDITOR_FIN_LIB_QQTMAP_H
