#include "tetramino.h"

#include <QImage>
#include <QtDebug>

QPoint* Tetramino::data_ = NULL;
QSize* Tetramino::size_ = NULL;
int* Tetramino::orientation_count_ = NULL;

const int Tetramino::kTypeCount = 7;
const int Tetramino::kMaxOrientationCount = 4;
const int Tetramino::kBlockSize = 4;
const int Tetramino::kPointsCount = 4;

boost::uniform_smallint<> Tetramino::random_range_(0, Tetramino::kTypeCount-1);

Tetramino::Tetramino()
{
  InitStatic();
}

void Tetramino::InitStatic() {
  if (data_ == NULL) {
    // Open the image containing tetramino data
    QImage image(":/data/tetraminos.png");
    Q_ASSERT(!image.isNull());
    Q_ASSERT(image.width() == kTypeCount * kBlockSize);
    Q_ASSERT(image.height() == kMaxOrientationCount * kBlockSize);

    // Allocate the data
    data_ = new QPoint[kTypeCount * kMaxOrientationCount * kPointsCount];
    size_ = new QSize[kTypeCount * kMaxOrientationCount];
    orientation_count_ = new int[kTypeCount];

    // Read the data from the image
    for (int type=0 ; type<kTypeCount ; ++type) {
      orientation_count_[type] = 0;

      for (int orientation=0 ; orientation<kMaxOrientationCount ; ++orientation) {
        QList<QPoint> points;
        int max_x = 0;
        int max_y = 0;

        for (int x=0 ; x<kBlockSize ; ++x) {
          for (int y=0 ; y<kBlockSize ; ++y) {
            int image_x = type * kBlockSize + x;
            int image_y = orientation * kBlockSize + y;

            if (qRed(image.pixel(image_x, image_y)) < 128) {
              points << QPoint(x, y);
              max_x = qMax(max_x, x);
              max_y = qMax(max_y, y);
            }
          }
        }

        if (points.count() == 0)
          break;

        orientation_count_[type] ++;

        Q_ASSERT(points.count() == kPointsCount);
        for (int i=0 ; i<kPointsCount ; ++i) {
          *(DataOffset(type, orientation, i)) = points[i];
        }
        SizeOffset(type, orientation) = QSize(max_x+1, max_y+1);
      }
    }
  }
}
