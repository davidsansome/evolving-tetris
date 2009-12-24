#include "test_tetramino.h"

#include <QTest>
#include <QtDebug>

#include <boost/random/mersenne_twister.hpp>

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

void Tetramino::TestRandomCtor() {
  // Check the sequence of tetraminos is deterministic with a given seed
  static const uint32_t seed = 42;
  static const int count = 10;
  static const int expected[] = {5,3,0,5,0,0,4,2,5,1};

  boost::mt19937 rng;

  for (int i=0 ; i<100 ; ++i) {
    rng.seed(seed);

    for (int j=0 ; j<count; ++j) {
      t_.InitRandom(rng);
      QCOMPARE(t_.Type(), expected[j]);
    }
  }
}

} // namespace Test
