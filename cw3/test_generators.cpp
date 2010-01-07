#include "test_generators.h"

#include <QtDebug>
#include <QTest>

#include <boost/random/uniform_real.hpp>

namespace Test {

Generators::Generators()
{
}

void Generators::init() {
  Utilities::global_rng.seed(1);
}

void Generators::OnePointCrossover() {
  const int size = 10;
  QList<int> ones;
  QList<int> twos;

  for (int i=0 ; i<size ; ++i) {
    ones << 1;
    twos << 2;
  }

  int output[size];
  std::generate(output, output+size, Utilities::OnePointCrossoverGenerator(
                ones.begin(), twos.begin(), size));

  QCOMPARE(output[0], 1);
  QCOMPARE(output[1], 1);
  QCOMPARE(output[2], 1);
  QCOMPARE(output[3], 1);
  QCOMPARE(output[4], 1);
  QCOMPARE(output[5], 1);
  QCOMPARE(output[6], 2);
  QCOMPARE(output[7], 2);
  QCOMPARE(output[8], 2);
  QCOMPARE(output[9], 2);
}

void Generators::UniformCrossover() {
  const int size = 1000;
  QList<int> ones;
  QList<int> twos;

  for (int i=0 ; i<size ; ++i) {
    ones << 1;
    twos << 2;
  }

  int output[size];
  std::generate(output, output+size, Utilities::UniformCrossoverGenerator(
                ones.begin(), twos.begin()));

  int ones_count = std::count(output, output+size, 1);
  int twos_count = std::count(output, output+size, 2);

  QVERIFY(ones_count > size/4);
  QVERIFY(twos_count > size/4);
  QCOMPARE(ones_count + twos_count, size);
}

void Generators::Range() {
  const int size = 1000;
  int output[size];
  std::generate(output, output+size, Utilities::RangeGenerator(1, 3));

  int ones_count = std::count(output, output+size, 1);
  int twos_count = std::count(output, output+size, 2);
  int threes_count = std::count(output, output+size, 3);

  QVERIFY(ones_count > size/6);
  QVERIFY(twos_count > size/6);
  QVERIFY(threes_count > size/6);
  QCOMPARE(ones_count + twos_count + threes_count, size);
}

void Generators::Mutate() {
  const int size = 1000;
  int buf[1000];
  std::fill(buf, buf+size, 1);

  auto dist = boost::uniform_real<>(3, 5);
  std::generate(buf, buf+size, Utilities::MutateGenerator(0.5, dist, buf));

  int ones_count = std::count(buf, buf+size, 1);
  int threes_count = std::count(buf, buf+size, 3);
  int fours_count = std::count(buf, buf+size, 4);
  QVERIFY(ones_count > size/4);
  QVERIFY(threes_count > size/8);
  QVERIFY(fours_count > size/8);
  QCOMPARE(ones_count + threes_count + fours_count, size);

  std::fill(buf, buf+size, 1);
  std::generate(buf, buf+size, Utilities::MutateGenerator(0.0, dist, buf));

  ones_count = std::count(buf, buf+size, 1);
  QCOMPARE(ones_count, size);
}

void Generators::MutateReplace() {
  const int size = 1000;
  int buf[1000];
  std::fill(buf, buf+size, 1);

  auto dist = Utilities::RangeGenerator(3, 4);
  std::generate(buf, buf+size, Utilities::MutateReplaceGenerator(0.5, dist, buf));

  int ones_count = std::count(buf, buf+size, 1);
  int threes_count = std::count(buf, buf+size, 3);
  int fours_count = std::count(buf, buf+size, 4);
  QVERIFY(ones_count > size/4);
  QVERIFY(threes_count > size/8);
  QVERIFY(fours_count > size/8);
  QCOMPARE(ones_count + threes_count + fours_count, size);

  std::fill(buf, buf+size, 1);
  std::generate(buf, buf+size, Utilities::MutateReplaceGenerator(0.0, dist, buf));

  ones_count = std::count(buf, buf+size, 1);
  QCOMPARE(ones_count, size);
}

} // namespace Test
