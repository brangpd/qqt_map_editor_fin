#include <chrono>
#include <fstream>
#include <iostream>
#include <QApplication>

#include "ui/Database.h"
#include "ui/MainWindow.h"
using namespace std;

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

#ifndef _DEBUG
  auto time = to_string(chrono::system_clock::now().time_since_epoch().count());
  ofstream log("log" + time + ".txt", ios::trunc);
  ofstream err("err" + time + ".txt", ios::trunc);
  std::clog.rdbuf(log.rdbuf());
  std::cerr.rdbuf(err.rdbuf());
#endif

  if (!Database::init()) {
    cerr << "数据库初始化失败" << endl;
    return -1;
  }

  MainWindow w;
  w.show();
  return QApplication::exec();
}
