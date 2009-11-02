#include "tetrisboard.h"
#include "tetramino.h"

#include <algorithm>

TetrisBoard::TetrisBoard(const QSize& size)
    : size_(size),
      cells_(new bool[size.width() * size.height()])
{
}

TetrisBoard::~TetrisBoard() {
  delete[] cells_;
}

void TetrisBoard::Clear() {
  std::fill(cells_, cells_ + Width()*Height(), false);
}

void TetrisBoard::CopyFrom(const TetrisBoard& other) {
  Q_ASSERT(size_ == other.size_);

  std::copy(other.cells_, other.cells_ + Width()*Height(), cells_);
}

void TetrisBoard::Add(const Tetramino& tetramino, int x, int y, int orientation) {
  const QPoint* point = tetramino.Points(orientation);
  for (int i=0 ; i<Tetramino::kPointsCount ; ++i) {
    Q_ASSERT(!Cell(x + point->x(), y + point->y()));

    Cell(x + point->x(), y + point->y()) = true;
    point++;
  }
}

int TetrisBoard::ClearRows() {
  int ret = 0;
  for (int y=0 ; y<Height() ; ++y) {
    ret += MaybeClearRow(y) ? 1 : 0;
  }
  return ret;
}

bool TetrisBoard::MaybeClearRow(int y) {
  for (int x=0 ; x<Width() ; ++x) {
    if (!Cell(x, y))
      return false;
  }

  // Move all the higher rows down one
  for (int y2=y-1 ; y2>=0 ; --y2) {
    for (int x=0 ; x<Width() ; ++x) {
      Cell(x, y2+1) = Cell(x, y2);
    }
  }

  // Fill the top row with empties
  for (int x=0 ; x<Width() ; ++x) {
    Cell(x, 0) = false;
  }

  return true;
}

void TetrisBoard::Analyse(int* pile_height, int* holes, int* connected_holes,
                          int* altitude_difference, int* max_well_depth) const {
  *pile_height = Height();
  *holes = 0;
  *connected_holes = 0;
  *altitude_difference = 0;
  *max_well_depth = 0;

  int max_pile_height = 0;

  for (int x=0 ; x<Width() ; ++x) {
    bool from_top = true;
    int well_depth = 0;

    for (int y=0 ; y<Height() ; ++y) {
      if (from_top) {
        if (Cell(x, y)) {
          // We've hit a filled block for the first time this row
          *pile_height = qMin(*pile_height, y);
          max_pile_height = qMax(max_pile_height, y);
          from_top = false;
        } else {
          if (well_depth) {
            // We're already in a well, and it's continuing
            well_depth ++;
          } else {
            // Maybe this is the start of a well?
            if ((x == 0 || Cell(x-1, y)) &&
                (x == Width()-1 || Cell(x+1, y))) {
              well_depth ++;
            }
          }
        }
      } else {
        if (!Cell(x, y)) {
          // We're in a hole
          (*holes) ++;

          // If the one above wasn't a hole as well then this is a new unique
          // connected hole
          if (y > 0 && Cell(x, y-1)) {
            (*connected_holes) ++;
          }
        }
      }
    }

    if (from_top) {
      // We've got to the bottom without hitting anything
      max_pile_height = Height();
    }

    *max_well_depth = qMax(*max_well_depth, well_depth);
  }

  *altitude_difference = max_pile_height - *pile_height;
}

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
