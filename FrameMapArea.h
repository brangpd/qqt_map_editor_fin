#ifndef FRAMEMAPAREA_H
#define FRAMEMAPAREA_H

#include <QFrame>
#include <memory>

namespace Ui {
class FrameMapArea;
}

class MainWindow;
class FrameMapArea : public QFrame {
  Q_OBJECT

public:
  explicit FrameMapArea(MainWindow *window, QWidget *parent = nullptr);
  ~FrameMapArea() override;

protected:
  void paintEvent(QPaintEvent *Event) override;

private:
  Ui::FrameMapArea *ui;
  MainWindow *_mainWindow{};
};

#endif // FRAMEMAPAREA_H
