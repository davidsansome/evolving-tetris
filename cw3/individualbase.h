#ifndef INDIVIDUALBASE_H
#define INDIVIDUALBASE_H

#include <cstdint>

class IndividualBase {
 public:
  IndividualBase();

  // Sets this individual's fitness from the results of some game
  void SetFitness(uint64_t fitness);
  bool HasFitness() const { return has_fitness_; }
  uint64_t Fitness() const { return fitness_; }

 private:
  bool has_fitness_;
  uint64_t fitness_;
};

#endif // INDIVIDUALBASE_H
