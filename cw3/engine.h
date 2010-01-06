#ifndef ENGINE_H
#define ENGINE_H

#include "population.h"
#include "game.h"
#include "individual.h"
#include "blockselector_sequence.h"
#include "blockselector_random.h"

#include <QtConcurrentMap>

#include <iostream>
#include <gflags/gflags.h>
#include <boost/bind.hpp>
#include <sys/time.h>

DEFINE_int32(pop, 128, "number of individuals in the population");
DEFINE_int32(generations, 30, "number of generations to run for");
DEFINE_int32(games, 32, "number of random games to compare each block selector against");
DEFINE_int32(threads, QThread::idealThreadCount(), "number of threads to use");
DECLARE_double(smrate);
DECLARE_double(pmrate);
DECLARE_double(pwmstddev);
DECLARE_double(pemstddev);
DECLARE_double(pdmstddev);

template <typename PlayerType, typename BoardType>
class Engine {
 public:
  Engine();

  typedef BlockSelector::Sequence<1000> SelectorType;
  typedef Game<PlayerType, SelectorType, BoardType> GameType;
  typedef Game<PlayerType, BlockSelector::Random, BoardType> RandomGameType;

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

template <typename PlayerType, typename BoardType>
Engine<PlayerType, BoardType>::Engine()
    : player_pop_(FLAGS_pop),
      selector_pop_(FLAGS_pop)
{
}

template <typename PlayerType, typename BoardType>
const PlayerType& Engine<PlayerType, BoardType>::FittestOf(
    const PlayerType& one, const PlayerType& two) {
  return (one.Fitness() > two.Fitness()) ? one : two;
}

template <typename PlayerType, typename BoardType>
void Engine<PlayerType, BoardType>::Run() {
  player_pop_.InitRandom();
  selector_pop_.InitRandom();

  using std::cout;
  using std::endl;

  QThreadPool::globalInstance()->setMaxThreadCount(FLAGS_threads);

  cout << "# Population size " << FLAGS_pop << endl;
  cout << "# Games " << FLAGS_games << endl;
  cout << "# Board size " << BoardType::kWidth << "x" << BoardType::kHeight << endl;
  cout << "# Mutation std dev (player weights) " << FLAGS_pwmstddev << endl;
  if (PlayerType::HasExponents())
    cout << "# Mutation std dev (player exponents) " << FLAGS_pemstddev << endl;
  if (PlayerType::HasDisplacements())
    cout << "# Mutation std dev (player displacements) " << FLAGS_pdmstddev << endl;
  cout << "# Mutation rate (player genes) " << FLAGS_pmrate << endl;
  cout << "# Mutation rate (block selector genes) " << FLAGS_smrate << endl;
  cout << "# Generations " << FLAGS_generations << endl;
  cout << "# Threads " << FLAGS_threads << endl;
  cout << "# Board rating function " << PlayerType::NameOfAlgorithm() << endl;

#ifndef QT_NO_DEBUG
  cout << "# Running in debug mode with assertions enabled" << endl;
#endif

  cout << endl;
  cout << "Gen\t"
          "Max-p\t"
          "Mean-p\t"
          "Min-p\t"
          "Max-s\t"
          "Mean-s\t"
          "Min-s\t";

  for (int i=0 ; i<Criteria_Count ; ++i)
    cout << "w" << i << "\t";

  if (PlayerType::HasExponents())
    for (int i=0 ; i<Criteria_Count ; ++i)
      cout << "e" << i << "\t";

  if (PlayerType::HasDisplacements())
    for (int i=0 ; i<Criteria_Count ; ++i)
      cout << "d" << i << "\t";

  cout << "Time\t"
       << "sd-w\t";
  if (PlayerType::HasExponents())
    cout << "sd-e\t";
  if (PlayerType::HasDisplacements())
    cout << "sd-d\t";
  cout << "sd-s";
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
            player_pop_.LeastFit().Fitness() << "\t" <<
            selector_pop_.Fittest().Fitness() << "\t" <<
            selector_pop_.MeanFitness() << "\t" <<
            selector_pop_.LeastFit().Fitness() << "\t";

    for (int i=0 ; i<Criteria_Count ; ++i)
      cout << player_pop_.Fittest().Weights()[i] << "\t";

    if (PlayerType::HasExponents())
      for (int i=0 ; i<Criteria_Count ; ++i)
        cout << player_pop_.Fittest().Exponents()[i] << "\t";

    if (PlayerType::HasDisplacements())
      for (int i=0 ; i<Criteria_Count ; ++i)
        cout << player_pop_.Fittest().Displacements()[i] << "\t";

    cout << time_taken << "\t" <<
        player_pop_.Diversity(boost::bind(&PlayerType::Weights, _1)) << "\t";

    if (PlayerType::HasExponents())
      cout << player_pop_.Diversity(boost::bind(&PlayerType::Exponents, _1)) << "\t";
    if (PlayerType::HasDisplacements())
      cout << player_pop_.Diversity(boost::bind(&PlayerType::Displacements, _1)) << "\t";
    cout << selector_pop_.Diversity(boost::bind(&SelectorType::GetSequence, _1));
    cout << endl;

    // Make new populations
    player_pop_.NextGeneration();
    selector_pop_.NextGeneration();
  }
}

template <typename PlayerType, typename BoardType>
void Engine<PlayerType, BoardType>::UpdateFitness() {
  // Create games
  std::vector<GameType*> games;

  for (int i = 0 ; i < FLAGS_pop ; ++i) {
    GameType* game = new GameType(player_pop_[i], selector_pop_[i]);
    games.push_back(game);
  }

  if (games.size() == 0)
    return;

  // Run games
  QFuture<void> future = QtConcurrent::map(
      games, PointerMemberFunctionWrapper<void, GameType>(&GameType::Play));
  future.waitForFinished();

  // Update the fitness for each player
  // And prepare more games for each player against random sequences
  std::vector<RandomGameType*> random_games;

  for (auto it = games.begin() ; it != games.end() ; ++it) {
    (*it)->GetPlayer().SetFitness((*it)->BlocksPlaced());

    for (int i=0 ; i<FLAGS_games ; ++i) {
      BlockSelector::Random* random_selector = new BlockSelector::Random;
      random_selector->SetSeed(rand());

      RandomGameType* random_game = new RandomGameType(
          (*it)->GetPlayer(), *random_selector);
      random_game->SetData(*it);

      random_games.push_back(random_game);
    }
  }

  // Run these random games
  future = QtConcurrent::map(
      random_games, PointerMemberFunctionWrapper<void, RandomGameType>(&RandomGameType::Play));
  future.waitForFinished();

  for (auto it = random_games.begin() ; it != random_games.end() ; ++it) {
    // Get the original game back out
    GameType* original_game = static_cast<GameType*>((*it)->Data());

    int64_t original_fitness = (*it)->GetPlayer().Fitness();
    int64_t random_fitness = (*it)->BlocksPlaced();
    int64_t diff = std::abs(original_fitness - random_fitness);

    original_game->GetBlockSelector().SetFitness(
        original_game->GetBlockSelector().Fitness() + diff);

    delete &((*it)->GetBlockSelector());
    delete (*it);
  }

  // Normalise the fitness of our sequences
  for (auto it = games.begin() ; it != games.end() ; ++it) {
    // The block selector's fitness now is the sum of all the fitnesses against
    // random sequences.  To find out how different this sequence was to the
    // random landscape we want to normalise for:
    //  a) The number of random games
    //  b) The original fitness
    // We also want to invert the score (since lower numbers were better).

    (*it)->GetBlockSelector().SetFitness(
        std::max(0.0, 2.0 - double((*it)->GetBlockSelector().Fitness()) /
            (FLAGS_games * (*it)->GetPlayer().Fitness())) * 1000);

    delete (*it);
  }
}

#endif // ENGINE_H
