#ifndef TETRAMINO_H
#define TETRAMINO_H

#include <QPoint>
#include <QSize>

class Tetramino {
 public:
  Tetramino();
  Tetramino(int type);

  int Type() const { return type_; }
  const QPoint* Points(int orientation) const { return DataOffset(type_, orientation, 0); }
  const QSize& Size(int orientation) const { return SizeOffset(type_, orientation); }

  static const int kTypeCount;
  static const int kOrientationCount;
  static const int kBlockSize;
  static const int kPointsCount;

 private:
  void Init();

  inline QPoint* DataOffset(int type, int orientation, int i) const;
  inline QSize& SizeOffset(int type, int orientation) const;

  int type_;

  static QPoint* data_;
  static QSize* size_;
};

QPoint* Tetramino::DataOffset(int type, int orientation, int i) const {
  return data_ +
      type * (kOrientationCount * kPointsCount) +
      orientation * kPointsCount +
      i;
}

QSize& Tetramino::SizeOffset(int type, int orientation) const {
  return *(size_ +
      type * kOrientationCount +
      orientation);
}

#endif // TETRAMINO_H
