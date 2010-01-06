#include "individualbase.h"

IndividualBase::IndividualBase()
  : has_fitness_(false),
    fitness_(0)
{
}

void IndividualBase::SetFitness(uint64_t fitness) {
  fitness_ = fitness;
  has_fitness_ = true;
}
