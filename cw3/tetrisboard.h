#ifndef TETRISBOARD_H
#define TETRISBOARD_H

#include <QSize>
#include <QtDebug>

#include "tetramino.h"

#include <algorithm>
#include <tr1/array>

struct BoardStats {
  int pile_height;
  int holes;
  int connected_holes;
  int altitude_difference;
  int max_well_depth;
  int sum_well_depth;
  int total_blocks;
  int weighted_blocks;
  int row_transitions;
  int column_transitions;
};

template <int W = 10, int H = 20>
class TetrisBoard {
 public:
  TetrisBoard() {}

  static const int kWidth;
  static const int kHeight;
  static QSize Size() { return QSize(W, H); }

  void Clear();
  void CopyFrom(const TetrisBoard& other);

  void Add(const Tetramino& tetramino, int x, int y, int orientation);
  int ClearRows();

  int TetraminoHeight(const Tetramino& tetramino, int x, int orientation) const;

  void Analyse(BoardStats* stats) const;

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

  std::tr1::array<bool, W*H> cells_;
  std::tr1::array<int, W> highest_cell_;
};

template <int W, int H>
const int TetrisBoard<W,H>::kWidth = W;

template <int W, int H>
const int TetrisBoard<W,H>::kHeight = H;

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
  std::fill(cells_.begin(), cells_.end(), false);
  std::fill(highest_cell_.begin(), highest_cell_.end(), H);

#ifndef QT_NO_DEBUG
  dirty_ = false;
#endif
}

template <int W, int H>
void TetrisBoard<W,H>::CopyFrom(const TetrisBoard& other) {
  std::copy(other.cells_.begin(), other.cells_.end(), cells_.begin());
  std::copy(other.highest_cell_.begin(), other.highest_cell_.end(), highest_cell_.begin());

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

  bool* row_start = cells_.begin();
  bool* row_end = row_start + W;

  // For each row...
  for ( ; row_start != cells_.end() ; row_start = row_end, row_end += W) {
    // Decide whether we need to clear the row
    if (std::find(row_start, row_end, false) != row_end)
      continue;

    // Move all the higher rows down one
    std::copy_backward(cells_.begin(), row_start, row_end);

    rows_cleared ++;
  }

  if (rows_cleared) {
    // Clear the new rows at the top
    std::fill(cells_.begin(), cells_.begin() + W*rows_cleared, false);

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
void TetrisBoard<W,H>::Analyse(BoardStats* stats) const {
  const_cast<TetrisBoard*>(this)->UpdateHighestCells();

  // Initialise the output variables
  stats->pile_height = H - *std::min_element(highest_cell_.begin(), highest_cell_.end());
  int holes = 0;
  int connected_holes = 0;
  int max_well_depth = 0;
  int sum_well_depth = 0;

  int max_pile_height = H - *std::max_element(highest_cell_.begin(), highest_cell_.end());

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

    sum_well_depth += qMax(0, well_depth);
    max_well_depth = qMax(max_well_depth, well_depth);

    // Start at the highest filled cell and go down.  Keep track of the cell
    // above us.
    bool cell_above = true;
    for (int y=highest_cell_[x]+1 ; y<H ; ++y) {
      const bool cell = Cell(x, y);
      if (!cell) {
        // We're in a hole
        ++ holes;

        // If the one above wasn't a hole as well then this is a new unique
        // connected hole
        if (cell_above) {
          ++ connected_holes;
        }
      }
      cell_above = cell;
    }
  }

  stats->holes = holes;
  stats->connected_holes = connected_holes;
  stats->max_well_depth = max_well_depth;
  stats->sum_well_depth = sum_well_depth;
  stats->altitude_difference = stats->pile_height - max_pile_height;
}

template <int W, int H>
int TetrisBoard<W,H>::TetraminoHeight(const Tetramino& tetramino,
                                      int x, int orientation) const {
  const_cast<TetrisBoard*>(this)->UpdateHighestCells();

  const QSize& size(tetramino.Size(orientation));

  // Work out where to start
  int y_start = *std::min_element(
      highest_cell_.begin() + x, highest_cell_.begin() + x + size.width()) - size.height();

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

    std::fill(highest_cell_.begin(), highest_cell_.end(), H);
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
  s.nospace() << "TetrisBoard(" << b.kWidth << "x" << b.kHeight << ")\n";

  for (int y=0 ; y<b.kHeight ; ++y) {
    QString row;
    row.sprintf("%2d  ", y);

    for (int x=0 ; x<b.kWidth ; ++x) {
      row += b(x, y) ? "X" : "_";
    }
    s.nospace() << row.toAscii().constData() << "\n";
  }

  return s.space();
}

#endif // TETRISBOARD_H
