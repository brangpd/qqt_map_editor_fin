#ifndef QQT_MAP_EDITOR_FIN_LIB_QQFDIMGREADER_H
#define QQT_MAP_EDITOR_FIN_LIB_QQFDIMGREADER_H

struct QQFDIMG;
class QQFDIMGReader {
public:
  virtual ~QQFDIMGReader() = default;
  /// 读取QQFDIMG的具体数据，不包含文件头和版本号。
  /// \param is 输入流
  /// \param qqfdimg 输出结果
  /// \return 是否读取成功
  virtual bool read(std::istream &is, QQFDIMG &qqfdimg) = 0;
};

#endif // QQT_MAP_EDITOR_FIN_LIB_QQFDIMGREADER_H
