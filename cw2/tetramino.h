#ifndef TETRAMINO_H
#define TETRAMINO_H

#include <QPoint>
#include <QSize>

class Tetramino {
 public:
  Tetramino();
  Tetramino(int type);

  int Type() const { return type_; }
  int OrientationCount() const { return orientation_count_[type_]; }
  const QPoint* Points(int orientation) const { return DataOffset(type_, orientation, 0); }
  const QSize& Size(int orientation) const { return SizeOffset(type_, orientation); }

  static const int kTypeCount;
  static const int kBlockSize;
  static const int kPointsCount;

 private:
  void Init();

  inline QPoint* DataOffset(int type, int orientation, int i) const;
  inline QSize& SizeOffset(int type, int orientation) const;

  int type_;

  static QPoint* data_;
  static QSize* size_;
  static int* orientation_count_;

  static const int kMaxOrientationCount;
};

QPoint* Tetramino::DataOffset(int type, int orientation, int i) const {
  Q_ASSERT(type >= 0 && type < kTypeCount);
  Q_ASSERT(orientation >= 0 && orientation < orientation_count_[type]);
  Q_ASSERT(i >= 0 && i < kPointsCount);

  return data_ +
      type * (kMaxOrientationCount * kPointsCount) +
      orientation * kPointsCount +
      i;
}

QSize& Tetramino::SizeOffset(int type, int orientation) const {
  Q_ASSERT(type >= 0 && type < kTypeCount);
  Q_ASSERT(orientation >= 0 && orientation < orientation_count_[type]);

  return *(size_ +
      type * kMaxOrientationCount +
      orientation);
}

#endif // TETRAMINO_H
