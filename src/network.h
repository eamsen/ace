// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_NETWORK_H_
#define SRC_NETWORK_H_

#include <unordered_map>
#include <utility>
#include <string>
#include <vector>
#include "./domain.h"
#include "./relation.h"
#include "./variable.h"
#include "./constraint.h"

namespace ace {

struct PairHash {
  size_t operator()(const std::pair<int, int>& pair) const {
    return (pair.first << 7) ^ pair.second;
  }
};

class Network {
 public:
  Network();

  int AddDomain(const Domain& domain);
  int AddVariable(const Variable& variable);
  int AddRelation(const Relation& relation);
  int AddConstraint(const Constraint& constraint);
  void Finalise();

  void StartTransaction();
  void CommitTransaction();
  void RollbackTransaction();

  const Domain& domain(const int id) const;
  const Variable& variable(const int id) const;
  Variable& variable(const int id);
  const Relation& relation(const int id) const;
  const Constraint& constraint(const std::vector<int>& scope) const;
  int constraint_id(const std::vector<int>& scope) const;
  const Constraint& constraint(const int id) const;
  Constraint& constraint(const int id);
  const std::vector<int>& path_variables(const int constraint_id) const;
  const std::vector<int>& constraints(const int variable) const;
  const std::vector<Constraint>& constraints() const;
  std::string name() const;
  void name(const std::string& name);
  int num_variables() const;
  int num_constraints() const;
  double num_states() const;

  // Returns a string representation according to exercise specification.
  std::string UniStr() const;

 private:
  void AddVarConstraints(const int constraint_id);
  void AddScopeConstraint(const int constraint_id);
  void AddPathVariables(const int constraint_id);

  std::vector<Domain> domains_;
  std::vector<Variable> variables_;
  std::vector<Relation> relations_;
  std::vector<Constraint> constraints_;
  std::vector<std::vector<int> > var_constraints_;
  std::unordered_map<std::pair<int, int>, int, PairHash> scope_constraints_;
  std::vector<std::vector<int> > path_variables_;
  double num_states_;
  std::string name_;
};

}  // namespace ace
#endif  // SRC_NETWORK_H_
