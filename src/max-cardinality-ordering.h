// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_MAX_CARDINALITY_ORDERING_H_
#define SRC_MAX_CARDINALITY_ORDERING_H_

#include <vector>
#include "./variable-ordering.h"

namespace ace {

class Network;

class MaxCardinalityOrdering : public VariableOrdering {
 public:
  explicit MaxCardinalityOrdering(const Network& network);
  std::vector<int> CreateOrdering() const;
 private:
  const Network& network_;
};

}  // namespace ace
#endif  // SRC_MAX_CARDINALITY_ORDERING_H_
