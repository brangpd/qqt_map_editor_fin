#pragma once

#include <QMainWindow>
#include <ranges>
#include <deque>
#include <list>
#include <memory>
#include <string>
#include <utility>
#include <unordered_set>

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
    void exec() override { for (auto &&cmd : _group) { cmd->exec(); } }
    void undo() override { for (auto &&cmd : _group | std::views::reverse) { cmd->undo(); } }
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
  bool isDirty() const { return _isDirty; }
  void setIsDirty() { _isDirty = true; }
  /// 地图发生修改后，在退出或关闭时调用询问是否保存或丢弃修改
  /// \return 确认（保存或丢弃）返回1，取消或关闭返回0，返回0说明步骤阻塞，即不能退出或关闭或打开新地图
  bool tipSaveIfNecessary();
  /// 地图打开或新建完成后，初始化编辑区域
  void openMapEdit();
  void closeMapEdit();
  std::pair<int, int> getMapAreaSize() const;
  MapElementListWidgetItem* getMapElementListItem(int id) const;
  const std::vector<int>& getAllMapElementIdsByCity(EQQTCity city) const {
    return _allMapElementIdsByCity[static_cast<int>(city) - 1];
  }
  std::vector<int>& getAllMapElementIdsByCity(EQQTCity city) {
    return _allMapElementIdsByCity[static_cast<int>(city) - 1];
  }
  void clearSpawnPointSelection() const;
  void clearMapElementSelection() const;
  void paintMap(QPainter &painter) const;
  void paintHoverMapElement(int gridX, int gridY) const;
  void paintHoverSpawnPoint() const;
  void put();
  void remove();
  void resize(int w, int h);
  void resetSize() const;
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

private slots:
  void undo();
  void redo();
  void newMap();
  void openMap();
  void closeMap();
  bool saveMap();
  bool saveMapAs();
  bool saveMapAsImage();
  void chooseMapElement(QListWidgetItem *lwi);

private:
  Ui::MainWindow *ui;

  QString _filename;
  std::shared_ptr<QQTMap> _qqtMap;
  std::shared_ptr<MapEditData> _mapEditData;
  bool _isDirty{false};
  QFrame *_mapAreaFrame{};
  std::unordered_map<int, MapElementListWidgetItem*> _mapElementId2MapElementListItem;
  std::vector<int> _allMapElementIdsByCity[std::size(allQQTCities)];
  int _mouseGridX{-1}, _mouseGridY{-1};
  int _selectedMapElementId{};
  int _selectedMapElementLayer{-1};
  int _selectedSpawnGroupId{-1};
  bool _isPutting{};
  bool _isRemoving{};
  std::list<MapEditCommandGroup> _mapEditCommandGroups;
  std::vector<std::unique_ptr<MapEditCommand>> _currentMapEditCommands;
  decltype(_mapEditCommandGroups)::iterator _currentMapEditCommandGroupIt;
};
