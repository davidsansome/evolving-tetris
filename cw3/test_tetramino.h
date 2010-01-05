#ifndef TEST_TETRAMINO_H
#define TEST_TETRAMINO_H

#include <QObject>

#include "tetramino.h"

namespace Test {

class Tetramino : public QObject {
  Q_OBJECT

 public:
  Tetramino();

 private slots:
  void TestFixedCtor();

 private:
  ::Tetramino t_;
};

} // namespace Test

#endif // TEST_TETRAMINO_H
