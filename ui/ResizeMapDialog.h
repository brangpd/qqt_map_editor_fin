#pragma once

#include <QDialog>

namespace Ui {
class ResizeMapDialog;
}

class ResizeMapDialog final : public QDialog
{
Q_OBJECT

public:
  explicit ResizeMapDialog(int w, int h, QWidget *parent = nullptr);
  ~ResizeMapDialog() override;

  int getWidth() const;
  int getHeight() const;

private:
  Ui::ResizeMapDialog *ui;
};
