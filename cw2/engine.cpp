#include "engine.h"
#include "individual.h"
#include "game.h"

#include <QtConcurrentMap>
#include <QtDebug>

#include <iostream>

using std::cout;
using std::endl;

const QSize Engine::kBoardSize(6, 12);
const int Engine::kPopulationSize = 256;
const int Engine::kGamesToRun = 12;
const int Engine::kMaxGenerations = 30;

Engine::Engine()
    : pop_(kPopulationSize)
{
}

const Individual& Engine::FittestOf(const Individual& one, const Individual& two) {
  return (one.Fitness() > two.Fitness()) ? one : two;
}

void Engine::Run() {
  pop_.InitRandom();

  cout << "# Population size " << kPopulationSize << endl;
  cout << "# Games " << kGamesToRun << endl;
  cout << "# Board size " << kBoardSize.width() << "x" << kBoardSize.height() << endl;
  cout << "# Mutation std dev " << Individual::kStandardDeviation << endl;
  cout << "# Running for " << kMaxGenerations << " generations" << endl;

  for (int generation_count=0 ; generation_count<kMaxGenerations ; ++generation_count) {
    // Play games to get the fitness of new individuals
    UpdateFitness();

    // Show output
    cout << generation_count << "\t" <<
            pop_.Fittest().Fitness() << "\t" <<
            pop_.MeanFitness() << "\t" <<
            pop_.LeastFit().Fitness() << "\t" <<
            pop_.Fittest().Weights()[0] << "\t" <<
            pop_.Fittest().Weights()[1] << "\t" <<
            pop_.Fittest().Weights()[2] << "\t" <<
            pop_.Fittest().Weights()[3] << "\t" <<
            pop_.Fittest().Weights()[4] << "\t" <<
            pop_.Fittest().Weights()[5] << endl;

    // Make a new population
    Population pop2(kPopulationSize);

    for (int i=0 ; i<kPopulationSize ; ++i) {
      // Pick two parents from the original population
      const Individual& parent1 = pop_.SelectFitnessProportionate();
      const Individual& parent2 = pop_.SelectFitnessProportionate(parent1);

      // Make a baby
      Individual child;
      child.Crossover(parent1, parent2);
      child.Mutate();

      // Put the child into the new population
      pop2.Replace(i, child);
    }

    // Use the new population
    pop_ = pop2;
  }
}

void Engine::UpdateFitness() {
  // Create games
  QList<Game*> games;
  QMap<int, Game*> games_for_individual;

  for (int i = 0 ; i < kPopulationSize ; ++i) {
    Individual& individual = pop_[i];
    if (individual.HasFitness())
      continue;

    for (int j = 0 ; j < kGamesToRun ; ++j) {
      Game* game = new Game(individual, kBoardSize);
      games << game;
      games_for_individual.insertMulti(i, game);
    }
  }

  if (games.isEmpty())
    return;

  // Run games
  QFuture<void> future = QtConcurrent::map(
      games, PointerMemberFunctionWrapper<void, Game>(&Game::Play));
  future.waitForFinished();

  // Update the fitness for each one
  foreach (int individual, games_for_individual.uniqueKeys()) {
    pop_[individual].SetFitness(games_for_individual.values(individual));
  }

  qDeleteAll(games);
}
