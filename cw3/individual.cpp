#include "individual.h"
#include "tetrisboard.h"
#include "game.h"

DEFINE_double(pwmstddev, 0.5, "standard deviation for the mutation operator on player weights");
DEFINE_double(pemstddev, 0.01, "standard deviation for the mutation operator on player exponents");
DEFINE_double(pdmstddev, 0.01, "standard deviation for the mutation operator on player displacements");
DEFINE_double(pmrate, 1.0 / Criteria_Count, "probability of a player gene being mutated");


template <>
void Individual<RatingAlgorithm_Linear>::CopyFrom(const Individual& other) {
  weights_ = other.weights_;
}
template <>
void Individual<RatingAlgorithm_Exponential>::CopyFrom(const Individual& other) {
  weights_ = other.weights_;
  exponents_ = other.exponents_;
}
template <>
void Individual<RatingAlgorithm_ExponentialWithDisplacement>::CopyFrom(const Individual& other) {
  weights_ = other.weights_;
  exponents_ = other.exponents_;
  displacements_ = other.displacements_;
}


template <>
void Individual<RatingAlgorithm_Linear>::InitRandom() {
  std::generate(weights_.begin(), weights_.end(),
                Utilities::RangeGenerator(-1000, 1000));
}
template <>
void Individual<RatingAlgorithm_Exponential>::InitRandom() {
  std::generate(weights_.begin(), weights_.end(),
                Utilities::RangeGenerator(-1000, 1000));
  std::generate(exponents_.begin(), exponents_.end(),
                Utilities::RangeGenerator(-2.0, 2.0));
}
template <>
void Individual<RatingAlgorithm_ExponentialWithDisplacement>::InitRandom() {
  std::generate(weights_.begin(), weights_.end(),
                Utilities::RangeGenerator(-1000, 1000));
  std::generate(exponents_.begin(), exponents_.end(),
                Utilities::RangeGenerator(-2.0, 2.0));
  std::generate(displacements_.begin(), displacements_.end(),
                Utilities::RangeGenerator(-10.0, 10.0));
}


template <>
void Individual<RatingAlgorithm_Linear>::MutateFrom(const Individual& parent) {
  std::generate(weights_.begin(), weights_.end(), Utilities::MutateGenerator(
               FLAGS_pmrate, sWeightDistribution, parent.weights_.begin()));
}
template <>
void Individual<RatingAlgorithm_Exponential>::MutateFrom(const Individual& parent) {
  std::generate(weights_.begin(), weights_.end(), Utilities::MutateGenerator(
                FLAGS_pmrate, sWeightDistribution, parent.weights_.begin()));
  std::generate(exponents_.begin(), exponents_.end(), Utilities::MutateGenerator(
                FLAGS_pmrate, sExponentDistribution, parent.exponents_.begin()));
}
template <>
void Individual<RatingAlgorithm_ExponentialWithDisplacement>::MutateFrom(const Individual& parent) {
  std::generate(weights_.begin(), weights_.end(), Utilities::MutateGenerator(
                FLAGS_pmrate, sWeightDistribution, parent.weights_.begin()));
  std::generate(exponents_.begin(), exponents_.end(), Utilities::MutateGenerator(
                FLAGS_pmrate, sExponentDistribution, parent.exponents_.begin()));
  std::generate(displacements_.begin(), displacements_.end(), Utilities::MutateGenerator(
                FLAGS_pmrate, sDisplacementDistribution, parent.displacements_.begin()));
}


template <>
void Individual<RatingAlgorithm_Linear>::Crossover(const Individual& one, const Individual& two) {
  std::generate(weights_.begin(), weights_.end(), Utilities::UniformCrossoverGenerator(
                one.weights_.begin(), two.weights_.begin()));
}
template <>
void Individual<RatingAlgorithm_Exponential>::Crossover(const Individual& one, const Individual& two) {
  std::generate(weights_.begin(), weights_.end(), Utilities::UniformCrossoverGenerator(
                one.weights_.begin(), two.weights_.begin()));
  std::generate(exponents_.begin(), exponents_.end(), Utilities::UniformCrossoverGenerator(
                one.exponents_.begin(), two.exponents_.begin()));
}
template <>
void Individual<RatingAlgorithm_ExponentialWithDisplacement>::Crossover(const Individual& one, const Individual& two) {
  std::generate(weights_.begin(), weights_.end(), Utilities::UniformCrossoverGenerator(
                one.weights_.begin(), two.weights_.begin()));
  std::generate(exponents_.begin(), exponents_.end(), Utilities::UniformCrossoverGenerator(
                one.exponents_.begin(), two.exponents_.begin()));
  std::generate(displacements_.begin(), displacements_.end(), Utilities::UniformCrossoverGenerator(
                one.displacements_.begin(), two.displacements_.begin()));
}


template <>
bool Individual<RatingAlgorithm_Linear>::CompareGenes(const Individual& other) const {
  return weights_ == other.weights_;
}
template <>
bool Individual<RatingAlgorithm_Exponential>::CompareGenes(const Individual& other) const {
  return weights_ == other.weights_ &&
         exponents_ == other.exponents_;
}
template <>
bool Individual<RatingAlgorithm_ExponentialWithDisplacement>::CompareGenes(const Individual& other) const {
  return weights_ == other.weights_ &&
         exponents_ == other.exponents_ &&
         displacements_ == other.displacements_;
}


template <>
double Individual<RatingAlgorithm_Linear>::Rating(const BoardStats& stats) const {
  auto stats_it = stats.begin();
  auto weights_it = weights_.begin();

  double ret = 0.0;
  while (stats_it != stats.end()) {
    ret += *weights_it * *stats_it;

    stats_it ++;
    weights_it ++;
  }

  return ret;
}

template <>
double Individual<RatingAlgorithm_Exponential>::Rating(const BoardStats& stats) const {
  auto stats_it = stats.begin();
  auto weights_it = weights_.begin();
  auto exponent_it = exponents_.begin();

  double ret = 0.0;
  while (stats_it != stats.end()) {
    ret += *weights_it * pow(*stats_it, *exponent_it);

    stats_it ++;
    weights_it ++;
    exponent_it ++;
  }

  return ret;
}

template <>
double Individual<RatingAlgorithm_ExponentialWithDisplacement>::Rating(const BoardStats& stats) const {
  auto stats_it = stats.begin();
  auto weights_it = weights_.begin();
  auto exponent_it = exponents_.begin();
  auto displacement_it = displacements_.begin();

  double ret = 0.0;
  while (stats_it != stats.end()) {
    ret += *weights_it * pow(*stats_it - *displacement_it, *exponent_it);

    stats_it ++;
    weights_it ++;
    exponent_it ++;
    displacement_it ++;
  }

  return ret;
}

template <>
const char* Individual<RatingAlgorithm_Linear>::NameOfAlgorithm() {
  return "Linear";
}
template <>
const char* Individual<RatingAlgorithm_Exponential>::NameOfAlgorithm() {
  return "Exponential";
}
template <>
const char* Individual<RatingAlgorithm_ExponentialWithDisplacement>::NameOfAlgorithm() {
  return "Exponential with displacement";
}

template <>
bool Individual<RatingAlgorithm_Linear>::HasExponents() {
  return false;
}
template <>
bool Individual<RatingAlgorithm_Exponential>::HasExponents() {
  return true;
}
template <>
bool Individual<RatingAlgorithm_ExponentialWithDisplacement>::HasExponents() {
  return true;
}

template <>
bool Individual<RatingAlgorithm_Linear>::HasDisplacements() {
  return false;
}
template <>
bool Individual<RatingAlgorithm_Exponential>::HasDisplacements() {
  return false;
}
template <>
bool Individual<RatingAlgorithm_ExponentialWithDisplacement>::HasDisplacements() {
  return true;
}

