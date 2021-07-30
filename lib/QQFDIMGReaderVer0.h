#ifndef QQT_MAP_EDITOR_FIN_LIB_QQFDIMGREADERVER0_H
#define QQT_MAP_EDITOR_FIN_LIB_QQFDIMGREADERVER0_H

#include "QQFDIMGReader.h"

class QQFDIMGReaderVer0 : public QQFDIMGReader {
public:
  bool read(std::istream &is, QQFDIMG &qqfdimg) override;
};

#endif // QQT_MAP_EDITOR_FIN_LIB_QQFDIMGREADERVER0_H
