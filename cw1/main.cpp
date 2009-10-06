#include "individual.h"
#include "population.h"

#include <string>
#include <iostream>
#include <vector>

static const char* kTargetString = "Methinks it is like a weasel";
static const int kPopulationSize = 500;
static const bool kCrossover = true;

const Individual& FittestOf(const Individual& one, const Individual& two) {
  return (one.Fitness() > two.Fitness()) ? one : two;
}

int main(int argc, char *argv[]) {
  using namespace std;

  srand(time(NULL));
  string target = kTargetString;

  // Initialise the population
  Population pop(target, kPopulationSize);

  int max_fitness = pop.Fittest().Fitness();
  int iteration_count = 0;

  while (pop.Fittest() != target) {
    // Pick two potential parents at random
    const Individual& parent1_one = pop[rand() % kPopulationSize];
    const Individual& parent1_two = pop[rand() % kPopulationSize];

    Individual child(target);
    if (kCrossover) {
      // Pick another two potential parents
      const Individual& parent2_one = pop[rand() % kPopulationSize];
      const Individual& parent2_two = pop[rand() % kPopulationSize];

      child.Crossover(FittestOf(parent1_one, parent1_two),
                      FittestOf(parent2_one, parent2_two));
      child.Mutate();
    } else {
      // Mutate the fitter one
      child.MutateFrom(FittestOf(parent1_one, parent1_two));
    }

    // Pick two individuals that we might replace
    int new_index_one = rand() % kPopulationSize;
    int new_index_two = rand() % kPopulationSize;

    // Replace the least fit one
    if (pop[new_index_one].Fitness() > pop[new_index_two].Fitness())
      pop.Replace(new_index_two, child);
    else
      pop.Replace(new_index_one, child);

    // Did the highest fitness of the population change?
    if (pop.Fittest().Fitness() > max_fitness) {
      max_fitness = pop.Fittest().Fitness();
      cout << pop.Fittest() << endl;
    }

    iteration_count ++;
  }

  cout << iteration_count << " iterations" << endl;
}
