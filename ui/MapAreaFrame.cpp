#include "MapAreaFrame.h"

#include <memory>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QStatusBar>
#include <QString>
#include <QtGui>
#include <utility>

#include "QQFDIMG.h"
#include "QQTMap.h"
#include "QQTMapDatabase.h"
#include "QQTMapElement.h"
#include "ui_MapAreaFrame.h"
using namespace std;

namespace {
// 每个元素占40像素格子
constexpr int kGridPixels = 40;
// 在画图区域周围添加80像素边界
constexpr int kCornerPixels = kGridPixels * 2;
}

MapAreaFrame::MapAreaFrame(QWidget *parent, const QQTMap &map)
  : QFrame(parent), _map(map),
    ui(new Ui::MapAreaFrame) {
  ui->setupUi(this);
  this->setMouseTracking(true);
}
MapAreaFrame::~MapAreaFrame() {
  delete ui;
}
void MapAreaFrame::paintMap(bool shouldPaintTop,
                            bool shouldPaintGround,
                            bool shouldPaintGrid,
                            bool shouldPaintSpawnPoints) {
  auto *dataProvider = QQTMapDatabase::getProvider();
  auto &map = _map;

  QPainter painter(this);
  // 画图区域左右上下分别添加80像素边缘
  setFixedSize(
      kCornerPixels * 2 + kGridPixels * map.getWidth(),
      kCornerPixels * 2 + kGridPixels * map.getHeight());

  // 从下到上：地板、网格、顶层、出生点
  // 底层
  if (shouldPaintGround) {
    // 从上往下每一行，先画上行
    for (int r = 0, re = map.getHeight(); r < re; ++r) {
      // 从右往左每一列，先画右列
      for (int c = map.getWidth() - 1, ce = 0; c >= ce; --c) {
        int id = map.getMapElementId(c, r, QQTMap::kGround);
        if (id > 0) {
          const QQTMapElement *elem = dataProvider->getMapElementById(id);
          const QQFDIMG *qqfdimg = dataProvider->getQqfdimgOfMapElementById(id);
          if (elem && qqfdimg) {
            painter.drawImage(
                kCornerPixels + c * kGridPixels - elem->xOffset,
                kCornerPixels + r * kGridPixels - elem->yOffset,
                qqfdimg->frames.front().image);
          }
        }
      }
    }
  }
  // 网格
  if (shouldPaintGrid) {
    auto oldPen = painter.pen();
    painter.setPen(Qt::black);
    // 横线
    for (int i = 0, ie = map.getHeight() + 1; i < ie; ++i) {
      painter.drawLine(
          kCornerPixels,
          kCornerPixels + kGridPixels * i,
          kCornerPixels + kGridPixels * map.getWidth(),
          kCornerPixels + kGridPixels * i);
    }
    // 竖线
    for (int i = 0, ie = map.getWidth() + 1; i < ie; ++i) {
      painter.drawLine(
          kCornerPixels + kGridPixels * i,
          kCornerPixels,
          kCornerPixels + kGridPixels * i,
          kCornerPixels + kGridPixels * map.getHeight());
    }
    painter.setPen(oldPen);
  }
  // 顶层
  if (shouldPaintTop) {
    for (int i = 0, ie = map.getHeight(); i < ie; ++i) {
      for (int j = map.getWidth() - 1; j >= 0; --j) {
        int id = map.getMapElementId(j, i, QQTMap::kTop);
        if (id > 0) {
          const QQTMapElement *elem = dataProvider->getMapElementById(id);
          const QQFDIMG *qqfdimg = dataProvider->getQqfdimgOfMapElementById(id);
          painter.drawImage(
              kCornerPixels + j * kGridPixels - elem->xOffset,
              kCornerPixels + i * kGridPixels - elem->yOffset,
              qqfdimg->frames.front().image);
        }
      }
    }
  }
  // 出生点
  if (shouldPaintSpawnPoints) {
    constexpr int kCircleCorner = 8;
    constexpr int kCircleSize = kGridPixels - kCircleCorner * 2;
    painter.setRenderHint(QPainter::Antialiasing);

    // 数字用的字体的样式
    auto font = painter.font();
    font.setBold(true);
    font.setPixelSize(kGridPixels);
    painter.setFont(font);

    // 画圈的笔
    QPen circlePen(QBrush(Qt::gray), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    // 画数字的笔
    QPen numberPen(Qt::white);

    // 两个出生组的颜色
    Qt::GlobalColor colorsForGroup[2] = {Qt::red, Qt::blue};

    // 两个组
    for (int i = 0; i < 2; ++i) {
      painter.setBrush(colorsForGroup[i]);
      auto &&spawnPoints = map.getSpawnPoints(i);
      for (int j = 0, je = static_cast<int>(spawnPoints.size()); j < je; ++j) {
        auto [x, y] = spawnPoints[j];
        QRect rect(
            kCornerPixels + x * kGridPixels + kCircleCorner,
            kCornerPixels + y * kGridPixels + kCircleCorner,
            kCircleSize,
            kCircleSize);
        painter.setPen(circlePen);
        painter.drawEllipse(rect);
        painter.setPen(numberPen);
        painter.drawText(rect, Qt::AlignCenter, QString::number(j + 1));
      }
    }
  }
}
void MapAreaFrame::paintHoverMapElement(int mapElementId, int gridX, int gridY) {
  auto *provider = QQTMapDatabase::getProvider();
  const QQTMapElement *elem = provider->getMapElementById(mapElementId);
  const QQFDIMG *qqfdimg = provider->getQqfdimgOfMapElementById(mapElementId);
  if (elem && qqfdimg) {
    const QImage &img = qqfdimg->previewImage;
    QPainter painter(this);
    // 网格转像素
    auto [px, py] = grids2LocalPixels(gridX, gridY);
    painter.setOpacity(0.5);
    painter.drawImage(px - elem->xOffset, py - elem->yOffset, img);
  }
}
std::pair<int, int> MapAreaFrame::localPixels2Grids(int x, int y) {
  x /= kGridPixels;
  y /= kGridPixels;
  x -= kCornerPixels / kGridPixels;
  y -= kCornerPixels / kGridPixels;
  return {x, y};
}
std::pair<int, int> MapAreaFrame::grids2LocalPixels(int x, int y) {
  x += kCornerPixels / kGridPixels;
  y += kCornerPixels / kGridPixels;
  x *= kGridPixels;
  y *= kGridPixels;
  return {x, y};
}
