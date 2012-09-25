// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_RANDOM_WALK_SOLVER_H_
#define SRC_RANDOM_WALK_SOLVER_H_

#include <vector>
#include "./solver.h"
#include "./clock.h"
#include "./random.h"
#include "./assignment.h"

namespace ace {

// The constraint network.
class Network;

// A constraint network solver based on Gaschnig's backjumping.
class RandomWalkSolver : public Solver {
 public:
  static const base::Clock::Diff kDefTimeLimit;
  static const int kDefMaxNumSolutions;
  static const int kDefMaxNumTries;
  static const int kDefMaxNumFlips;

  // Initialises the solver with the given network.
  explicit RandomWalkSolver(Network* network);

  // Searches for a solution for the network.
  // Returns whether it found a solution.
  bool Solve();

  // Resets the solver meta-information, which is collected during search.
  void Reset();

  void max_num_tries(const int n);
  void max_num_flips(const int n);
  void random_probability(const int p);
  int max_num_tries() const;
  int max_num_flips() const;
  int random_probability() const;

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

  // Returns the number of random walk steps during the last search.
  int num_random_steps() const;

  // Returns the number of greedy steps during the last search.
  int num_greedy_steps() const;

  // Returns the number of backtracks used during the last search.
  int num_backtracks() const;

  // Returns the number of states explored during the last search.
  double num_explored_states() const;

 private:
  Assignment RandomAssignment();
  bool SelectValue(const int var_seq, Assignment* assignment);

  Network& network_;
  std::vector<Assignment> solutions_;
  double num_explored_states_;
  int num_backtracks_;
  int num_random_steps_;
  int num_greedy_steps_;
  base::Clock begin_clock_;
  base::Clock::Diff duration_;
  base::Clock::Diff time_limit_;
  size_t max_num_solutions_;
  int max_num_tries_;
  int max_num_flips_;
  int random_probability_;
  base::RandomGenerator<float> random_gen_;
};

}  // namespace ace
#endif  // SRC_RANDOM_WALK_SOLVER_H_

