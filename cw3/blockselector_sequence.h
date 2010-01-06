#ifndef BLOCKSELECTOR_SEQUENCE_H
#define BLOCKSELECTOR_SEQUENCE_H

#include <cstdint>
#include <algorithm>
#include <boost/random.hpp>
#include <tr1/array>
#include <google/gflags.h>

#include "tetramino.h"
#include "utilities.h"
#include "individualbase.h"

DECLARE_double(smrate);

namespace BlockSelector {

  template <int N>
  class Sequence : public IndividualBase {
   public:
    Sequence();

    typedef uint8_t GeneType;
    typedef std::tr1::array<GeneType, N> SequenceType;

    // This one is required because GCC is stupid
    typedef typename SequenceType::const_iterator SequenceIteratorType;

    // BlockSelector
    void Reset();
    int operator()();

    // Individual
    void InitRandom();
    void MutateFrom(const Sequence& parent);
    void CopyFrom(const Sequence& other);
    void Crossover(const Sequence& one, const Sequence& two);
    void Mutate();
    bool operator ==(const Sequence& other) const;

    SequenceType GetSequence() const { return sequence_; }

   private:
    SequenceType sequence_;
    uint64_t next_index_;

    typedef boost::variate_generator<
        boost::mt19937, boost::uniform_smallint<> > RandomGenType;
    static RandomGenType* sRandomGen;
  };


  template <int N>
  typename Sequence<N>::RandomGenType* Sequence<N>::sRandomGen = NULL;


  template <int N>
  Sequence<N>::Sequence()
    : next_index_(0)
  {
    if (sRandomGen == NULL) {
      sRandomGen = new RandomGenType(
          boost::mt19937(), Tetramino::kTypeRange);
    }
  }

  template <int N>
  int Sequence<N>::operator ()() {
    int ret = sequence_[next_index_];
    next_index_ = (next_index_ + 1) % N;

    return ret;
  }

  template <int N>
  void Sequence<N>::InitRandom() {
    std::generate(sequence_.begin(), sequence_.end(),
        Utilities::RangeGenerator<GeneType>(0, Tetramino::kTypeCount-1));
  }

  template <int N>
  void Sequence<N>::MutateFrom(const Sequence &parent) {
    std::generate(sequence_.begin(), sequence_.end(),
        Utilities::MutateReplaceGenerator<SequenceIteratorType, RandomGenType>(
            FLAGS_smrate, sRandomGen, parent.sequence_.begin()));
  }

  template <int N>
  void Sequence<N>::CopyFrom(const Sequence &other) {
    sequence_ = other.sequence_;
  }

  template <int N>
  void Sequence<N>::Crossover(const Sequence &one, const Sequence &two) {
    std::generate(sequence_.begin(), sequence_.end(),
        Utilities::CrossoverGenerator<SequenceIteratorType>(
            one.sequence_.begin(), two.sequence_.begin()));
  }

  template <int N>
  void Sequence<N>::Mutate() {
    MutateFrom(*this);
  }

  template <int N>
  void Sequence<N>::Reset() {
    next_index_ = 0;
  }

  template <int N>
  bool Sequence<N>::operator ==(const Sequence& other) const {
    return sequence_ == other.sequence_;
  }

} // namespace BlockSelector

#endif // BLOCKSELECTOR_SEQUENCE_H
