#include "ResizeMapDialog.h"
#include "ui_ResizeMapDialog.h"

ResizeMapDialog::ResizeMapDialog(int w, int h, QWidget *parent)
  : QDialog(parent),
    ui(new Ui::ResizeMapDialog) {
  ui->setupUi(this);
  ui->spinBoxHeight->setValue(h);
  ui->spinBoxWidth->setValue(w);
}
ResizeMapDialog::~ResizeMapDialog() {
  delete ui;
}
int ResizeMapDialog::getWidth() const {
  return ui->spinBoxWidth->value();
}
int ResizeMapDialog::getHeight() const {
  return ui->spinBoxHeight->value();
}
