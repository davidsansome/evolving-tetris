#ifndef UTILITIES_H
#define UTILITIES_H

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

}

#endif // UTILITIES_H
