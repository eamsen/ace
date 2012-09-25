// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./ac3.h"
#include <cassert>
#include <vector>
#include "./network.h"

using std::vector;
using std::queue;
using base::Clock;

namespace ace {

Ac3::Ac3(Network* network)
    : Preprocessor("AC3"),
      network_(network) {
  Reset();
}

bool Ac3::Propagate(const int _var_id) {
  Reset();
  Clock beg;

  Queue queue;
  const vector<int>& cons = network_->constraints(_var_id);
  for (auto it = cons.cbegin(), end = cons.cend(); it != end; ++it) {
    const int con_id = *it;
    const Constraint& constraint = network_->constraint(con_id);
    assert(constraint.arity() == 2);
    const int var_index = constraint.scope(0) == _var_id ? 1 : 0;
    queue.push(ReviseItem(con_id, var_index));
  }

  const bool consistent = Enforce(&queue);

  duration_ = Clock() - beg;
  return consistent;
}

bool Ac3::Preprocess() {
  Reset();
  Clock beg;

  Queue queue;
  const int num_constraints = network_->num_constraints();
  for (int c = 0; c < num_constraints; ++c) {
    const Constraint& constraint = network_->constraint(c);
    assert(constraint.arity() == 2);
    queue.push(ReviseItem(c, 0));
    queue.push(ReviseItem(c, 1));
  }

  const bool consistent = Enforce(&queue);

  duration_ = Clock() - beg;
  return consistent;
}

bool Ac3::Enforce(Queue* queue) {
  bool consistent = true;
  while (queue->size()) {
    ++num_iterations_;
    ReviseItem item = queue->front();
    queue->pop();
    if (Revise(item)) {
      const Constraint& constraint = network_->constraint(item.constraint);
      const int var_id = constraint.scope(item.var_index);
      const Variable& var = network_->variable(var_id);
      if (var.domain().empty()) {
        consistent = false;
        break;
      }
      const vector<int>& constraints = network_->constraints(var_id);
      for (auto it = constraints.cbegin(), end = constraints.cend();
           it != end; ++it) {
        const int constraint_id = *it;
        if (constraint_id != item.constraint) {
          const Constraint& c = network_->constraint(constraint_id);
          if (constraint.arity() == 2) {
            const int var2_index = c.scope(0) == var_id ? 1 : 0;
            queue->push(ReviseItem(constraint_id, var2_index));
          }
        }
      }
    }
  }
  return consistent;
}

bool Ac3::Revise(const Ac3::ReviseItem& item) {
  const Constraint& constraint = network_->constraint(item.constraint);
  assert(constraint.arity() == 2);
  const int var_id = constraint.scope(item.var_index);
  const int var2_index = 1 - item.var_index;
  const int var2_id = constraint.scope(var2_index);
  Variable& var = network_->variable(var_id);
  const vector<int> domain = var.valid_value_ids();
  const Variable& var2 = network_->variable(var2_id);
  const vector<int> domain2 = var2.valid_value_ids();
  auto const end2 = domain2.cend();
  int reduction = 0;
  vector<int> values(2);
  for (auto it = domain.cbegin(), end = domain.cend(); it != end; ++it) {
    const int value = *it;
    values[item.var_index] = value;
    bool consistent = false;
    for (auto it2 = domain2.cbegin(); it2 != end2; ++it2) {
      values[var2_index] = *it2;
      if (constraint.Supports(values)) {
        // The value is consistent.
        consistent = true;
        break;
      }
    }
    if (!consistent) {
      ++reduction;
      var.RemoveValue(value);
    }
  }
  num_removed_ += reduction;
  return reduction;
}

void Ac3::Reset() {
  duration_ = 0;
  num_iterations_ = 0;
  num_removed_ = 0;
}

int Ac3::num_iterations() const {
  return num_iterations_;
}

int Ac3::num_processed() const {
  return num_removed_;
}

Clock::Diff Ac3::duration() const {
  return duration_;
}

}  // namespace ace
