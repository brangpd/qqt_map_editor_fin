#ifndef QQT_MAP_EDITOR_FIN_LIB_QQTMAPELEMENTMANAGER_H
#define QQT_MAP_EDITOR_FIN_LIB_QQTMAPELEMENTMANAGER_H

#include "EQQTCity.h"
#include <iostream>
#include <vector>

struct QQTMapElement;
class QQTMapElementManager {
public:
  static bool init(std::istream &is);
  static const QQTMapElement *getElementById(int id);
  static const std::vector<int> &getAllElementIds();
};

#endif // QQT_MAP_EDITOR_FIN_LIB_QQTMAPELEMENTMANAGER_H
