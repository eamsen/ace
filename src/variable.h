// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_VARIABLE_H_
#define SRC_VARIABLE_H_

#include <string>
#include <set>
#include <vector>

namespace ace {

// Representation of a constraint variable.
class Variable {
 public:
  // Initialized the variable with given name, reference domain id and valid
  // domain values.
  Variable(const std::string& name, const int domain_id,
           const std::set<int>& domain);

  // Starts a transaction. All domain modifications are recorded and reversable
  // during the transaction.
  void StartTransaction();
  
  // Commits the transaction making the domain modifications of the current
  // transactions persistent and irreversable.
  // Closes the active transaction.
  void CommitTransaction();

  // Rolls back all domain modifications of the currently active transaction.
  // Closes the active transaction.
  void RollbackTransaction();
 
  // Reduces the valid domain value ids to given value id.
  void ReduceDomain(const int value_id);

  // Removes the given value id from the valid domain value ids. 
  void RemoveValue(const int value_id);

  // Returns the value for given value id.
  int value(const int value_id) const;

  // Returns the number of valid values.
  int num_values() const;

  // Returns a copy of the valid domain values.
  std::vector<int> domain() const;

  // Returns a copy of the valid domain value ids.
  std::vector<int> valid_value_ids() const;

  // Returns whether the given value is valid in the domain (by id).
  bool valid(const int value_id) const;

  // Returns the name of the variable.
  std::string name() const;

  // Returns the reference domain id of the variable.
  int domain_id() const;

 private:
  int domain_id_;
  std::vector<int> domain_;
  std::vector<bool> valid_;
  std::vector<bool> transactions_;
  std::vector<std::vector<bool> > valid_snapshots_;
  std::string name_;
};

}  // namespace ace
#endif  // SRC_VARIABLE_H_
