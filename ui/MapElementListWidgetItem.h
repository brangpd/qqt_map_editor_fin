#pragma once

#include <QListWidgetItem>

struct QQTMapElement;

class MapElementListWidgetItem final : public QListWidgetItem
{
public:
  enum { ElementIdRole = Qt::UserRole };
  explicit MapElementListWidgetItem(int id, QListWidget *view = nullptr);
  int getElementId() const { return _elementId; }
  QVariant data(int role) const override;

private:
  const int _elementId;
};
