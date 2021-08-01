#include "QQFDIMG.h"
#include "MapElementListWidgetItem.h"
#include "QQTMapDatabase.h"
using namespace std;
MapElementListWidgetItem::MapElementListWidgetItem(int id, QListWidget *view)
  : QListWidgetItem(view),
    _elementId(id) {
  auto *provider = QQTMapDatabase::getProvider();
  if (!provider) {
    cerr << "没有数据提供器，无法初始化元素列表项" << endl;
    return;
  }
  auto *qqfdimg = provider->getQqfdimgOfMapElementById(id);
  if (!qqfdimg) {
    cerr << "找不到元素的QQFDIMG：" << id << endl;
    return;
  }

  setIcon(QIcon(QPixmap::fromImage(qqfdimg->previewImage)));
}
QVariant MapElementListWidgetItem::data(int role) const {
  switch (role) {
  case ElementIdRole:
    return _elementId;
  default:
    return QListWidgetItem::data(role);
  }
}
