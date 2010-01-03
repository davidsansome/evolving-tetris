#include <QApplication>
#include <QtDebug>
#include <QThreadPool>

#include <google/gflags.h>
#include <sys/time.h>

#include "engine.h"

DEFINE_string(algo, "l", "board rating function - l, e or ed");
DEFINE_string(size, "6x12", "board size");

#ifndef QT_NO_DEBUG
# include <QTest>
# include "test_board.h"
# include "test_tetramino.h"

  template <typename T>
  void RunTest(const QStringList& args) {
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

  QApplication a(argc, argv);

#ifndef QT_NO_DEBUG
  QStringList args(a.arguments());
  if (args.count() >= 2 && args[1] == "t") {
    args.removeAt(1);
    RunTest<Test::Board>(args);
    RunTest<Test::Tetramino>(args);

    return 0;
  }
#endif

  // Seed random number generator
  timeval tv;
  gettimeofday(&tv, NULL);
  qsrand(tv.tv_usec * tv.tv_sec);

  if      (FLAGS_size == "5x10") Run<5,10>();
  else if (FLAGS_size == "6x12") Run<6,12>();
  else if (FLAGS_size == "7x14") Run<7,14>();
  else if (FLAGS_size == "8x16") Run<8,16>();
  else if (FLAGS_size == "9x18") Run<9,18>();
  else if (FLAGS_size == "10x20") Run<10,20>();
  else {
    qFatal("Non-standard board size %s", FLAGS_size.c_str());
  }

  return 0;
}
