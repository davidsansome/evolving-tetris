#ifndef POPULATION_H
#define POPULATION_H

#include "individual.h"

#include <QList>

class Population {
 public:
  Population(int size);

  void InitRandom();

  Individual& operator[](int i) { return individuals_[i]; }
  Individual& Fittest();
  Individual& SelectFitnessProportionate(const Individual& excluding = Individual());

  void Replace(int i, const Individual& replacement);

 private:
  QList<Individual> individuals_;

  struct CompareFitness {
    bool operator()(const Individual& left, const Individual& right) const {
      Q_ASSERT(left.HasFitness());
      Q_ASSERT(right.HasFitness());

      return left.Fitness() < right.Fitness();
    }
  };
};

#endif // POPULATION_H
