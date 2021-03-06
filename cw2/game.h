#ifndef GAME_H
#define GAME_H

#include <QSize>
#include <QThread>

#include "tetrisboard.h"
#include "tetramino.h"

#include <limits>
#include <math.h>

class Individual;

template <int W, int H>
class Game {
 public:
  Game(Individual& individual);

  typedef TetrisBoard<W, H> BoardType;

  Individual& GetIndividual() const { return individual_; }
  BoardType& GetBoard() const { return board_; }

  // Plays a game of tetris, finishing when there's no room for any more blocks
  void Play();

  // The number of blocks that we managed to place.  The more the better
  quint64 BlocksPlaced() const { return blocks_placed_; }

 private:
  bool Step();

  Individual& individual_;

  BoardType board_;
  Tetramino next_tetramino_;

  quint64 blocks_placed_;
};

#include "individual.h"

template <int W, int H>
Game<W,H>::Game(Individual& individual)
    : individual_(individual),
      blocks_placed_(0)
{
  board_.Clear();
}

template <int W, int H>
void Game<W,H>::Play() {
  // Each game is played in its own thread
  qsrand(time(NULL) + QThread::currentThreadId());

  board_.Clear();
  next_tetramino_ = Tetramino();

  blocks_placed_ = 0;
  while (Step()) {
    blocks_placed_ ++;
  }
}

template <int W, int H>
bool Game<W, H>::Step() {
  // Pick the next two tetraminos
  Tetramino tetramino1(next_tetramino_);
  Tetramino tetramino2; // Random

  const int oc1 = tetramino1.OrientationCount();
  const int oc2 = tetramino2.OrientationCount();

  double best_score = std::numeric_limits<double>::max();

  // Best x position and orientation of first and second tetramino
  int best_x1, best_o1, best_x2, best_o2;

  for (int o1=0 ; o1<oc1 ; ++o1) {
    int width1 = tetramino1.Size(o1).width();
    for (int x1=0 ; x1<=W - width1 ; ++x1) {
      BoardType board1;
      board1.CopyFrom(board_);

      // Add this first tetramino to the new board
      double score1 = individual_.Rating(board1, tetramino1, x1, o1);
      if (isnan(score1))
        continue;

      for (int o2=0 ; o2<oc2 ; ++o2) {
        int width2 = tetramino2.Size(o2).width();
        for (int x2=0 ; x2<=W - width2 ; ++x2) {
          BoardType board2;
          board2.CopyFrom(board1);

          // Add the second tetramino to the board
          double score2 = individual_.Rating(board2, tetramino2, x2, o2);
          if (isnan(score2))
            continue;

          // Was this combination better than before?
          if (score1 + score2 < best_score) {
            best_score = score1 + score2;
            best_x1 = x1;
            best_x2 = x2;
            best_o1 = o1;
            best_o2 = o2;
          }
        }
      }
    }
  }

  if (best_score == std::numeric_limits<double>::max())
    return false;

  // Apply the best first move to the board
  int best_y1 = board_.TetraminoHeight(tetramino1, best_x1, best_o1);
  board_.Add(tetramino1, best_x1, best_y1, best_o1);
  board_.ClearRows();

  next_tetramino_ = tetramino2;

  return true;
}

#endif // GAME_H
