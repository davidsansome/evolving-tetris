#ifndef UTILITIES_H
#define UTILITIES_H

#include <cstdlib>

#include <boost/type_traits/remove_pointer.hpp>

// Source-compatibility with QPoint and QSize
class Int2 {
 public:
  Int2() {}
  Int2(int x, int y) : x_(x), y_(y) {}

  int x() const { return x_; }
  int y() const { return y_; }
  int width() const { return x_; }
  int height() const { return y_; }

 private:
  int x_;
  int y_;
};

namespace Utilities {
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
  class RangeGenerator {
   public:
    RangeGenerator(T min, T max) : min_(min), range_(max-min) {}

    T operator()() const {
      return min_ + T((double(rand()) / RAND_MAX) * range_);
    }

   private:
    T min_;
    T range_;
  };

  // Mutates values with probability p by multiplying it by a random number
  // taken from gen
  template <typename iterator_type, typename random_gen_type>
  class MutateGenerator {
   public:
    typedef typename boost::remove_pointer<iterator_type>::type value_type;

    MutateGenerator(double p, random_gen_type* gen, iterator_type original_it)
        : p_(p), original_it_(original_it), gen_(gen) {}

    value_type operator()() {
      double r = double(rand()) / RAND_MAX;
      if (r < p_)
        return double(*(original_it_++)) * (*gen_)();
      else
        return *(original_it_++);
    }

   private:
    double p_;
    iterator_type original_it_;
    random_gen_type* gen_;
  };

  // Mutates values with probablity p by replacing with a number from gen
  template <typename iterator_type, typename replace_gen_type>
  class MutateReplaceGenerator {
   public:
    typedef typename boost::remove_pointer<iterator_type>::type value_type;

    MutateReplaceGenerator(double p, replace_gen_type* gen, iterator_type original_it)
        : p_(p), original_it_(original_it), gen_(gen) {}

    value_type operator()() {
      double r = double(rand()) / RAND_MAX;
      value_type ret = *(original_it_++);
      if (r < p_)
        return (*gen_)();
      return ret;
    }

   private:
    double p_;
    iterator_type original_it_;
    replace_gen_type* gen_;
  };

  template <typename iterator_type>
  class CrossoverGenerator {
   public:
    typedef typename boost::remove_pointer<iterator_type>::type value_type;

    CrossoverGenerator(iterator_type a, iterator_type b)
      : a_(a), b_(b) {}

    value_type operator()() {
      value_type ret = (rand() % 2) ? *a_ : *b_;

      ++ a_; ++ b_;
      return ret;
    }

   private:
    iterator_type a_;
    iterator_type b_;
  };

  unsigned int RandomSeed();
}

#endif // UTILITIES_H
