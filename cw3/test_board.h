#ifndef TEST_BOARD_H
#define TEST_BOARD_H

#include <QObject>

#include "tetrisboard.h"

namespace Test {

class Board : public QObject {
  Q_OBJECT

 public:
  Board();

  typedef TetrisBoard<4, 4> BoardType;

 private slots:
  void init();
  void cleanup();

  void Size();
  void GetAndSet();
  void ClearRows();

  void PileHeight();
  void Holes();
  void AltitudeDifference();
  void WellDepth();

  void TetraminoHeight();

 private:
  BoardType* board_;
};

} // namespace Test

#endif // TEST_BOARD_H
