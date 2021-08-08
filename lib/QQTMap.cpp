#include "QQTMap.h"

#include "QQTMapDatabase.h"
#include "QQTMapElement.h"

using namespace std;

#include "DefineIOMacros.h"

namespace {
std::vector<int> getSpecials(EQQTGameMode mode) {
  switch (mode) {
  case EQQTGameMode::bun: return {8009, 8010};
  case EQQTGameMode::machine: return {13016, 13017};
  case EQQTGameMode::sculpture: return {12008, 12009};
  case EQQTGameMode::tank: return {19030, 19031};
  default: return {};
  }
}
}

void QQTMap::resize(int width, int height) {
  _w = static_cast<uint16_t>(width);
  _h = static_cast<uint16_t>(height);
  auto capacityH = _elementIds[0].size();
  // 只改大不改小，方便撤回修改
  if (capacityH < _h) {
    for (auto &layer : _elementIds) {
      layer.resize(_h);
    }
  }
  auto capacityW = _elementIds[0][0].size();
  if (capacityW < _w) {
    for (auto &layer : _elementIds) {
      for (std::vector<int32_t> &row : layer) {
        row.resize(_w);
      }
    }
  }
}
bool QQTMap::removeMapElementAt(int x, int y, Layer layer, std::pair<int, std::pair<int, int>> *outRemoved) {
  if (isOutOfBound(x, y)) {
    return false;
  }
  auto *provider = QQTMapDatabase::getProvider();
  if (!provider) {
    qCritical() << "没有数据提供器，无法移除元素";
    return false;
  }
  auto &curLayer = _elementIds[layer];
  int id = curLayer[y][x];
  // 没有元素
  if (id == 0) {
    return false;
  }
  // 负数先找到原点
  if (id < 0) {
    tie(x, y) = findOrigin(x, y, layer);
    id = -id;
  }
  // 正数表明在原点
  if (id > 0) {
    const QQTMapElement *elem = provider->getMapElementById(id);
    // 范围清空，两个循环不能颠倒，增加缓存命中率
    for (int i = 0, ie = elem->h; i < ie; ++i) {
      for (int j = 0, je = elem->w; j < je; ++j) {
        if (!isOutOfBound(x + j, y + i)) {
          curLayer[y + i][x + j] = 0;
        }
      }
    }
    if (outRemoved) {
      *outRemoved = {id, {x, y}};
    }
    return true;
  }

  return false;
}
bool QQTMap::putMapElementAt(int x, int y, int id, Layer layer,
                             std::vector<std::pair<int, std::pair<int, int>>> *outRemoved) {
  if (id <= 0) {
    qCritical() << "不能添加非正ID元素" << id;
    return false;
  }
  auto *provider = QQTMapDatabase::getProvider();
  if (!provider) {
    qCritical() << "没有数据提供器，无法添加元素";
    return false;
  }
  const QQTMapElement *elem = provider->getMapElementById(id);
  if (!elem) {
    qCritical("没有元素%i的数据", id);
    return false;
  }
  if (isOutOfBound(x, y) || isOutOfBound(x + elem->w - 1, y + elem->h - 1)) {
    qCritical("放置的元素超出范围：ID: %i, x: %i, y: %i, w: %i, h: %i", id, x, y, elem->w, elem->h);
    return false;
  }
  if (outRemoved) {
    outRemoved->clear();
  }
  auto &curLayer = _elementIds[layer];
  // 范围置负数
  pair<int, pair<int, int>> removedBuf;
  for (int i = 0, ie = elem->h; i < ie; ++i) {
    for (int j = 0, je = elem->w; j < je; ++j) {
      if (curLayer[y + i][x + j]) {
        if (removeMapElementAt(x + j, y + i, layer, &removedBuf)) {
          if (outRemoved) {
            outRemoved->push_back(removedBuf);
          }
        }
      }
      curLayer[y + i][x + j] = -id;
    }
  }
  // 最后置左上为正数
  curLayer[y][x] = id;
  return true;
}
bool QQTMap::putSpawnPointAt(int x, int y, int group, int index, int *outRemovedGroup, int *outRemovedIndex) {
  if (group != 0 && group != 1) {
    qCritical() << "错误的出生组" << group;
    return false;
  }
  if (isOutOfBound(x, y)) {
    return false;
  }
  auto &curGroup = _spawnPoints[group];
  if (index < -1 || index > static_cast<int>(curGroup.size())) {
    qCritical("出生点放在错误的序号%i，当前出生组%i大小为%llu", index, group, curGroup.size());
    return false;
  }
  int otherGroup = 1 - group;
  if (removeSpawnPointAt(x, y, group, outRemovedIndex)) {
    // 删除本组成功
    if (outRemovedGroup) {
      *outRemovedGroup = group;
    }
  }
  else if (removeSpawnPointAt(x, y, otherGroup, outRemovedIndex)) {
    // 删除另一组成功
    if (outRemovedGroup) {
      *outRemovedGroup = otherGroup;
    }
  }
  // 放置在当前组
  if (index == -1) {
    // 默认放在最后
    curGroup.emplace_back(x, y);
  }
  else {
    curGroup.emplace(curGroup.begin() + index, x, y);
  }
  return true;
}
bool QQTMap::removeSpawnPointAt(int x, int y, int group, int *outRemovedIndex) {
  if (group != 0 && group != 1) {
    qCritical() << "错误的出生组" << group;
    return false;
  }
  if (isOutOfBound(x, y)) {
    return false;
  }
  auto &spawnPoints = _spawnPoints[group];
  for (int i = 0, ie = spawnPoints.size(); i < ie; ++i) {
    auto [curX, curY] = spawnPoints[i];
    if (curX == x && curY == y) {
      if (outRemovedIndex) {
        *outRemovedIndex = i;
      }
      spawnPoints.erase(spawnPoints.begin() + i);
      return true;
    }
  }
  return false;
}
bool QQTMap::read(std::istream &is) {
  if (!is) {
    return false;
  }
  READT(_version, int32_t);
  switch (_version) {
  default:
    qInfo() << "不支持的地图版本：" << _version;
    return false;
  case 3:
  case 4:
    break;
  }
  READT(_gameMode, int32_t);
  READT(_nMaxPlayers, int32_t);

  // 版本3为探险模式开发之前，长宽固定
  if (_version == 3) {
    _w = 15;
    _h = 13;
  }
  else {
    READT(_w, int32_t);
    READT(_h, int32_t);
  }

  // 将上下两层每层的地图元素ID都扩展为h*w
  for (auto &elementLayer : _elementIds) {
    elementLayer.clear();
    elementLayer.reserve(_h);
    for (int i = 0; i < _h; ++i) {
      elementLayer.emplace_back(_w);
    }
  }

  // 文件中有3层，存为两层
  for (int k = 0; k < 3; ++k) {
    for (int i = 0; i < _h; ++i) {
      for (int j = 0; j < _w; ++j) {
        int32_t id;
        READT(id, int32_t);
        if (k == 2) {
          _elementIds[1][i][j] = id;
        }
        else {
          _elementIds[0][i][j] = id;
        }
      }
    }
  }

  // 中间是版本4以前的地图保留的数据：道具类型及其掉落概率，版本4开始这一字段全为0，估计是换了道具概率的配置机制，后续不再需要
  int32_t nItem;
  READT(nItem, int32_t);
  is.seekg(nItem * static_cast<long>(sizeof(int32_t) * 3 + sizeof(float)),
           ios::cur);

  // 这些点放的是可以被炸毁的元素的点，也就是允许的道具生成点。这些点只需要后面写出的时候扫描一遍元素的属性就行了，不需要保存
  int32_t nItemPoint;
  READT(nItemPoint, int32_t);
  is.seekg(nItemPoint * static_cast<long>(sizeof(int16_t) * 2), ios::cur);

  // 两个队伍的玩家出生点。在竞技的标准模式（两组）下有用。在自由模式中没有队伍的区别，可以只放第一组
  for (std::vector<std::pair<int32_t, int32_t>> &currentGroup : _spawnPoints) {
    int32_t nBirthPoint;
    READT(nBirthPoint, int32_t);
    currentGroup.clear();
    currentGroup.reserve(nBirthPoint);
    // 第j个出生点
    for (int j = 0; j < nBirthPoint; ++j) {
      int16_t c, r;
      READT(r, int16_t);
      READT(c, int16_t);
      currentGroup.emplace_back(c, r);
    }
  }

  // 这些是用于一些特殊模式下，标识属于双方队伍各自的特殊地图元素，例如包子铺、雕塑塔、机械大炮、糖客战基地等
  // 读进来统一作为顶层元素对待，写出的时候特殊处理
  int32_t nSpecialPoint;
  READT(nSpecialPoint, int32_t);
  auto specials = getSpecials(_gameMode);
  for (int i = 0; i < nSpecialPoint; ++i) {
    if (i >= 2) {
      qWarning("特殊元素超过2个");
      break;
    }
    int16_t c, r;
    READT(r, int16_t);
    READT(c, int16_t);
    putMapElementAt(c, r, specials[i], kTop);
  }

  return true;
}
bool QQTMap::write(std::ostream &os) {
  auto dataProvider = QQTMapDatabase::getProvider();
  if (dataProvider == nullptr) {
    qCritical() << "没有数据提供器";
    return false;
  }
  WRITET(/*version*/ 4, int32_t);
  WRITET(_gameMode, int32_t);
  WRITET(_nMaxPlayers, int32_t);

  WRITET(_w, int32_t);
  WRITET(_h, int32_t);

  auto specialIds = getSpecials(_gameMode);
  if (specialIds.empty()) {
    _specialPoints.clear();
  }
  else {
    _specialPoints.resize(2);
  }

  for (int k = 0; k < 3; ++k) {
    for (int i = 0; i < _h; ++i) {
      for (int j = 0; j < _w; ++j) {
        if (k == 2) {
          WRITET(_elementIds[kGround][i][j], int32_t);
        }
        else {
          int id = _elementIds[kTop][i][j];
          if (id == specialIds[0]) {
            _specialPoints[0] = {j, i};
            WRITET(0, int32_t);
            continue;
          }
          if (id == specialIds[1]) {
            _specialPoints[1] = {j, i};
            WRITET(0, int32_t);
            continue;
          }
          if (auto elem = dataProvider->getMapElementById(id);
            elem && (elem->canBeHidden() ^ (k == 1))) {
            WRITET(id, int32_t);
          }
          else {
            WRITET(0, int32_t);
          }
        }
      }
    }
  }

  WRITET(0, int32_t);

  vector<tuple<int, int>> itemPoints;
  for (std::vector<std::vector<int32_t>> &layer : _elementIds) {
    // i行
    for (int i = 0; i < _h; ++i) {
      // j列
      for (int j = 0; j < _w; ++j) {
        int id = layer[i][j];
        if (const QQTMapElement *elem = dataProvider->getMapElementById(id);
          elem && elem->canBeDestroyed()) {
          itemPoints.emplace_back(j, i);
        }
      }
    }
  }

  WRITET(itemPoints.size(), int32_t);
  for (auto &&[x, y] : itemPoints) {
    WRITET(y, int16_t);
    WRITET(x, int16_t);
  }

  for (std::vector<std::pair<int32_t, int32_t>> &group : _spawnPoints) {
    WRITET(group.size(), int32_t);
    for (auto &&[x, y] : group) {
      WRITET(y, int16_t);
      WRITET(x, int16_t);
    }
  }

  WRITET(_specialPoints.size(), int32_t);
  for (int i = 0, ie = _specialPoints.size(); i < ie; ++i) {
    if (i >= 2) {
      qWarning("特殊元素超过2个");
      break;
    }
    auto [x, y] = _specialPoints[i];
    WRITET(y, int16_t);
    WRITET(x, int16_t);
  }

  return true;
}
int QQTMap::getMapElementId(int x, int y, Layer layer) const {
  return isOutOfBound(x, y) ? 0 : _elementIds[layer][y][x];
}
std::pair<int, int> QQTMap::findOrigin(int x, int y, Layer layer) {
  // 原先地图中，当一个地图元素占据空间大于1x1时，除了左上角为元素ID，其他格子都是该ID的负数。
  // 这里用来获取任意格子中包含的元素的左上角位置。需要通过数据提供器拿到元素的属性：长、宽
  auto provider = QQTMapDatabase::getProvider();
  if (provider == nullptr) {
    qCritical() << "没有数据提供器";
    return {-1, -1};
  }
  if (isOutOfBound(x, y)) {
    return {-1, -1};
  }
  auto id = _elementIds[layer][y][x];
  if (id >= 0) {
    return {x, y};
  }
  if (const QQTMapElement *elem = provider->getMapElementById(abs(id))) {
    for (int i = 0; i < elem->h; ++i) {
      for (int j = 0; j < elem->w; ++j) {
        int curX = x - j;
        int curY = y - i;
        if (isOutOfBound(curX, curY)) {
          continue;
        }
        if (_elementIds[layer][curY][curX] == -id) {
          return {curX, curY};
        }
      }
    }
  }
  return {-1, -1};
}
