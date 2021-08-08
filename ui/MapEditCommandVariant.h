#pragma once

#include "MapEditCommand.h"
#include <memory>
#include <utility>
#include <vector>

enum class EQQTGameMode;
class QQTMap;

class MapEditPutElementCommand : public MapEditCommand
{
public:
  MapEditPutElementCommand(QQTMap &map, int x, int y, int id, int layer)
    : _map(map), _x(x), _y(y), _id(id), _layer(layer) {}
  void exec() override;
  void undo() override;

private:
  QQTMap &_map;
  const int _x;
  const int _y;
  const int _id;
  const int _layer;
  std::vector<std::pair<int, std::pair<int, int>>> _removed;
};

class MapEditRemoveElementCommand : public MapEditCommand
{
public:
  MapEditRemoveElementCommand(QQTMap &map, int x, int y, int id, int layer)
    : _map(map), _x(x), _y(y), _id(id), _layer(layer) {}
  void exec() override;
  void undo() override;

private:
  QQTMap &_map;
  const int _x;
  const int _y;
  const int _id;
  const int _layer;
  std::pair<int, std::pair<int, int>> _removed;
};

class MapEditPutSpawnPointCommand : public MapEditCommand
{
public:
  MapEditPutSpawnPointCommand(QQTMap &map, int x, int y, int group)
    : _map(map), _x(x), _y(y), _group(group) {}

  void exec() override;
  void undo() override;
private:
  QQTMap &_map;;
  const int _x;
  const int _y;
  const int _group;
  bool _hasRemoved{};
  int _removedGroup{-1}; ///< 删除的是哪个组的出生点
  int _removedIndex{-1}; ///< 删除的出生点的编号
};

class MapEditRemoveSpawnPointCommand : public MapEditCommand
{
public:
  MapEditRemoveSpawnPointCommand(QQTMap &map, int x, int y, int group)
    : _map(map), _x(x), _y(y), _group(group) {}
  void exec() override;
  void undo() override;
private:
  QQTMap &_map;;
  const int _x;
  const int _y;
  const int _group;
  int _removedIndex{-1};
  bool _hasRemoved{};
};

class MapEditResizeCommand final : public MapEditCommand
{
public:
  MapEditResizeCommand(QQTMap &map, int w, int h)
    : _map(map), _w(w), _h(h) {}

  void exec() override;
  void undo() override;
private:
  QQTMap &_map;;
  const int _w;
  const int _h;
  int _oldW;
  int _oldH;
};

class MapEditShiftMapCommand final : public MapEditCommand
{ };

class MapEditChangeGameModeCommand final : public MapEditCommand
{
public:
  MapEditChangeGameModeCommand(QQTMap &map, EQQTGameMode mode)
    : _map(map), _mode(mode) {}

  void exec() override;
  void undo() override;
private:
  QQTMap &_map;;
  const EQQTGameMode _mode;
  EQQTGameMode _old;
};

class MapEditChangeNMaxPlayersCommand final : public MapEditCommand
{
public:
  MapEditChangeNMaxPlayersCommand(QQTMap &map, int n)
    : _n(n), _map(map) {}

  void exec() override;
  void undo() override;
private:
  QQTMap &_map;;
  const int _n;
  int _old;
};
