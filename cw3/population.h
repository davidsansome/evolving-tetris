#ifndef POPULATION_H
#define POPULATION_H

#include "individual.h"
#include "utilities.h"

#include <QList>

#include <algorithm>
#include <cstdlib>

template <typename IndividualType>
class Population {
 public:
  Population(int size);

  void InitRandom();

  IndividualType& operator[](int i) { return individuals_[i]; }
  IndividualType& SelectFitnessProportionate(const IndividualType& excluding = IndividualType());

  IndividualType& Fittest();
  IndividualType& LeastFit();
  quint64 MeanFitness() const;

  double WeightDiversity() const;
  double ExponentDiversity() const;
  double DisplacementDiversity() const;

  void Replace(int i, const IndividualType& replacement);

 private:
  QList<IndividualType> individuals_;

  struct CompareFitness {
    bool operator()(const IndividualType& left, const IndividualType& right) const {
      Q_ASSERT(left.HasFitness());
      Q_ASSERT(right.HasFitness());

      return left.Fitness() < right.Fitness();
    }
  };
};

template <typename IndividualType>
Population<IndividualType>::Population(int size)
{
  for (int i=0 ; i<size ; ++i) {
    IndividualType individual;
    individuals_ << individual;
  }
}

template <typename IndividualType>
void Population<IndividualType>::InitRandom() {
  for (int i=0 ; i<individuals_.count() ; ++i) {
    IndividualType& individual = individuals_[i];
    individual.InitRandom();
  }
}

template <typename IndividualType>
IndividualType& Population<IndividualType>::Fittest() {
  return *(std::max_element(
      individuals_.begin(), individuals_.end(), CompareFitness()));
}

template <typename IndividualType>
IndividualType& Population<IndividualType>::LeastFit() {
  return *(std::min_element(
      individuals_.begin(), individuals_.end(), CompareFitness()));
}

template <typename IndividualType>
quint64 Population<IndividualType>::MeanFitness() const {
  quint64 total_fitness = 0;
  foreach (const IndividualType& i, individuals_) {
    total_fitness += i.Fitness();
  }
  return total_fitness / individuals_.count();
}

template <typename IndividualType>
double Population<IndividualType>::WeightDiversity() const {
  int diversity = 0;
  for (int i=0 ; i<Criteria_Count ; ++i) {
    QVector<int> genes;
    foreach (const IndividualType& individual, individuals_) {
      genes << individual.Weights()[i];
    }
    diversity += Utilities::StandardDeviation(genes);
  }
  return double(diversity) / Criteria_Count;
}

template <typename IndividualType>
double Population<IndividualType>::ExponentDiversity() const {
  double diversity = 0;
  for (int i=0 ; i<Criteria_Count ; ++i) {
    QVector<double> genes;
    foreach (const IndividualType& individual, individuals_) {
      genes << individual.Exponents()[i];
    }
    diversity += Utilities::StandardDeviation(genes);
  }
  return double(diversity) / Criteria_Count;
}

template <typename IndividualType>
double Population<IndividualType>::DisplacementDiversity() const {
  double diversity = 0;
  for (int i=0 ; i<Criteria_Count ; ++i) {
    QVector<double> genes;
    foreach (const IndividualType& individual, individuals_) {
      genes << individual.Displacements()[i];
    }
    diversity += Utilities::StandardDeviation(genes);
  }
  return double(diversity) / Criteria_Count;
}

template <typename IndividualType>
IndividualType& Population<IndividualType>::SelectFitnessProportionate(const IndividualType& excluding) {
  // This is really nasty

  // Get the sum of all individuals' fitness
  quint64 total_fitness = 0;
  foreach (const IndividualType& i, individuals_) {
    Q_ASSERT(i.HasFitness());

    if (i == excluding)
      continue;

    total_fitness += i.Fitness();
  }

  quint64 selection = double(qrand()) / RAND_MAX * total_fitness;
  for (int i=0 ; i<individuals_.count() ; ++i) {
    IndividualType& individual = individuals_[i];
    if (individual == excluding)
      continue;

    if (individual.Fitness() >= selection)
      return individual;

    selection -= individual.Fitness();
  }

  qFatal("Fitness proportionate selection didn't find an individual :(");
  return individuals_.first(); // To stop compiler warning
}

template <typename IndividualType>
void Population<IndividualType>::Replace(int i, const IndividualType& replacement) {
  individuals_[i] = replacement;
}

#endif // POPULATION_H
