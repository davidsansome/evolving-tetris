#include "individual.h"
#include "tetramino.h"

#include <limits>
#include <math.h>

Individual::Individual(const QSize& board_height)
    : board_(board_height)
{
  board_.Clear();

  // TODO
  for (int i=0 ; i<Criteria_Count ; ++i) {
    weights_ << 1;
  }
}

void Individual::Start() {
  board_.Clear();
  next_tetramino_ = Tetramino();

  int iterations = 0;
  while (Step()) {
    iterations ++;
    qDebug() << iterations << board_;
  }
}

bool Individual::Step() {
  // Pick the next two tetraminos
  Tetramino tetramino1(next_tetramino_);
  Tetramino tetramino2; // Random

  double best_score = std::numeric_limits<double>::max();

  // Best x position and orientation of first and second tetramino
  int best_x1, best_o1, best_x2, best_o2;

  for (int o1=0 ; o1<Tetramino::kOrientationCount ; ++o1) {
    int width1 = tetramino1.Size(o1).width();
    for (int x1=0 ; x1<=board_.Width() - width1 ; ++x1) {
      TetrisBoard board1(board_.Size());
      board1.CopyFrom(board_);

      // Add this first tetramino to the new board
      double score1 = Rating(board1, tetramino1, x1, o1);
      if (isnan(score1))
        continue;

      for (int o2=0 ; o2<Tetramino::kOrientationCount ; ++o2) {
        int width2 = tetramino2.Size(o2).width();
        for (int x2=0 ; x2<=board_.Width() - width2 ; ++x2) {
          TetrisBoard board2(board_.Size());
          board2.CopyFrom(board1);

          // Add the second tetramino to the board
          double score2 = Rating(board2, tetramino2, x2, o2);
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

double Individual::Rating(TetrisBoard& board, const Tetramino& tetramino,
                          int x, int orientation) const {
  int y = board.TetraminoHeight(tetramino, x, orientation);

  if (y == -1) {
    // We can't add the tetramino here
    return std::numeric_limits<double>::quiet_NaN();
  }

  // Add the tetramino to the board
  board.Add(tetramino, x, y, orientation);

  // Count the rows that were removed by adding this tetramino
  int removed_lines = board.ClearRows();

  int pile_height;
  int holes;
  int connected_holes;
  int altitude_difference;
  int max_well_depth;

  board.Analyse(&pile_height, &holes, &connected_holes,
                &altitude_difference, &max_well_depth);

  return
      weights_[PileHeight] * pile_height +
      weights_[Holes] * holes +
      weights_[ConnectedHoles] * connected_holes +
      weights_[RemovedLines] * removed_lines +
      weights_[AltitudeDifference] * altitude_difference +
      weights_[MaxWellDepth] * max_well_depth;
}
