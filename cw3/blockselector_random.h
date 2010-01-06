#ifndef BLOCKSELECTOR_RANDOM_H
#define BLOCKSELECTOR_RANDOM_H

#include <tr1/array>

#include <boost/random/mersenne_twister.hpp>

#include "individualbase.h"

namespace BlockSelector {

  class Random : public IndividualBase {
   public:
    Random();

    typedef uint32_t SeedType;
    typedef std::tr1::array<SeedType, 1> GeneType;

    void SetSeed(SeedType seed);
    SeedType GetSeed() const { return seed_[0]; }

    // BlockSelector
    void Reset();
    int operator()();

    // Individual
    void InitRandom();
    /*void MutateFrom(const Random& parent);
    void CopyFrom(const Random& other);
    void Crossover(const Random& one, const Random& two);
    void Mutate();*/

    const GeneType& Gene() const { return seed_; }

   private:
    GeneType seed_;

    boost::mt19937 random_engine_;
  };

} // namespace BlockSelector

#endif
