#include "individual.h"

#include <cstdlib>
#include <cassert>

const char Individual::kMin = ' ';
const char Individual::kMax = '~';

char Individual::RangeGenerator::operator()() const {
  return min_ + rand() % range_;
}

char Individual::MutateGenerator::operator()() {
  char original = *(original_it_++);

  if (float(rand()) / RAND_MAX < mutation_probability_) {
    return range_gen_();
  } else {
    return original;
  }
}

Individual::Individual(const std::string& target)
    : target_(target),
      cached_fitness_(-1)
{
  data_.resize(target_.size());
}

void Individual::Init() {
  cached_fitness_ = -1;
  std::generate(data_.begin(), data_.end(), RangeGenerator(kMin, kMax));
}

void Individual::MutateFrom(const Individual& parent) {
  cached_fitness_ = -1;
  std::generate(data_.begin(), data_.end(), MutateGenerator(
      1.0/target_.size(), RangeGenerator(kMin, kMax), parent.data_.begin()));
}

void Individual::Mutate() {
  MutateFrom(*this);
}

void Individual::CopyFrom(const Individual& other) {
  assert(other.target_ == target_);

  data_ = other.data_;
  cached_fitness_ = other.cached_fitness_;
}

int Individual::Fitness() const {
  using std::string;

  if (cached_fitness_ != -1)
    return cached_fitness_;

  string::const_iterator data_it = data_.begin();
  string::const_iterator target_it = target_.begin();

  int ret = 0;
  while (data_it != data_.end()) {
    if (*data_it == *target_it)
      ret ++;

    data_it ++;
    target_it ++;
  }

  const_cast<Individual*>(this)->cached_fitness_ = ret;
  return ret;
}

std::ostream& operator <<(std::ostream& stream, const Individual& i) {
  return stream << i.data_;
}
