#ifndef FRAMEMAPAREA_H
#define FRAMEMAPAREA_H

#include <QFrame>
#include <memory>

class QQTMap;

namespace Ui {
class MapAreaFrame;
}

class QPainter;
class MainWindow;
struct MapEditData;

class MapAreaFrame final : public QFrame
{
Q_OBJECT

public:
  explicit MapAreaFrame(QWidget *parent, const QQTMap &map);
  ~MapAreaFrame() override;
  void paintMap(bool shouldPaintTop,
                bool shouldPaintGround,
                bool shouldPaintGrid,
                bool shouldPaintSpawnPoints);
  void paintHoverMapElement(int mapElementId, int gridX, int gridY);
  void paintHoverSpawnPoint(int group);
  static std::pair<int, int> localPixels2Grids(int x, int y);
  static std::pair<int, int> grids2LocalPixels(int x, int y);

private:
  Ui::MapAreaFrame *ui;
  const QQTMap &_map;
};

#endif // FRAMEMAPAREA_H
