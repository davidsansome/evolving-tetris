#ifndef TETRISBOARD_H
#define TETRISBOARD_H

#include <QSize>
#include <QtDebug>

class Tetramino;

class TetrisBoard {
 public:
  TetrisBoard(const QSize& board_size);
  ~TetrisBoard();

  void Clear();
  void CopyFrom(const TetrisBoard& other);

  void Add(const Tetramino& tetramino, int x, int y, int orientation);
  int ClearRows();

  QSize Size() const { return size_; }
  int Width() const { return size_.width(); }
  int Height() const { return size_.height(); }

  int TetraminoHeight(const Tetramino& tetramino, int x, int orientation) const;

  void Analyse(int* pile_height, int* holes, int* connected_holes,
               int* altitude_difference, int* max_well_depth) const;

  inline const bool& Cell(int x, int y) const;
  inline bool& Cell(int x, int y);
  inline const bool& operator()(int x, int y) const { return Cell(x, y); }
  inline bool& operator()(int x, int y) { return Cell(x, y); }

 private:
  TetrisBoard(const TetrisBoard&) {}
  void operator =(const TetrisBoard&) {}

  bool MaybeClearRow(int y);

  const QSize size_;
  bool* cells_;
};

QDebug operator<<(QDebug s, const TetrisBoard& b);

const bool& TetrisBoard::Cell(int x, int y) const {
  Q_ASSERT(x >= 0 && x < Width());
  Q_ASSERT(y >= 0 && y < Height());

  return cells_[y*Width() + x];
}

bool& TetrisBoard::Cell(int x, int y) {
  Q_ASSERT(x >= 0 && x < Width());
  Q_ASSERT(y >= 0 && y < Height());

  return cells_[y*Width() + x];
}

#endif // TETRISBOARD_H
