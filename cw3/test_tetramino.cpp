#include "test_tetramino.h"

#include <QTest>
#include <QtDebug>

namespace Test {

Tetramino::Tetramino() {
}

void Tetramino::TestFixedCtor() {
  t_.InitFrom(0);
  QCOMPARE(t_.Type(), 0);

  t_.InitFrom(2);
  QCOMPARE(t_.Type(), 2);

  ::Tetramino other;
  other.InitFrom(t_);
  QCOMPARE(other.Type(), 2);
}

} // namespace Test
