#ifndef TETRISBOARD_H
#define TETRISBOARD_H

#include <QSize>
#include <QtDebug>

#include "tetramino.h"

#include <algorithm>
#include <tr1/array>

template <int W = 10, int H = 20>
class TetrisBoard {
 public:
  TetrisBoard(bool* cells = NULL, int* highest_cell = NULL);
  ~TetrisBoard();

  static int CellsSize() { return W*H; }
  static int HighestCellSize() { return W; }

  void Clear();
  void CopyFrom(const TetrisBoard& other);

  void Add(const Tetramino& tetramino, int x, int y, int orientation);
  int ClearRows();

  QSize Size() const { return QSize(W, H); }
  int Width() const { return W; }
  int Height() const { return H; }

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
  TetrisBoard(const TetrisBoard&) {}
  void operator =(const TetrisBoard&) {}

  inline void SetCell(int x, int y, bool value);

#ifndef QT_NO_DEBUG
  bool dirty_;
  void UpdateHighestCells();
#else
  void UpdateHighestCells() { qt_noop(); }
#endif

  bool has_ownership_;
  bool* cells_;
  bool* cells_end_;
  int* highest_cell_;
  int* highest_cell_end_;
};

template <int W, int H>
TetrisBoard<W,H>::TetrisBoard(bool* cells, int* highest_cell)
    : has_ownership_(false),
      cells_(cells ? cells : new bool[CellsSize()]),
      cells_end_(cells_ + CellsSize()),
      highest_cell_(highest_cell ? highest_cell : new int[HighestCellSize()]),
      highest_cell_end_(highest_cell_ + HighestCellSize())
{
  if (!cells || !highest_cell)
    has_ownership_ = true;
}

template <int W, int H>
TetrisBoard<W,H>::~TetrisBoard() {
  if (has_ownership_) {
    delete[] cells_;
    delete[] highest_cell_;
  }
}

template <int W, int H>
const bool& TetrisBoard<W,H>::Cell(int x, int y) const {
  Q_ASSERT(x >= 0 && x < W);
  Q_ASSERT(y >= 0 && y < H);

  return cells_[y*W + x];
}

template <int W, int H>
inline void TetrisBoard<W,H>::SetCell(int x, int y, bool value) {
  Q_ASSERT(x >= 0 && x < W);
  Q_ASSERT(y >= 0 && y < H);

  cells_[y*W + x] = value;
}

#ifndef QT_NO_DEBUG
template <int W, int H>
bool& TetrisBoard<W,H>::Cell(int x, int y) {
  Q_ASSERT(x >= 0 && x < W);
  Q_ASSERT(y >= 0 && y < H);

  dirty_ = true;
  return cells_[y*W + x];
}
#endif

template <int W, int H>
void TetrisBoard<W,H>::Clear() {
  std::fill(cells_, cells_end_, false);
  std::fill(highest_cell_, highest_cell_end_, H);

#ifndef QT_NO_DEBUG
  dirty_ = false;
#endif
}

template <int W, int H>
void TetrisBoard<W,H>::CopyFrom(const TetrisBoard& other) {
  std::copy(other.cells_, other.cells_end_, cells_);
  std::copy(other.highest_cell_, other.highest_cell_end_, highest_cell_);

#ifndef QT_NO_DEBUG
  dirty_ = false;
#endif
}

template <int W, int H>
void TetrisBoard<W,H>::Add(const Tetramino& tetramino, int x, int y, int orientation) {
  Q_ASSERT(x + tetramino.Size(orientation).width() <= W);
  Q_ASSERT(y + tetramino.Size(orientation).height() <= H);
  Q_ASSERT(x >= 0 && y >= 0);

  const QPoint* point = tetramino.Points(orientation);
  for (int i=0 ; i<Tetramino::kPointsCount ; ++i) {
    const int px = x + point->x();
    const int py = y + point->y();

    Q_ASSERT(!Cell(px, py));

    SetCell(px, py, true);
    highest_cell_[px] = qMin(highest_cell_[px], py);

    point++;
  }
}

template <int W, int H>
int TetrisBoard<W,H>::ClearRows() {
  UpdateHighestCells();

  int rows_cleared = 0;

  bool* row_start = cells_;
  bool* row_end = row_start + W;

  // For each row...
  for ( ; row_start != cells_end_ ; row_start = row_end, row_end += W) {
    // Decide whether we need to clear the row
    if (std::find(row_start, row_end, false) != row_end)
      continue;

    // Move all the higher rows down one
    std::copy_backward(cells_, row_start, row_end);

    rows_cleared ++;
  }

  if (rows_cleared) {
    // Clear the new rows at the top
    std::fill(cells_, cells_ + W*rows_cleared, false);

    // Update highest_cell_
    for (int x=0 ; x<W ; ++x) {
      for (int y=highest_cell_[x]+rows_cleared ; y<=H ; ++y) {
        if (y == H || Cell(x, y)) {
          highest_cell_[x] = y;
          break;
        }
      }
    }
  }

  return rows_cleared;
}

template <int W, int H>
void TetrisBoard<W,H>::Analyse(int* pile_height, int* holes, int* connected_holes,
                               int* altitude_difference, int* max_well_depth) const {
  const_cast<TetrisBoard*>(this)->UpdateHighestCells();

  // Initialise the output variables
  *pile_height = H - *std::min_element(highest_cell_, highest_cell_end_);
  int my_holes = 0;
  int my_connected_holes = 0;
  int my_max_well_depth = 0;

  int max_pile_height = H - *std::max_element(highest_cell_, highest_cell_end_);

  // For each column...
  for (int x=0 ; x<W ; ++x) {
    int well_depth;

    // A well is a narrow 1-cell wide hole that is open from the top.
    // Special cases for the edges of the board.
    if (x == 0) {
      well_depth = highest_cell_[x] - highest_cell_[1];
    } else if (x == W-1) {
      well_depth = highest_cell_[x] - highest_cell_[x-1];
    } else {
      well_depth = highest_cell_[x] - qMax(highest_cell_[x-1], highest_cell_[x+1]);
    }

    my_max_well_depth = qMax(my_max_well_depth, well_depth);

    // Start at the highest filled cell and go down.  Keep track of the cell
    // above us.
    bool cell_above = true;
    for (int y=highest_cell_[x]+1 ; y<H ; ++y) {
      const bool cell = Cell(x, y);
      if (!cell) {
        // We're in a hole
        ++ my_holes;

        // If the one above wasn't a hole as well then this is a new unique
        // connected hole
        if (cell_above) {
          ++ my_connected_holes;
        }
      }
      cell_above = cell;
    }
  }

  *holes = my_holes;
  *connected_holes = my_connected_holes;
  *max_well_depth = my_max_well_depth;
  *altitude_difference = *pile_height - max_pile_height;
}

template <int W, int H>
int TetrisBoard<W,H>::TetraminoHeight(const Tetramino& tetramino,
                                      int x, int orientation) const {
  const_cast<TetrisBoard*>(this)->UpdateHighestCells();

  const QSize& size(tetramino.Size(orientation));

  // Work out where to start
  int y_start = *std::min_element(
      highest_cell_ + x, highest_cell_ + x + size.width()) - size.height();

  if (y_start < 0)
    return y_start;

  // "Drop" the tetramino
  for (int y=y_start ; y<=H - size.height() ; ++y) {
    // Check to see if any of the points on the tetramino at this position are occupied
    const QPoint* point = tetramino.Points(orientation);
    for (int i=0 ; i<Tetramino::kPointsCount ; ++i) {
      // If any point is occupied, return the previous y coord
      if (Cell(x + point->x(), y + point->y())) {
        return y - 1;
      }
      point++;
    }
  }
  return H - size.height();
}

#ifndef QT_NO_DEBUG
  template <int W, int H>
  void TetrisBoard<W,H>::UpdateHighestCells() {
    if (!dirty_)
      return;

    std::fill(highest_cell_, highest_cell_end_, H);
    for (int x=0 ; x<W ; ++x) {
      for (int y=0 ; y<H ; ++y) {
        if (Cell(x, y)) {
          highest_cell_[x] = y;
          break;
        }
      }
    }

    dirty_ = false;
  }
#endif

template <int W, int H>
QDebug operator<<(QDebug s, const TetrisBoard<W,H>& b) {
  s.nospace() << "TetrisBoard(" << b.Width() << "x" << b.Height() << ")\n";

  for (int y=0 ; y<b.Height() ; ++y) {
    QString row;
    for (int x=0 ; x<b.Width() ; ++x) {
      row += b(x, y) ? "X" : "_";
    }
    s.nospace() << y << "  " << row.toAscii().constData() << "\n";
  }

  return s.space();
}

#endif // TETRISBOARD_H
