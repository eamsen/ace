// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_BACKJUMP_SOLVER_H_
#define SRC_BACKJUMP_SOLVER_H_

#include <vector>
#include "./solver.h"
#include "./clock.h"
#include "./assignment.h"
#include "./ac3.h"

namespace ace {

// The constraint network.
class Network;

// The variable ordering.
class VariableOrdering;

// A constraint network solver based on Gaschnig's backjumping.
class BackjumpSolver : public Solver {
 public:
  static const base::Clock::Diff kDefTimeLimit;
  static const int kDefMaxNumSolutions;

  // Initialises the solver with the given network.
  explicit BackjumpSolver(Network* network);

  // Searches for a solution for the network.
  // Returns whether it found a solution.
  bool Solve();

  // Resets the solver meta-information, which is collected during search.
  void Reset();

  // Sets the variable ordering.
  void variable_ordering(const VariableOrdering& var_ordering);

  // Sets the time limit for the search. Search will be terminated if the time
  // limit is exceeded, returning false.
  void time_limit(const base::Clock::Diff& limit);

  // Returns the set time limit.
  base::Clock::Diff time_limit() const;

  // Sets the maximum number of solutions to be searched for.
  void max_num_solutions(const int num);

  // Returns the set maximum number of solutions to be searched for.
  int max_num_solutions() const;

  // Returns a const reference to the solutions found by the last search.
  const std::vector<Assignment>& solutions() const;

  // Returns the duration of the last search in microseconds.
  base::Clock::Diff duration() const;

  // Returns the number of backtracks used during the last search.
  int num_backtracks() const;

  // Returns the number of states explored during the last search.
  double num_explored_states() const;

 private:
  bool SelectValue(const int var_seq, Assignment* assignment);

  Network& network_;
  Ac3 preprocessor_;
  std::vector<int> var_ordering_;
  std::vector<int> latest_;
  std::vector<std::vector<int> > domains_;
  std::vector<Assignment> solutions_;
  double num_explored_states_;
  int num_backtracks_;
  base::Clock begin_clock_;
  base::Clock::Diff duration_;
  base::Clock::Diff time_limit_;
  size_t max_num_solutions_;
};

}  // namespace ace
#endif  // SRC_BACKJUMP_SOLVER_H_

