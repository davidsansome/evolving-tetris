#ifndef INDIVIDUAL_H
#define INDIVIDUAL_H

#include <QVector>
#include <QtDebug>

#include <cstdlib>
#include <boost/random/variate_generator.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/mersenne_twister.hpp>

class TetrisBoard;
class Tetramino;
class Game;

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
  double Rating(TetrisBoard& board, const Tetramino& tetramino,
                int x, int orientation) const;

  // Sets this individual's fitness from the results of some games
  void SetFitness(QList<Game*> games);
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

#endif // INDIVIDUAL_H
