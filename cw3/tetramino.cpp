#include "tetramino.h"

#include <vector>

Int2* Tetramino::data_ = NULL;
Int2* Tetramino::size_ = NULL;
int* Tetramino::orientation_count_ = NULL;

const int Tetramino::kTypeCount = 7;
const int Tetramino::kMaxOrientationCount = 4;
const int Tetramino::kBlockSize = 4;
const int Tetramino::kPointsCount = 4;
Utilities::_RangeGenerator<int> Tetramino::kTypeRange(0, Tetramino::kTypeCount-1);

#include "data/tetraminos.c"

Tetramino::Tetramino()
{
  InitStatic();
}

void Tetramino::InitStatic() {
  if (data_ == NULL) {
    // Load the image containing tetramino data
    assert(tetraminos_png.width == kTypeCount * kBlockSize);
    assert(tetraminos_png.height == kMaxOrientationCount * kBlockSize);

    // Allocate the data
    data_ = new Int2[kTypeCount * kMaxOrientationCount * kPointsCount];
    size_ = new Int2[kTypeCount * kMaxOrientationCount];
    orientation_count_ = new int[kTypeCount];

    // Read the data from the image
    for (int type=0 ; type<kTypeCount ; ++type) {
      orientation_count_[type] = 0;

      for (int orientation=0 ; orientation<kMaxOrientationCount ; ++orientation) {
        std::vector<Int2> points;
        int max_x = 0;
        int max_y = 0;

        for (int x=0 ; x<kBlockSize ; ++x) {
          for (int y=0 ; y<kBlockSize ; ++y) {
            int image_x = type * kBlockSize + x;
            int image_y = orientation * kBlockSize + y;

            if (tetraminos_png.pixel_data[image_y * tetraminos_png.width * tetraminos_png.bytes_per_pixel +
                                          image_x * tetraminos_png.bytes_per_pixel] < 128) {
              points.push_back(Int2(x, y));
              max_x = std::max(max_x, x);
              max_y = std::max(max_y, y);
            }
          }
        }

        if (points.size() == 0)
          break;

        orientation_count_[type] ++;

        assert(points.size() == kPointsCount);
        for (int i=0 ; i<kPointsCount ; ++i) {
          *(DataOffset(type, orientation, i)) = points[i];
        }
        SizeOffset(type, orientation) = Int2(max_x+1, max_y+1);
      }
    }
  }
}
