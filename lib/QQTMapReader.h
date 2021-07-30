#ifndef QQT_MAP_EDITOR_FIN_LIB_QQTMAPREADER_H
#define QQT_MAP_EDITOR_FIN_LIB_QQTMAPREADER_H

#include <iostream>

struct QQTMap;
class QQTMapReader {
public:
  virtual ~QQTMapReader() = default;
  /// 读取地图信息，不包含版本号（已在外层QQTMapFactory读取）
  /// \param is 输入流
  /// \param qqtMap 输出
  /// \return 是否读取成功
  virtual bool read(std::istream &is, QQTMap &qqtMap) = 0;
};

#endif // QQT_MAP_EDITOR_FIN_LIB_QQTMAPREADER_H
