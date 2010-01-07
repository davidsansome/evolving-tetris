#include "blockselector_random.h"
#include "tetramino.h"

namespace BlockSelector {

Random::Random() {
}

void Random::SetSeed(unsigned int seed) {
  original_seed_ = seed;

  Reset();
}

void Random::Reset() {
  seed_ = original_seed_;
}

int Random::operator ()() {
  return rand_r(&seed_) % Tetramino::kTypeCount;
}

void Random::InitRandom() {
  SetSeed(Utilities::FastRand());
}

void Random::ToMessage(Messages::BlockSelectorRandom* message) {
  message->set_seed(original_seed_);
}

void Random::FromMessage(const Messages::GameRequest& req) {
  original_seed_ = req.selector_random().seed();
  Reset();
}

} // namespace BlockSelector
