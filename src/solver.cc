// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./solver.h"
#include <limits>
#include "./clock.h"

using base::Clock;
using std::numeric_limits;

namespace ace {

const Clock::Diff Solver::kDefTimeLimit = numeric_limits<Clock::Diff>::max() -
                                          Clock::kMicroInSec * 10;
const int Solver::kDefMaxNumSolutions = numeric_limits<int>::max();

}  // namespace ace

