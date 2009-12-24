#include "individual.h"
#include "tetrisboard.h"
#include "game.h"

#include <limits>
#include <algorithm>

#include <google/gflags.h>

DEFINE_double(mutation, 0.5, "standard deviation for the mutation operator");

Individual::RandomGenType* Individual::sRandomGen = NULL;

Individual::Individual()
    : weights_(Criteria_Count),
      has_fitness_(false),
      fitness_(0)
{
  if (!sRandomGen) {
    sRandomGen = new RandomGenType(
        boost::mt19937(), boost::normal_distribution<double>(1.0, FLAGS_mutation));
  }
}

void Individual::InitRandom() {
  std::generate(weights_.begin(), weights_.end(),
                RangeGenerator<int>(-1000, 1000));
}

void Individual::MutateFrom(const Individual& parent) {
  std::generate(weights_.begin(), weights_.end(),
                MutateGenerator<int>(parent.weights_.begin()));
}

void Individual::Mutate() {
  MutateFrom(*this);
}

void Individual::CopyFrom(const Individual& other) {
  weights_ = other.weights_;
}

void Individual::Crossover(const Individual& one, const Individual& two) {
  QVector<int>::const_iterator one_it = one.weights_.begin();
  QVector<int>::const_iterator two_it = two.weights_.begin();
  QVector<int>::iterator my_it = weights_.begin();

  while (my_it != weights_.end()) {
    *(my_it) = (rand() % 2) ? *one_it : *two_it;

    my_it ++;
    one_it ++;
    two_it ++;
  }
}

void Individual::SetFitness(QList<quint64> games) {
  quint64 running_total = 0;

  foreach (quint64 game, games) {
    running_total += game;
  }
  fitness_ = float(running_total) / games.count();
  has_fitness_ = true;
}

bool Individual::operator ==(const Individual& other) const {
  if (!has_fitness_ || !other.has_fitness_)
    return false;
  if (fitness_ != other.fitness_)
    return false;
  return weights_ == other.weights_;
}

QDebug operator<<(QDebug s, const Individual& i) {
  return s.space() << (i.HasFitness() ? QString::number(i.Fitness()).toAscii().constData() : "??")
                   << "-" << i.Weights();
}
