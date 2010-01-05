#ifndef POPULATION_H
#define POPULATION_H

#include "individual.h"
#include "utilities.h"

#include <algorithm>
#include <vector>
#include <cstdlib>
#include <cassert>

template <typename IndividualType>
class Population {
 public:
  Population(int size);

  void InitRandom();

  IndividualType& operator[](int i) { return individuals_[i]; }
  IndividualType& SelectFitnessProportionate(const IndividualType& excluding = IndividualType());

  IndividualType& Fittest();
  IndividualType& LeastFit();
  uint64_t MeanFitness() const;

  template <typename ChromosomeAccessor>
  double Diversity(const ChromosomeAccessor& accessor) const;

  void Replace(int i, const IndividualType& replacement);

 private:
  std::vector<IndividualType> individuals_;

  struct CompareFitness {
    bool operator()(const IndividualType& left, const IndividualType& right) const {
      assert(left.HasFitness());
      assert(right.HasFitness());

      return left.Fitness() < right.Fitness();
    }
  };
};

template <typename IndividualType>
Population<IndividualType>::Population(int size)
{
  for (int i=0 ; i<size ; ++i) {
    IndividualType individual;
    individuals_.push_back(individual);
  }
}

template <typename IndividualType>
void Population<IndividualType>::InitRandom() {
  for (uint i=0 ; i<individuals_.size() ; ++i) {
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
uint64_t Population<IndividualType>::MeanFitness() const {
  uint64_t total_fitness = 0;

  for (auto it = individuals_.begin() ; it != individuals_.end() ; ++it) {
    total_fitness += it->Fitness();
  }
  return total_fitness / individuals_.size();
}

template <typename IndividualType>
template <typename ChromosomeAccessor>
double Population<IndividualType>::Diversity(
    const ChromosomeAccessor& accessor) const {
  typedef typename ChromosomeAccessor::result_type::value_type GeneType;

  const int count = accessor(&individuals_[0]).size();

  GeneType diversity = 0;
  for (int i=0 ; i<count ; ++i) {
    std::vector<GeneType> genes;
    for (auto it = individuals_.begin() ; it != individuals_.end() ; ++it) {
      genes.push_back(accessor(&(*it))[i]);
    }
    diversity += Utilities::StandardDeviation(genes);
  }
  return double(diversity) / count;
}

template <typename IndividualType>
IndividualType& Population<IndividualType>::SelectFitnessProportionate(const IndividualType& excluding) {
  // This is really nasty

  // Get the sum of all individuals' fitness
  uint64_t total_fitness = 0;
  for (auto it = individuals_.begin() ; it != individuals_.end() ; ++it) {
    assert(it->HasFitness());

    if (*it == excluding)
      continue;

    total_fitness += it->Fitness();
  }

  uint64_t selection = double(rand()) / RAND_MAX * total_fitness;
  for (uint i=0 ; i<individuals_.size() ; ++i) {
    IndividualType& individual = individuals_[i];
    if (individual == excluding)
      continue;

    if (individual.Fitness() >= selection)
      return individual;

    selection -= individual.Fitness();
  }

  std::cerr << "Fitness proportionate selection didn't find an individual :(";
  return *individuals_.begin(); // To stop compiler warning
}

template <typename IndividualType>
void Population<IndividualType>::Replace(int i, const IndividualType& replacement) {
  individuals_[i] = replacement;
}

#endif // POPULATION_H
