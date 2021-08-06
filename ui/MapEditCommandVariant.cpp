#include "MapEditCommandVariant.h"
#include "QQTMap.h"
#include <iostream>
using namespace std;

void MapEditPutElementCommand::exec() {
  if (auto map = _map.lock()) {
    map->putMapElementAt(_x, _y, _id, static_cast<QQTMap::Layer>(_layer), &_removed);
    qDebug("放置元素%i: (%i, %i) layer: %i", _id, _x, _y, _layer);
    for (auto &&[id, point] : _removed) {
      auto [x, y] = point;
      qDebug("移除了%i: (%i, %i)", id, x, y);
    }
  }
}
void MapEditPutElementCommand::undo() {
  if (auto map = _map.lock()) {
    pair<int, pair<int, int>> removed;
    map->removeMapElementAt(_x, _y, static_cast<QQTMap::Layer>(_layer), &removed);
    qDebug("移除了%i: (%i, %i)", removed.first, removed.second.first, removed.second.second);
    for (auto &&[id, point] : _removed) {
      auto [x, y] = point;
      map->putMapElementAt(x, y, id, static_cast<QQTMap::Layer>(_layer));
      qDebug("放置元素%i: (%i, %i) layer: %i", id, x, y, _layer);
    }
  }
}
void MapEditRemoveElementCommand::exec() {
  if (auto map = _map.lock()) {
    map->removeMapElementAt(_x, _y, static_cast<QQTMap::Layer>(_layer), &_removed);
  }
}
void MapEditRemoveElementCommand::undo() {
  if (auto map = _map.lock()) {
    auto [id, point] = _removed;
    auto [x, y] = point;
    map->putMapElementAt(x, y, id, static_cast<QQTMap::Layer>(_layer));
  }
}
void MapEditPutSpawnPointCommand::exec() {
  if (auto map = _map.lock()) {
    _hasRemoved = map->putSpawnPointAt(_x, _y, _group, -1, &_removedGroup, &_removedIndex);
  }
}
void MapEditPutSpawnPointCommand::undo() {
  if (auto map = _map.lock()) {
    map->removeSpawnPointAt(_x, _y, _group);
    if (_hasRemoved) {
      map->putSpawnPointAt(_x, _y, _removedGroup, _removedIndex);
    }
  }
}
void MapEditRemoveSpawnPointCommand::exec() {
  if (auto map = _map.lock()) {
    _hasRemoved = map->removeSpawnPointAt(_x, _y, _group, &_removedIndex);
  }
}
void MapEditRemoveSpawnPointCommand::undo() {
  if (auto map = _map.lock()) {
    if (_hasRemoved) {
      map->putSpawnPointAt(_x, _y, _group, _removedIndex);
    }
  }
}
void MapEditResizeCommand::exec() {
  if (auto map = _map.lock()) {
    _oldW = map->getWidth();
    _oldH = map->getHeight();
    map->resize(_w, _h);
  }
}
void MapEditResizeCommand::undo() {
  if (auto map = _map.lock()) {
    map->resize(_oldW, _oldH);
  }
}
void MapEditChangeGameModeCommand::exec() {
  if (auto map = _map.lock()) {
    _old = map->getGameMode();
    map->setGameMode(_mode);
  }
}
void MapEditChangeGameModeCommand::undo() {
  if (auto map = _map.lock()) {
    map->setGameMode(_old);
  }
}
void MapEditChangeNMaxPlayersCommand::exec() {
  if (auto map = _map.lock()) {
    _old = map->getNMaxPlayers();
    map->setNMaxPlayers(_n);
  }
}
void MapEditChangeNMaxPlayersCommand::undo() {
  if (auto map = _map.lock()) {
    map->setNMaxPlayers(_old);
  }
}
