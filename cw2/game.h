#ifndef GAME_H
#define GAME_H

#include <QSize>
#include <QThread>

#include "tetrisboard.h"
#include "tetramino.h"
#include "boardrating.h"

#include <limits>
#include <math.h>

#include <cuda_runtime.h>

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

  // Calculate the max size of search space
  const int size_1 = oc1 * W;
  const int size_2 = size_1 * oc2 * W;

  // The actual size of the search space will be smaller than this because
  // it assumes tetraminos with width=1.  Real tetraminos will be bigger,
  // therefore there will be W - tetramino.width possible places for them.
  // We keep the count of actual filled boards in here:
  int real_size = 0;

  // Allocate contiguous blocks of memory for our boards
  bool* const cells_1 = new bool[BoardType::CellsSize() * size_1];
  bool* const cells_2 = new bool[BoardType::CellsSize() * size_2];
  int* const highest_cell_1 = new int[BoardType::HighestCellSize() * size_1];
  int* const highest_cell_2 = new int[BoardType::HighestCellSize() * size_2];

  // We need to remember [ox]1 for our boards as well
  int2* const board_properties = new int2[size_2];

  // And the number of rows cleared
  int* const rows_cleared = new int[size_2];

  // We start by initialising the boards in our search space with
  // all possible combinations of tetramino placement and orientation
  bool* cells_1_p = cells_1;
  bool* cells_2_p = cells_2;
  int* highest_cell_1_p = highest_cell_1;
  int* highest_cell_2_p = highest_cell_2;
  int2* board_properties_p = board_properties;
  int* rows_cleared_p = rows_cleared;

  for (int o1=0 ; o1<oc1 ; ++o1) {
    for (int x1=0 ; x1<=W - tetramino1.Size(o1).width() ; ++x1) {
      BoardType board1(cells_1_p, highest_cell_1_p);
      board1.CopyFrom(board_);

      cells_1_p += BoardType::CellsSize();
      highest_cell_1_p += BoardType::HighestCellSize();

      // Add this first tetramino to the new board
      int y1 = board1.TetraminoHeight(tetramino1, x1, o1);
      if (y1 < 0)
        continue;

      board1.Add(tetramino1, x1, y1, o1);
      board1.ClearRows();

      for (int o2=0 ; o2<oc2 ; ++o2) {
        for (int x2=0 ; x2<=W - tetramino2.Size(o2).width() ; ++x2) {
          BoardType board2(cells_2_p, highest_cell_2_p);
          board2.CopyFrom(board1);

          // Add the second tetramino to the board
          int y2 = board1.TetraminoHeight(tetramino2, x2, o2);
          if (y2 < 0)
            continue;

          *board_properties_p = make_int2(o1, x1);

          cells_2_p += BoardType::CellsSize();
          highest_cell_2_p += BoardType::HighestCellSize();
          board_properties_p++;
          real_size ++;

          board2.Add(tetramino2, x2, y2, o2);
          *rows_cleared_p = board2.ClearRows();

          ++rows_cleared_p;
        }
      }
    }
  }

  if (real_size == 0) {
    // No possible places for the tetramino - end the game
    return false;
  }

  // Now we calculate the ratings for each possible board on the GPU
  // [output, output2, rows_cleared] for each board expands to the 5 board
  // rating numbers
  int4* output = new int4[size_2];
  int* output2 = new int[size_2];
  CudaFunctions::board_rating<W, H>(cells_2, highest_cell_2, real_size, output, output2);

  // We're done with the board data now
  delete[] cells_1;
  delete[] cells_2;
  delete[] highest_cell_1;
  delete[] highest_cell_2;

  // Find the best board.  This is the one with the lowest rating.
  double best_rating = std::numeric_limits<double>::max();
  int best_index = 0;

  for (int i=0 ; i<real_size ; ++i) {
    double rating = individual_.Rating(output[i].w, output[i].x, output[i].y,
                                       output2[i], output[i].z, rows_cleared[i]);
    if (rating < best_rating) {
      best_rating = rating;
      best_index = i;
    }
  }

  // Now we have the best board.  Add its tetramino to our board.
  int best_o1 = board_properties[best_index].x;
  int best_x1 = board_properties[best_index].y;
  int best_y1 = board_.TetraminoHeight(tetramino1, best_x1, best_o1);

  board_.Add(tetramino1, best_x1, best_y1, best_o1);
  board_.ClearRows();

  next_tetramino_ = tetramino2;

  // Cleanup
  delete[] output;
  delete[] output2;
  delete[] board_properties;
  delete[] rows_cleared;

  return true;
}

#endif // GAME_H
