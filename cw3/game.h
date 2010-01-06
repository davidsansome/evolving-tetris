#ifndef GAME_H
#define GAME_H

#include "tetrisboard.h"
#include "tetramino.h"

#include <limits>
#include <math.h>

template <typename PlayerType, typename SelectorType, typename BoardType>
class Game {
 public:
  Game(PlayerType& player, SelectorType& selector);

  PlayerType& GetPlayer() const { return player_; }
  SelectorType& GetBlockSelector() const { return block_selector_; }
  const BoardType& GetBoard() const { return board_; }

  void SetData(void* data) { data_ = data; }
  void* Data() const { return data_; }

  // Plays a game of tetris, finishing when there's no room for any more blocks
  void Play();

  // The number of blocks that we managed to place.  The more the better
  uint64_t BlocksPlaced() const { return blocks_placed_; }

 private:
  bool Step();

  PlayerType& player_;
  SelectorType& block_selector_;
  void* data_;

  BoardType board_;
  Tetramino next_tetramino_;

  uint64_t blocks_placed_;
};

#include "individual.h"

template <typename PlayerType, typename SelectorType, typename BoardType>
Game<PlayerType, SelectorType, BoardType>::Game(PlayerType& player,
                                                SelectorType& block_selector)
    : player_(player),
      block_selector_(block_selector),
      data_(NULL),
      blocks_placed_(0)
{
  board_.Clear();
}

template <typename PlayerType, typename SelectorType, typename BoardType>
void Game<PlayerType, SelectorType, BoardType>::Play() {
  block_selector_.Reset();
  board_.Clear();

  next_tetramino_.InitFrom(block_selector_());

  blocks_placed_ = 0;
  while (Step()) {
    blocks_placed_ ++;
  }
}

template <typename PlayerType, typename SelectorType, typename BoardType>
bool Game<PlayerType, SelectorType, BoardType>::Step() {
  // Pick the next two tetraminos
  Tetramino tetramino1;
  Tetramino tetramino2;
  tetramino1.InitFrom(next_tetramino_);
  tetramino2.InitFrom(block_selector_());

  const int oc1 = tetramino1.OrientationCount();
  const int oc2 = tetramino2.OrientationCount();

  double best_score = std::numeric_limits<double>::max();

  // Best x position and orientation of first and second tetramino
  int best_x1 = -1;
  int best_o1 = -1;
  int best_x2 = -1;
  int best_o2 = -1;

  for (int o1=0 ; o1<oc1 ; ++o1) {
    int width1 = tetramino1.Size(o1).width();
    for (int x1=0 ; x1<=BoardType::kWidth - width1 ; ++x1) {
      BoardType board1;
      board1.CopyFrom(board_);

      // Add this first tetramino to the new board
      double score1 = player_.Rating(board1, tetramino1, x1, o1);
      if (isnan(score1))
        continue;

      for (int o2=0 ; o2<oc2 ; ++o2) {
        int width2 = tetramino2.Size(o2).width();
        for (int x2=0 ; x2<=BoardType::kWidth - width2 ; ++x2) {
          BoardType board2;
          board2.CopyFrom(board1);

          // Add the second tetramino to the board
          double score2 = player_.Rating(board2, tetramino2, x2, o2);
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

  next_tetramino_.InitFrom(tetramino2);

  return true;
}

#endif // GAME_H
