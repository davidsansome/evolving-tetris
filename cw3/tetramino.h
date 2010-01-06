#ifndef TETRAMINO_H
#define TETRAMINO_H

#include "utilities.h"

#include <boost/random/uniform_smallint.hpp>
#include <cassert>

class Tetramino {
 public:
  Tetramino();

  void InitFrom(int type) { type_ = type; }
  void InitFrom(const Tetramino& other) { type_ = other.type_; }

  int Type() const { return type_; }
  int OrientationCount() const { return orientation_count_[type_]; }
  const Int2* Points(int orientation) const { return DataOffset(type_, orientation, 0); }
  const Int2& Size(int orientation) const { return SizeOffset(type_, orientation); }

  static const int kTypeCount;
  static const int kBlockSize;
  static const int kPointsCount;
  static boost::uniform_smallint<> kTypeRange;

 private:
  Tetramino(const Tetramino&) {}
  void operator =(const Tetramino&) {}

  void InitStatic();

  inline Int2* DataOffset(int type, int orientation, int i) const;
  inline Int2& SizeOffset(int type, int orientation) const;

  int type_;

  static Int2* data_;
  static Int2* size_;
  static int* orientation_count_;

  static const int kMaxOrientationCount;
};

Int2* Tetramino::DataOffset(int type, int orientation, int i) const {
  assert(type >= 0 && type < kTypeCount);
  assert(orientation >= 0 && orientation < orientation_count_[type]);
  assert(i >= 0 && i < kPointsCount);

  return data_ +
      type * (kMaxOrientationCount * kPointsCount) +
      orientation * kPointsCount +
      i;
}

Int2& Tetramino::SizeOffset(int type, int orientation) const {
  assert(type >= 0 && type < kTypeCount);
  assert(orientation >= 0 && orientation < orientation_count_[type]);

  return *(size_ +
      type * kMaxOrientationCount +
      orientation);
}

#endif // TETRAMINO_H
