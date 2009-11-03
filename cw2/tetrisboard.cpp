#include "tetrisboard.h"
#include "tetramino.h"

#include <algorithm>

TetrisBoard::TetrisBoard(const QSize& size)
    : width_(size.width()),
      height_(size.height()),
      cells_(new bool[width_ * height_]),
      cells_end_(cells_ + width_ * height_),
      highest_cell_(new int[width_]),
      highest_cell_end_(highest_cell_ + width_)
{
}

TetrisBoard::~TetrisBoard() {
  delete[] cells_;
  delete[] highest_cell_;
}

void TetrisBoard::Clear() {
  std::fill(cells_, cells_end_, false);
  std::fill(highest_cell_, highest_cell_end_, height_);

#ifndef QT_NO_DEBUG
  dirty_ = false;
#endif
}

void TetrisBoard::CopyFrom(const TetrisBoard& other) {
  Q_ASSERT(width_ == other.width_);
  Q_ASSERT(height_ == other.height_);

  std::copy(other.cells_, other.cells_end_, cells_);
  std::copy(other.highest_cell_, other.highest_cell_end_, highest_cell_);

#ifndef QT_NO_DEBUG
  dirty_ = false;
#endif
}

void TetrisBoard::Add(const Tetramino& tetramino, int x, int y, int orientation) {
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

int TetrisBoard::ClearRows() {
  UpdateHighestCells();

  int rows_cleared = 0;

  bool* row_start = cells_;
  bool* row_end = row_start + width_;

  for ( ; row_start != cells_end_ ; row_start = row_end, row_end += width_) {
    // Decide whether we need to clear the row
    if (std::find(row_start, row_end, false) != row_end)
      continue;

    // Move all the higher rows down one
    std::copy_backward(cells_, row_start, row_end);

    rows_cleared ++;
  }

  if (rows_cleared) {
    // Clear the new rows at the top
    std::fill(cells_, cells_ + width_*rows_cleared, false);

    // Update highest_cell_
    for (int x=0 ; x<width_ ; ++x) {
      for (int y=highest_cell_[x]+rows_cleared ; y<=height_ ; ++y) {
        if (y == height_ || Cell(x, y)) {
          highest_cell_[x] = y;
          break;
        }
      }
    }
  }

  return rows_cleared;
}

void TetrisBoard::Analyse(int* pile_height, int* holes, int* connected_holes,
                          int* altitude_difference, int* max_well_depth) const {
  const_cast<TetrisBoard*>(this)->UpdateHighestCells();

  *pile_height = *std::min_element(highest_cell_, highest_cell_end_);
  *holes = 0;
  *connected_holes = 0;
  *altitude_difference = 0;
  *max_well_depth = 0;

  int max_pile_height = *std::max_element(highest_cell_, highest_cell_end_);

  for (int x=0 ; x<width_ ; ++x) {
    int well_depth;

    if (x == 0) {
      well_depth = highest_cell_[x] - highest_cell_[1];
    } else if (x == width_-1) {
      well_depth = highest_cell_[x] - highest_cell_[x-1];
    } else {
      well_depth = highest_cell_[x] - qMax(highest_cell_[x-1], highest_cell_[x+1]);
    }

    *max_well_depth = qMax(*max_well_depth, well_depth);

    bool cell_above = true;
    for (int y=highest_cell_[x]+1 ; y<height_ ; ++y) {
      bool cell = Cell(x, y);
      if (!cell) {
        // We're in a hole
        (*holes) ++;

        // If the one above wasn't a hole as well then this is a new unique
        // connected hole
        if (cell_above) {
          (*connected_holes) ++;
        }
      }
      cell_above = cell;
    }
  }

  *altitude_difference = max_pile_height - *pile_height;
}

int TetrisBoard::TetraminoHeight(const Tetramino& tetramino,
                                 int x, int orientation) const {
  const_cast<TetrisBoard*>(this)->UpdateHighestCells();

  const QSize& size(tetramino.Size(orientation));

  // Work out where to start
  int y_start = *std::min_element(
      highest_cell_ + x, highest_cell_ + x + size.width()) - size.height();

  if (y_start < 0)
    return y_start;

  // "Drop" the tetramino
  for (int y=y_start ; y<=height_ - size.height() ; ++y) {
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
  return height_ - size.height();
}

#ifndef QT_NO_DEBUG
  void TetrisBoard::UpdateHighestCells() {
    if (!dirty_)
      return;

    std::fill(highest_cell_, highest_cell_end_, height_);
    for (int x=0 ; x<width_ ; ++x) {
      for (int y=0 ; y<height_ ; ++y) {
        if (Cell(x, y)) {
          highest_cell_[x] = y;
          break;
        }
      }
    }

    dirty_ = false;
  }
#endif

QDebug operator<<(QDebug s, const TetrisBoard& b) {
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

