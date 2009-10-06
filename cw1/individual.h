#ifndef INDIVIDUAL_H
#define INDIVIDUAL_H

#include <boost/array.hpp>

class Individual {
  friend std::ostream& operator <<(std::ostream& stream, const Individual& i);

 public:
  Individual(const std::string& target_);

  // Initialise the Individual in different ways
  void Init(); // Random
  void MutateFrom(const Individual& parent);
  void CopyFrom(const Individual& other);
  void Crossover(const Individual& one, const Individual& two);

  int Fitness() const;
  void Mutate();

  bool operator ==(const Individual& o) const { return data_ == o.data_; }
  bool operator !=(const Individual& o) const { return data_ != o.data_; }
  bool operator ==(const std::string& o) const { return data_ == o; }
  bool operator !=(const std::string& o) const { return data_ != o; }

 private:
  class RangeGenerator {
   public:
    RangeGenerator(char min, char max) : min_(min), range_(max-min) {}
    char operator()() const;

   private:
    char min_;
    char range_;
  };

  class MutateGenerator {
   public:
    MutateGenerator(float mutation_probability, RangeGenerator range_gen,
                    std::string::const_iterator original_it)
        : mutation_probability_(mutation_probability),
          range_gen_(range_gen),
          original_it_(original_it) {}
    char operator()();

   private:
    float mutation_probability_;
    RangeGenerator range_gen_;
    std::string::const_iterator original_it_;
  };

  static const char kMin;
  static const char kMax;

  std::string target_;
  int cached_fitness_;
  std::string data_;
};

std::ostream& operator <<(std::ostream& stream, const Individual& i);

#endif // INDIVIDUAL_H
