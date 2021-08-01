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
#include "MapAreaFrame.h"
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
  connect(ui->actionClose_Map, &QAction::triggered, this, &MainWindow::closeMap);
  connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close);
  connect(ui->listWidgetTop, &QListWidget::itemClicked, this, &MainWindow::chooseMapElement);

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
  for (auto &listItem : this->_mapElementId2MapElementListItem | views::values) {
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
  if (_qqtMap) {
    ostringstream oss(ios::binary);
    _qqtMap->write(oss);
    QFile file(_filename);
    file.open(QIODevice::WriteOnly);
    file.write(oss.view().data(), oss.view().size());
    _isDirty = false;
    return true;
  }
  return false;
}
void MainWindow::clearSpawnPointSelection() {
  ui->actionSpawn_Point_A->setChecked(false);
  ui->actionSpawn_Point_B->setChecked(false);
}
void MainWindow::clearMapElementSelection() {}
void MainWindow::undo() {}
void MainWindow::redo() {}
MapElementListWidgetItem* MainWindow::getMapElementListItem(int id) const {
  if (auto it = _mapElementId2MapElementListItem.find(id);
    it != _mapElementId2MapElementListItem.end()) {
    return it->second;
  }
  return nullptr;
}
void MainWindow::closeMapEdit() {
  // 删除旧绘图区域
  delete _mapAreaFrame;
  _mapAreaFrame = nullptr;
  // 关闭操作UI
  setGameMapEditEnabled(false);
}
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
  _mapAreaFrame = new MapAreaFrame(this, *_qqtMap);
  // 捕捉该对象的绘图事件，因为其绘图需要MainWindow中ui的信息：各个层是否绘制，以及地图数据
  _mapAreaFrame->installEventFilter(this);
  _mapAreaFrame->setMouseTracking(true);
  ui->scrollAreaMapArea->setWidget(_mapAreaFrame);

  // 开启操作UI
  setGameMapEditEnabled(true);

  // 更新GUI绘制
  update();
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
      this, QStringLiteral("另存为"), QString(), ::getMapFileFilter());
  if (!filename.isEmpty()) {
    _filename = std::move(filename);
    return saveMapToFile();
  }
  return false;
}
void MainWindow::chooseMapElement(QListWidgetItem *lwi) {
  // 地图物体选择和出生点选择互斥
  clearSpawnPointSelection();
  QVariant data = lwi->data(MapElementListWidgetItem::ElementIdRole);
  if (!data.isNull()) {
    int id = data.toInt();
    // 同一个物体点两次，取消选中
    if (id == _selectedMapElementId) {
      _selectedMapElementId = 0;
      lwi->setSelected(false);
    }
    else {
      _selectedMapElementId = id;
    }
  }
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
    switch (event->type()) {
    default:
      return false;
    case QEvent::Paint:
      {
        // 按照Qt的机制，QPainter在QWidget上画图，只能在相应QWidget的Paint事件中完成
        _mapAreaFrame->paintMap(
            ui->actionShow_Top->isChecked(),
            ui->actionShow_Ground->isChecked(),
            ui->actionShow_Grid->isChecked(),
            ui->actionShow_Spawn_Points->isChecked());
        if (_selectedMapElementId) {
          if (!_qqtMap->isOutOfBound(_mouseGridX, _mouseGridY)) {
            _mapAreaFrame->paintHoverMapElement(_selectedMapElementId, _mouseGridX, _mouseGridY);
          }
        }
        _mapAreaFrame->update();
        return true;
      }
    case QEvent::MouseMove:
      {
        auto e = dynamic_cast<QMouseEvent*>(event);
        int oldX = _mouseGridX;
        int oldY = _mouseGridY;
        // 获取当前网格坐标
        tie(_mouseGridX, _mouseGridY) = _mapAreaFrame->localPixels2Grids(e->x(), e->y());
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
        if (!_isPutting) {
          return true;
        }
        // 如果选中了元素：继续放置
      }
    case QEvent::MouseButtonPress:
      {
        if (_selectedMapElementId > 0) {
          if (!_qqtMap->isOutOfBound(_mouseGridX, _mouseGridY)) {
            _isPutting = true;
            const QQTMapElement *elem = QQTMapDatabase::getProvider()->getMapElementById(_selectedMapElementId);
            auto layer = elem->isGround() ? QQTMap::kGround : QQTMap::kTop;
            int removedId = _qqtMap->removeMapElementAt(_mouseGridX, _mouseGridY, layer);
            _qqtMap->putMapElementAt(_mouseGridX, _mouseGridY, _selectedMapElementId, layer);
            _mapAreaFrame->update();
          }
        }
        return true;
      }
    case QEvent::MouseButtonRelease:
      {
        _isPutting = false;
        return true;
      }
    }
  }
  return false;
}
