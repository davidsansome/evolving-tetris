#include "individual.h"
#include "tetrisboard.h"
#include "game.h"

#include <limits>
#include <algorithm>

const double Individual::kStandardDeviation = 0.3;
Individual::RandomGenType* Individual::sRandomGen = NULL;

Individual::Individual()
    : weights_(Criteria_Count),
      has_fitness_(false),
      fitness_(0)
{
  if (!sRandomGen) {
    sRandomGen = new RandomGenType(
        boost::mt19937(), boost::normal_distribution<double>(1.0, kStandardDeviation));
  }
}

void Individual::InitRandom() {
  std::generate(weights_.begin(), weights_.end(),
                RangeGenerator<int>(-100, 100));
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

double Individual::Rating(TetrisBoard& board, const Tetramino& tetramino,
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

void Individual::SetFitness(QList<Game*> games) {
  quint64 running_total = 0;

  foreach (const Game* game, games) {
    running_total += game->BlocksPlaced();
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
