#ifndef POPULATION_H
#define POPULATION_H

#include "individual.h"

#include <QList>

class Population {
 public:
  Population(int size);

  void InitRandom();

  Individual& operator[](int i) { return individuals_[i]; }
  Individual& SelectFitnessProportionate(const Individual& excluding = Individual());

  Individual& Fittest();
  Individual& LeastFit();
  quint64 MeanFitness() const;

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
