// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_VARIABLE_ORDERING_H_
#define SRC_VARIABLE_ORDERING_H_

#include <vector>
#include "./clock.h"

namespace ace {

class VariableOrdering {
 public:
  virtual ~VariableOrdering() {}
  virtual std::vector<int> CreateOrdering() const = 0;
};

}  // namespace ace
#endif  // SRC_VARIABLE_ORDERING_H_
