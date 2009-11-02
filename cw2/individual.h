#ifndef INDIVIDUAL_H
#define INDIVIDUAL_H

#include <QVector>

#include <cstdlib>

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

 private:
  QVector<int> weights_;
  QVector<double> exponents_;
  QVector<double> displacements_;
  bool has_fitness_;
  quint64 fitness_;

  template <typename T>
  class RangeGenerator {
   public:
    RangeGenerator(T min, T max) : min_(min), range_(max-min) {}
    T operator()() const;

   private:
    T min_;
    T range_;
  };

  template <typename T>
  class MutateGenerator {
   public:
    typedef typename QVector<T>::const_iterator iterator_type;

    MutateGenerator(double mutation_probability, RangeGenerator<T> range_gen,
                    iterator_type original_it)
        : mutation_probability_(mutation_probability),
          range_gen_(range_gen),
          original_it_(original_it) {}
    T operator()();

   private:
    double mutation_probability_;
    RangeGenerator<T> range_gen_;
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
  T original = *(original_it_++);

  if (float(qrand()) / RAND_MAX < mutation_probability_) {
    return range_gen_();
  } else {
    return original;
  }
}

#endif // INDIVIDUAL_H
