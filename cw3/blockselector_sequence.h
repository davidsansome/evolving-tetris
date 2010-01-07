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
#include "messages.pb.h"

DECLARE_double(smrate);
DECLARE_bool(sonepoint);

namespace BlockSelector {

  template <int N = 1000000>
  class Sequence : public IndividualBase {
   public:
    Sequence();

    static const uint64_t kSize = N;
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

    const SequenceType& GetSequence() const { return sequence_; }

    void ToMessage(Messages::BlockSelectorSequence* message);
    void FromMessage(const Messages::GameRequest&);

   private:
    SequenceType sequence_;
    uint64_t next_index_;
  };


  template <int N>
  Sequence<N>::Sequence()
    : next_index_(0)
  {
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
                  Tetramino::kTypeRange);
  }

  template <int N>
  void Sequence<N>::MutateFrom(const Sequence &parent) {
    std::generate(sequence_.begin(), sequence_.end(), Utilities::MutateReplaceGenerator(
                  FLAGS_smrate, Tetramino::kTypeRange, parent.sequence_.begin()));
  }

  template <int N>
  void Sequence<N>::CopyFrom(const Sequence &other) {
    sequence_ = other.sequence_;
  }

  template <int N>
  void Sequence<N>::Crossover(const Sequence &one, const Sequence &two) {
    if (FLAGS_sonepoint) {
      std::generate(sequence_.begin(), sequence_.end(),
          Utilities::OnePointCrossoverGenerator(
              one.sequence_.begin(), two.sequence_.begin(), N));
    } else {
      std::generate(sequence_.begin(), sequence_.end(),
          Utilities::UniformCrossoverGenerator(
              one.sequence_.begin(), two.sequence_.begin()));
    }
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

  template <int N>
  void Sequence<N>::ToMessage(Messages::BlockSelectorSequence* message) {
    message->mutable_sequence()->resize(N);
    std::copy(sequence_.begin(), sequence_.end(), message->mutable_sequence()->begin());
  }

  template <int N>
  void Sequence<N>::FromMessage(const Messages::GameRequest& req) {
    std::copy(req.selector_sequence().sequence().begin(),
              req.selector_sequence().sequence().end(), sequence_.begin());
  }

} // namespace BlockSelector

#endif // BLOCKSELECTOR_SEQUENCE_H
