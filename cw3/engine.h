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
DECLARE_double(emstddev);
DECLARE_double(dmstddev);

template <typename IndividualType, typename BoardType>
class Engine {
 public:
  Engine();

  typedef Game<IndividualType, BoardType> GameType;

  void Run();

 private:
  void UpdateFitness();
  static const IndividualType& FittestOf(const IndividualType& one, const IndividualType& two);

  Population<IndividualType> pop_;

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

template <typename IndividualType, typename BoardType>
Engine<IndividualType, BoardType>::Engine()
    : pop_(FLAGS_pop)
{
}

template <typename IndividualType, typename BoardType>
const IndividualType& Engine<IndividualType, BoardType>::FittestOf(const IndividualType& one, const IndividualType& two) {
  return (one.Fitness() > two.Fitness()) ? one : two;
}

template <typename IndividualType, typename BoardType>
void Engine<IndividualType, BoardType>::Run() {
  pop_.InitRandom();

  using std::cout;
  using std::endl;

  QThreadPool::globalInstance()->setMaxThreadCount(FLAGS_threads);

  cout << "# Population size " << FLAGS_pop << endl;
  cout << "# Games " << FLAGS_games << endl;
  cout << "# Board size " << BoardType::kWidth << "x" << BoardType::kHeight << endl;
  cout << "# Mutation std dev (weights) " << FLAGS_mstddev << endl;
  if (IndividualType::HasExponents())
    cout << "# Mutation std dev (exponents) " << FLAGS_emstddev << endl;
  if (IndividualType::HasDisplacements())
    cout << "# Mutation std dev (displacements) " << FLAGS_dmstddev << endl;
  cout << "# Mutation rate " << FLAGS_mrate << endl;
  cout << "# Generations " << FLAGS_generations << endl;
  cout << "# Threads " << FLAGS_threads << endl;
  cout << "# Board rating function " << IndividualType::NameOfAlgorithm() << endl;

#ifndef QT_NO_DEBUG
  cout << "# Running in debug mode with assertions enabled" << endl;
#endif

  cout << endl;
  cout << "Gen\t"
          "Max\t"
          "Mean\t"
          "Min\t";

  for (int i=0 ; i<Criteria_Count ; ++i)
    cout << "w" << i << "\t";

  if (IndividualType::HasExponents())
    for (int i=0 ; i<Criteria_Count ; ++i)
      cout << "e" << i << "\t";

  if (IndividualType::HasDisplacements())
    for (int i=0 ; i<Criteria_Count ; ++i)
      cout << "d" << i << "\t";

  cout << "Time\t" <<
          "Diversity" << endl;

  cout.precision(3);

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
            pop_.LeastFit().Fitness() << "\t";

    for (int i=0 ; i<Criteria_Count ; ++i)
      cout << pop_.Fittest().Weights()[i] << "\t";

    if (IndividualType::HasExponents())
      for (int i=0 ; i<Criteria_Count ; ++i)
        cout << pop_.Fittest().Exponents()[i] << "\t";

    if (IndividualType::HasDisplacements())
      for (int i=0 ; i<Criteria_Count ; ++i)
        cout << pop_.Fittest().Displacements()[i] << "\t";

    cout << time_taken << "\t" <<
            pop_.Diversity() << endl;

    // Make a new population
    Population<IndividualType> pop2(FLAGS_pop);

    for (int i=0 ; i<FLAGS_pop ; ++i) {
      // Pick two parents from the original population
      const IndividualType& parent1 = pop_.SelectFitnessProportionate();
      const IndividualType& parent2 = pop_.SelectFitnessProportionate(parent1);

      // Make a baby
      IndividualType child;
      child.Crossover(parent1, parent2);
      child.Mutate();

      // Put the child into the new population
      pop2.Replace(i, child);
    }

    // Use the new population
    pop_ = pop2;
  }
}

template <typename IndividualType, typename BoardType>
void Engine<IndividualType, BoardType>::UpdateFitness() {
  // Create games
  QList<GameType*> games;
  QMap<int, GameType*> games_for_individual;

  for (int i = 0 ; i < FLAGS_pop ; ++i) {
    IndividualType& individual = pop_[i];
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
