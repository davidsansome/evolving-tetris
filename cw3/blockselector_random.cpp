#include "blockselector_random.h"
#include "tetramino.h"

namespace BlockSelector {

Random::Random() {
}

void Random::SetSeed(SeedType seed) {
  seed_[0] = seed;

  Reset();
}

void Random::Reset() {
  random_engine_.seed(boost::mt19937::result_type(seed_[0]));
}

int Random::operator ()() {
  return Tetramino::kTypeRange(random_engine_);
}

void Random::InitRandom() {
  SetSeed(rand());
}

} // namespace BlockSelector
