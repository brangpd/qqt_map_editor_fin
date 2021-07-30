#include <memory>
#include <utility>
#include <QLabel>
#include <QMouseEvent>
#include <QStatusBar>
#include <QString>
#include <QPainter>
#include <QtGui>
#include "FrameMapArea.h"
#include "ui_FrameMapArea.h"
#include "MainWindow.h"

FrameMapArea::FrameMapArea(MainWindow *window, QWidget *parent) :
	QFrame(parent), ui(new Ui::FrameMapArea) {
	ui->setupUi(this);
	this->setMouseTracking(true);
}

FrameMapArea::~FrameMapArea() {
	delete ui;
}
void FrameMapArea::paintEvent(QPaintEvent *Event) {
  QFrame::paintEvent(Event);
}
