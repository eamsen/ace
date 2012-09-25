// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_RANDOM_H_
#define SRC_RANDOM_H_

#include <random>

namespace base {

template<typename RealType, typename Gen = std::mt19937>
class RandomGenerator {
 public:
  explicit RandomGenerator(const uint32_t seed)
      : seed_(seed) {
    gen_.seed(seed);
  }

  RealType Next() {
    return std::generate_canonical<RealType, 32>(gen_);
  }

  uint32_t seed() const {
    return seed_;
  }

 private:
  uint32_t seed_;
  Gen gen_;
};

}  // namespace base
#endif  // SRC_RANDOM_H_
