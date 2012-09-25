// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_MIN_WIDTH_ORDERING_H_
#define SRC_MIN_WIDTH_ORDERING_H_

#include <vector>
#include "./variable-ordering.h"

namespace ace {

class Network;

class MinWidthOrdering : public VariableOrdering {
 public:
  explicit MinWidthOrdering(const Network& network);
  std::vector<int> CreateOrdering() const;
 private:
  const Network& network_;
};

}  // namespace ace
#endif  // SRC_MIN_WIDTH_ORDERING_H_
