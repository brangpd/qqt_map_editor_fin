#include "QQFDIMGFactory.h"
#include "QQFDIMG.h"
#include "QQFDIMGReaderVer0.h"
#include "QQFDIMGReaderVer1.h"
#include <iostream>
#include <string_view>
using namespace std;

#include "DefineIOMacros.h"

bool QQFDIMGFactory::write(std::ostream &os, const QQFDIMG &qqfdimg) { // NOLINT
  cerr << "写出QQFDIMG未实现\n";
  return false;
}
bool QQFDIMGFactory::read(istream &is, QQFDIMG &qqfdimg) {
  if (!is) {
    cerr << "输入流为空\n";
    return false;
  }
  // 判断文件头是否为 QQF DIMG
  char head[9];
  is.read(head, 8);
  head[8] = 0;
  if (head != "QQF\x1a"
              "DIMG"sv) {
    cerr << "QQFDIMG文件头格式错误\n";
    return false;
  }

  int16_t version;
  readt(version, int16_t);
  switch (version) {
  default:
    cerr << "QQFDIMG版本号不支持\n";
    return false;
  case 0:
    return QQFDIMGReaderVer0().read(is, qqfdimg);
  case 1:
    return QQFDIMGReaderVer1().read(is, qqfdimg);
  }
}
