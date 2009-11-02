#ifndef TEST_BOARD_H
#define TEST_BOARD_H

#include <QObject>

class TetrisBoard;

namespace Test {

class Board : public QObject {
  Q_OBJECT

 public:
  Board();

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

 private:
  TetrisBoard* board_;
};

} // namespace Test

#endif // TEST_BOARD_H
