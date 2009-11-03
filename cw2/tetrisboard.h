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

  QSize Size() const { return QSize(width_, height_); }
  int Width() const { return width_; }
  int Height() const { return height_; }

  int TetraminoHeight(const Tetramino& tetramino, int x, int orientation) const;

  void Analyse(int* pile_height, int* holes, int* connected_holes,
               int* altitude_difference, int* max_well_depth) const;

  inline const bool& Cell(int x, int y) const;
  inline const bool& operator()(int x, int y) const { return Cell(x, y); }

#ifndef QT_NO_DEBUG
  // Only for unit tests that need to set cells explicitly
  inline bool& Cell(int x, int y);
  inline bool& operator()(int x, int y) { return Cell(x, y); }
#endif

 private:
  TetrisBoard(const TetrisBoard&) : width_(0), height_(0) {}
  void operator =(const TetrisBoard&) {}

  inline void SetCell(int x, int y, bool value);

#ifndef QT_NO_DEBUG
  bool dirty_;
  void UpdateHighestCells();
#else
  void UpdateHighestCells() { qt_noop(); }
#endif

  const int width_;
  const int height_;
  bool* cells_;
  bool* cells_end_;
  int* highest_cell_;
  int* highest_cell_end_;
};

QDebug operator<<(QDebug s, const TetrisBoard& b);

const bool& TetrisBoard::Cell(int x, int y) const {
  Q_ASSERT(x >= 0 && x < Width());
  Q_ASSERT(y >= 0 && y < Height());

  return cells_[y*Width() + x];
}

inline void TetrisBoard::SetCell(int x, int y, bool value) {
  Q_ASSERT(x >= 0 && x < Width());
  Q_ASSERT(y >= 0 && y < Height());

  cells_[y*Width() + x] = value;
}

#ifndef QT_NO_DEBUG
bool& TetrisBoard::Cell(int x, int y) {
  Q_ASSERT(x >= 0 && x < Width());
  Q_ASSERT(y >= 0 && y < Height());

  dirty_ = true;
  return cells_[y*Width() + x];
}
#endif

#endif // TETRISBOARD_H
