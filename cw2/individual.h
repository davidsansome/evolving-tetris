#ifndef INDIVIDUAL_H
#define INDIVIDUAL_H

#include <QList>
#include <QSize>

#include "tetrisboard.h"
#include "tetramino.h"

class Individual {
 public:
  Individual(const QSize& board_size);

  enum Algorithm {
    Linear,
    Exponential,
    ExponentialWithDisplacement
  };

  enum Criteria {
    PileHeight = 0,
    Holes,
    ConnectedHoles,
    RemovedLines,
    AltitudeDifference,
    MaxWellDepth,

    Criteria_Count
  };

  void Start();

  const TetrisBoard& Board() const { return board_; }

 private:
  bool Step();

  double Rating(TetrisBoard& board, const Tetramino& tetramino,
                int x, int orientation) const;
  // TODO: Move to TetrisBoard
  int TetraminoHeight(const TetrisBoard& board,
                      const Tetramino& tetramino, int x, int orientation) const;

  QList<int> weights_;
  QList<double> exponents_;
  QList<double> displacements_;

  TetrisBoard board_;
  Tetramino next_tetramino_;
};

#endif // INDIVIDUAL_H
