#ifndef BLOCKSELECTOR_RANDOM_H
#define BLOCKSELECTOR_RANDOM_H

#include <tr1/array>

#include "individualbase.h"
#include "messages.pb.h"

namespace BlockSelector {

  class Random : public IndividualBase {
   public:
    Random();

    void SetSeed(unsigned int seed);
    unsigned int GetSeed() const { return original_seed_; }

    // BlockSelector
    void Reset();
    int operator()();

    // Individual
    void InitRandom();

    void ToMessage(Messages::BlockSelectorRandom* message);
    void FromMessage(const Messages::GameRequest& req);

   private:
    unsigned int original_seed_;
    unsigned int seed_;
  };

} // namespace BlockSelector

#endif
