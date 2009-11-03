#include "test_board.h"
#include "tetrisboard.h"
#include "tetramino.h"

#include <QTest>

namespace Test {

Board::Board()
    : board_(NULL)
{
}

void Board::init() {
  board_ = new TetrisBoard(QSize(4, 4));
  board_->Clear();
}

void Board::cleanup() {
  delete board_;
  board_ = NULL;
}

void Board::Size() {
  QCOMPARE(board_->Size(), QSize(4, 4));
  QCOMPARE(board_->Width(), 4);
  QCOMPARE(board_->Height(), 4);
}

void Board::GetAndSet() {
  // Check everything is clear
  for (int x=0 ; x<4 ; ++x) {
    for (int y=0 ; y<4 ; ++y) {
      QCOMPARE(board_->Cell(x, y), false);
      QCOMPARE((*board_)(x, y), false);
    }
  }

  // Set 2 cells
  board_->Cell(1,1) = true;
  (*board_)(2,2) = true;

  QCOMPARE(board_->Cell(1,1), true);
  QCOMPARE(board_->Cell(2,2), true);
  QCOMPARE(board_->Cell(3,3), false);

  // Clear
  board_->Clear();

  QCOMPARE(board_->Cell(1,1), false);
  QCOMPARE(board_->Cell(2,2), false);
}

void Board::ClearRows() {
  // XXXX
  // _X__
  // XXXX
  // __X_

  board_->Cell(0, 0) = true;
  board_->Cell(1, 0) = true;
  board_->Cell(2, 0) = true;
  board_->Cell(3, 0) = true;

  board_->Cell(1, 1) = true;

  board_->Cell(0, 2) = true;
  board_->Cell(1, 2) = true;
  board_->Cell(2, 2) = true;
  board_->Cell(3, 2) = true;

  board_->Cell(2, 3) = true;

  QCOMPARE(board_->ClearRows(), 2);

  // Check the board now looks like this:
  // ____
  // ____
  // _X__
  // __X_

  QCOMPARE(board_->Cell(0,0), false);
  QCOMPARE(board_->Cell(0,1), false);
  QCOMPARE(board_->Cell(0,2), false);
  QCOMPARE(board_->Cell(0,3), false);
  QCOMPARE(board_->Cell(1,2), true);
  QCOMPARE(board_->Cell(2,3), true);

  QCOMPARE(board_->ClearRows(), 0);
}

void Board::PileHeight() {
  // ____
  // _X__
  // _X_X
  // _X_X
  board_->Cell(1,1) = true;
  board_->Cell(1,2) = true;
  board_->Cell(1,3) = true;

  board_->Cell(3,2) = true;
  board_->Cell(3,3) = true;

  int pile_height, b, c, d, e;

  board_->Analyse(&pile_height, &b, &c, &d, &e);
  QCOMPARE(pile_height, 1);

  // Knock off the top one
  board_->Cell(1,1) = false;

  board_->Analyse(&pile_height, &b, &c, &d, &e);
  QCOMPARE(pile_height, 2);

  // Knock off another
  board_->Cell(1,2) = false;

  board_->Analyse(&pile_height, &b, &c, &d, &e);
  QCOMPARE(pile_height, 2);
}

void Board::Holes() {
  // _X__
  // ____
  // ____
  // __X_
  board_->Cell(1,0) = true;
  board_->Cell(2,3) = true;

  int a, holes, connected_holes, d, e;

  board_->Analyse(&a, &holes, &connected_holes, &d, &e);
  QCOMPARE(holes, 3);
  QCOMPARE(connected_holes, 1);

  // _X__
  // __X_
  // ____
  // __X_
  board_->Cell(2,1) = true;

  board_->Analyse(&a, &holes, &connected_holes, &d, &e);
  QCOMPARE(holes, 4);
  QCOMPARE(connected_holes, 2);
}

void Board::AltitudeDifference() {
  // ____
  // _X__
  // ____
  // X_XX
  board_->Cell(0,3) = true;
  board_->Cell(1,1) = true;
  board_->Cell(2,3) = true;
  board_->Cell(3,3) = true;

  int a, b, c, altitude_diff, e;

  board_->Analyse(&a, &b, &c, &altitude_diff, &e);
  QCOMPARE(altitude_diff, 2);

  // _X__
  // _X__
  // ____
  // X_XX
  board_->Cell(1,0) = true;

  board_->Analyse(&a, &b, &c, &altitude_diff, &e);
  QCOMPARE(altitude_diff, 3);

  // _X__
  // _X__
  // ____
  // ____
  board_->Cell(0,3) = false;
  board_->Cell(2,3) = false;
  board_->Cell(3,3) = false;

  board_->Analyse(&a, &b, &c, &altitude_diff, &e);
  QCOMPARE(altitude_diff, 4);
}

void Board::WellDepth() {
  // _XXX
  // ____
  // ____
  // ____
  board_->Cell(1,0) = true;
  board_->Cell(2,0) = true;
  board_->Cell(3,0) = true;

  int a, b, c, d, well_depth;

  board_->Analyse(&a, &b, &c, &d, &well_depth);
  QCOMPARE(well_depth, 4);

  // ____
  // X__X
  // X_XX
  // X_XX
  board_->Clear();
  board_->Cell(0,1) = true;
  board_->Cell(3,1) = true;
  board_->Cell(0,2) = true;
  board_->Cell(2,2) = true;
  board_->Cell(3,2) = true;
  board_->Cell(0,3) = true;
  board_->Cell(2,3) = true;
  board_->Cell(3,3) = true;

  board_->Analyse(&a, &b, &c, &d, &well_depth);
  QCOMPARE(well_depth, 2);

  // ____
  // X_XX
  // X_XX
  // X_XX
  board_->Cell(2,1) = true;

  board_->Analyse(&a, &b, &c, &d, &well_depth);
  QCOMPARE(well_depth, 3);
}

void Board::TetraminoHeight() {
  // Type 5 is:
  //  XXX
  //   X
  Tetramino tetramino(5);

  // 3x2 orientations
  QCOMPARE(board_->TetraminoHeight(tetramino, 0, 0), 2);
  QCOMPARE(board_->TetraminoHeight(tetramino, 0, 2), 2);

  // 2x3 orientations
  QCOMPARE(board_->TetraminoHeight(tetramino, 0, 1), 1);
  QCOMPARE(board_->TetraminoHeight(tetramino, 0, 3), 1);

  board_->Add(tetramino, 0, 2, 0);
  // ____
  // ____
  // _X__
  // XXX_

  // Type 3 is the square
  tetramino = Tetramino(3);

  QCOMPARE(board_->TetraminoHeight(tetramino, 0, 0), 0);
  QCOMPARE(board_->TetraminoHeight(tetramino, 1, 0), 0);
  QCOMPARE(board_->TetraminoHeight(tetramino, 2, 0), 1);

  board_->Add(tetramino, 1, 0, 0);
  // _XX_
  // _XX_
  // _X__
  // XXX_

  // Type 0 is the line
  tetramino = Tetramino(0);

  QCOMPARE(board_->TetraminoHeight(tetramino, 0, 0), -1);

  QCOMPARE(board_->TetraminoHeight(tetramino, 0, 1), -1);
  QCOMPARE(board_->TetraminoHeight(tetramino, 1, 1), -4);
  QCOMPARE(board_->TetraminoHeight(tetramino, 2, 1), -4);
  QCOMPARE(board_->TetraminoHeight(tetramino, 3, 1), 0);

  board_->Add(tetramino, 3, 0, 1);
  // _XXX
  // _XXX
  // _X_X
  // XXXX

  QCOMPARE(board_->ClearRows(), 1);
  // ____
  // _XXX
  // _XXX
  // _X_X

  QCOMPARE(board_->TetraminoHeight(tetramino, 0, 1), 0);
  QCOMPARE(board_->TetraminoHeight(tetramino, 1, 1), -3);
  QCOMPARE(board_->TetraminoHeight(tetramino, 2, 1), -3);
  QCOMPARE(board_->TetraminoHeight(tetramino, 3, 1), -3);

  board_->Add(tetramino, 0, 0, 1);
  // X___
  // XXXX
  // XXXX
  // XX_X

  QCOMPARE(board_->ClearRows(), 2);
  // ____
  // ____
  // X___
  // XX_X

  QCOMPARE(board_->TetraminoHeight(tetramino, 0, 1), -2);
  QCOMPARE(board_->TetraminoHeight(tetramino, 1, 1), -1);
  QCOMPARE(board_->TetraminoHeight(tetramino, 2, 1), 0);
  QCOMPARE(board_->TetraminoHeight(tetramino, 3, 1), -1);
}

} // namespace Test
