// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./random-walk-solver.h"
#include <cassert>
#include <vector>
#include <set>
#include <limits>
#include <iostream>
#include "./clock.h"
#include "./network.h"
#include "./variable-ordering.h"
#include "./random.h"

using std::vector;
using std::set;
using base::Clock;
using base::RandomGenerator;
using std::numeric_limits;
using std::min;
using std::max;
using std::swap;

namespace ace {

const int RandomWalkSolver::kDefMaxNumTries = 100;
const int RandomWalkSolver::kDefMaxNumFlips = 100;

RandomWalkSolver::RandomWalkSolver(Network* network)
    : Solver(),
      network_(*network),
      time_limit_(Solver::kDefTimeLimit),
      max_num_solutions_(Solver::kDefMaxNumSolutions),
      random_gen_(Clock().value() ^ 0x5a8aff10) {
  Reset();
}

bool RandomWalkSolver::Solve() {
  Reset();
  begin_clock_ = Clock();

  const int num_constraints = network_.num_constraints();
  Assignment best_assignment = RandomAssignment();
  const int best_score = best_assignment.num_violated_constraints();
  for (int t = 0; t < max_num_tries_; ++t) {
    if (solutions_.size() >= max_num_solutions_) {
      break;
    }
    Assignment assignment = RandomAssignment();
    const int score = assignment.num_violated_constraints();
    if (score < best_score) {
      best_assignment = assignment;
    }
    for (int f = 0; f < max_num_flips_; ++f) {
      if (assignment.Consistent()) {
        // Solution found.
        solutions_.push_back(assignment);
        break;
      }
      const vector<int> violated_cons = assignment.violated_constraints();
      // Select a random violated constraint.
      const int constraint_id = violated_cons[random_gen_.Next() *
                                              violated_cons.size()];
      const Constraint& constraint = network_.constraint(constraint_id);
      const int scope_size = constraint.scope().size();
      const int random_step = random_gen_.Next() * 100 < random_probability_;
      int variable = 0;
      int value = assignment.value(variable);
      if (random_step) {
        // Random walk step.
        ++num_random_steps_;
        const float var_chance = 1.0f / scope_size;
        while (value == assignment.value(variable)) {
          for (int i = 0; i < scope_size; ++i) {
            if (random_gen_.Next() * scope_size < var_chance) {
              const int var_id = constraint.scope(i);
              const Variable& var = network_.variable(var_id);
              const vector<int>& domain = var.domain();
              const int num_values = domain.size();
              if (num_values) {
                variable = var_id;
                value = domain[random_gen_.Next() * num_values];
                break;
              }
            }
          }
        }
      } else {
        // Greedy step.
        ++num_greedy_steps_;
        // Used to break the bias for early variable-value pairs.
        const int even = num_greedy_steps_ % 2 == 0;
        int best_score = num_constraints;
        for (int i = 0; i < scope_size; ++i) {
          const int var_id = constraint.scope(i);
          const int org_value = assignment.value(var_id);
          const Variable& var = network_.variable(var_id);
          const vector<int>& domain = var.domain();
          for (auto it = domain.cbegin(), end = domain.cend();
               it != end; ++it) {
            assignment.Reassign(var_id, *it);
            const int new_score = assignment.num_violated_constraints();
            if (new_score - even < best_score) {
              best_score = new_score;
              variable = var_id;
              value = *it;
            }
          }
          assignment.Reassign(var_id, org_value);
        }
      }
      assignment.Reassign(variable, value);
    }
  }

  duration_ = Clock() - begin_clock_;
  return solutions_.size();
}

Assignment RandomWalkSolver::RandomAssignment() {
  Assignment assignment(network_);
  const int num_variables = network_.num_variables();
  for (int v = 0; v < num_variables; ++v) {
    const Variable& var = network_.variable(v);
    const vector<int>& domain = var.domain();
    const int value = domain[random_gen_.Next() * domain.size()];
    assignment.Assign(v, value);
  }
  return assignment;
}

void RandomWalkSolver::Reset() {
  duration_ = 0;
  num_backtracks_ = 0;
  num_explored_states_ = 0.0;
  num_random_steps_ = 0;
  num_greedy_steps_ = 0;
}

void RandomWalkSolver::max_num_tries(const int n) {
  max_num_tries_ = n > 0 ? n : kDefMaxNumTries;
}

void RandomWalkSolver::max_num_flips(const int n) {
  max_num_flips_ = n > 0 ? n : kDefMaxNumFlips;
}

void RandomWalkSolver::random_probability(const int p) {
  random_probability_ = max(0, p);
}

int RandomWalkSolver::max_num_tries() const {
  return max_num_tries_;
}

int RandomWalkSolver::max_num_flips() const {
  return max_num_flips_;
}

int RandomWalkSolver::random_probability() const {
  return random_probability_;
}

void RandomWalkSolver::time_limit(const Clock::Diff& limit) {
  time_limit_ = min(limit, Solver::kDefTimeLimit);
}

Clock::Diff RandomWalkSolver::time_limit() const {
  return time_limit_;
}

void RandomWalkSolver::max_num_solutions(const int num) {
  max_num_solutions_ = num > 0 ? num : Solver::kDefMaxNumSolutions;
}

int RandomWalkSolver::max_num_solutions() const {
  return max_num_solutions_;
}

const vector<Assignment>& RandomWalkSolver::solutions() const {
  return solutions_;
}

double RandomWalkSolver::num_explored_states() const {
  return num_explored_states_;
}

int RandomWalkSolver::num_random_steps() const {
  return num_random_steps_;
}

int RandomWalkSolver::num_greedy_steps() const {
  return num_greedy_steps_;
}

int RandomWalkSolver::num_backtracks() const {
  return num_backtracks_;
}

Clock::Diff RandomWalkSolver::duration() const {
  return duration_;
}

}  // namespace ace

