#include "QQTMapFactory.h"
#include "QQTMap.h"
#include "QQTMapReader.h"
#include "QQTMapReaderVer3.h"
#include "QQTMapReaderVer4.h"
#include <fstream>

using namespace std;
namespace {
QQTMapReader *mapReader(int version) {
  switch (version) {
  default:
    return nullptr;
  case 3: {
    static QQTMapReaderVer3 mapReader3;
    return &mapReader3;
  }
  case 4: {
    static QQTMapReaderVer4 mapReader4;
    return &mapReader4;
  }
  }
}
} // namespace
bool QQTMapFactory::read(const std::string &filename, QQTMap &map) {
  // 读取版本号，然后交给恰当的Reader读取
  ifstream is(filename, ios::binary);
  if (!is) {
    return false;
  }
  is.read((char *)&map.version, sizeof(int32_t));
  if (QQTMapReader *reader = ::mapReader(map.version)) {
    return reader->read(is, map);
  }
  return false;
}
bool QQTMapFactory::write(const std::string &filename, const QQTMap &map) { return false; }
