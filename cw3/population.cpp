#include "population.h"

#include <algorithm>
#include <cstdlib>

Population::Population(int size)
{
  for (int i=0 ; i<size ; ++i) {
    Individual individual;
    individuals_ << individual;
  }
}

void Population::InitRandom() {
  for (int i=0 ; i<individuals_.count() ; ++i) {
    Individual& individual = individuals_[i];
    individual.InitRandom();
  }
}

Individual& Population::Fittest() {
  return *(std::max_element(
      individuals_.begin(), individuals_.end(), CompareFitness()));
}

Individual& Population::LeastFit() {
  return *(std::min_element(
      individuals_.begin(), individuals_.end(), CompareFitness()));
}

quint64 Population::MeanFitness() const {
  quint64 total_fitness = 0;
  foreach (const Individual& i, individuals_) {
    total_fitness += i.Fitness();
  }
  return total_fitness / individuals_.count();
}

Individual& Population::SelectFitnessProportionate(const Individual& excluding) {
  // This is really nasty

  // Get the sum of all individuals' fitness
  quint64 total_fitness = 0;
  foreach (const Individual& i, individuals_) {
    Q_ASSERT(i.HasFitness());

    if (i == excluding)
      continue;

    total_fitness += i.Fitness();
  }

  quint64 selection = double(qrand()) / RAND_MAX * total_fitness;
  for (int i=0 ; i<individuals_.count() ; ++i) {
    Individual& individual = individuals_[i];
    if (individual == excluding)
      continue;

    if (individual.Fitness() >= selection)
      return individual;

    selection -= individual.Fitness();
  }

  qFatal("Fitness proportionate selection didn't find an individual :(");
  return individuals_.first(); // To stop compiler warning
}

void Population::Replace(int i, const Individual& replacement) {
  individuals_[i] = replacement;
}


