#ifndef TEST_GENERATORS_H
#define TEST_GENERATORS_H

#include <QObject>

#include "utilities.h"

namespace Test {

class Generators : public QObject {
  Q_OBJECT
 public:
  Generators();

 private slots:
  void init();
  void OnePointCrossover();
  void UniformCrossover();
  void Range();
  void Mutate();
  void MutateReplace();
};

} // namespace Test

#endif // TEST_GENERATORS_H
