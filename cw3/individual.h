#ifndef INDIVIDUAL_H
#define INDIVIDUAL_H

#include <QVector>
#include <QtDebug>

#include <cstdlib>
#include <boost/random/variate_generator.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/mersenne_twister.hpp>

#include "tetrisboard.h"

class Tetramino;

class Individual {
 public:
  Individual();

  enum Algorithm {
    Linear,
    Exponential,
    ExponentialWithDisplacement
  };

  enum Criteria {
    PileHeight = 0,
    Holes,
    ConnectedHoles,
    RemovedLines,
    AltitudeDifference,
    MaxWellDepth,

    Criteria_Count
  };

  // Creating a new individual
  void InitRandom();
  void MutateFrom(const Individual& parent);
  void CopyFrom(const Individual& other);
  void Crossover(const Individual& one, const Individual& two);

  // Mutate this individual
  void Mutate();

  // Get the weights for this individual
  QVector<int> Weights() const { return weights_; }

  // Adds the given tetramino to this board and computes a score based on
  // this individual's weightings
  template <int W, int H>
  double Rating(TetrisBoard<W, H>& board, const Tetramino& tetramino,
                int x, int orientation) const;

  // Sets this individual's fitness from the results of some games
  void SetFitness(QList<quint64> games);
  bool HasFitness() const { return has_fitness_; }
  quint64 Fitness() const { return fitness_; }

  // Compares the fitness and weights.  Will always return false unless both
  // have a fitness (to implement "invalid" default constructed values).
  bool operator ==(const Individual& other) const;

  static const double kStandardDeviation;

 private:
  QVector<int> weights_;
  QVector<double> exponents_;
  QVector<double> displacements_;
  bool has_fitness_;
  quint64 fitness_;

  typedef boost::variate_generator<
      boost::mt19937, boost::normal_distribution<double> > RandomGenType;
  static RandomGenType* sRandomGen;

  // These generators are compatiable with std::generate from <algorithms>.
  // They are used to set new values for an individual.

  // Generates random numbers from a range.
  template <typename T>
  class RangeGenerator {
   public:
    RangeGenerator(T min, T max) : min_(min), range_(max-min) {}
    T operator()() const;

   private:
    T min_;
    T range_;
  };

  // Mutates each value by multiplying it by a normally distributed random
  // number with mean = 1.0 and kStandardDeviation.
  template <typename T>
  class MutateGenerator {
   public:
    typedef typename QVector<T>::const_iterator iterator_type;

    MutateGenerator(iterator_type original_it)
        : original_it_(original_it) {}
    T operator()();

   private:
    iterator_type original_it_;
  };
};

QDebug operator<<(QDebug s, const Individual& i);

template <typename T>
T Individual::RangeGenerator<T>::operator()() const {
  return min_ + qrand() % range_;
}

template <typename T>
T Individual::MutateGenerator<T>::operator()() {
  return double(*(original_it_++)) * (*sRandomGen)();
}

template <int W, int H>
double Individual::Rating(TetrisBoard<W, H>& board, const Tetramino& tetramino,
                          int x, int orientation) const {
  Q_ASSERT(weights_.count() == Criteria_Count);

  int y = board.TetraminoHeight(tetramino, x, orientation);

  if (y < 0) {
    // We can't add the tetramino here
    return std::numeric_limits<double>::quiet_NaN();
  }

  // Add the tetramino to the board
  board.Add(tetramino, x, y, orientation);

  // Count the rows that were removed by adding this tetramino
  int removed_lines = board.ClearRows();

  int pile_height;
  int holes;
  int connected_holes;
  int altitude_difference;
  int max_well_depth;

  board.Analyse(&pile_height, &holes, &connected_holes,
                &altitude_difference, &max_well_depth);

  return
      weights_[PileHeight] * pile_height +
      weights_[Holes] * holes +
      weights_[ConnectedHoles] * connected_holes +
      weights_[RemovedLines] * removed_lines +
      weights_[AltitudeDifference] * altitude_difference +
      weights_[MaxWellDepth] * max_well_depth;
}

#endif // INDIVIDUAL_H
