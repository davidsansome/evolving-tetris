#ifndef INDIVIDUAL_H
#define INDIVIDUAL_H

#include <QVector>
#include <QtDebug>

#include <tr1/array>

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
    ExponentialWithDisplacement,
  };

  enum Criteria {
    PileHeight = 0,
    Holes,
    ConnectedHoles,
    RemovedLines,
    AltitudeDifference,
    MaxWellDepth,
    SumWellDepth,
    LandingHeight,
    Blocks,
    WeightedBlocks,
    RowTransitions,
    ColumnTransitions,

    Criteria_Count,
  };

  typedef std::tr1::array<int, Criteria_Count> IntegerGenes;
  typedef std::tr1::array<double, Criteria_Count> RealGenes;

  template <Algorithm A>
  static const char* NameOfAlgorithm();

  // Creating a new individual
  void InitRandom();
  void MutateFrom(const Individual& parent);
  void CopyFrom(const Individual& other);
  void Crossover(const Individual& one, const Individual& two);

  // Mutate this individual
  void Mutate();

  // Get the weights for this individual
  IntegerGenes Weights() const { return weights_; }

  // Adds the given tetramino to this board and computes a score based on
  // this individual's weightings
  template <Algorithm A, int W, int H>
  double Rating(TetrisBoard<W, H>& board, const Tetramino& tetramino,
                int x, int orientation) const;

  // Sets this individual's fitness from the results of some games
  void SetFitness(QList<quint64> games);
  bool HasFitness() const { return has_fitness_; }
  quint64 Fitness() const { return fitness_; }

  // Compares the fitness and weights.  Will always return false unless both
  // have a fitness (to implement "invalid" default constructed values).
  bool operator ==(const Individual& other) const;

 private:
  IntegerGenes weights_;
  RealGenes exponents_;
  RealGenes displacements_;
  bool has_fitness_;
  quint64 fitness_;

  typedef boost::variate_generator<
      boost::mt19937, boost::normal_distribution<double> > RandomGenType;
  static RandomGenType* sRandomGen;

  // These functions use our weights, exponents and displacements to find a
  // rating for a BoardStats struct
  template <Algorithm A>
  double Rating(const BoardStats& stats) const;

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

    MutateGenerator(double p, iterator_type original_it)
        : p_(p), original_it_(original_it) {}
    T operator()();

   private:
    double p_;
    iterator_type original_it_;
  };
};

template <typename T, std::size_t N>
QDebug operator<<(QDebug s, const std::tr1::array<T, N>& a);

QDebug operator<<(QDebug s, const Individual& i);




template <typename T>
T Individual::RangeGenerator<T>::operator()() const {
  return min_ + qrand() % range_;
}

template <typename T>
T Individual::MutateGenerator<T>::operator()() {
  double r = double(qrand()) / RAND_MAX;
  if (r < p_)
    return double(*(original_it_++)) * (*sRandomGen)();
  else
    return *(original_it_++);
}

template <Individual::Algorithm A, int W, int H>
double Individual::Rating(TetrisBoard<W, H>& board, const Tetramino& tetramino,
                          int x, int orientation) const {
  Q_ASSERT(weights_.size() == Criteria_Count);

  int y = board.TetraminoHeight(tetramino, x, orientation);

  if (y < 0) {
    // We can't add the tetramino here
    return std::numeric_limits<double>::quiet_NaN();
  }

  // Add the tetramino to the board
  board.Add(tetramino, x, y, orientation);

  BoardStats stats;
  stats.landing_height = y + tetramino.Size(orientation).height();

  // Count the rows that were removed by adding this tetramino
  stats.removed_lines = board.ClearRows();

  board.Analyse(&stats);

  return Rating<A>(stats);
}

template <typename T, std::size_t N>
QDebug operator<<(QDebug s, const std::tr1::array<T, N>& a) {
  s.nospace() << "(";
  for (auto it = a.begin() ; it != a.end() ; ++it) {
    s.nospace() << *it << ", ";
  }
  s.nospace() << ")";

  return s.maybeSpace();
}

#endif // INDIVIDUAL_H
