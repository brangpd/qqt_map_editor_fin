#pragma once

#include <QMainWindow>
#include <memory>
#include <string>
#include <utility>
#include <unordered_set>

#include "EQQTCity.h"

class MapElementListWidgetItem;

namespace Ui {
class MainWindow;
}

class MapAreaFrame;
class QPainter;

struct MapEditData;
class QQTMap;
enum class EQQTCity;
enum class EQQTGameMode;

class MainWindow final : public QMainWindow
{
Q_OBJECT

public:
  explicit MainWindow();
  ~MainWindow() override;

private:
  void closeEvent(QCloseEvent *event) override;
  void paintEvent(QPaintEvent *event) override;
  bool eventFilter(QObject *watched, QEvent *event) override;

  /// 开启或关闭所有用于地图编辑的UI组件
  /// \param b 开启或关闭
  void setGameMapEditEnabled(bool b) const;
  void changeCity(EQQTCity city) const;
  void changeGameMode(EQQTGameMode mode) const;
  /// 保存地图到文件
  /// \return 是否保存成功
  bool saveMapToFile();
  bool isDirty() const { return _isDirty; }
  /// 地图发生修改后，在退出或关闭时调用询问是否保存或丢弃修改
  /// \return 确认（保存或丢弃）返回1，取消或关闭返回0，返回0说明步骤阻塞，即不能退出或关闭或打开新地图
  bool tipSaveIfNecessary();
  /// 地图打开或新建完成后，初始化编辑区域
  void openMapEdit();
  void closeMapEdit();
  MapElementListWidgetItem* getMapElementListItem(int id) const;
  const std::vector<int>& getAllMapElementIdsByCity(EQQTCity city) const {
    return _allMapElementIdsByCity[static_cast<int>(city) - 1];
  }
  std::vector<int>& getAllMapElementIdsByCity(EQQTCity city) {
    return _allMapElementIdsByCity[static_cast<int>(city) - 1];
  }
  void clearSpawnPointSelection();
  void clearMapElementSelection();

private slots:
  void undo();
  void redo();
  void newMap();
  void openMap();
  void closeMap();
  bool saveMap();
  bool saveMapAs();
  void chooseMapElement(QListWidgetItem *lwi);

private:
  Ui::MainWindow *ui;

  QString _filename;
  std::shared_ptr<QQTMap> _qqtMap;
  std::shared_ptr<MapEditData> _mapEditData;
  bool _isDirty{false};
  MapAreaFrame *_mapAreaFrame{};
  std::unordered_map<int, MapElementListWidgetItem*> _mapElementId2MapElementListItem;
  std::vector<int> _allMapElementIdsByCity[std::size(allQQTCities)];
  int _mouseGridX{-1}, _mouseGridY{-1};
  int _selectedMapElementId{};
  bool _isPutting{};
};
