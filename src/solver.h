// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_SOLVER_H_
#define SRC_SOLVER_H_

#include <vector>
#include "./clock.h"
#include "./assignment.h"
#include "./ac3.h"

namespace ace {

// A constraint network solver base class. Handles general solver options.
class Solver {
 public:
  static const base::Clock::Diff kDefTimeLimit;
  static const int kDefMaxNumSolutions;

  virtual ~Solver() {}

  // Searches for a solution for the network.
  // Returns whether it found a solution.
  virtual bool Solve() = 0;

  // Resets the solver meta-information, which is collected during search.
  virtual void Reset() = 0;

  // Sets the time limit for the search. Search will be terminated if the time
  // limit is exceeded, returning false.
  virtual void time_limit(const base::Clock::Diff& limit) = 0;

  // Returns the set time limit.
  virtual base::Clock::Diff time_limit() const = 0;

  // Sets the maximum number of solutions to be searched for.
  virtual void max_num_solutions(const int num) = 0;

  // Returns the set maximum number of solutions to be searched for.
  virtual int max_num_solutions() const = 0;

  // Returns a const reference to the solutions found by the last search.
  virtual const std::vector<Assignment>& solutions() const = 0;

  // Returns the duration of the last search in microseconds.
  virtual base::Clock::Diff duration() const = 0;

  // Returns the number of backtracks used during the last search.
  virtual int num_backtracks() const = 0;

  // Returns the number of states explored during the last search.
  virtual double num_explored_states() const = 0;
};

}  // namespace ace
#endif  // SRC_SOLVER_H_

