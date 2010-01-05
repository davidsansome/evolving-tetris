#include "blockselector_random.h"
#include "tetramino.h"

BlockSelector::Random::Random() {
}

void BlockSelector::Random::SetSeed(SeedType seed) {
  seed_[0] = seed;

  random_engine_.seed(boost::mt19937::result_type(seed));
}

int BlockSelector::Random::operator ()() {
  // boost::uniform_smallint::operator () really should be const - it doesn't
  // modify the object.
  return const_cast<boost::uniform_smallint<>& >(Tetramino::kTypeRange)(
      random_engine_);
}

void BlockSelector::Random::InitRandom() {
  SetSeed(rand());
}
