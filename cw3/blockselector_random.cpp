#include "blockselector_random.h"
#include "tetramino.h"

namespace BlockSelector {

Random::Random() {
}

void Random::Reset() {
  rng_.seed(seed_);
}

int Random::operator ()() {
  return Tetramino::kTypeRange();
}

void Random::InitRandom() {
  seed_ = Utilities::global_rng() * std::numeric_limits<uint32_t>::max();
  rng_.seed(seed_);
}

void Random::ToMessage(Messages::BlockSelectorRandom* message) {
  message->set_seed(seed_);
}

void Random::FromMessage(const Messages::GameRequest& req) {
  seed_ = req.selector_random().seed();
  rng_.seed(seed_);
}

} // namespace BlockSelector
