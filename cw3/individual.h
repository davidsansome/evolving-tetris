#ifndef INDIVIDUAL_H
#define INDIVIDUAL_H

#include <QVector>
#include <QtDebug>

#include <tr1/array>
#include <limits>
#include <algorithm>
#include <cstdlib>

#include <boost/random/variate_generator.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/mersenne_twister.hpp>

#include <google/gflags.h>

#include "tetrisboard.h"

class Tetramino;

enum RatingAlgorithm {
  RatingAlgorithm_Linear,
  RatingAlgorithm_Exponential,
  RatingAlgorithm_ExponentialWithDisplacement,
};

enum Criteria {
  Criteria_PileHeight = 0,
  Criteria_Holes,
  Criteria_ConnectedHoles,
  Criteria_RemovedLines,
  Criteria_AltitudeDifference,
  Criteria_MaxWellDepth,
  Criteria_SumWellDepth,
  Criteria_LandingHeight,
  Criteria_Blocks,
  Criteria_WeightedBlocks,
  Criteria_RowTransitions,
  Criteria_ColumnTransitions,

  Criteria_Count,
};

DECLARE_double(mstddev);
DECLARE_double(mrate);


template <RatingAlgorithm A>
class Individual {
 public:
  Individual();

  typedef std::tr1::array<int, Criteria_Count> IntegerGenes;
  typedef std::tr1::array<double, Criteria_Count> RealGenes;

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
  RealGenes Exponents() const { return exponents_; }
  RealGenes Displacements() const { return displacements_; }

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
  bool CompareGenes(const Individual& other) const;

 private:
  IntegerGenes weights_;
  RealGenes exponents_;
  RealGenes displacements_;
  bool has_fitness_;
  quint64 fitness_;

  typedef boost::variate_generator<
      boost::mt19937, boost::normal_distribution<double> > RandomGenType;
  static RandomGenType* sRandomGen;

  // The specialisations of this function use our weights, exponents and
  // displacements to find a rating for a BoardStats struct
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
    typedef const T* iterator_type;

    MutateGenerator(double p, iterator_type original_it)
        : p_(p), original_it_(original_it) {}
    T operator()();

   private:
    double p_;
    iterator_type original_it_;
  };

  template <typename T>
  class CrossoverGenerator {
   public:
    typedef const T* iterator_type;

    CrossoverGenerator(iterator_type a, iterator_type b)
      : a_(a), b_(b) {}
    T operator()();

   private:
    iterator_type a_;
    iterator_type b_;
  };
};

template <typename T, std::size_t N>
QDebug operator<<(QDebug s, const std::tr1::array<T, N>& a);

template <RatingAlgorithm A>
QDebug operator<<(QDebug s, const Individual<A>& i);

template <RatingAlgorithm A>
typename Individual<A>::RandomGenType* Individual<A>::sRandomGen = NULL;


template <RatingAlgorithm A>
Individual<A>::Individual()
    : has_fitness_(false),
      fitness_(0)
{
  if (!sRandomGen) {
    sRandomGen = new RandomGenType(
        boost::mt19937(), boost::normal_distribution<double>(1.0, FLAGS_mstddev));
  }
}

template <RatingAlgorithm A>
void Individual<A>::Mutate() {
  MutateFrom(*this);
}

template <RatingAlgorithm A>
void Individual<A>::SetFitness(QList<quint64> games) {
  quint64 running_total = 0;

  foreach (quint64 game, games) {
    running_total += game;
  }
  fitness_ = float(running_total) / games.count();
  has_fitness_ = true;
}

template <RatingAlgorithm A>
bool Individual<A>::operator ==(const Individual<A>& other) const {
  if (!has_fitness_ || !other.has_fitness_)
    return false;
  if (fitness_ != other.fitness_)
    return false;
  return CompareGenes(other);
}


template <RatingAlgorithm A>
template <typename T>
T Individual<A>::RangeGenerator<T>::operator()() const {
  return min_ + T((double(qrand()) / RAND_MAX) * range_);
}

template <RatingAlgorithm A>
template <typename T>
T Individual<A>::MutateGenerator<T>::operator()() {
  double r = double(qrand()) / RAND_MAX;
  if (r < p_)
    return double(*(original_it_++)) * (*sRandomGen)();
  else
    return *(original_it_++);
}

template <RatingAlgorithm A>
template <typename T>
T Individual<A>::CrossoverGenerator<T>::operator()() {
  T ret = (qrand() % 2) ? *a_ : *b_;

  ++ a_;
  ++ b_;
  return ret;
}

template <RatingAlgorithm A>
template <int W, int H>
double Individual<A>::Rating(TetrisBoard<W, H>& board, const Tetramino& tetramino,
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

  return Rating(stats);
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

template <RatingAlgorithm A>
QDebug operator<<(QDebug s, const Individual<A>& i) {
  return s.space() << (i.HasFitness() ? QString::number(i.Fitness()).toAscii().constData() : "??")
                   << "-" << i.Weights();
}

#endif // INDIVIDUAL_H
