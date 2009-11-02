#ifndef GAME_H
#define GAME_H

#include <QSize>

#include "tetrisboard.h"
#include "tetramino.h"

class Individual;

class Game {
 public:
  Game(Individual& individual, const QSize& board_size);

  void Play();

  Individual& GetIndividual() const { return individual_; }
  quint64 BlocksPlaced() const { return blocks_placed_; }

 private:
  bool Step();

  Individual& individual_;

  TetrisBoard board_;
  Tetramino next_tetramino_;

  quint64 blocks_placed_;
};

#endif // GAME_H
