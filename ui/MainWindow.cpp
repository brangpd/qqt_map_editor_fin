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
#include "MapEditCommandVariant.h"
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
// 两个出生组的颜色
constexpr Qt::GlobalColor colorsForSpawnGroup[2] = {Qt::red, Qt::blue};
} // namespace

MainWindow::MainWindow()
  : QMainWindow(nullptr),
    ui(new Ui::MainWindow) {
  ui->setupUi(this);
  _spawnGroupAction[0] = ui->actionSpawn_Point_1;
  _spawnGroupAction[1] = ui->actionSpawn_Point_2;
  _mapElementListWidget[QQTMap::kTop] = ui->listWidgetTop;
  _mapElementListWidget[QQTMap::kGround] = ui->listWidgetGround;

  _currentMapEditCommandGroupIt = _mapEditCommandGroups.end();

  auto *provider = QQTMapDatabase::getProvider();
  if (provider == nullptr) {
    // provider 目前一旦初始化成功，全局不变
    qFatal("没有数据提供器，初始化MainWindow失败");
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
  connect(ui->comboBoxCity, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
    int value = ui->comboBoxCity->itemData(index).toInt();
    this->switchCity(static_cast<EQQTCity>(value));
  });
  connect(ui->comboBoxGameMode, QOverload<int>::of(&QComboBox::activated), [this](int index) {
    int value = ui->comboBoxGameMode->itemData(index).toInt();
    this->changeGameMode(static_cast<EQQTGameMode>(value));
  });
  connect(ui->comboBoxMaxPlayers, QOverload<int>::of(&QComboBox::activated), [this](int index) {
    int n = ui->comboBoxMaxPlayers->itemData(index).toInt();
    this->changeNMaxPlayers(n);
  });
  connect(ui->pushButtonResize, &QPushButton::clicked, [this] {
    this->resize(ui->spinBoxWidth->value(), ui->spinBoxHeight->value());
  });
  connect(ui->pushButtonResetSize, &QPushButton::clicked, this, &MainWindow::resetSize);
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
  connect(ui->actionSpawn_Point_1, &QAction::triggered, [this](bool clicked) {
    chooseSpawnGroup(clicked ? 0 : -1);
  });
  connect(ui->actionSpawn_Point_2, &QAction::triggered, [this](bool clicked) {
    chooseSpawnGroup(clicked ? 1 : -1);
  });

  // 初始化city下拉菜单，同时会触发 city 列表的 currentIndexChanged 的更新
  ui->comboBoxCity->clear();
  for (EQQTCity city : allQQTCities) {
    ui->comboBoxCity->addItem(toDescription(city), static_cast<int>(city));
  }
  // 初始化游戏模式下拉菜单
  ui->comboBoxGameMode->clear();
  for (EQQTGameMode mode : allQQTGameModes) {
    ui->comboBoxGameMode->addItem(toDescription(mode), static_cast<int>(mode));
  }
  ui->comboBoxMaxPlayers->clear();
  for (int n : {2, 4, 6, 8}) {
    ui->comboBoxMaxPlayers->addItem(QString::number(n), n);
  }
}
MainWindow::~MainWindow() {
  delete ui;
  delete _mapAreaFrame;
  for (auto &listItem : _mapElementId2MapElementListItem | views::values) {
    delete listItem;
  }
}
void MainWindow::MapEditCommandGroup::exec() {
  for (auto &&cmd : _group) {
    cmd->exec();
  }
}
void MainWindow::MapEditCommandGroup::undo() {
  for (auto &&cmd : _group | views::reverse) {
    cmd->undo();
  }
}
void MainWindow::switchCity(EQQTCity city) const {
  // 修改场景是UI上的变化，地图本身不修改
  // 这里做的事情是清空原先的地图物件选择列表，并添加新场景的地图物件到列表
  auto *provider = QQTMapDatabase::getProvider();
  for (QListWidget *lw : _mapElementListWidget) {
    // 手动移除，为了重复利用 QListItem* 不能直接 clear()
    while (lw->count()) {
      lw->takeItem(0);
    }
  }
  for (int id : getAllMapElementIdsByCity(city)) {
    const QQTMapElement *elem = provider->getMapElementById(id);
    if (!elem) {
      qWarning("在数据提供器中元素%i不存在", id);
      continue;
    }
    QListWidgetItem *listItem = getMapElementListItem(id);
    if (!listItem) {
      qWarning("元素%i列表项不存在", id);
      continue;
    }
    (elem->isGround() ? ui->listWidgetGround : ui->listWidgetTop)
        ->addItem(listItem);
  }
}
void MainWindow::changeGameMode(EQQTGameMode mode) {
  // 修改模式是地图的数据
  if (_qqtMap) {
    recordImmediateCommand<MapEditChangeGameModeCommand>(*_qqtMap, mode);
  }
}
void MainWindow::changeNMaxPlayers(int n) {
  if (_qqtMap) {
    recordImmediateCommand<MapEditChangeNMaxPlayersCommand>(*_qqtMap, n);
  }
}
bool MainWindow::saveMapToFile() {
  if (!_qqtMap) {
    qCritical() << "地图为空，无法保存到文件";
    return false;
  }
  ostringstream oss(ios::binary);
  if (!_qqtMap->write(oss)) {
    qCritical() << "写出地图二进制数据到内存失败";
    return false;
  }

  QFile file(_filename);
  if (!file.open(QIODevice::WriteOnly)) {
    qCritical() << "无法打开文件" << _filename;
    return false;
  }
  auto dataView = oss.view();
  if (file.write(dataView.data(), dataView.size()) != dataView.size()) {
    qCritical() << "写出地图字节数与原数据不匹配";
    return false;
  }
  // 清空命令buf
  resetCommands();

  return true;
}
void MainWindow::clearSpawnPointSelection() {
  ui->actionSpawn_Point_1->setChecked(false);
  ui->actionSpawn_Point_2->setChecked(false);
  _selectedSpawnGroupId = -1;
}
void MainWindow::clearMapElementSelection() {
  ui->listWidgetTop->clearSelection();
  ui->listWidgetGround->clearSelection();
  _selectedMapElementId = 0;
  _selectedMapElementLayer = -1;
}
void MainWindow::paintMap(QPaintDevice &device) const {
  if (!_qqtMap) {
    return;
  }
  const auto &map = *_qqtMap;
  // 从下到上：地板、网格、顶层、出生点
  // 底层
  bool shouldPaintGround = ui->actionShow_Ground->isChecked();
  if (shouldPaintGround) {
    for (int i = 0, ie = map.getHeight(); i < ie; ++i) {
      for (int j = map.getWidth() - 1; j >= 0; --j) {
        int id = map.getMapElementId(j, i, QQTMap::kGround);
        if (id > 0) {
          paintMapElement(device, j, i, id, QQTMap::kGround);
        }
      }
    }
  }
  // 网格
  bool shouldPaintGrid = ui->actionShow_Grid->isChecked();
  if (shouldPaintGrid) {
    paintGrid(device, _qqtMap->getWidth(), _qqtMap->getHeight());
  }
  // 顶层
  bool shouldPaintTop = ui->actionShow_Top->isChecked();
  if (shouldPaintTop) {
    for (int i = 0, ie = map.getHeight(); i < ie; ++i) {
      for (int j = map.getWidth() - 1; j >= 0; --j) {
        int id = map.getMapElementId(j, i, QQTMap::kTop);
        if (id > 0) {
          paintMapElement(device, j, i, id, QQTMap::kTop);
        }
      }
    }
  }
  // 出生点
  bool shouldPaintSpawnPoints = ui->actionShow_Spawn_Points->isChecked();
  if (shouldPaintSpawnPoints) {
    // 两个组
    for (int i = 0; i < 2; ++i) {
      auto &&spawnPoints = map.getSpawnPoints(i);
      for (int j = 0, je = static_cast<int>(spawnPoints.size()); j < je; ++j) {
        auto [x, y] = spawnPoints[j];
        paintSpawnPoint(*_mapAreaFrame, x, y, i, j);
      }
    }
  }
}
void MainWindow::paintGrid(QPaintDevice &device, int w, int h) {
  QPainter painter(&device);
  painter.setPen(Qt::black);
  // 横线
  for (int i = 0, ie = h; i <= ie; ++i) {
    painter.drawLine(
        kCornerPixels,
        kCornerPixels + kGridPixels * i,
        kCornerPixels + kGridPixels * w,
        kCornerPixels + kGridPixels * i);
  }
  // 竖线
  for (int i = 0, ie = w; i <= ie; ++i) {
    painter.drawLine(
        kCornerPixels + kGridPixels * i,
        kCornerPixels,
        kCornerPixels + kGridPixels * i,
        kCornerPixels + kGridPixels * h);
  }
}
void MainWindow::paintMapElement(QPaintDevice &device, int x, int y, int id, int layer, bool transparent) {
  QPainter painter(&device);
  if (transparent) {
    painter.setOpacity(0.5);
  }
  auto dataProvider = QQTMapDatabase::getProvider();
  const QQTMapElement *elem = dataProvider->getMapElementById(id);
  const QQFDIMG *qqfdimg = dataProvider->getQqfdimgOfMapElementById(id);
  if (elem && qqfdimg) {
    painter.drawImage(
        kCornerPixels + x * kGridPixels - elem->xOffset,
        kCornerPixels + y * kGridPixels - elem->yOffset,
        qqfdimg->previewImage);
  }
}
void MainWindow::paintSpawnPoint(QPaintDevice &device, int x, int y, int group, int index, bool transparent) {
  constexpr int kCircleCorner = 8;
  constexpr int kCircleSize = kGridPixels - kCircleCorner * 2;
  QPainter painter(&device);
  if (transparent) {
    painter.setOpacity(0.5);
  }
  painter.setRenderHint(QPainter::Antialiasing);
  // 数字用的字体的样式
  auto font = painter.font();
  font.setBold(true);
  font.setPixelSize(kGridPixels / 2);
  painter.setFont(font);
  // 画圈的笔
  QPen circlePen(QBrush(Qt::gray), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
  // 画数字的笔
  QPen numberPen(Qt::white);
  painter.setBrush(colorsForSpawnGroup[group]);
  QRect rect(
      kCornerPixels + x * kGridPixels + kCircleCorner,
      kCornerPixels + y * kGridPixels + kCircleCorner,
      kCircleSize,
      kCircleSize);
  painter.setPen(circlePen);
  painter.drawEllipse(rect);
  painter.setPen(numberPen);
  if (index != -1) {
    int number = index + 1;
    painter.drawText(rect, Qt::AlignCenter, QString::number(number));
  }
}
void MainWindow::paintHoverMapElement() const {
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
    auto [px, py] = grids2LocalPixels(_mouseGridX, _mouseGridY);
    painter.setOpacity(0.5);
    painter.drawImage(px - elem->xOffset, py - elem->yOffset, img);
  }
}
void MainWindow::paintHoverSpawnPoint() const {
  if (_selectedSpawnGroupId != 0 && _selectedSpawnGroupId != 1) {
    return;
  }
  QPainter painter(_mapAreaFrame);
  painter.setOpacity(0.5);
}
void MainWindow::put() {
  if (_qqtMap && !_qqtMap->isOutOfBound(_mouseGridX, _mouseGridY)) {
    if (_selectedMapElementId > 0) {
      // 判断元素右/下出界
      const QQTMapElement *elem = QQTMapDatabase::getProvider()->getMapElementById(_selectedMapElementId);
      if (elem && !_qqtMap->isOutOfBound(_mouseGridX + elem->w - 1, _mouseGridY + elem->h - 1)) {
        // 放置元素
        recordCommand<MapEditPutElementCommand>(*_qqtMap, _mouseGridX, _mouseGridY, _selectedMapElementId,
                                                _selectedMapElementLayer);
      }
    }
    else if (_selectedSpawnGroupId != -1) {
      recordCommand<MapEditPutSpawnPointCommand>(*_qqtMap, _mouseGridX, _mouseGridY, _selectedSpawnGroupId);
    }
  }
}
void MainWindow::remove() {
  if (_qqtMap && !_qqtMap->isOutOfBound(_mouseGridX, _mouseGridY)) {
    if (_selectedMapElementLayer != -1) {
      // 移除元素
      recordCommand<MapEditRemoveElementCommand>(*_qqtMap, _mouseGridX, _mouseGridY, _selectedMapElementId,
                                                 _selectedMapElementLayer);
    }
    else if (_selectedSpawnGroupId != -1) {
      // 移除出生点
      recordCommand<MapEditRemoveSpawnPointCommand>(*_qqtMap, _mouseGridX, _mouseGridY, _selectedSpawnGroupId);
    }
  }
}
void MainWindow::resize(int w, int h) {
  if (_qqtMap) {
    recordImmediateCommand<MapEditResizeCommand>(*_qqtMap, w, h);
  }
}
void MainWindow::resetSize() const {
  // 仅重置UI显示
  if (_qqtMap) {
    ui->spinBoxWidth->setValue(_qqtMap->getWidth());
    ui->spinBoxHeight->setValue(_qqtMap->getHeight());
  }
}
void MainWindow::chooseSpawnGroup(int group) {
  clearMapElementSelection();
  _selectedSpawnGroupId = group;
}
template <class T, class ... Args>
void MainWindow::recordCommand(Args &&... args) {
  if (_currentMapEditCommands.empty()) {
    // 当前命令容器为空，说明当前是新一轮记录动作，删除当前 group 迭代器之后的所有动作。UI刷新后，重做选项不可选。
    _currentMapEditCommandGroupIt =
        _mapEditCommandGroups.erase(_currentMapEditCommandGroupIt, _mapEditCommandGroups.end());
  }
  // 记录新动作
  _currentMapEditCommands.emplace_back(make_unique<T>(args...))->exec();
  update();
  _mapAreaFrame->update();
}
void MainWindow::endCommand() {
  _mapEditCommandGroups.emplace_back(_currentMapEditCommands);
  _currentMapEditCommandGroupIt = _mapEditCommandGroups.end();
}
void MainWindow::resetCommands() {
  _mapEditCommandGroups.clear();
  _currentMapEditCommands.clear();
  _currentMapEditCommandGroupIt = _mapEditCommandGroups.end();
  update();
}
void MainWindow::undo() {
  if (_currentMapEditCommandGroupIt != _mapEditCommandGroups.begin()) {
    (--_currentMapEditCommandGroupIt)->undo();
    update();
  }
  else {
    qCritical() << "撤销操作异常，迭代器已到达头部";
  }
}
void MainWindow::redo() {
  if (_currentMapEditCommandGroupIt != _mapEditCommandGroups.end()) {
    (_currentMapEditCommandGroupIt++)->exec();
    update();
  }
  else {
    qCritical() << "重做操作异常，迭代器已到达尾部";
  }
}
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

  if (!_qqtMap) {
    qCritical() << "打开地图编辑区域失败：地图为空";
    return;
  }
  // 建立新绘图区域
  delete _mapAreaFrame;
  _mapAreaFrame = new QFrame(this);
  // 捕捉该对象的绘图事件，因为其绘图需要MainWindow中ui的信息：各个层是否绘制，以及地图数据
  _mapAreaFrame->installEventFilter(this);
  _mapAreaFrame->setMouseTracking(true);
  ui->scrollAreaMapArea->setWidget(_mapAreaFrame);

  // 更新GUI绘制
  update();
}
void MainWindow::newMap() {
  if (closeMap()) {
    _qqtMap = make_shared<QQTMap>();
    _filename.clear();
    // 加载一些默认参数
    _qqtMap->setGameMode(EQQTGameMode::normal);
    _qqtMap->resize(15, 13);
    _qqtMap->setNMaxPlayers(8);
    // 打开绘制UI
    openMapEdit();
  }
}
void MainWindow::openMap() {
  if (closeMap()) {
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
bool MainWindow::closeMap() {
  if (tipSaveIfNecessary()) {
    _filename.clear();
    // 删除旧绘图区域
    delete _mapAreaFrame;
    _mapAreaFrame = nullptr;
    // 删除地图
    _qqtMap.reset();
    // 删除UNDO buf
    resetCommands();
    return true;
  }
  return false;
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
    this->paintMap(image);
    if (image.save(filename)) {
      return true;
    }
    qCritical() << "保存为图片失败";
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
      clearSpawnPointSelection();
      // 省一次查找元素ID
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
void MainWindow::paintEvent(QPaintEvent *event) {
    // undo & redo 选项可选性
  ui->actionRedo->setEnabled(_currentMapEditCommandGroupIt != _mapEditCommandGroups.end());
  ui->actionUndo->setEnabled(_currentMapEditCommandGroupIt != _mapEditCommandGroups.begin());
  setGameMapEditEnabled(!!_qqtMap);
  if (_selectedSpawnGroupId != -1 && _selectedMapElementId > 0) {
    qCritical("同时选中出生组和地图元素");
    _selectedSpawnGroupId = 0;
    _selectedMapElementId = 0;
  }
  ui->actionSpawn_Point_1->setChecked(_selectedSpawnGroupId == 0);
  ui->actionSpawn_Point_2->setChecked(_selectedSpawnGroupId == 1);
  if (auto it = _mapElementId2MapElementListItem.find(_selectedMapElementId);
    it != _mapElementId2MapElementListItem.end()) {
    it->second->setSelected(true);
  }
  else {
    ui->listWidgetGround->clearSelection();
    ui->listWidgetTop->clearSelection();
  }

  // 地图元数据
  if (_qqtMap) {
    ui->comboBoxGameMode->setCurrentIndex(static_cast<int>(_qqtMap->getGameMode()) - 1);
    ui->comboBoxMaxPlayers->setCurrentText(QString::number(_qqtMap->getNMaxPlayers()));
    // 宽高要求用户手动复原
  }
}
void MainWindow::closeEvent(QCloseEvent *event) {
  // 没有确定保存或丢弃拒绝退出
  if (!tipSaveIfNecessary()) {
    event->ignore();
    return;
  }
  QWidget::closeEvent(event);
}
bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
  if (watched == _mapAreaFrame) {
    if (!_qqtMap) {
      return true;
    }
    switch (event->type()) {
    default:
      return false;
    case QEvent::Paint:
      {
        // 按照Qt的机制，QPainter在QWidget上画图，只能在相应QWidget的Paint事件中完成
        auto [sizeW, sizeH] = getMapAreaSize();
        _mapAreaFrame->setFixedSize(sizeW, sizeH);
        // 绘制地图
        this->paintMap(*_mapAreaFrame);

        if (!_qqtMap->isOutOfBound(_mouseGridX, _mouseGridY)) {
          // 绘制选中的元素的悬浮，半透明显示
          if (_selectedMapElementId > 0) {
            const QQTMapElement *elem = QQTMapDatabase::getProvider()->getMapElementById(_selectedMapElementId);
            if (!_qqtMap->isOutOfBound(_mouseGridX + elem->w - 1, _mouseGridY + elem->h - 1)) {
              paintMapElement
                  (*_mapAreaFrame, _mouseGridX, _mouseGridY, _selectedMapElementId, _selectedMapElementLayer, true);
            }
          }
          // 绘制悬浮出生点
          if (_selectedSpawnGroupId != -1) {
            paintSpawnPoint(*_mapAreaFrame, _mouseGridX, _mouseGridY, _selectedSpawnGroupId, -1, true);
          }
        }
        // 更新绘制
        _mapAreaFrame->update();
        return true;
      }
    case QEvent::MouseMove:
      {
        auto e = static_cast<QMouseEvent*>(event); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
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
        // 鼠标点住持续移动时继续放置或移除
        if (e->buttons() & Qt::LeftButton) {
          _isPutting = true;
          put();
        }
        else if (_isPutting) {
          _isPutting = false;
          endCommand();
        }
        if (e->buttons() & Qt::RightButton) {
          _isRemoving = true;
          remove();
        }
        else if (_isRemoving) {
          _isRemoving = false;
          endCommand();
        }
        // 如果选中了元素：继续放置
        return true;
      }
    case QEvent::MouseButtonPress:
      {
        auto e = static_cast<QMouseEvent*>(event); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
        if (e->button() == Qt::LeftButton) {
          // 左键放置
          _isPutting = true;
          put();
        }
          // 两者互斥，用else，不允许一起出现
        else if (e->button() == Qt::RightButton) {
          // 右键移除
          _isRemoving = true;
          remove();
        }
        return true;
      }
    case QEvent::MouseButtonRelease:
      {
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
