// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./constraint.h"
#include <cassert>
#include "./network.h"
#include "./relation.h"

using std::string;
using std::vector;

namespace ace {

Constraint::MatrixCache Constraint::matrix_cache_;

Constraint::Constraint(const string& name, const int relation_id,
                       const vector<int>& scope, const Network& network)
    : scope_(scope),
      relation_id_(relation_id),
      name_(name) {
  InitMatrix(network);
}

void Constraint::InitMatrix(const Network& network) {
  assert(scope_.size() == 2);
  const Relation& relation = network.relation(relation_id_);
  const Variable& var1 = network.variable(scope_[0]);
  const Variable& var2 = network.variable(scope_[1]);
  const int domain_size1 = var1.num_values();
  const int domain_size2 = var2.num_values();
  matrix_offset_ = domain_size1;

  vector<vector<int> > domains;
  domains.push_back(var1.domain());
  domains.push_back(var2.domain());
  auto const key = make_pair(domains, relation_id_);
  DomainsRelationHash hasher;
  assert(hasher(key) == hasher(make_pair(domains, relation_id_)));
  auto cached_matrix = matrix_cache_.find(key);
  if (cached_matrix == matrix_cache_.end()) {
    // Matrix is not cached, create it.
    matrix_.resize(domain_size1 * domain_size2);
    vector<int> values(2);
    for (int v1 = 0; v1 < domain_size1; ++v1) {
      values[0] = var1.value(v1);
      for (int v2 = 0; v2 < domain_size2; ++v2) {
        values[1] = var2.value(v2);
        matrix_[v1 + matrix_offset_ * v2] = relation.Supports(values);
      }
    }
    matrix_cache_[key] = matrix_;
  } else {
    matrix_ = cached_matrix->second;
  }
}

bool Constraint::Supports(const vector<int>& values) const {
  assert(values.size() == 2);
  assert(values[0] >= 0 && values[1] >= 0);
  assert(values[0] + matrix_offset_ * values[1] <
         static_cast<int>(matrix_.size()));
  return matrix_[values[0] + matrix_offset_ * values[1]];
}

bool Constraint::Conflicts(const vector<int>& values) const {
  return !Supports(values);
}

const string& Constraint::name() const {
  return name_;
}

int Constraint::arity() const {
  return scope_.size();
}

int Constraint::scope(const int index) const {
  assert(index >= 0 && index < arity());
  return scope_[index];
}

const vector<int>& Constraint::scope() const {
  return scope_;
}

}  // namespace ace

