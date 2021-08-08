#include "MapEditCommandVariant.h"
#include "QQTMap.h"
#include <iostream>
using namespace std;

void MapEditPutElementCommand::exec() {
  _map.putMapElementAt(_x, _y, _id, static_cast<QQTMap::Layer>(_layer), &_removed);
}
void MapEditPutElementCommand::undo() {
  pair<int, pair<int, int>> removed;
  _map.removeMapElementAt(_x, _y, static_cast<QQTMap::Layer>(_layer), &removed);
  for (auto &&[id, point] : _removed) {
    auto [x, y] = point;
    _map.putMapElementAt(x, y, id, static_cast<QQTMap::Layer>(_layer));
  }
}
void MapEditRemoveElementCommand::exec() {
  _map.removeMapElementAt(_x, _y, static_cast<QQTMap::Layer>(_layer), &_removed);
}
void MapEditRemoveElementCommand::undo() {
  auto [id, point] = _removed;
  auto [x, y] = point;
  _map.putMapElementAt(x, y, id, static_cast<QQTMap::Layer>(_layer));
}
void MapEditPutSpawnPointCommand::exec() {
  _hasRemoved = _map.putSpawnPointAt(_x, _y, _group, -1, &_removedGroup, &_removedIndex);
}
void MapEditPutSpawnPointCommand::undo() {
  _map.removeSpawnPointAt(_x, _y, _group);
  if (_hasRemoved) {
    _map.putSpawnPointAt(_x, _y, _removedGroup, _removedIndex);
  }
}
void MapEditRemoveSpawnPointCommand::exec() {
  _hasRemoved = _map.removeSpawnPointAt(_x, _y, _group, &_removedIndex);
}
void MapEditRemoveSpawnPointCommand::undo() {
  if (_hasRemoved) {
    _map.putSpawnPointAt(_x, _y, _group, _removedIndex);
  }
}
void MapEditResizeCommand::exec() {

  _oldW = _map.getWidth();
  _oldH = _map.getHeight();
  _map.resize(_w, _h);

}
void MapEditResizeCommand::undo() {

  _map.resize(_oldW, _oldH);

}
void MapEditChangeGameModeCommand::exec() {

  _old = _map.getGameMode();
  _map.setGameMode(_mode);

}
void MapEditChangeGameModeCommand::undo() {

  _map.setGameMode(_old);

}
void MapEditChangeNMaxPlayersCommand::exec() {

  _old = _map.getNMaxPlayers();
  _map.setNMaxPlayers(_n);

}
void MapEditChangeNMaxPlayersCommand::undo() {

  _map.setNMaxPlayers(_old);

}
