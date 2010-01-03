#ifndef ENGINE_H
#define ENGINE_H

#include <QSize>
#include <QTime>
#include <QtConcurrentMap>
#include <QtDebug>

#include <iostream>
#include <gflags/gflags.h>

#include "population.h"
#include "game.h"
#include "individual.h"

DEFINE_int32(pop, 128, "number of individuals in the population");
DEFINE_int32(games, 1, "number of games for each individual to play");
DEFINE_int32(generations, 30, "number of generations to run for");
DEFINE_int32(threads, QThread::idealThreadCount(), "number of threads to use");
DECLARE_double(mrate);
DECLARE_double(mstddev);

template <Individual::Algorithm A, typename BoardType>
class Engine {
 public:
  Engine();

  typedef Game<A, BoardType> GameType;

  void Run();

 private:
  void UpdateFitness();
  static const Individual& FittestOf(const Individual& one, const Individual& two);

  Population pop_;

  // Functor for using QtConcurrentMap with object pointers
  template <typename T, typename C>
  class PointerMemberFunctionWrapper
  {
   public:
    typedef T (C::*FunctionPointerType)();
    typedef T result_type;
    inline PointerMemberFunctionWrapper(FunctionPointerType _functionPointer)
    :functionPointer(_functionPointer) { }

    inline T operator()(C* c)
    {
        return (c->*functionPointer)();
    }

   private:
    FunctionPointerType functionPointer;
  };
};

template <Individual::Algorithm A, typename BoardType>
Engine<A, BoardType>::Engine()
    : pop_(FLAGS_pop)
{
}

template <Individual::Algorithm A, typename BoardType>
const Individual& Engine<A, BoardType>::FittestOf(const Individual& one, const Individual& two) {
  return (one.Fitness() > two.Fitness()) ? one : two;
}

template <Individual::Algorithm A, typename BoardType>
void Engine<A, BoardType>::Run() {
  pop_.InitRandom();

  using std::cout;
  using std::endl;

  QThreadPool::globalInstance()->setMaxThreadCount(FLAGS_threads);

  cout << "# Population size " << FLAGS_pop << endl;
  cout << "# Games " << FLAGS_games << endl;
  cout << "# Board size " << BoardType::kWidth << "x" << BoardType::kHeight << endl;
  cout << "# Mutation std dev " << FLAGS_mstddev << endl;
  cout << "# Mutation rate " << FLAGS_mrate << endl;
  cout << "# Generations " << FLAGS_generations << endl;
  cout << "# Threads " << FLAGS_threads << endl;
  cout << "# Board rating function " << Individual::NameOfAlgorithm<A>() << endl;

#ifndef QT_NO_DEBUG
  cout << "# Running in debug mode with assertions enabled" << endl;
#endif

  cout << endl;
  cout << "Generation\t"
          "Highest fitness\t"
          "Mean fitness\t"
          "Lowest fitness\t"
          "w1\t"
          "w2\t"
          "w3\t"
          "w4\t"
          "w5\t"
          "w6\t"
          "Time taken" << endl;

  for (int generation_count=0 ; generation_count<FLAGS_generations ; ++generation_count) {
    // Play games to get the fitness of new individuals
    QTime t;
    t.start();
    UpdateFitness();
    int time_taken = t.elapsed();

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
            pop_.Fittest().Weights()[5] << "\t" <<
            time_taken << endl;

    // Make a new population
    Population pop2(FLAGS_pop);

    for (int i=0 ; i<FLAGS_pop ; ++i) {
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

template <Individual::Algorithm A, typename BoardType>
void Engine<A, BoardType>::UpdateFitness() {
  // Create games
  QList<GameType*> games;
  QMap<int, GameType*> games_for_individual;

  for (int i = 0 ; i < FLAGS_pop ; ++i) {
    Individual& individual = pop_[i];
    if (individual.HasFitness())
      continue;

    for (int j = 0 ; j < FLAGS_games ; ++j) {
      GameType* game = new GameType(individual);
      games << game;
      games_for_individual.insertMulti(i, game);
    }
  }

  if (games.isEmpty())
    return;

  // Run games
  QFuture<void> future = QtConcurrent::map(
      games, PointerMemberFunctionWrapper<void, GameType>(&GameType::Play));
  future.waitForFinished();

  // Update the fitness for each one
  foreach (int individual, games_for_individual.uniqueKeys()) {
    QList<quint64> scores;
    foreach (const GameType* game, games_for_individual.values(individual)) {
      scores << game->BlocksPlaced();
    }

    pop_[individual].SetFitness(scores);
  }

  qDeleteAll(games);
}

#endif // ENGINE_H
