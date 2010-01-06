#include "engine.h"

#include <google/gflags.h>

DEFINE_string(algo, "l", "board rating function - l, e or ed");
DEFINE_string(size, "6x12", "board size");

#ifndef QT_NO_DEBUG
# include <QTest>
# include <QStringList>
# include "test_board.h"
# include "test_tetramino.h"

  template <typename T>
  void RunTest(const QStringList& args = QStringList()) {
    T* test = new T;
    int ret = QTest::qExec(test, args);
    if (ret)
      exit(ret);
    delete test;
  }
#endif

template <typename IndividualType, int W, int H>
void Run2() {
  Engine<IndividualType, TetrisBoard<W, H> > e;
  e.Run();
}

template <int W, int H>
void Run() {
  if (FLAGS_algo == "l")
    Run2<Individual<RatingAlgorithm_Linear>, W, H>();
  else if (FLAGS_algo == "e")
    Run2<Individual<RatingAlgorithm_Exponential>, W, H>();
  else if (FLAGS_algo == "ed")
    Run2<Individual<RatingAlgorithm_ExponentialWithDisplacement>, W, H>();
  else
    qFatal("Unknown algorithm %s", FLAGS_algo.c_str());
}

int main(int argc, char** argv) {
  std::string usage("COMP6026 tetris evolver.  Usage:\n  ");
  usage += argv[0] + std::string(" -size 6x12");
  google::SetUsageMessage(usage);
  google::ParseCommandLineFlags(&argc, &argv, true);

#ifndef QT_NO_DEBUG
  if (argc >= 2 && argsv[1] == "t") {
    RunTest<Test::Board>();
    RunTest<Test::Tetramino>();

    return 0;
  }
#endif

  // Seed random number generator
  srand(Utilities::RandomSeed());

  if      (FLAGS_size == "5x10") Run<5,10>();
  else if (FLAGS_size == "6x12") Run<6,12>();
  else if (FLAGS_size == "7x14") Run<7,14>();
  else if (FLAGS_size == "8x16") Run<8,16>();
  else if (FLAGS_size == "9x18") Run<9,18>();
  else if (FLAGS_size == "10x20") Run<10,20>();
  else {
    std::cerr << "Non-standard board size " << FLAGS_size;
    return 1;
  }

  return 0;
}
