#ifndef INDIVIDUAL_H
#define INDIVIDUAL_H

#include <tr1/array>
#include <limits>
#include <algorithm>
#include <cstdlib>

#include <boost/bind.hpp>
#include <boost/random/normal_distribution.hpp>

#include <google/gflags.h>

#include "tetrisboard.h"
#include "individualbase.h"
#include "messages.pb.h"

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

DECLARE_double(pwmstddev);
DECLARE_double(pemstddev);
DECLARE_double(pdmstddev);


template <RatingAlgorithm A>
class Individual : public IndividualBase {
 public:
  Individual();

  typedef std::tr1::array<int, Criteria_Count> IntegerGenes;
  typedef std::tr1::array<double, Criteria_Count> RealGenes;

  static const char* NameOfAlgorithm();
  static bool HasExponents();
  static bool HasDisplacements();

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

  // Compares the fitness and weights.  Will always return false unless both
  // have a fitness (to implement "invalid" default constructed values).
  bool operator ==(const Individual& other) const;
  bool CompareGenes(const Individual& other) const;

  void ToMessage(Messages::Player* message) const;
  void FromMessage(const Messages::Player& message);

 private:
  IntegerGenes weights_;
  RealGenes exponents_;
  RealGenes displacements_;

  typedef boost::normal_distribution<double> DistributionType;
  static DistributionType sWeightDistribution;
  static DistributionType sExponentDistribution;
  static DistributionType sDisplacementDistribution;

  // The specialisations of this function use our weights, exponents and
  // displacements to find a rating for a BoardStats struct
  double Rating(const BoardStats& stats) const;
};

#ifndef QT_NO_DEBUG
  template <typename T, std::size_t N>
  QDebug operator<<(QDebug s, const std::tr1::array<T, N>& a);

  template <RatingAlgorithm A>
  QDebug operator<<(QDebug s, const Individual<A>& i);
#endif // QT_NO_DEBUG

template <RatingAlgorithm A>
typename Individual<A>::DistributionType Individual<A>::sWeightDistribution(1.0, FLAGS_pwmstddev);
template <RatingAlgorithm A>
typename Individual<A>::DistributionType Individual<A>::sExponentDistribution(1.0, FLAGS_pemstddev);
template <RatingAlgorithm A>
typename Individual<A>::DistributionType Individual<A>::sDisplacementDistribution(1.0, FLAGS_pdmstddev);


template <RatingAlgorithm A>
Individual<A>::Individual()
{
}

template <RatingAlgorithm A>
void Individual<A>::Mutate() {
  MutateFrom(*this);
}

template <RatingAlgorithm A>
bool Individual<A>::operator ==(const Individual<A>& other) const {
  if (!HasFitness() || !other.HasFitness())
    return false;
  if (Fitness() != other.Fitness())
    return false;
  return CompareGenes(other);
}

template <RatingAlgorithm A>
template <int W, int H>
double Individual<A>::Rating(TetrisBoard<W, H>& board, const Tetramino& tetramino,
                          int x, int orientation) const {
  assert(weights_.size() == Criteria_Count);

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

template <RatingAlgorithm A>
void Individual<A>::ToMessage(Messages::Player* message) const {
  switch (A) {
   case RatingAlgorithm_Linear:
    message->set_algorithm(Messages::Player_Algorithm_LINEAR);
    break;

   case RatingAlgorithm_Exponential:
    message->set_algorithm(Messages::Player_Algorithm_EXPONENTIAL);
    break;

   case RatingAlgorithm_ExponentialWithDisplacement:
    message->set_algorithm(Messages::Player_Algorithm_EXPONENTIAL_WITH_DISPLACEMENT);
    break;
  }

  message->mutable_weights()->Reserve(weights_.size());
  message->mutable_exponents()->Reserve(exponents_.size());
  message->mutable_displacements()->Reserve(displacements_.size());

  std::for_each(weights_.begin(), weights_.end(), boost::bind(
      &google::protobuf::RepeatedField<google::protobuf::int32>::Add,
      message->mutable_weights(), _1));
  std::for_each(exponents_.begin(), exponents_.end(), boost::bind(
      &google::protobuf::RepeatedField<double>::Add,
      message->mutable_exponents(), _1));
  std::for_each(displacements_.begin(), displacements_.end(), boost::bind(
      &google::protobuf::RepeatedField<double>::Add,
      message->mutable_displacements(), _1));
}

template <RatingAlgorithm A>
void Individual<A>::FromMessage(const Messages::Player& message) {
  std::copy(message.weights().begin(), message.weights().end(), weights_.begin());
  std::copy(message.exponents().begin(), message.exponents().end(), exponents_.begin());
  std::copy(message.displacements().begin(), message.displacements().end(), displacements_.begin());
}

#ifndef QT_NO_DEBUG
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
#endif // QT_NO_DEBUG

#endif // INDIVIDUAL_H
