// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./relation.h"
#include <cassert>
#include <sstream>
#include <iostream>
#include "./assignment.h"
#include "./domain.h"

using std::string;
using std::vector;
using std::stringstream;
using std::max;
using std::min;

namespace ace {

bool Relation::VectorLess::operator()(const Tuple& lhs,
                                      const Tuple& rhs) const {
  assert(lhs.size() == rhs.size());
  const int size = lhs.size();
  int i = 0;
  while (i < size && lhs[i] == rhs[i]) {
    ++i;
  }
  return i < size && lhs[i] < rhs[i];
}

size_t Relation::TupleHash::operator()(const Tuple& tuple) const {
  const int size = tuple.size();
  size_t h = size ^ 0x550944F8;
  for (int i = 0; i < size; ++i) {
    const int j = i * 3;
    h ^= (tuple[i] << j) ^ (h >> j);
  }
  return h;
}

Relation::Relation(const string& name, const Semantics semantics,
                   const TupleSet& tuples)
    : supporting_(semantics == kSupports),
      tuples_(tuples),
      name_(name) {}

bool Relation::Supports(const vector<int>& values) const {
  const bool matched = tuples_.find(values) != tuples_.end();
  // The relation supports the given tuple iff
  // - tuple is in the relation and it is a supporting relation or
  // - the tuple is not in the relation and it is a conflicting relation.
  return matched == supporting_;
}

bool Relation::Conflicts(const vector<int>& values) const {
  return !Supports(values);
}

void Relation::AddConflicting(const Tuple& tuple) {
  if (supporting_) {
    tuples_.erase(tuple);
  } else {
    tuples_.insert(tuple);
  }
}

const Relation::TupleSet& Relation::tuples() const {
  return tuples_;
}

Relation::TupleSet& Relation::tuples() {
  return tuples_;
}

Relation::TupleSet Relation::Supporting(const std::vector<std::vector<int> >&
                                        domains) const {
  TupleSet supporting;
  vector<int> indices(domains.size(), 0);
  vector<int> values;
  values.reserve(domains.size());
  for (auto it = domains.cbegin(), end = domains.cend(); it != end; ++it) {
    const vector<int>& d = *it;
    assert(d.size());
    values.push_back(*d.begin());
  }
  if (Supports(values)) {
    supporting.insert(values);
  }
  size_t i = 0;
  while (i < values.size()) {
    i = 0u;
    while (i < values.size() && values[i] == domains[i].back()) {
      values[i] = domains[i][(indices[i] = 0)];
      ++i;
    }
    if (i < values.size()) {
      values[i] = domains[i][++indices[i]];
      if (Supports(values)) {
        supporting.insert(values);
      }
    }
  }
  return supporting;
}

Relation::TupleSet Relation::Supporting(const vector<const vector<int>*>&
                                        domains) const {
  TupleSet supporting;
  vector<int> indices(domains.size(), 0);
  vector<int> values;
  values.reserve(domains.size());
  for (auto it = domains.cbegin(), end = domains.cend(); it != end; ++it) {
    const vector<int>& d = **it;
    assert(d.size());
    values.push_back(*d.begin());
  }
  if (Supports(values)) {
    supporting.insert(values);
  }
  size_t i = 0;
  while (i < values.size()) {
    i = 0u;
    while (i < values.size() && values[i] == domains[i]->back()) {
      values[i] = (*domains[i])[(indices[i] = 0)];
      ++i;
    }
    if (i < values.size()) {
      values[i] = (*domains[i])[++indices[i]];
      if (Supports(values)) {
        supporting.insert(values);
      }
    }
  }
  return supporting;
}

Relation::TupleSet Relation::Conflicting(const std::vector<std::vector<int> >&
                                         domains) const {
  TupleSet conflicting;
  vector<int> indices(domains.size(), 0);
  vector<int> values;
  for (auto it = domains.begin(), end = domains.end(); it != end; ++it) {
    const vector<int>& d = *it;
    assert(d.size());
    values.push_back(*d.begin());
  }
  if (Conflicts(values)) {
    conflicting.insert(values);
  }
  size_t i = 0;
  while (i < values.size()) {
    i = 0u;
    while (i < values.size() && values[i] == domains[i].back()) {
      values[i] = domains[i][(indices[i] = 0)];
      ++i;
    }
    if (i < values.size()) {
      values[i] = domains[i][++indices[i]];
      if (Conflicts(values)) {
        conflicting.insert(values);
      }
    }
  }
  return conflicting;
}

Relation::TupleSet Relation::Conflicting(const vector<const vector<int>*>&
                                         domains) const {
  TupleSet conflicting;
  vector<int> indices(domains.size(), 0);
  vector<int> values;
  for (auto it = domains.begin(), end = domains.end(); it != end; ++it) {
    const vector<int>& d = **it;
    assert(d.size());
    values.push_back(*d.begin());
  }
  if (Conflicts(values)) {
    conflicting.insert(values);
  }
  size_t i = 0;
  while (i < values.size()) {
    i = 0u;
    while (i < values.size() && values[i] == domains[i]->back()) {
      values[i] = (*domains[i])[(indices[i] = 0)];
      ++i;
    }
    if (i < values.size()) {
      values[i] = (*domains[i])[++indices[i]];
      if (Conflicts(values)) {
        conflicting.insert(values);
      }
    }
  }
  return conflicting;
}

Relation::Semantics Relation::semantics() const {
  return supporting_ ? kSupports : kConflicts;
}

const string& Relation::name() const {
  return name_;
}

string Relation::UniStr(const Relation::TupleSet& tuples) {
  stringstream ss;
  for (auto it = tuples.begin(); it != tuples.end(); ++it) {
    const Tuple& t = *it;
    if (it != tuples.begin()) {
      ss << ", ";
    }
    for (auto it2 = t.begin(); it2 != t.end(); ++it2) {
      if (it2 != t.begin()) {
        ss << " ";
      }
      ss << *it2;
    }
  }
  return ss.str();
}

}  // namespace ace
