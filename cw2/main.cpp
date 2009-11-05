#include <QApplication>
#include <QtDebug>
#include <QThreadPool>

#include "engine.h"

#ifndef QT_NO_DEBUG
# include <QTest>
# include "test_board.h"

  template <typename T>
  void RunTest(const QStringList& args) {
    T* test = new T;
    int ret = QTest::qExec(test, args);
    if (ret)
      exit(ret);
    delete test;
  }
#endif

int main(int argc, char** argv) {
  QApplication a(argc, argv);

#ifndef QT_NO_DEBUG
  QStringList args(a.arguments());
  if (args.count() >= 2 && args[1] == "t") {
    args.removeAt(1);
    RunTest<Test::Board>(args);

    return 0;
  }
#endif

  QThreadPool::globalInstance()->setMaxThreadCount(1);
  Engine e;
  e.Run();

  return 0;
}
