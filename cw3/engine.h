#ifndef ENGINE_H
#define ENGINE_H

#include "population.h"
#include "game.h"
#include "individual.h"

#include <QtConcurrentMap>

#include <iostream>
#include <gflags/gflags.h>
#include <boost/bind.hpp>
#include <sys/time.h>

DEFINE_int32(ppop, 128, "number of individuals in the player population");
DEFINE_int32(spop, 128, "number of individuals in the block selector population");
DEFINE_int32(generations, 30, "number of generations to run for");
DEFINE_int32(threads, QThread::idealThreadCount(), "number of threads to use");
DECLARE_double(mrate);
DECLARE_double(mstddev);
DECLARE_double(emstddev);
DECLARE_double(dmstddev);

template <typename PlayerType, typename SelectorType, typename BoardType>
class Engine {
 public:
  Engine();

  typedef Game<PlayerType, SelectorType, BoardType> GameType;

  void Run();

 private:
  void UpdateFitness();
  static const PlayerType& FittestOf(const PlayerType& one, const PlayerType& two);

  Population<PlayerType> player_pop_;
  Population<SelectorType> selector_pop_;

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

template <typename PlayerType, typename SelectorType, typename BoardType>
Engine<PlayerType, SelectorType, BoardType>::Engine()
    : player_pop_(FLAGS_ppop),
      selector_pop_(FLAGS_spop)
{
}

template <typename PlayerType, typename SelectorType, typename BoardType>
const PlayerType& Engine<PlayerType, SelectorType, BoardType>::FittestOf(
    const PlayerType& one, const PlayerType& two) {
  return (one.Fitness() > two.Fitness()) ? one : two;
}

template <typename PlayerType, typename SelectorType, typename BoardType>
void Engine<PlayerType, SelectorType, BoardType>::Run() {
  player_pop_.InitRandom();
  selector_pop_.InitRandom();

  using std::cout;
  using std::endl;

  QThreadPool::globalInstance()->setMaxThreadCount(FLAGS_threads);

  cout << "# Population size (players) " << FLAGS_ppop << endl;
  cout << "# Population size (block selectors) " << FLAGS_spop << endl;
  cout << "# Board size " << BoardType::kWidth << "x" << BoardType::kHeight << endl;
  cout << "# Mutation std dev (weights) " << FLAGS_mstddev << endl;
  if (PlayerType::HasExponents())
    cout << "# Mutation std dev (exponents) " << FLAGS_emstddev << endl;
  if (PlayerType::HasDisplacements())
    cout << "# Mutation std dev (displacements) " << FLAGS_dmstddev << endl;
  cout << "# Mutation rate " << FLAGS_mrate << endl;
  cout << "# Generations " << FLAGS_generations << endl;
  cout << "# Threads " << FLAGS_threads << endl;
  cout << "# Board rating function " << PlayerType::NameOfAlgorithm() << endl;

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

  if (PlayerType::HasExponents())
    for (int i=0 ; i<Criteria_Count ; ++i)
      cout << "e" << i << "\t";

  if (PlayerType::HasDisplacements())
    for (int i=0 ; i<Criteria_Count ; ++i)
      cout << "d" << i << "\t";

  cout << "Time\t"
       << "sd-w";
  if (PlayerType::HasExponents())
    cout << "\tsd-e";
  if (PlayerType::HasDisplacements())
    cout << "\tsd-d";
  cout << endl;

  cout.precision(3);

  for (int generation_count=0 ; generation_count<FLAGS_generations ; ++generation_count) {
    timeval start_time, end_time;

    // Play games to get the fitness of new individuals
    gettimeofday(&start_time, NULL);
    UpdateFitness();
    gettimeofday(&end_time, NULL);

    uint64_t time_taken = (end_time.tv_sec - start_time.tv_sec) * 1000000 +
                           end_time.tv_usec - start_time.tv_usec;
    time_taken /= 1000; // msec

    // Show output
    cout << generation_count << "\t" <<
            player_pop_.Fittest().Fitness() << "\t" <<
            player_pop_.MeanFitness() << "\t" <<
            player_pop_.LeastFit().Fitness() << "\t";

    for (int i=0 ; i<Criteria_Count ; ++i)
      cout << player_pop_.Fittest().Weights()[i] << "\t";

    if (PlayerType::HasExponents())
      for (int i=0 ; i<Criteria_Count ; ++i)
        cout << player_pop_.Fittest().Exponents()[i] << "\t";

    if (PlayerType::HasDisplacements())
      for (int i=0 ; i<Criteria_Count ; ++i)
        cout << player_pop_.Fittest().Displacements()[i] << "\t";

    cout << time_taken << "\t" <<
        player_pop_.Diversity(boost::bind(&PlayerType::Weights, _1));

    if (PlayerType::HasExponents())
      cout << "\t" << player_pop_.Diversity(boost::bind(&PlayerType::Exponents, _1));
    if (PlayerType::HasDisplacements())
      cout << "\t" << player_pop_.Diversity(boost::bind(&PlayerType::Displacements, _1));
    cout << endl;

    // Make a new population
    Population<PlayerType> player_pop2(FLAGS_ppop);

    for (int i=0 ; i<FLAGS_ppop ; ++i) {
      // Pick two parents from the original population
      const PlayerType& parent1 = player_pop_.SelectFitnessProportionate();
      const PlayerType& parent2 = player_pop_.SelectFitnessProportionate(parent1);

      // Make a baby
      PlayerType child;
      child.Crossover(parent1, parent2);
      child.Mutate();

      // Put the child into the new population
      player_pop2.Replace(i, child);
    }

    // Use the new population
    player_pop_ = player_pop2;
  }
}

template <typename PlayerType, typename SelectorType, typename BoardType>
void Engine<PlayerType, SelectorType, BoardType>::UpdateFitness() {
  // Create games
  std::vector<GameType*> games;

  for (int i = 0 ; i < FLAGS_ppop ; ++i) {
    PlayerType& individual = player_pop_[i];
    if (individual.HasFitness())
      continue;

    GameType* game = new GameType(individual, selector_pop_[0]);
    games.push_back(game);
  }

  if (games.size() == 0)
    return;

  // Run games
  QFuture<void> future = QtConcurrent::map(
      games, PointerMemberFunctionWrapper<void, GameType>(&GameType::Play));
  future.waitForFinished();

  // Update the fitness for each one
  for (auto it = games.begin() ; it != games.end() ; ++it) {
    (*it)->GetPlayer().SetFitness((*it)->BlocksPlaced());
    delete *it;
  }
}

#endif // ENGINE_H
