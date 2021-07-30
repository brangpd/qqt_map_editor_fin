#include "QQTMapReaderVer3.h"
#include "QQTMap.h"

#include "DefineIOMacros.h"
using namespace std;
bool QQTMapReaderVer3::read(std::istream &is, QQTMap &qqtMap) {
  readt(qqtMap.gameMode, int32_t);
  readt(qqtMap.nMaxPlayers, int32_t);

  // 版本3为探险模式开发之前，长宽固定
  qqtMap.w = 15;
  qqtMap.h = 13;

  // 将上下两层每层的地图元素ID都扩展为h*w
  for (auto &elementLayer : qqtMap.elementIds) {
    elementLayer.reserve(qqtMap.h);
    for (int i = 0; i < qqtMap.h; ++i) {
      elementLayer.emplace_back(qqtMap.w);
    }
  }

  // 文件中有3层，存为两层，
  for (int k = 0; k < 3; ++k) {
    for (int i = 0; i < qqtMap.h; ++i) {
      for (int j = 0; j < qqtMap.w; ++j) {
        int32_t id;
        readt(id, int32_t);
        if (k == 2) {
          qqtMap.elementIds[1][i][j] = id;
        } else {
          qqtMap.elementIds[0][i][j] = id;
        }
      }
    }
  }

  int32_t nItem;
  readt(nItem, int32_t);
  is.seekg(static_cast<int>(sizeof(int32_t) * 3 + sizeof(float)) * nItem, ios::cur);

  int32_t nItemPoint;
  readt(nItemPoint, int32_t);
  is.seekg(static_cast<int>(sizeof(int16_t) * 2 * nItemPoint), ios::cur);

  // 第i个队伍
  for (int i = 0; i < 2; ++i) { // NOLINT(modernize-loop-convert)
    int32_t nBirthPoint;
    readt(nBirthPoint, int32_t);
    // 第j个出生点
    for (int j = 0; j < nBirthPoint; ++j) {
      int16_t c, r;
      readt(r, int16_t);
      readt(c, int16_t);
      qqtMap.spawnPoints[i].emplace_back(c, r);
    }
  }

  int32_t nSpecialPoint;
  readt(nSpecialPoint, int32_t);
  for (int i = 0; i < nSpecialPoint; ++i) {
    int16_t c, r;
    readt(r, int16_t);
    readt(c, int16_t);
    qqtMap.specialPoints.emplace_back(c, r);
  }

  return true;
}
