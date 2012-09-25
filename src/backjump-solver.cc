// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./backjump-solver.h"
#include <cassert>
#include <vector>
#include <set>
#include <limits>
#include <iostream>
#include "./clock.h"
#include "./network.h"
#include "./variable-ordering.h"

using std::vector;
using std::set;
using base::Clock;
using std::numeric_limits;
using std::min;
using std::max;

namespace ace {

BackjumpSolver::BackjumpSolver(Network* network)
    : Solver(),
      network_(*network),
      preprocessor_(network),
      time_limit_(Solver::kDefTimeLimit),
      max_num_solutions_(Solver::kDefMaxNumSolutions) {
  Reset();
  // Set the lexicographical variable ordering.
  const int num_vars = network_.num_variables();
  var_ordering_.resize(num_vars, 0);
  for (int i = 0; i < num_vars; ++i) {
    var_ordering_[i] = i;
  }
}

bool BackjumpSolver::Solve() {
  static const int kInvalidId = -1;

  Reset();
  begin_clock_ = Clock();

  Assignment assignment(network_);
  const int num_variables = network_.num_variables();
  domains_.resize(num_variables);
  latest_.resize(num_variables + 1, kInvalidId);

  int var_seq = 0;
  {
    int var_id = var_ordering_[var_seq];
    assert(var_id >= 0 && var_id < num_variables);
    const Variable& var = network_.variable(var_id);
    domains_[var_id] = var.domain();
  }
  while (var_seq != kInvalidId && var_seq < num_variables) {
    if (solutions_.size() >= max_num_solutions_ ||
        Clock() - begin_clock_ > time_limit_) {
      // Enough solutions found or time limit reached.
      break;
    }
    if (SelectValue(var_seq, &assignment)) {
      latest_[++var_seq] = kInvalidId;
      if (var_seq == num_variables) {
        // Solution found.
        assert(assignment.Complete() && assignment.Consistent());
        solutions_.push_back(assignment);
      } else if (var_seq != kInvalidId) {
        const int var_id = var_ordering_[var_seq];
        assert(var_id >= 0 && var_id < num_variables);
        const Variable& var = network_.variable(var_id);
        domains_[var_id] = var.domain();
      }
    } else {
      ++num_backtracks_;
      const int jump_height = var_seq - latest_[var_seq];
      assert(jump_height > 0);
      var_seq = latest_[var_seq];
      if (var_seq != kInvalidId) {
        // const int num_ass = assignment.num_assigned();
        for (int i = 0; i < jump_height; ++i) {
          assignment.Revert();
        }
        assert(assignment.num_assigned() == var_seq);
      }
    }
  }

  duration_ = Clock() - begin_clock_;
  return solutions_.size();
}

bool BackjumpSolver::SelectValue(const int var_seq, Assignment* assignment) {
  const int var_id = var_ordering_[var_seq];
  vector<int>& domain = domains_[var_id];
  while (domain.size()) {
    const int value = domain.back();
    Assignment test_assignment(network_);
    test_assignment.Assign(var_id, value);

    bool consistent = test_assignment.Consistent();
    int k = 0;
    while (k < var_seq && consistent) {
      latest_[var_seq] = max(latest_[var_seq], k);
      const int k_var_id = var_ordering_[k];
      const int k_value = assignment->value(k_var_id);
      test_assignment.Assign(k_var_id, k_value);
      consistent = test_assignment.Consistent();
      ++k;
    }

    domain.pop_back();
    if (consistent) {
      assignment->Assign(var_id, value);
      return true;
    }
  }
  return false;
}

void BackjumpSolver::Reset() {
  duration_ = 0;
  num_backtracks_ = 0;
  num_explored_states_ = 0.0;
  latest_.clear();
  domains_.clear();
}

void BackjumpSolver::variable_ordering(const VariableOrdering& var_ordering) {
  var_ordering_ = var_ordering.CreateOrdering();
}

void BackjumpSolver::time_limit(const Clock::Diff& limit) {
  time_limit_ = min(limit, Solver::kDefTimeLimit);
}

Clock::Diff BackjumpSolver::time_limit() const {
  return time_limit_;
}

void BackjumpSolver::max_num_solutions(const int num) {
  max_num_solutions_ = num > 0 ? num : Solver::kDefMaxNumSolutions;
}

int BackjumpSolver::max_num_solutions() const {
  return max_num_solutions_;
}

const vector<Assignment>& BackjumpSolver::solutions() const {
  return solutions_;
}

double BackjumpSolver::num_explored_states() const {
  return num_explored_states_;
}

int BackjumpSolver::num_backtracks() const {
  return num_backtracks_;
}

Clock::Diff BackjumpSolver::duration() const {
  return duration_;
}

}  // namespace ace

