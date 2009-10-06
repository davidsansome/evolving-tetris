#include "individual.h"

#include <string>
#include <iostream>
#include <vector>

static const char* kTargetString = "Methinks it is like a weasel";
static const int kPopulationSize = 500;

int main(int argc, char *argv[]) {
  using namespace std;

  srand(time(NULL));
  string target = kTargetString;

  // Initialise the population
  vector<Individual> population;
  population.reserve(kPopulationSize);

  Individual* fittest = NULL;
  for (int i=0 ; i<kPopulationSize ; ++i) {
    Individual individual(target);
    individual.Init();
    population.push_back(individual);

    // Keep track of the fittest one
    if (!fittest || individual.Fitness() >
                    fittest->Fitness())
      fittest = &(population.at(i));
  }

  while (*fittest != target) {
    // Pick two potential parents at random
    Individual& one = population.at(rand() % kPopulationSize);
    Individual& two = population.at(rand() % kPopulationSize);

    // Mutate the fitter one
    Individual child(target);
    child.MutateFrom((one.Fitness() > two.Fitness()) ? one : two);

    // Pick two individuals that we might replace
    one = population.at(rand() % kPopulationSize);
    two = population.at(rand() % kPopulationSize);

    // Replace the least fit one
    ((one.Fitness() > two.Fitness()) ? two : one) = child;

    // Was this one fitter than the last fittest?
    if (child.Fitness() > fittest->Fitness()) {
      fittest = &((one.Fitness() > two.Fitness()) ? two : one);
      cout << "New fittest: " << *fittest << endl;
    }
  }
}
