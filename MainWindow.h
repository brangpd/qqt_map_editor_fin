#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QListWidgetItem>
#include <QMainWindow>
#include <list>
#include <memory>
#include <string>
#include <unordered_set>

namespace Ui {
class MainWindow;
}

class FrameMapArea;

class QQTMap;
enum class EQQTCity;
enum class EQQTGameMode;
class MainWindow : public QMainWindow {
Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow() override;

protected:
  void closeEvent(QCloseEvent *event) override;

private:
  void setGameMapEditEnabled(bool b);
  void changeCity(EQQTCity city);
  void changeGameMode(EQQTGameMode mode);
  /// 保存地图到文件
  /// \return 是否保存成功
  bool saveMapToFile();
  bool isDirty() const { return _isDirty; }
  /// 地图发生修改后，在退出或关闭时调用询问是否保存或丢弃修改
  /// \return 确认（保存或丢弃）返回1，取消或关闭返回0，返回0说明步骤阻塞，即不能退出或关闭或打开新地图
  bool tipSaveIfNecessary();
  /// 地图打开或新建完成后，初始化编辑区域
  void openMapEdit();
  void closeMapEdit();

private slots:
  void undo();
  void redo();
  void newMap();
  void openMap();
  void closeMap();
  bool saveMap();
  bool saveMapAs();

private:
  Ui::MainWindow *ui;

  std::string _filename;
  std::shared_ptr<QQTMap> _qqtMap;
  bool _isDirty{false};
  FrameMapArea *_frameMapArea{};
};

#endif // MAINWINDOW_H
