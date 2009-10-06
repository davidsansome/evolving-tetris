#include "population.h"

Population::Population(const std::string& target, int size)
    : fittest_(NULL)
{
  individuals_.reserve(size);
  for (int i=0 ; i<size ; ++i) {
    Individual individual(target);
    individual.Init();
    individuals_.push_back(individual);

    // Keep track of the fittest one
    if (!fittest_ || individual.Fitness() >
                     fittest_->Fitness())
      fittest_ = &(individuals_.at(i));
  }
}

const Individual& Population::Fittest() const {
  return *fittest_;
}

const Individual& Population::operator[](int i) const {
  return individuals_.at(i);
}

void Population::Replace(int i, const Individual& replacement) {
  individuals_.at(i) = replacement;

  if (replacement.Fitness() > fittest_->Fitness())
    fittest_ = &(individuals_.at(i));
}


