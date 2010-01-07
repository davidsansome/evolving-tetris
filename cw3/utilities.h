#ifndef UTILITIES_H
#define UTILITIES_H

#include <cstdlib>

#include <boost/random/lagged_fibonacci.hpp>
#include <boost/random/variate_generator.hpp>

// Source-compatibility with QPoint and QSize
class Int2 {
 public:
  Int2() {}
  Int2(int x, int y) : x_(x), y_(y) {}

  int x() const { return x_; }
  int y() const { return y_; }
  int width() const { return x_; }
  int height() const { return y_; }

  bool operator == (const Int2& other) const { return x_ == other.x_ && y_ == other.y_; }

 private:
  int x_;
  int y_;
};

namespace Utilities {

  unsigned int RandomSeed();

  static boost::lagged_fibonacci607 global_rng;

  template <typename Container>
  typename Container::value_type Sum(typename Container::const_iterator begin,
                                     typename Container::const_iterator end) {
    typename Container::value_type ret = 0;
    for (auto p = begin ; p != end ; ++p) {
      ret += *p;
    }
    return ret;
  }

  template <typename Container>
  typename Container::value_type Mean(const Container& c) {
    return Sum<Container>(c.begin(), c.end()) / c.size();
  }

  template <typename Container>
  typename Container::value_type Variance(const Container& c) {
    typename Container::value_type m = Mean(c);
    typename Container::value_type diff_sum = 0;
    for (auto p = c.begin() ; p != c.end() ; ++p) {
      diff_sum += pow(*p - m, 2.0);
    }
    return diff_sum;
  }

  template <typename Container>
  typename Container::value_type StandardDeviation(const Container& c) {
    return sqrt(Variance(c) / c.size());
  }

  // These generators are compatiable with std::generate from <algorithms>.
  // They are used to set new values for an individual.

  // Generates random numbers from a range.
  template <typename T>
  class _RangeGenerator {
   public:
    _RangeGenerator(T min, T max) : min_(min), range_(max-min) {}

    T operator()() const {
      return min_ + T(Utilities::global_rng() * (range_+1));
    }

   private:
    T min_;
    T range_;
  };

  template <typename T>
  _RangeGenerator<T> RangeGenerator(T min, T max) {
    return _RangeGenerator<T>(min, max);
  }

  // Mutates values with probability p by multiplying it by a random number
  // taken from the given distribution
  template <typename iterator_type, typename distribution_type>
  class _MutateGenerator {
   public:
    typedef typename std::iterator_traits<iterator_type>::value_type value_type;

    _MutateGenerator(double p, distribution_type& dist, iterator_type original_it)
        : p_(p), original_it_(original_it), dist_(dist) {}

    value_type operator()() {
      double r = Utilities::global_rng();
      if (r < p_)
        return double(*(original_it_++)) * dist_(Utilities::global_rng);
      else
        return *(original_it_++);
    }

   private:
    double p_;
    iterator_type original_it_;
    distribution_type& dist_;
  };

  template <typename iterator_type, typename distribution_type>
  _MutateGenerator<iterator_type, distribution_type> MutateGenerator(double p, distribution_type& dist, iterator_type original_it) {
    return _MutateGenerator<iterator_type, distribution_type>(p, dist, original_it);
  }

  // Mutates values with probablity p by replacing with a number from gen
  template <typename iterator_type, typename replace_gen_type>
  class _MutateReplaceGenerator {
   public:
    typedef typename std::iterator_traits<iterator_type>::value_type value_type;

    _MutateReplaceGenerator(double p, replace_gen_type& gen, iterator_type original_it)
        : p_(p), original_it_(original_it), gen_(gen) {}

    value_type operator()() {
      double r = Utilities::global_rng();
      value_type ret = *(original_it_++);
      if (r < p_)
        return gen_();
      return ret;
    }

   private:
    double p_;
    iterator_type original_it_;
    replace_gen_type& gen_;
  };

  template <typename iterator_type, typename replace_gen_type>
  _MutateReplaceGenerator<iterator_type, replace_gen_type> MutateReplaceGenerator(double p, replace_gen_type& gen, iterator_type original_it) {
    return _MutateReplaceGenerator<iterator_type, replace_gen_type>(p, gen, original_it);
  }

  // Uniform crossover with two parents
  template <typename iterator_type>
  class _UniformCrossoverGenerator {
   public:
    typedef typename std::iterator_traits<iterator_type>::value_type value_type;

    _UniformCrossoverGenerator(iterator_type a, iterator_type b)
      : a_(a), b_(b) {}

    value_type operator()() {
      value_type ret = (Utilities::global_rng() > 0.5) ? *a_ : *b_;

      ++ a_; ++ b_;
      return ret;
    }

   private:
    iterator_type a_;
    iterator_type b_;
  };

  template <typename iterator_type>
  _UniformCrossoverGenerator<iterator_type> UniformCrossoverGenerator(iterator_type a, iterator_type b) {
    return _UniformCrossoverGenerator<iterator_type>(a,b);
  }

  // One point crossover with two parents
  template <typename iterator_type>
  class _OnePointCrossoverGenerator {
   public:
    typedef typename std::iterator_traits<iterator_type>::value_type value_type;

    _OnePointCrossoverGenerator(iterator_type a, iterator_type b, size_t size)
      : a_(a), b_(b), i_(0) {
      crossover_point_ = Utilities::global_rng() * (size+1);
    }

    value_type operator()() {
      value_type ret = (i_++ > crossover_point_) ? *b_ : *a_;

      ++ a_; ++ b_;
      return ret;
    }

   private:
    iterator_type a_;
    iterator_type b_;
    size_t i_;
    size_t crossover_point_;
  };

  template <typename iterator_type>
  _OnePointCrossoverGenerator<iterator_type> OnePointCrossoverGenerator(iterator_type a, iterator_type b, size_t size) {
    return _OnePointCrossoverGenerator<iterator_type>(a,b,size);
  }
}

#endif // UTILITIES_H
