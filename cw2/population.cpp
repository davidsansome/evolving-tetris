#include "population.h"

#include <algorithm>

Population::Population(int size)
{
  for (int i=0 ; i<size ; ++i) {
    Individual individual;
    individual.InitRandom();
    individuals_ << individual;
  }
}

Individual& Population::Fittest() {
  return *(std::max_element(
      individuals_.begin(), individuals_.end(), CompareFitness()));
}

void Population::Replace(int i, const Individual& replacement) {
  individuals_[i] = replacement;
}


