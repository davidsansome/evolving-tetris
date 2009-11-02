#include "engine.h"
#include "individual.h"
#include "game.h"

#include <QtConcurrentMap>
#include <QtDebug>

const QSize Engine::kBoardSize(10, 20);
const int Engine::kPopulationSize = 10;
const int Engine::kGamesToRun = 10;
const bool Engine::kCrossover = true;

Engine::Engine()
    : pop_(kPopulationSize)
{
}

const Individual& Engine::FittestOf(const Individual& one, const Individual& two) {
  return (one.Fitness() > two.Fitness()) ? one : two;
}

void Engine::Run() {
  int generation_count = 0;
  quint64 max_fitness = 0;

  forever {
    // Play games to get the fitness of new individuals
    UpdateFitness();

    // Pick two potential parents at random
    const Individual& parent1_one = pop_[qrand() % kPopulationSize];
    const Individual& parent1_two = pop_[qrand() % kPopulationSize];

    Individual child;
    if (kCrossover) {
      // Pick another two potential parents
      const Individual& parent2_one = pop_[qrand() % kPopulationSize];
      const Individual& parent2_two = pop_[qrand() % kPopulationSize];

      child.Crossover(FittestOf(parent1_one, parent1_two),
                      FittestOf(parent2_one, parent2_two));
      child.Mutate();
    } else {
      // Mutate the fitter one
      child.MutateFrom(FittestOf(parent1_one, parent1_two));
    }

    // Pick two individuals that we might replace
    int new_index_one = qrand() % kPopulationSize;
    int new_index_two = qrand() % kPopulationSize;

    // Replace the least fit one
    if (pop_[new_index_one].Fitness() > pop_[new_index_two].Fitness())
      pop_.Replace(new_index_two, child);
    else
      pop_.Replace(new_index_one, child);

    // Did the highest fitness of the population change?
    if (pop_.Fittest().Fitness() > max_fitness) {
      max_fitness = pop_.Fittest().Fitness();
      qDebug() << "After" << generation_count << "fittest is" << pop_.Fittest();
    }

    generation_count ++;
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
    qDebug() << pop_[individual];
  }

  qDeleteAll(games);
}
