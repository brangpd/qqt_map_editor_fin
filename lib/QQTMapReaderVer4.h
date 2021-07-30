#ifndef QQT_MAP_EDITOR_FIN_LIB_QQTMAPREADERVER4_H
#define QQT_MAP_EDITOR_FIN_LIB_QQTMAPREADERVER4_H

#include "QQTMapReader.h"

class QQTMapReaderVer4 : public QQTMapReader {
public:
  bool read(std::istream &is, QQTMap &qqtMap) override;
};

#endif // QQT_MAP_EDITOR_FIN_LIB_QQTMAPREADERVER4_H
