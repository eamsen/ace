// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./backtrack-solver.h"
#include <cassert>
#include <vector>
#include <set>
#include <limits>
#include "./clock.h"
#include "./network.h"
#include "./variable-ordering.h"

using std::vector;
using std::set;
using base::Clock;
using std::numeric_limits;
using std::min;

namespace ace {

BacktrackSolver::BacktrackSolver(Network* network)
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

bool BacktrackSolver::Solve() {
  Reset();
  begin_clock_ = Clock();
  Assignment assignment(network_);
  SolveRec(&assignment);
  duration_ = Clock() - begin_clock_;
  return solutions_.size();
}

bool BacktrackSolver::SolveRec(Assignment* assignment) {
  if (assignment->Complete()) {
    // Solution found.
    assert(assignment->Consistent());
    solutions_.push_back(*assignment);
    return solutions_.size() >= max_num_solutions_;
  } else if (Clock() - begin_clock_ > time_limit_) {
    // Time limit reached.
    return false;
  }
  // Select the next variable according to the ordering.
  const int var_id = var_ordering_[assignment->num_assigned()];
  Variable& var = network_.variable(var_id);
  const vector<int> domain = var.valid_value_ids();
  for (auto it = domain.rbegin(), end = domain.rend(); it != end; ++it) {
    const int value = *it;
    ++num_explored_states_;
    assignment->Assign(var_id, value);
    if (assignment->Consistent()) {
      // Start a transaction to track all domain changes.
      network_.StartTransaction();
      // Reduce the domain of the selected variable for the consistency test.
      var.ReduceDomain(value);
      if (preprocessor_.Propagate(var_id) &&
          SolveRec(assignment)) {
        // Commit the transaction, stops tracking changes.
        network_.CommitTransaction();
        return true;
      }
      // Rollback all tracked domain changes.
      network_.RollbackTransaction();
      ++num_backtracks_;
    }
    // Revert the last assignment.
    assignment->Revert();
  }
  return false;
}

bool BacktrackSolver::SolveIterative() {
  Reset();
  const Clock beg;
  // Initialise the assignments stack with the empty assignment.
  vector<Assignment> stack(1, Assignment(network_));
  while (stack.size()) {
    Assignment assignment = stack.back();
    stack.pop_back();
    if (assignment.Complete()) {
      // Solution found.
      assert(assignment.Consistent());
      solutions_.push_back(assignment);
      if (solutions_.size() >= max_num_solutions_) {
        break;
      }
    } else if (Clock() - begin_clock_ > time_limit_) {
      break;
    }
    const int var_id = assignment.SelectUnassigned();
    const Variable& var = network_.variable(var_id);
    const vector<int> domain = var.domain();
    for (auto it = domain.cbegin(), end = domain.cend(); it != end; ++it) {
      ++num_explored_states_;
      const int value = *it;
      assignment.Assign(var_id, value);
      if (assignment.Consistent()) {
        stack.push_back(Assignment(assignment));
      }
      // Revert the last assignment.
      assignment.Revert();
    }
  }
  duration_ = Clock() - beg;
  return solutions_.size();
}

void BacktrackSolver::Reset() {
  duration_ = 0;
  num_backtracks_ = 0;
  num_explored_states_ = 0.0;
}

void BacktrackSolver::variable_ordering(const VariableOrdering& var_ordering) {
  var_ordering_ = var_ordering.CreateOrdering();
}

void BacktrackSolver::time_limit(const Clock::Diff& limit) {
  time_limit_ = min(limit, Solver::kDefTimeLimit);
}

Clock::Diff BacktrackSolver::time_limit() const {
  return time_limit_;
}

void BacktrackSolver::max_num_solutions(const int num) {
  max_num_solutions_ = num > 0 ? num : Solver::kDefMaxNumSolutions;
}

int BacktrackSolver::max_num_solutions() const {
  return max_num_solutions_;
}

const vector<Assignment>& BacktrackSolver::solutions() const {
  return solutions_;
}

double BacktrackSolver::num_explored_states() const {
  return num_explored_states_;
}

int BacktrackSolver::num_backtracks() const {
  return num_backtracks_;
}

Clock::Diff BacktrackSolver::duration() const {
  return duration_;
}

}  // namespace ace

