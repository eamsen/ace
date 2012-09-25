// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_VARIABLE_H_
#define SRC_VARIABLE_H_

#include <string>
#include <set>
#include <vector>

namespace ace {

class Variable {
 public:
  Variable(const std::string& name, const int domain_id,
           const std::set<int>& domain);

  void StartTransaction();
  void CommitTransaction();
  void RollbackTransaction();
  void ReduceDomain(const int value_id);
  void RemoveValue(const int value_id);
  // void SwapDomain(std::vector<int>* values);
  // void domain(const std::vector<int>& values);
  int value(const int value_id) const;
  int num_values() const;
  std::vector<int> domain() const;
  std::vector<int> valid_value_ids() const;
  bool valid(const int value_id) const;
  std::string name() const;
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
