#include "Database.h"

#include <algorithm>
#include <memory>
#include <QFile>
#include <QString>
#include <strstream>
#include <unordered_map>
#include <vector>

#include "QQFDIMG.h"
#include "QQTMapDatabase.h"
#include "QQTMapElement.h"

using namespace std;

#include "DefineIOMacros.h"

namespace {
QString makePath(EQQTCity city, int idInCity) {
  return QString::asprintf(":/mapElem/%s/elem%i_stand.img", toString(city), idInCity);
}

class QQTMapEditorDatabaseProvider final : public QQTMapDatabase::Provider
{
public:
  bool init() {
    if (!initMapElementProperties()) {
      qCritical("初始化mapElem.prop失败");
      return false;
    }
    if (!initMapElementQqfdimgs()) {
      qCritical("初始化地图元素QQFDIMG失败");
      return false;
    }
    return true;
  }
  const std::vector<int>& getAllMapElementIds() override { return _allMapElementIds; }
  const QQTMapElement* getMapElementById(int id) override {
    if (id <= 0) { return nullptr; }
    auto it = _mapElementId2mapElement.find(id);
    return it == _mapElementId2mapElement.end() ? nullptr : &it->second;
  }
  const QQFDIMG* getQqfdimgOfMapElementById(int id) override {
    if (id <= 0) { return nullptr; }
    auto it = _mapElementId2Qqfdimg.find(id);
    return it == _mapElementId2Qqfdimg.end() ? nullptr : &it->second;
  }

private:
  bool initMapElementProperties() {
    try {
      QFile prop(":/mapElem.prop");
      prop.open(QIODevice::ReadOnly);
      QByteArray contentProp = prop.readAll();
      istrstream is(contentProp.data(), contentProp.size());

      int32_t version;
      int32_t nElements;

      READT(version, int32_t);
      READT(nElements, int32_t);
      _allMapElementIds.reserve(nElements);
      for (int i = 0; i < nElements; ++i) {
        int32_t id;
        READT(id, int32_t);
        _allMapElementIds.push_back(id);

        QQTMapElement &elem = _mapElementId2mapElement[id];
        elem.id = id;

        READT(elem.w, int16_t);
        READT(elem.h, int16_t);
        READT(elem.xOffset, int16_t);
        READT(elem.yOffset, int16_t);

        READT(elem.life, int32_t);
        READT(elem.level, int32_t);
        READT(elem.special, int32_t);

        elem.attributes.reserve(static_cast<int64_t>(elem.w) * elem.h);
        uint32_t attr;
        for (int j = 0, je = elem.w * elem.h; j < je; ++j) {
          READT(attr, uint32_t);
          elem.attributes.emplace_back(attr);
        }
      }
      ranges::sort(_allMapElementIds);
      return true;
    }
    catch (exception const &e) {
      qCritical("初始化元素属性出现异常：%s", e.what());
      return false;
    }
  }
  bool initMapElementQqfdimgs() {
    try {
      for (int id : _allMapElementIds) {
        EQQTCity city = QQTMapElement::getCity(id);
        int idInCity = QQTMapElement::getIdInCity(id);
        QString path = makePath(city, idInCity);
        QFile file(path);
        file.open(QIODevice::ReadOnly);
        QByteArray content = file.readAll();
        istrstream iss(content.data(), content.size());
        if (!_mapElementId2Qqfdimg[id].read(iss)) {
          qCritical("读取元素%iQQFDIMG失败", id);
          return false;
        }
      }
      return true;
    }
    catch (exception const &e) {
      qCritical("初始化元素QQFDIMG属性出现异常：%s", e.what());
      return false;
    }
  }
  vector<int> _allMapElementIds;
  unordered_map<int, QQTMapElement> _mapElementId2mapElement;
  unordered_map<int, QQFDIMG> _mapElementId2Qqfdimg;
};

unique_ptr<QQTMapDatabase::Provider> CurrentProvider;
}

void Database::exit() {
  CurrentProvider.reset();
}
bool Database::init() {
  if (auto provider = make_unique<QQTMapEditorDatabaseProvider>();
    provider->init()) {
    CurrentProvider = std::move(provider);
    QQTMapDatabase::setProvider(CurrentProvider.get());
    return true;
  }
  qCritical() << "初始化地图数据失败";
  return false;
}
