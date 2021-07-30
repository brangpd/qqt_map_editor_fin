#ifndef QQT_MAP_EDITOR_FIN_LIB_QQTMAPFACTORY_H
#define QQT_MAP_EDITOR_FIN_LIB_QQTMAPFACTORY_H

#include <iostream>

struct QQTMap;
class QQTMapFactory {
public:
  /// 读取地图到结构体
  /// \param filename 文件名
  /// \param map 输出结构体
  /// \return 是否读取成功
  static bool read(const std::string &filename, QQTMap &map);
  /// 写出地图
  /// \param filename 文件名
  /// \param map 地图，需包含合法的版本号
  /// \return 是否写出成功
  static bool write(const std::string &filename, const QQTMap &map);
};

#endif // QQT_MAP_EDITOR_FIN_LIB_QQTMAPFACTORY_H
