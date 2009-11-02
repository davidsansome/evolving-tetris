#include <QApplication>
#include <QtDebug>

#include "individual.h"

int main(int argc, char** argv) {
  QApplication a(argc, argv);

  Individual i(QSize(10, 20));
  qDebug() << i.Board();

  i.Next();
  qDebug() << i.Board();

  return 0;
}
