// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./variable.h"
#include <cassert>

using std::string;
using std::set;
using std::vector;

namespace ace {

Variable::Variable(const string& name, const int domain_id,
                   const set<int>& domain)
    : domain_id_(domain_id),
      domain_(domain.begin(), domain.end()),
      valid_(domain.size(), true),
      name_(name) {}

void Variable::StartTransaction() {
  transactions_.push_back(false);
}

void Variable::CommitTransaction() {
  assert(transactions_.size());
  if (transactions_.back()) {
    // Something has been modified in this transaction.
    assert(valid_snapshots_.size());
    valid_snapshots_.pop_back();
  }
  transactions_.pop_back();
}

void Variable::RollbackTransaction() {
  assert(transactions_.size());
  if (transactions_.back()) {
    // Something has been modified in this transaction.
    assert(valid_snapshots_.size());
    valid_.swap(valid_snapshots_.back());
    valid_snapshots_.pop_back();
  }
  transactions_.pop_back();
}

void Variable::ReduceDomain(const int value) {
  if (transactions_.size() && !transactions_.back()) {
    // We have not made a snapshop during this transaction yet.
    valid_snapshots_.push_back(vector<bool>(valid_.size(), false));
    valid_.swap(valid_snapshots_.back());
    transactions_.back() = true;
  } else {
    valid_.assign(valid_.size(), false);
  }
  valid_[value] = true;
}

void Variable::RemoveValue(const int value_id) {
  if (transactions_.size() && !transactions_.back()) {
    // We have not made a snapshop during this transaction yet.
    valid_snapshots_.push_back(valid_);
    transactions_.back() = true;
  }
  valid_[value_id] = false;
}

string Variable::name() const {
  return name_;
}

int Variable::value(const int value_id) const {
  assert(value_id >= 0 && value_id < num_values());
  return domain_[value_id];
}

int Variable::num_values() const {
  return domain_.size();
}

vector<int> Variable::domain() const {
  vector<int> _domain;
  const int size = valid_.size();
  for (int i = 0; i < size; ++i) {
    if (valid_[i]) {
      _domain.push_back(domain_[i]);
    }
  }
  return _domain;
}

vector<int> Variable::valid_value_ids() const {
  vector<int> valid_ids;
  const int size = valid_.size();
  for (int i = 0; i < size; ++i) {
    if (valid_[i]) {
      valid_ids.push_back(i);
    }
  }
  return valid_ids;
}

bool Variable::valid(const int value_id) const {
  assert(value_id >= 0 && value_id < num_values());
  return valid_[value_id];
}

int Variable::domain_id() const {
  return domain_id_;
}

}  // namespace ace

