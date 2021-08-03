#include "MainWindow.h"

#include <memory>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <sstream>
#include <strstream>
#include <utility>

#include "EQQTCity.h"
#include "EQQTGameMode.h"
#include "MapElementListWidgetItem.h"
#include "QQFDIMG.h"
#include "QQTMap.h"
#include "QQTMapDatabase.h"
#include "QQTMapElement.h"
#include "ui_MainWindow.h"

using namespace std;

namespace {
// 用于打开和保存文件的扩展名
QString getMapFileFilter() {
  return QStringLiteral("地图文件 (*.map);;所有文件 (*.*)");
}
// 用于保存为图片的扩展名
QString getPngFileFilter() {
  return QStringLiteral("PNG文件 (*.png);;所有文件 (*.*)");
}
// 每个元素占40像素格子
constexpr int kGridPixels = 40;
// 在画图区域周围添加80像素边界
constexpr int kCornerPixels = kGridPixels * 2;
} // namespace

MainWindow::MainWindow()
  : QMainWindow(nullptr), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  auto *provider = QQTMapDatabase::getProvider();
  if (provider == nullptr) {
    // provider 目前一旦初始化成功，全局不变
    cerr << "没有数据提供器，初始化MainWindow失败" << endl;
    exit(1);
  }
  for (int id : provider->getAllMapElementIds()) {
    EQQTCity city = QQTMapElement::getCity(id);
    // 初始化每个场景下各自的ID
    getAllMapElementIdsByCity(city).push_back(id);
    // 用于后面变换city时使用
    _mapElementId2MapElementListItem[id] = new MapElementListWidgetItem(id);
  }

  // emmm，最后还是没时间实现，谁有兴趣自己实现一下
  ui->menuEdit->removeAction(ui->actionMove_Right);
  ui->menuEdit->removeAction(ui->actionMove_Up);
  ui->menuEdit->removeAction(ui->actionMove_Down);
  ui->menuEdit->removeAction(ui->actionMove_Left);
  ui->menuEdit->removeAction(ui->actionMove);

  // SIGNALS <=> SLOTS
  connect(ui->comboBoxCity, QOverload<int>::of(&QComboBox::currentIndexChanged),
          [this](int index) {
            this->changeCity(static_cast<EQQTCity>(
              ui->comboBoxCity->itemData(index).toInt()));
          });
  connect(ui->comboBoxGameMode, QOverload<int>::of(&QComboBox::currentIndexChanged),
          [this](int index) {
            this->changeGameMode(static_cast<EQQTGameMode>(
              ui->comboBoxGameMode->itemData(index).toInt()));
          });
  connect(ui->actionUndo, &QAction::triggered, this, &MainWindow::undo);
  connect(ui->actionRedo, &QAction::triggered, this, &MainWindow::redo);
  connect(ui->actionNew_Map, &QAction::triggered, this, &MainWindow::newMap);
  connect(ui->actionOpen_Map, &QAction::triggered, this, &MainWindow::openMap);
  connect(ui->actionSave, &QAction::triggered, this, &MainWindow::saveMap);
  connect(ui->actionSave_As, &QAction::triggered, this, &MainWindow::saveMapAs);
  connect(ui->actionSave_Image, &QAction::triggered, this, &MainWindow::saveMapAsImage);
  connect(ui->actionClose_Map, &QAction::triggered, this, &MainWindow::closeMap);
  connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close);
  connect(ui->listWidgetTop, &QListWidget::itemClicked, this, &MainWindow::chooseMapElement);
  connect(ui->listWidgetGround, &QListWidget::itemClicked, this, &MainWindow::chooseMapElement);

  // 初始化city下拉菜单，同时会触发 city 列表的 currentIndexChanged 的更新
  for (EQQTCity city : allQQTCities) {
    ui->comboBoxCity->addItem(toDescription(city), static_cast<int>(city));
  }
  // 初始化游戏模式下拉菜单
  for (EQQTGameMode mode : allQQTGameModes) {
    ui->comboBoxGameMode->addItem(toDescription(mode), static_cast<int>(mode));
  }
  // 程序开始没打开地图，所有编辑操作不可用
  setGameMapEditEnabled(false);
}
MainWindow::~MainWindow() {
  delete ui;
  delete _mapAreaFrame;
  for (auto &listItem : _mapElementId2MapElementListItem | views::values) {
    delete listItem;
  }
}
void MainWindow::changeCity(EQQTCity city) const {
  // 修改场景是UI上的变化，地图本身不修改
  // 这里做的事情是清空原先的地图物件选择列表，并添加新场景的地图物件到列表
  auto *provider = QQTMapDatabase::getProvider();
  for (QListWidget *lw : {ui->listWidgetTop, ui->listWidgetGround}) {
    // 手动移除，为了重复利用 QListItem* 不能直接 clear()
    while (lw->count()) {
      lw->takeItem(0);
    }
  }
  for (int id : getAllMapElementIdsByCity(city)) {
    if (const QQTMapElement *elem = provider->getMapElementById(id)) {
      if (QListWidgetItem *listItem = getMapElementListItem(id)) {
        (elem->isGround() ? ui->listWidgetGround : ui->listWidgetTop)
            ->addItem(listItem);
      }
      else {
        cerr << "列表元素不存在：" << id << endl;
      }
    }
    else {
      cerr << "在数据提供器中元素不存在：" << id << endl;
    }
  }
}
void MainWindow::changeGameMode(EQQTGameMode mode) const {
  // 修改模式是地图的数据
  if (_qqtMap) {
    _qqtMap->setGameMode(mode);
  }
}
bool MainWindow::saveMapToFile() {
  if (!_qqtMap) {
    cerr << "地图为空，无法保存到文件" << endl;
    return false;
  }
  ostringstream oss(ios::binary);
  if (!_qqtMap->write(oss)) {
    cerr << "写出地图二进制数据到内存失败" << endl;
    return false;
  }

  QFile file(_filename);
  if (!file.open(QIODevice::WriteOnly)) {
    cerr << "无法打开文件" << _filename.data() << endl;
    return false;
  }
  auto dataView = oss.view();
  if (file.write(dataView.data(), dataView.size()) != dataView.size()) {
    cerr << "写出地图字节数与原数据不匹配" << endl;
    return false;
  }
  _isDirty = false;
  return true;
}
void MainWindow::clearSpawnPointSelection() const {
  ui->actionSpawn_Point_A->setChecked(false);
  ui->actionSpawn_Point_B->setChecked(false);
}
void MainWindow::clearMapElementSelection() const {
  ui->listWidgetTop->clearSelection();
  ui->listWidgetGround->clearSelection();
}
void MainWindow::paintMap(QPainter &painter) const {
  if (!_qqtMap) {
    return;
  }
  const auto &map = *_qqtMap;
  auto *dataProvider = QQTMapDatabase::getProvider();

  // 从下到上：地板、网格、顶层、出生点
  // 底层
  bool shouldPaintGround = ui->actionShow_Ground->isChecked();
  if (shouldPaintGround) {
    // 从上往下每一行，先画上行
    for (int r = 0, re = map.getHeight(); r < re; ++r) {
      // 从右往左每一列，先画右列
      for (int c = map.getWidth() - 1, ce = 0; c >= ce; --c) {
        int id = map.getMapElementId(c, r, QQTMap::kGround);
        if (id > 0) {
          const QQTMapElement *elem = dataProvider->getMapElementById(id);
          const QQFDIMG *qqfdimg = dataProvider->getQqfdimgOfMapElementById(id);
          if (elem && qqfdimg) {
            painter.drawImage(
                kCornerPixels + c * kGridPixels - elem->xOffset,
                kCornerPixels + r * kGridPixels - elem->yOffset,
                qqfdimg->previewImage);
          }
        }
      }
    }
  }
  // 网格
  bool shouldPaintGrid = ui->actionShow_Grid->isChecked();
  if (shouldPaintGrid) {
    auto oldPen = painter.pen();
    painter.setPen(Qt::black);
    // 横线
    for (int i = 0, ie = map.getHeight(); i <= ie; ++i) {
      painter.drawLine(
          kCornerPixels,
          kCornerPixels + kGridPixels * i,
          kCornerPixels + kGridPixels * map.getWidth(),
          kCornerPixels + kGridPixels * i);
    }
    // 竖线
    for (int i = 0, ie = map.getWidth(); i <= ie; ++i) {
      painter.drawLine(
          kCornerPixels + kGridPixels * i,
          kCornerPixels,
          kCornerPixels + kGridPixels * i,
          kCornerPixels + kGridPixels * map.getHeight());
    }
    painter.setPen(oldPen);
  }
  // 顶层
  bool shouldPaintTop = ui->actionShow_Top->isChecked();
  if (shouldPaintTop) {
    for (int i = 0, ie = map.getHeight(); i < ie; ++i) {
      for (int j = map.getWidth() - 1; j >= 0; --j) {
        int id = map.getMapElementId(j, i, QQTMap::kTop);
        if (id > 0) {
          const QQTMapElement *elem = dataProvider->getMapElementById(id);
          const QQFDIMG *qqfdimg = dataProvider->getQqfdimgOfMapElementById(id);
          painter.drawImage(
              kCornerPixels + j * kGridPixels - elem->xOffset,
              kCornerPixels + i * kGridPixels - elem->yOffset,
              qqfdimg->previewImage);
        }
      }
    }
  }
  // 出生点
  bool shouldPaintSpawnPoints = ui->actionShow_Spawn_Points->isChecked();
  if (shouldPaintSpawnPoints) {
    constexpr int kCircleCorner = 8;
    constexpr int kCircleSize = kGridPixels - kCircleCorner * 2;
    painter.setRenderHint(QPainter::Antialiasing);

    // 数字用的字体的样式
    auto font = painter.font();
    font.setBold(true);
    font.setPixelSize(kGridPixels);
    painter.setFont(font);

    // 画圈的笔
    QPen circlePen(QBrush(Qt::gray), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    // 画数字的笔
    QPen numberPen(Qt::white);

    // 两个出生组的颜色
    Qt::GlobalColor colorsForGroup[2] = {Qt::red, Qt::blue};

    // 两个组
    for (int i = 0; i < 2; ++i) {
      painter.setBrush(colorsForGroup[i]);
      auto &&spawnPoints = map.getSpawnPoints(i);
      for (int j = 0, je = static_cast<int>(spawnPoints.size()); j < je; ++j) {
        auto [x, y] = spawnPoints[j];
        QRect rect(
            kCornerPixels + x * kGridPixels + kCircleCorner,
            kCornerPixels + y * kGridPixels + kCircleCorner,
            kCircleSize,
            kCircleSize);
        painter.setPen(circlePen);
        painter.drawEllipse(rect);
        painter.setPen(numberPen);
        painter.drawText(rect, Qt::AlignCenter, QString::number(j + 1));
      }
    }
  }
}
void MainWindow::paintHoverMapElement(int gridX, int gridY) const {
  if (_selectedMapElementId <= 0) {
    return;
  }
  auto *provider = QQTMapDatabase::getProvider();
  const QQTMapElement *elem = provider->getMapElementById(_selectedMapElementId);
  const QQFDIMG *qqfdimg = provider->getQqfdimgOfMapElementById(_selectedMapElementId);
  if (elem && qqfdimg) {
    const QImage &img = qqfdimg->previewImage;
    QPainter painter(_mapAreaFrame);
    // 网格转像素
    auto [px, py] = grids2LocalPixels(gridX, gridY);
    painter.setOpacity(0.5);
    painter.drawImage(px - elem->xOffset, py - elem->yOffset, img);
  }
}
void MainWindow::paintHoverSpawnPoint() const {}
void MainWindow::put() {
  if (_qqtMap && !_qqtMap->isOutOfBound(_mouseGridX, _mouseGridY)) {
    if (_selectedMapElementId > 0) {
      auto layer = static_cast<QQTMap::Layer>(_selectedMapElementLayer);
      int removedId = _qqtMap->removeMapElementAt(_mouseGridX, _mouseGridY, layer);
      _qqtMap->putMapElementAt(_mouseGridX, _mouseGridY, _selectedMapElementId, layer);
      _mapAreaFrame->update();
    }
  }
}
void MainWindow::remove() {
  if (_qqtMap && !_qqtMap->isOutOfBound(_mouseGridX, _mouseGridY)) {
    _qqtMap->removeMapElementAt(_mouseGridX, _mouseGridY, static_cast<QQTMap::Layer>(_selectedMapElementLayer));
    _mapAreaFrame->update();
  }
}
void MainWindow::undo() {}
void MainWindow::redo() {}
MapElementListWidgetItem* MainWindow::getMapElementListItem(int id) const {
  if (auto it = _mapElementId2MapElementListItem.find(id);
    it != _mapElementId2MapElementListItem.end()) {
    return it->second;
  }
  return nullptr;
}
std::pair<int, int> MainWindow::getMapAreaSize() const {
  if (_qqtMap) {
    return {
      kCornerPixels * 2 + kGridPixels * _qqtMap->getWidth(),
      kCornerPixels * 2 + kGridPixels * _qqtMap->getHeight()
    };
  }
  return {};
}
///@{ 文件菜单交互项
void MainWindow::openMapEdit() {
  // 地图信息数据在UI上展示的初始化
  ui->spinBoxWidth->setValue(_qqtMap->getWidth());
  ui->spinBoxHeight->setValue(_qqtMap->getHeight());
  ui->comboBoxMaxPlayers->setCurrentText(QString::number(_qqtMap->getNMaxPlayers()));
  ui->comboBoxGameMode->setCurrentIndex(static_cast<int>(_qqtMap->getGameMode()) - 1);

  // 建立新绘图区域
  delete _mapAreaFrame;
  if (!_qqtMap) {
    cerr << "打开地图编辑区域失败：地图为空" << endl;
    return;
  }
  _mapAreaFrame = new QFrame(this);
  // 捕捉该对象的绘图事件，因为其绘图需要MainWindow中ui的信息：各个层是否绘制，以及地图数据
  _mapAreaFrame->installEventFilter(this);
  _mapAreaFrame->setMouseTracking(true);
  ui->scrollAreaMapArea->setWidget(_mapAreaFrame);

  // 开启操作UI
  setGameMapEditEnabled(true);

  // 更新GUI绘制
  update();
}
void MainWindow::closeMapEdit() {
  // 删除旧绘图区域
  delete _mapAreaFrame;
  _mapAreaFrame = nullptr;
  // 关闭操作UI
  setGameMapEditEnabled(false);
}
void MainWindow::newMap() {
  if (tipSaveIfNecessary()) {
    _qqtMap = make_shared<QQTMap>();
    // 加载一些默认参数
    _qqtMap->setGameMode(EQQTGameMode::normal);
    _qqtMap->resize(15, 13);
    _qqtMap->setNMaxPlayers(8);
    // 打开绘制UI
    openMapEdit();
  }
}
void MainWindow::openMap() {
  if (tipSaveIfNecessary()) {
    QString filename = QFileDialog::getOpenFileName(
        this, QStringLiteral("打开地图"), QString(), ::getMapFileFilter());
    if (filename.isEmpty()) {
      return;
    }
    _filename = std::move(filename);
    QFile file(_filename);
    file.open(QIODevice::ReadOnly);
    QByteArray content = file.readAll();
    istrstream iss(content.data(), content.size());
    auto newMap = make_shared<QQTMap>();
    if (!newMap->read(iss)) {
      return;
    }
    _qqtMap = std::move(newMap);
    openMapEdit();
  }
}
void MainWindow::closeMap() {
  if (tipSaveIfNecessary()) {
    _filename.clear();
    closeMapEdit();
  }
}
bool MainWindow::saveMap() {
  if (_filename.isEmpty()) {
    // 新建的地图，还未保存过，没有文件名，调用另存为
    return saveMapAs();
  }
  return saveMapToFile();
}
bool MainWindow::saveMapAs() {
  QString filename = QFileDialog::getSaveFileName(
      this, QStringLiteral("另存为"), QString(), getMapFileFilter());
  if (!filename.isEmpty()) {
    _filename = std::move(filename);
    return saveMapToFile();
  }
  return false;
}
bool MainWindow::saveMapAsImage() {
  QString filename = QFileDialog::getSaveFileName(
      this, QStringLiteral("另存为图片"), QString(), getPngFileFilter());
  if (!filename.isEmpty()) {
    auto [sizeW, sizeH] = getMapAreaSize();
    QImage image(sizeW, sizeH, QImage::Format_ARGB32);
    QPainter painter(&image);
    this->paintMap(painter);
    if (image.save(filename)) {
      return true;
    }
    cerr << "保存为图片失败" << endl;
    return false;
  }
  return false;
}
bool MainWindow::tipSaveIfNecessary() {
  if (isDirty()) {
    auto ret = QMessageBox::question(
        this, QStringLiteral("保存地图？"), QStringLiteral("地图已修改，是否保存？"),
        QMessageBox::StandardButtons(QMessageBox::Save | QMessageBox::Discard |
            QMessageBox::Cancel));
    switch (ret) {
    case QMessageBox::Save:
      // 选择了保存，调用保存逻辑，如果保存成功返回1，可以进行后面的逻辑
      return saveMap();
    case QMessageBox::Discard:
      // 选择丢弃修改，返回1，即可以继续进行下面的逻辑
      return true;
    default:
      // 没有确认操作
      return false;
    }
  }
  // 不需要确认时返回1
  return true;
}
///@}
void MainWindow::chooseMapElement(QListWidgetItem *lwi) {
  // 地图物体选择和出生点选择互斥
  clearSpawnPointSelection();
  QVariant data = lwi->data(MapElementListWidgetItem::ElementIdRole);
  if (!data.isNull()) {
    int id = data.toInt();
    // 同一个物体点两次，取消选中
    if (id == _selectedMapElementId) {
      _selectedMapElementId = 0;
      _selectedMapElementLayer = -1;
      lwi->setSelected(false);
    }
    else {
      _selectedMapElementLayer = ui->tabWidgetElement->currentWidget() == ui->tabGround
                                   ? QQTMap::kGround
                                   : QQTMap::kTop;
      _selectedMapElementId = id;
    }
  }
}
void MainWindow::setGameMapEditEnabled(bool b) const {
  ui->spinBoxHeight->setEnabled(b);
  ui->spinBoxWidth->setEnabled(b);
  ui->comboBoxGameMode->setEnabled(b);
  ui->comboBoxMaxPlayers->setEnabled(b);
  ui->actionSave->setEnabled(b);
  ui->actionSave_As->setEnabled(b);
  ui->actionSave_Image->setEnabled(b);
  ui->actionClose_Map->setEnabled(b);
  ui->actionMove_Up->setEnabled(b);
  ui->actionMove_Down->setEnabled(b);
  ui->actionMove_Left->setEnabled(b);
  ui->actionMove_Right->setEnabled(b);
  ui->pushButtonResize->setEnabled(b);
  ui->pushButtonResetSize->setEnabled(b);
}
void MainWindow::closeEvent(QCloseEvent *event) {
  // 没有确定保存或丢弃拒绝退出
  if (!tipSaveIfNecessary()) {
    event->ignore();
    return;
  }
  QWidget::closeEvent(event);
}
void MainWindow::paintEvent(QPaintEvent *event) {
  QWidget::paintEvent(event);
}
bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
  if (watched == _mapAreaFrame) {
    if (!_qqtMap) {
      return false;
    }
    switch (event->type()) {
    default:
      return false;
    case QEvent::Paint:
      {
        // 按照Qt的机制，QPainter在QWidget上画图，只能在相应QWidget的Paint事件中完成
        auto [sizeW, sizeH] = getMapAreaSize();
        _mapAreaFrame->setFixedSize(sizeW, sizeH);
        QPainter painter(_mapAreaFrame);
        // 绘制地图
        this->paintMap(painter);

        // 绘制选中的元素的悬浮，半透明显示
        if (_selectedMapElementId) {
          const QQTMapElement *elem = QQTMapDatabase::getProvider()->getMapElementById(_selectedMapElementId);
          if (!_qqtMap->isOutOfBound(_mouseGridX, _mouseGridY)
            && !_qqtMap->isOutOfBound(_mouseGridX + elem->w - 1, _mouseGridY + elem->h - 1)) {
            this->paintHoverMapElement(_mouseGridX, _mouseGridY);
          }
        }
        // 更新绘制
        _mapAreaFrame->update();
        return true;
      }
    case QEvent::MouseMove:
      {
        auto e = static_cast<QMouseEvent*>(event);
        int oldX = _mouseGridX;
        int oldY = _mouseGridY;
        // 获取当前网格坐标
        tie(_mouseGridX, _mouseGridY) = localPixels2Grids(e->x(), e->y());
        if (oldX == _mouseGridX && oldY == _mouseGridY) {
          // 没有任何变化，不用额外操作
          return true;
        }
        // 在状态栏显示网格坐标
        if (!_qqtMap->isOutOfBound(_mouseGridX, _mouseGridY)) {
          statusBar()->showMessage(QString::asprintf("(%i, %i)", _mouseGridX, _mouseGridY));
        }
        else {
          statusBar()->clearMessage();
        }
        // 更新悬浮物体显示
        _mapAreaFrame->update();
        if (_isPutting) {
          put();
        }
        if (_isRemoving) {
          remove();
        }
        // 如果选中了元素：继续放置
        return true;
      }
    case QEvent::MouseButtonPress:
      {
        auto e = static_cast<QMouseEvent*>(event);
        if (e->button() == Qt::LeftButton) {
          // 左键放置
          _isPutting = true;
          put();
        }
        else if (e->button() == Qt::RightButton) {
          // 右键移除
          _isRemoving = true;
          remove();
        }
        return true;
      }
    case QEvent::MouseButtonRelease:
      {
        _isPutting = _isRemoving = false;
        return true;
      }
    }
  }
  return false;
}
std::pair<int, int> MainWindow::localPixels2Grids(int x, int y) {
  x /= kGridPixels;
  y /= kGridPixels;
  x -= kCornerPixels / kGridPixels;
  y -= kCornerPixels / kGridPixels;
  return {x, y};
}
std::pair<int, int> MainWindow::grids2LocalPixels(int x, int y) {
  x += kCornerPixels / kGridPixels;
  y += kCornerPixels / kGridPixels;
  x *= kGridPixels;
  y *= kGridPixels;
  return {x, y};
}
