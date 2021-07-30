#include "MainWindow.h"
#include "EQQTCity.h"
#include "EQQTGameMode.h"
#include "FrameMapArea.h"
#include "QQTMap.h"
#include "QQTMapFactory.h"
#include "ui_MainWindow.h"
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <fstream>
#include <iostream>
#include <memory>
#include <utility>

using namespace std;

namespace {
QString getMapFileFilter() {
  return QStringLiteral("Map Files (*.map);;All Files (*.*)");
}
} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  // emmm，最后还是没时间实现，谁有兴趣自己实现一下
  ui->menuEdit->removeAction(ui->actionMove_Right);
  ui->menuEdit->removeAction(ui->actionMove_Up);
  ui->menuEdit->removeAction(ui->actionMove_Down);
  ui->menuEdit->removeAction(ui->actionMove_Left);
  ui->menuEdit->removeAction(ui->actionMove);

  // 初始化city下拉菜单
  for (EQQTCity city : allQQTCities) {
    ui->comboBoxCity->addItem(toDescription(city), (int)city);
  }
  // 初始化游戏模式下拉菜单
  for (EQQTGameMode mode : allQQTGameModes) {
    ui->comboBoxGameMode->addItem(toDescription(mode), (int)mode);
  }

  // 程序开始没打开地图，所有编辑操作不可用
  setGameMapEditEnabled(false);

  // SIGNALS <=> SLOTS
  connect(
      ui->comboBoxCity, QOverload<int>::of(&QComboBox::activated),
      [this](int index) { this->changeCity(static_cast<EQQTCity>(index)); });
  connect(ui->comboBoxGameMode, QOverload<int>::of(&QComboBox::activated),
          [this](int index) { this->changeGameMode(static_cast<EQQTGameMode>(index)); });
  connect(ui->actionUndo, &QAction::toggled, this, &MainWindow::undo);
  connect(ui->actionRedo, &QAction::toggled, this, &MainWindow::redo);
  connect(ui->actionNew_Map, &QAction::toggled, this, &MainWindow::newMap);
  connect(ui->actionOpen_Map, &QAction::toggled, this, &MainWindow::openMap);
  connect(ui->actionSave, &QAction::toggled, this, &MainWindow::saveMap);
  connect(ui->actionSave_As, &QAction::toggled, this, &MainWindow::saveMapAs);
  connect(ui->actionClose_Map, &QAction::toggled, this, &MainWindow::closeMap);
  connect(ui->actionExit, &QAction::toggled, this, &MainWindow::close);
}

MainWindow::~MainWindow() { delete ui; }
void MainWindow::changeCity(EQQTCity city) {}
void MainWindow::changeGameMode(EQQTGameMode mode) {}
bool MainWindow::saveMapToFile() {
  if (_qqtMap) {
//    return false;
    return QQTMapFactory::write(_filename, *_qqtMap);
  }
  return false;
}
void MainWindow::undo() {}
void MainWindow::redo() {}
bool MainWindow::tipSaveIfNecessary() {
  if (isDirty()) {
    auto ret = QMessageBox::question(
        this, "保存地图？", "地图已修改，是否保存？",
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
void MainWindow::closeMapEdit() {
  // 删除旧绘图区域
  delete _frameMapArea;
  _frameMapArea = nullptr;
  // 关闭操作UI
  setGameMapEditEnabled(false);
}
void MainWindow::openMapEdit() {
  // 地图信息数据在UI上展示
  ui->spinBoxWidth->setValue(_qqtMap->w);
  ui->spinBoxHeight->setValue(_qqtMap->h);
  ui->comboBoxMaxPlayers->setCurrentText(QString::number(_qqtMap->nMaxPlayers));
  ui->comboBoxGameMode->setCurrentIndex(static_cast<int>(_qqtMap->gameMode));

  // 建立新绘图区域
  _frameMapArea = new FrameMapArea(this);
  ui->scrollAreaMapArea->setWidget(_frameMapArea);

  // 开启操作UI
  setGameMapEditEnabled(true);
}
void MainWindow::newMap() {
  if (tipSaveIfNecessary()) {
    _qqtMap = make_shared<QQTMap>();
    // 加载一些默认参数
    _qqtMap->gameMode = EQQTGameMode::normal;
    _qqtMap->w = 15;
    _qqtMap->h = 13;
    _qqtMap->version = 4;
    _qqtMap->nMaxPlayers = 8;
    openMapEdit();
  }
}
void MainWindow::openMap() {
  if (tipSaveIfNecessary()) {
    auto filename = QFileDialog::getOpenFileName(
        this, "打开地图", QString(), ::getMapFileFilter());
    if (filename.isEmpty()) {
      return;
    }
    _filename = filename.toLocal8Bit().toStdString();
    _qqtMap = make_shared<QQTMap>();
    QQTMapFactory::read(_filename, *_qqtMap);
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
  if (_filename.empty()) {
    // 新建的地图，还未保存过，没有文件名，调用另存为
    return saveMapAs();
  }
  return saveMapToFile();
}
bool MainWindow::saveMapAs() {
  auto filename = QFileDialog::getSaveFileName(
      this, QStringLiteral("另存为"), QString(), ::getMapFileFilter());
  if (!filename.isEmpty()) {
    _filename = filename.toLocal8Bit().toStdString();
    return saveMapToFile();
  }
  return false;
}
void MainWindow::setGameMapEditEnabled(bool b) {
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
  if (tipSaveIfNecessary()) {
    QWidget::closeEvent(event);
  }
  event->ignore();
}
