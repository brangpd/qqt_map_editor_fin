#pragma once

#include <QMainWindow>
#include <list>
#include <memory>
#include <utility>
#include <unordered_map>
#include <vector>

#include "EQQTCity.h"
#include "MapEditCommand.h"

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
  MainWindow(const MainWindow &) = delete;
  ~MainWindow() override;

private:
  class MapEditCommandGroup final : public MapEditCommand
  {
  public:
    explicit MapEditCommandGroup(std::vector<std::unique_ptr<MapEditCommand>> &group) {
      _group.swap(group);
    }
    void exec() override;
    void undo() override;
  private:
    std::vector<std::unique_ptr<MapEditCommand>> _group;
  };

  void paintEvent(QPaintEvent *event) override;
  void closeEvent(QCloseEvent *event) override;
  bool eventFilter(QObject *watched, QEvent *event) override;

  /// 开启或关闭所有用于地图编辑的UI组件
  void setGameMapEditEnabled(bool b) const;
  void switchCity(EQQTCity city) const;
  void changeGameMode(EQQTGameMode mode);
  void changeNMaxPlayers(int n);
  /// 保存地图到文件
  /// \return 是否保存成功
  bool saveMapToFile();
  /// 地图发生修改后，在退出或关闭时调用询问是否保存或丢弃修改
  /// \return 确认（保存或丢弃）返回1，取消或关闭返回0，返回0说明步骤阻塞，即不能退出或关闭或打开新地图
  bool tipSaveIfNecessary();
  /// 地图打开或新建完成后，初始化编辑区域
  void openMapEdit();
  static std::pair<int, int> getMapAreaSize(const QQTMap &map);
  MapElementListWidgetItem* getMapElementListItem(int id) const;
  const std::vector<int>& getAllMapElementIdsByCity(EQQTCity city) const {
    return _allMapElementIdsByCity[static_cast<int>(city) - 1];
  }
  std::vector<int>& getAllMapElementIdsByCity(EQQTCity city) {
    return _allMapElementIdsByCity[static_cast<int>(city) - 1];
  }
  void clearSpawnPointSelection();
  void clearMapElementSelection();
  static void paintMap(QPaintDevice &device, const QQTMap &map,
                       bool shouldPaintGround,
                       bool shouldPaintTop,
                       bool shouldPaintGrid,
                       bool shouldPaintSpawnPoints);
  static void paintGrid(QPaintDevice &device, int w, int h);
  static void paintMapElement(QPaintDevice &device, int x, int y, int id, int layer, bool transparent = false);
  static void paintSpawnPoint(QPaintDevice &device, int x, int y, int group, int index, bool transparent = false);
  void paintHoverMapElement() const;
  void paintHoverSpawnPoint() const;
  void put();
  void remove();
  void resizeMap();
  void resetSize() const;
  void chooseSpawnGroup(int group);
  bool isDirty() const { return !_mapEditCommandGroups.empty(); }
  static std::pair<int, int> localPixels2Grids(int x, int y);
  static std::pair<int, int> grids2LocalPixels(int x, int y);

  template <class T, class ...Args>
  void recordCommand(Args &&... args);
  template <class T, class ...Args>
  void recordImmediateCommand(Args &&... args) {
    recordCommand<T>(args...);
    endCommand();
  }
  void endCommand();
  void resetCommands();

private slots:
  void undo();
  void redo();
  void newMap();
  void openMap();
  bool closeMap();
  bool saveMap();
  bool saveMapAs();
  bool saveMapAsImage();
  void chooseMapElement(QListWidgetItem *lwi);

private:
  Ui::MainWindow *ui;

  QString _filename;
  std::shared_ptr<QQTMap> _qqtMap;
  std::shared_ptr<MapEditData> _mapEditData;
  QFrame *_mapAreaFrame{};
  std::unordered_map<int, MapElementListWidgetItem*> _mapElementId2MapElementListItem;
  std::vector<int> _allMapElementIdsByCity[std::size(allQQTCities)];
  int _mouseGridX{-1}, _mouseGridY{-1};
  int _selectedMapElementId{}; ///< 当前选中的地图元素ID，正数有效
  int _selectedMapElementLayer{-1}; ///< 当前选中的地图元素层，0上，1下，-1未选中
  int _selectedSpawnGroupId{-1}; ///< 当前选中的出生组，0/1，-1为未选中
  bool _isPutting{};
  bool _isRemoving{};
  std::list<MapEditCommandGroup> _mapEditCommandGroups;
  std::vector<std::unique_ptr<MapEditCommand>> _currentMapEditCommands;
  decltype(_mapEditCommandGroups)::iterator _currentMapEditCommandGroupIt;
  QListWidget *_mapElementListWidget[2];
  QAction *_spawnGroupAction[2];
};
