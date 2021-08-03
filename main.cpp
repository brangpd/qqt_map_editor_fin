#include <bit>
#include <chrono>
#include <fstream>
#include <iostream>
#include <QApplication>

#include "ui/Database.h"
#include "ui/MainWindow.h"
using namespace std;

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  if constexpr (std::endian::native != std::endian::little) {
    cerr << "不支持大端机器" << endl;
    return -1;
  }

#ifndef _DEBUG
  auto time = chrono::system_clock::now().time_since_epoch().count();
  ofstream log("log.txt", ios::app);
  ofstream err("err.txt", ios::app);
  clog.rdbuf(log.rdbuf());
  cerr.rdbuf(err.rdbuf());
  clog << "\ntime:" << time << endl;
  cerr << "\ntime:" << time << endl;
#endif

  if (!Database::init()) {
    cerr << "数据库初始化失败" << endl;
    return -1;
  }

  MainWindow w;
  w.show();
  return QApplication::exec();
}
