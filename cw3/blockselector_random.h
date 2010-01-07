#ifndef BLOCKSELECTOR_RANDOM_H
#define BLOCKSELECTOR_RANDOM_H

#include <tr1/array>
#include <boost/random/lagged_fibonacci.hpp>

#include "individualbase.h"
#include "messages.pb.h"

namespace BlockSelector {

  class Random : public IndividualBase {
   public:
    Random();

    // BlockSelector
    void Reset();
    int operator()();

    // Individual
    void InitRandom();

    void ToMessage(Messages::BlockSelectorRandom* message);
    void FromMessage(const Messages::GameRequest& req);

   private:
    uint32_t seed_;
    boost::lagged_fibonacci607 rng_;
  };

} // namespace BlockSelector

#endif
