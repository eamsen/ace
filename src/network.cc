// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./network.h"
#include <unordered_set>
#include <cassert>
#include <set>
#include <sstream>
#include <algorithm>

using std::unordered_set;
using std::string;
using std::vector;
using std::set;
using std::stringstream;
using std::min;
using std::max;
using std::pair;
using std::make_pair;

namespace ace {

Network::Network()
    : num_states_(0.0) {}

string Network::name() const {
  return name_;
}

void Network::name(const string& name) {
  name_ = name;
}

int Network::AddDomain(const Domain& domain) {
  domains_.push_back(domain);
  return domains_.size() - 1;
}

int Network::AddVariable(const Variable& variable) {
  variables_.push_back(variable);
  var_constraints_.push_back(vector<int>());
  return variables_.size() - 1;
}

int Network::AddRelation(const Relation& relation) {
  relations_.push_back(relation);
  return relations_.size() - 1;
}

int Network::AddConstraint(const Constraint& constraint) {
  constraints_.push_back(constraint);
  const int constraint_id = constraints_.size() - 1;
  AddVarConstraints(constraint_id);
  AddScopeConstraint(constraint_id);
  return constraint_id;
}

void Network::AddVarConstraints(const int constraint_id) {
  const Constraint& c = constraint(constraint_id);
  const vector<int>& scope = c.scope();
  for (auto it = scope.begin(); it != scope.end(); ++it) {
    const int scope_var = *it;
    var_constraints_[scope_var].push_back(constraint_id);
  }
}

void Network::AddScopeConstraint(const int constraint_id) {
  const Constraint& c = constraint(constraint_id);
  pair<int, int> key(min(c.scope(0), c.scope(1)),
                     max(c.scope(0), c.scope(1)));
  scope_constraints_[key] = constraint_id;
}

void Network::Finalise() {
  // Collect transitive variables on a path between two other variables on the
  // primal graph. Required for PC2.
  path_variables_.resize(num_constraints());
  for (int i = 0; i < num_constraints(); ++i) {
    if (constraint(i).arity() == 2) {
      AddPathVariables(i);
    }
  }
  // Calculate the number of states.
  num_states_ = 1.0;
  for (auto it = variables_.cbegin(), end = variables_.cend();
       it != end; ++it) {
    const int num_values = it->domain().size();
    num_states_ *= num_values;
  }
}

void Network::AddPathVariables(const int constraint_id) {
  const Constraint& con = constraint(constraint_id);
  assert(con.arity() == 2);
  const int var1_id = con.scope(0);
  const int var2_id = con.scope(1);
  const vector<int>& var1_cons = constraints(var1_id);
  const vector<int>& var2_cons = constraints(var2_id);
  set<int> var1_neighs;
  for (auto it = var1_cons.begin(), end = var1_cons.end(); it != end; ++it) {
    const Constraint& neigh_con = constraint(*it);
    if (neigh_con.arity() == 2) {
      var1_neighs.insert(neigh_con.scope(0));
      var1_neighs.insert(neigh_con.scope(1));
    }
  }
  unordered_set<int> var2_neighs;
  for (auto it = var2_cons.begin(), end = var2_cons.end(); it != end; ++it) {
    const Constraint& neigh_con = constraint(*it);
    if (neigh_con.arity() == 2) {
      var2_neighs.insert(neigh_con.scope(0));
      var2_neighs.insert(neigh_con.scope(1));
    }
  }
  vector<int> path_vars;
  path_vars.reserve(min(var1_neighs.size(), var2_neighs.size()));
  auto const end2 = var2_neighs.end();
  for (auto it = var1_neighs.begin(), end = var1_neighs.end();
       it != end; ++it) {
    const int var_id = *it;
    if (var2_neighs.find(var_id) != end2 &&
        var_id != var1_id &&
        var_id != var2_id) {
      path_vars.push_back(var_id);
    }
  }
  path_variables_[constraint_id].swap(path_vars);
}

void Network::StartTransaction() {
  for (auto it = variables_.begin(), end = variables_.end(); it != end; ++it) {
    Variable& var = *it;
    var.StartTransaction();
  }
}

void Network::CommitTransaction() {
  for (auto it = variables_.begin(), end = variables_.end(); it != end; ++it) {
    Variable& var = *it;
    var.CommitTransaction();
  }
}

void Network::RollbackTransaction() {
  for (auto it = variables_.begin(), end = variables_.end(); it != end; ++it) {
    Variable& var = *it;
    var.RollbackTransaction();
  }
}

const Domain& Network::domain(const int id) const {
  assert(id >= 0 && id < static_cast<int>(domains_.size()));
  return domains_[id];
}

const Variable& Network::variable(const int id) const {
  assert(id >= 0 && id < static_cast<int>(variables_.size()));
  return variables_[id];
}

Variable& Network::variable(const int id) {
  assert(id >= 0 && id < static_cast<int>(variables_.size()));
  return variables_[id];
}

const Relation& Network::relation(const int id) const {
  assert(id >= 0 && id < static_cast<int>(relations_.size()));
  return relations_[id];
}

const Constraint& Network::constraint(const vector<int>& scope) const {
  assert(scope.size() == 2);
  pair<int, int> key(min(scope[0], scope[1]),
                     max(scope[0], scope[1]));
  const int constraint_id = scope_constraints_.at(key);
  return constraint(constraint_id);
}

int Network::constraint_id(const vector<int>& scope) const {
  assert(scope.size() == 2);
  pair<int, int> key(min(scope[0], scope[1]),
                     max(scope[0], scope[1]));
  return scope_constraints_.at(key);
}

const Constraint& Network::constraint(const int id) const {
  assert(id >= 0 && id < static_cast<int>(constraints_.size()));
  return constraints_[id];
}

Constraint& Network::constraint(const int id) {
  assert(id >= 0 && id < static_cast<int>(constraints_.size()));
  return constraints_[id];
}

const vector<Constraint>& Network::constraints() const {
  return constraints_;
}

const vector<int>& Network::constraints(const int variable) const {
  assert(variable >= 0 && variable < static_cast<int>(variables_.size()));
  return var_constraints_[variable];
}

const vector<int>& Network::path_variables(const int constraint_id) const {
  assert(constraint_id >= 0 && constraint_id < num_constraints());
  return path_variables_[constraint_id];
}

int Network::num_variables() const {
  return variables_.size();
}

int Network::num_constraints() const {
  return constraints_.size();
}

double Network::num_states() const {
  return num_states_;
}

string Network::UniStr() const {
  stringstream ss;
  // Variables.
  for (auto it = variables_.begin(); it != variables_.end(); ++it) {
    if (it != variables_.begin()) {
      ss << ", ";
    }
    ss << it->name();
  }
  ss << ";\n";
  // Constraints.
  for (auto it = constraints_.begin(); it != constraints_.end(); ++it) {
    const Constraint& c = *it;
    // Scope.
    vector<const vector<int>*> scope_domains;
    for (auto it2 = c.scope().begin(); it2 != c.scope().end(); ++it2) {
      if (it2 != c.scope().begin()) {
        ss << ", ";
      }
      const Variable& v = variables_[*it2];
      ss << v.name();
      // TODO(esawin): scope_domains.push_back(&v.domain());
    }
    ss << " | ";
    // TODO(esawin): ss << Relation::UniStr(c.Conflicting(*this));
    ss << ";\n";
  }
  return ss.str();
}

}  // namespace ace
