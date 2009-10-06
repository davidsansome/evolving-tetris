#ifndef POPULATION_H
#define POPULATION_H

#include "individual.h"

#include <vector>

class Population {
 public:
  Population(const std::string& target, int size);

  const Individual& operator[](int i) const;
  const Individual& Fittest() const;

  void Replace(int i, const Individual& replacement);

 private:
  std::vector<Individual> individuals_;
  Individual* fittest_;
};

#endif // POPULATION_H
