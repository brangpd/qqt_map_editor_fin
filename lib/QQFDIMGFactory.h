#ifndef QQT_MAP_EDITOR_FIN_LIB_QQFDIMGFACTORY_H
#define QQT_MAP_EDITOR_FIN_LIB_QQFDIMGFACTORY_H

#include <iostream>
#include <memory>

struct QQFDIMG;
class QQFDIMGFactory {
public:
  static bool read(std::istream &is, QQFDIMG &qqfdimg);
  static bool write(std::ostream &os, const QQFDIMG &qqfdimg);
};

#endif // QQT_MAP_EDITOR_FIN_LIB_QQFDIMGFACTORY_H
