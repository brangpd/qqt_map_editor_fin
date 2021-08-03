#include "QQTMap.h"

#include "QQTMapDatabase.h"
#include "QQTMapElement.h"

using namespace std;

#include "DefineIOMacros.h"
void QQTMap::resize(int width, int height) {
  _w = static_cast<uint16_t>(width);
  _h = static_cast<uint16_t>(height);
  for (std::vector<std::vector<int32_t>> &layer : _elementIds) {
    layer.resize(_h);
    for (std::vector<int32_t> &row : layer) {
      row.resize(_w);
    }
  }
}
int QQTMap::removeMapElementAt(int x, int y, Layer layer) {
  if (isOutOfBound(x, y)) {
    return 0;
  }
  auto *provider = QQTMapDatabase::getProvider();
  if (!provider) {
    cerr << "没有数据提供器，无法移除元素" << endl;
    return 0;
  }
  auto &curLayer = _elementIds[layer];
  int id = curLayer[y][x];
  // 没有元素
  if (id == 0) {
    return 0;
  }
  // 负数先找到原点
  if (id < 0) {
    tie(x, y) = findOrigin(x, y, layer);
    id = -id;
  }
  // 正数表明在原点
  const QQTMapElement *elem = provider->getMapElementById(id);
  // 范围清空，两个循环不能颠倒，增加缓存命中率
  for (int i = 0, ie = elem->h; i < ie; ++i) {
    for (int j = 0, je = elem->w; j < je; ++j) {
      if (!isOutOfBound(x + j, y + i)) {
        curLayer[y + i][x + j] = 0;
      }
    }
  }

  return id;
}
bool QQTMap::putMapElementAt(int x, int y, int id, Layer layer) {
  if (id <= 0) {
    cerr << "不能添加非正ID元素：" << id << endl;
    return false;
  }
  auto *provider = QQTMapDatabase::getProvider();
  if (!provider) {
    cerr << "没有数据提供器，无法添加元素" << endl;
    return false;
  }
  const QQTMapElement *elem = provider->getMapElementById(id);
  if (!elem) {
    cerr << "没有元素数据：" << id << endl;
    return false;
  }
  if (isOutOfBound(x, y) || isOutOfBound(x + elem->w - 1, y + elem->h - 1)) {
    cerr << QString::asprintf("放置的元素超出范围：ID: %i, x: %i, y: %i, w: %i, h: %i", id, x, y, elem->w, elem->h).data() <<
        endl;
    return false;
  }
  auto &curLayer = _elementIds[layer];
  // 范围置负数
  for (int i = 0, ie = elem->h; i < ie; ++i) {
    for (int j = 0, je = elem->w; j < je; ++j) {
      if (curLayer[y + i][x + j]) {
        removeMapElementAt(x + j, y + i, layer);
      }
      curLayer[y + i][x + j] = -id;
    }
  }
  // 最后置左上为正数
  curLayer[y][x] = id;
  return true;
}
bool QQTMap::read(std::istream &is) {
  if (!is) {
    return false;
  }
  READT(_version, int32_t);
  switch (_version) {
  default:
    cerr << "不支持的地图版本：" << _version << endl;
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
  int32_t nSpecialPoint;
  READT(nSpecialPoint, int32_t);
  _specialPoints.clear();
  _specialPoints.reserve(nSpecialPoint);
  for (int i = 0; i < nSpecialPoint; ++i) {
    int16_t c, r;
    READT(r, int16_t);
    READT(c, int16_t);
    _specialPoints.emplace_back(c, r);
  }

  return true;
}
bool QQTMap::write(std::ostream &os) {
  auto dataProvider = QQTMapDatabase::getProvider();
  if (dataProvider == nullptr) {
    cerr << "没有数据提供器" << endl;
    return false;
  }
  WRITET(/*version*/ 4, int32_t);
  WRITET(_gameMode, int32_t);
  WRITET(_nMaxPlayers, int32_t);

  WRITET(_w, int32_t);
  WRITET(_h, int32_t);

  for (int k = 0; k < 3; ++k) {
    for (int i = 0; i < _h; ++i) {
      for (int j = 0; j < _w; ++j) {
        if (k == 2) {
          WRITET(_elementIds[kGround][i][j], int32_t);
        }
        else {
          int id = _elementIds[kTop][i][j];
          if (const QQTMapElement *elem = dataProvider->getMapElementById(id);
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
    cerr << "没有数据提供器" << endl;
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
