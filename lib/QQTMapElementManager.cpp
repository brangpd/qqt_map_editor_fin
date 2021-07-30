#include "QQTMapElementManager.h"
#include "QQFDIMG.h"
#include "QQFDIMGFactory.h"
#include "QQTMapElement.h"
#include <map>
#include <memory>
#include <vector>

#include "DefineIOMacros.h"
using namespace std;
namespace {
map<int, std::shared_ptr<QQTMapElement>>
    id2mapElement; // NOLINT(cert-err58-cpp)
vector<int> mapElementIds;
} // namespace
bool QQTMapElementManager::init(std::istream &is) {
  auto &m = id2mapElement;
  int32_t version;
  int32_t nElements;

  readt(version, int32_t);
  readt(nElements, int32_t);

  mapElementIds.reserve(nElements);

  for (int i = 0; i < nElements; ++i) {
    int32_t id;
    readt(id, int32_t);
    mapElementIds.push_back(id);

    auto &pElem = m[id];
    if (!pElem) {
      pElem = make_shared<QQTMapElement>();
    }
    auto &elem = *pElem;
    elem.id = id;

    readt(elem.w, int16_t);
    readt(elem.h, int16_t);
    readt(elem.offsetW, int16_t);
    readt(elem.offsetH, int16_t);

    readt(elem.life, int32_t);
    readt(elem.level, int32_t);
    readt(elem.special, uint32_t);

    elem.attributes.reserve((int64_t)elem.w * elem.h);
    uint32_t attr;
    for (int j = 0; j < elem.w * elem.h; ++j) {
      readt(attr, uint32_t);
      elem.attributes.emplace_back(attr);
    }

    elem.qqfdimg = make_shared<QQFDIMG>();
    if (QQFDIMGFactory::read(is, *elem.qqfdimg)) {
      cerr << "读取元素失败。ID为" << id << endl;
      return false;
    }
  }

  return true;
}
const QQTMapElement *QQTMapElementManager::getElementById(int id) {
  auto it = id2mapElement.find(id);
  if (it == id2mapElement.end()) {
    clog << "找不到元素。ID为" << id << endl;
    return nullptr;
  }
  return it->second.get();
}
const std::vector<int> &QQTMapElementManager::getAllElementIds() {
  return mapElementIds;
}
