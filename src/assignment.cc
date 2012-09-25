// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./assignment.h"
#include <cassert>
#include <limits>
#include <sstream>
#include "./network.h"

using std::string;
using std::stringstream;
using std::vector;
using std::numeric_limits;

namespace ace {

const int Assignment::kUnassigned = numeric_limits<int>::min();

Assignment::Assignment(const Network& network)
    : network_(&network),
      values_(network.num_variables(), kUnassigned),
      constraint_marks_(network.num_constraints(), 0) {
  assigned_.reserve(values_.size());
}

void Assignment::Assign(const int variable, const int value) {
  assert(variable >= 0 && variable < static_cast<int>(values_.size()));
  assert(!Assigned(variable));
  assert(variable != kUnassigned);
  values_[variable] = value;
  assigned_.push_back(variable);
  UpdateConstraintMarks(variable);
}

void Assignment::Reassign(const int variable, const int value) {
  assert(variable >= 0 && variable < static_cast<int>(values_.size()));
  assert(Assigned(variable));
  assert(variable != kUnassigned);
  values_[variable] = value;
}

void Assignment::Revert() {
  assert(num_assigned());
  RevertConstraintMarks();
  values_[assigned_.back()] = kUnassigned;
  assigned_.pop_back();
}

bool Assignment::Assigned(const int variable) const {
  assert(variable >= 0 && variable < static_cast<int>(values_.size()));
  return values_[variable] != kUnassigned;
}

bool Assignment::Complete() const {
  return num_assigned() == size();
}

bool Assignment::Consistent() const {
  const int num_constraints = constraint_marks_.size();
  for (int i = 0; i < num_constraints; ++i) {
    const int constraint_id = i;
    const Constraint& c = network_->constraint(constraint_id);
    const int arity = c.arity();
    if (constraint_marks_[constraint_id] == arity) {
      vector<int> values(arity, 0);
      for (int scope_i = 0; scope_i < arity; ++scope_i) {
        const int var = c.scope(scope_i);
        values[scope_i] = values_[var];
      }
      if (!c.Supports(values)) {
        return false;
      }
    }
  }
  return true;
}

vector<int> Assignment::violated_constraints() const {
  const int num_constraints = constraint_marks_.size();
  vector<int> violated;
  for (int i = 0; i < num_constraints; ++i) {
    const int constraint_id = i;
    const Constraint& c = network_->constraint(constraint_id);
    const int arity = c.arity();
    if (constraint_marks_[constraint_id] == arity) {
      vector<int> values(arity, 0);
      for (int scope_i = 0; scope_i < arity; ++scope_i) {
        const int var = c.scope(scope_i);
        values[scope_i] = values_[var];
      }
      if (!c.Supports(values)) {
        violated.push_back(constraint_id);
      }
    }
  }
  return violated;
}

int Assignment::num_violated_constraints() const {
  return violated_constraints().size();
}

int Assignment::SelectUnassigned() const {
  assert(!Complete());
  int i = 0;
  while (values_[i] != kUnassigned) {
    ++i;
    assert(i < size());
  }
  assert(values_[i] == kUnassigned);
  return i;
}

string Assignment::Str() const {
  stringstream ss;
  for (int i = 0; i < size(); ++i) {
    if (i != 0) {
      ss << ", ";
    }
    const Variable& var = network_->variable(i);
    ss << var.name() << ": " << var.value(values_[i]);
  }
  return ss.str();
}

void Assignment::UpdateConstraintMarks(const int variable) {
  const vector<int>& constraints = network_->constraints(variable);
  for (auto it = constraints.cbegin(), end = constraints.cend();
       it != end; ++it) {
    const int constraint_id = *it;
    ++constraint_marks_[constraint_id];
  }
}

void Assignment::RevertConstraintMarks() {
  const int variable = assigned_.back();
  const vector<int>& constraints = network_->constraints(variable);
  for (auto it = constraints.cbegin(), end = constraints.cend();
       it != end; ++it) {
    const int constraint_id = *it;
    --constraint_marks_[constraint_id];
  }
}

int Assignment::value(const int variable) const {
  assert(Assigned(variable));
  return values_[variable];
}

int Assignment::size() const {
  return values_.size();
}

int Assignment::num_assigned() const {
  return assigned_.size();
}

}  // namespace ace

