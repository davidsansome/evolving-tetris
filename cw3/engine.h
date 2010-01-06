#ifndef ENGINE_H
#define ENGINE_H

#include "population.h"
#include "game.h"
#include "individual.h"
#include "blockselector_sequence.h"
#include "blockselector_random.h"
#include "gamemapper.h"

#include <QtConcurrentMap>

#include <iostream>
#include <gflags/gflags.h>
#include <boost/bind.hpp>
#include <sys/time.h>

DEFINE_int32(pop, 128, "number of individuals in the population");
DEFINE_int32(generations, 30, "number of generations to run for");
DEFINE_int32(games, 32, "number of random games to compare each block selector against");
DEFINE_int32(threads, QThread::idealThreadCount(), "number of threads to use");

DEFINE_bool(dumpseq, false, "dump the best tetramino sequence after each generation");
DEFINE_bool(watchseq, false, "watch the best tetramino sequence after each generation");
DEFINE_int32(watchseqdelay, 10, "delay between each move in milliseconds");

DECLARE_double(smrate);
DECLARE_double(pmrate);
DECLARE_double(pwmstddev);
DECLARE_double(pemstddev);
DECLARE_double(pdmstddev);

template <typename PlayerType, typename BoardType>
class Engine {
 public:
  Engine();

  typedef BlockSelector::Sequence<> SelectorType;
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
  std::vector<Messages::GameRequest> requests;

  for (int i = 0 ; i < FLAGS_pop ; ++i) {
    Messages::GameRequest req;
    req.set_player_id(i);
    req.set_selector_id(i);
    player_pop_[i].ToMessage(req.mutable_player());
    selector_pop_[i].ToMessage(req.mutable_selector_sequence());
    BoardType::ToMessage(req.mutable_board());

    requests.push_back(req);
  }

  if (requests.size() == 0)
    return;

  // Run games
  QFuture<Messages::GameResponse> future = QtConcurrent::mapped(
      requests, boost::bind(&GameMapper::Map, _1));
  future.waitForFinished();
  QList<Messages::GameResponse> responses = future.results();

  // Update the fitness for each player
  // And prepare more games for each player against random sequences
  requests.clear();

  for (auto it = responses.begin() ; it != responses.end() ; ++it) {
    const Messages::GameResponse& resp = *it;

    player_pop_[resp.player_id()].SetFitness(resp.blocks_placed());

    for (int i=0 ; i<FLAGS_games ; ++i) {
      Messages::GameRequest req;
      req.set_player_id(resp.player_id());
      req.set_selector_id(resp.selector_id());
      player_pop_[resp.player_id()].ToMessage(req.mutable_player());
      req.mutable_selector_random();
      BoardType::ToMessage(req.mutable_board());

      requests.push_back(req);
    }
  }

  // Run these random games
  future = QtConcurrent::mapped(
      requests, boost::bind(&GameMapper::Map, _1));
  future.waitForFinished();
  responses = future.results();

  for (auto it = responses.begin() ; it != responses.end() ; ++it) {
    const Messages::GameResponse& resp = *it;

    int64_t original_fitness = player_pop_[resp.player_id()].Fitness();
    int64_t random_fitness = resp.blocks_placed();
    int64_t diff = std::abs(original_fitness - random_fitness);

    selector_pop_[resp.selector_id()].SetFitness(
        selector_pop_[resp.selector_id()].Fitness() + diff);
  }

  // Normalise the fitness of our sequences
  for (int i = 0 ; i < FLAGS_pop ; ++i) {
    // The block selector's fitness now is the sum of all the fitnesses against
    // random sequences.  To find out how different this sequence was to the
    // random landscape we want to normalise for:
    //  a) The number of random games
    //  b) The original fitness
    // We also want to invert the score (since lower numbers were better).
    SelectorType& selector = selector_pop_[i];

    selector.SetFitness(
        std::max(0.0, 2.0 - double(selector.Fitness()) /
            (FLAGS_games * player_pop_[i].Fitness())) * 1000);
  }

  if (FLAGS_dumpseq || FLAGS_watchseq) {
    // Hack:
    int best_index = -1;
    uint64_t best_fitness = 0;
    for (int i = 0 ; i < FLAGS_pop ; ++i) {
      if (player_pop_[i].Fitness() > best_fitness) {
        best_fitness = player_pop_[i].Fitness();
        best_index = i;
      }
    }

    if (FLAGS_watchseq) {
      GameType game(player_pop_[best_index], selector_pop_[best_index]);
      game.SetWatchDelay(FLAGS_watchseqdelay);
      game.Play();
      std::cerr << game.BlocksPlaced() << "," << best_fitness << std::endl;
    }
    if (FLAGS_dumpseq) {
      auto seq = selector_pop_[best_index].GetSequence();
      for (uint64_t i=0 ; i<best_fitness ; ++i) {
        std::cerr << int(seq[i]);
      }
      std::cerr << std::endl;
    }
  }

}

#endif // ENGINE_H
