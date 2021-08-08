#include <bit>
#include <chrono>
#include <format>
#include <fstream>
#include <iostream>
#include <QApplication>

#include "ui/Database.h"
#include "ui/MainWindow.h"
using namespace std;

inline namespace message_handler_stuff {
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
  using namespace chrono;
  const char *file = context.file ? context.file : "";
  const char *function = context.function ? context.function : "";
  auto time = zoned_time(get_tzdb().current_zone(), system_clock::now());
  auto str = format(
      "{:%c} {} ({}:{} {})",
      time,
      msg.toLocal8Bit().constData(),
      QFileInfo(file).fileName().toLocal8Bit().constData(),
      context.line,
      function);

  switch (type) {
  case QtDebugMsg:
    cout << "[DEBUG] " << str << endl;
    break;
  case QtInfoMsg:
    clog << "[INFO ] " << str << endl;
    break;
  case QtWarningMsg:
    clog << "[WARN ] " << str << endl;
    break;
  case QtCriticalMsg:
    cerr << "[ERROR] " << str << endl;
#ifndef _DEBUG
    QMessageBox::critical(nullptr, "错误", QString::fromStdString(str));
#endif
    break;
  case QtFatalMsg:
    cerr << "[FATAL] " << str << endl;
#ifndef _DEBUG
    QMessageBox::critical(nullptr, "严重错误", QString::fromStdString(str));
    exit(-1);
#else
    throw;
#endif
  }
}
}

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  QTranslator qtBaseTranslator;
  if (qtBaseTranslator.load(":/translations/qtbase_zh_CN.qm")) {
    qDebug() << "qtBaseTranslator ok";
    QApplication::installTranslator(&qtBaseTranslator);
  }

  qInstallMessageHandler(messageHandler);
#ifndef _DEBUG
  ofstream log("log.txt", ios::app);
  ofstream err("err.txt", ios::app);
  clog.rdbuf(log.rdbuf());
  cerr.rdbuf(err.rdbuf());
  cout.rdbuf(nullptr);
#endif
  static_assert(endian::native == endian::little, "暂不支持大端机器");

  if (!Database::init()) {
    qFatal("数据库初始化失败");
  }

  MainWindow w;
  w.show();
  return QApplication::exec();
}
