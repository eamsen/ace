// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_ASSIGNMENT_H_
#define SRC_ASSIGNMENT_H_

#include <vector>
#include <string>

namespace ace {

// A constraint network.
class Network;

// A (consistent) variable assignment for a constraint network, also called
// partial solution.
class Assignment {
 public:
  // Unassigned variable value.
  static const int kUnassigned;

  // Initialises the assignment to the empty assignment.
  explicit Assignment(const Network& network);

  // Assigns an unassigned variable with given value.
  void Assign(const int variable, const int value);

  // Reassigns a previously assigned variable with given value.
  void Reassign(const int variable, const int value);

  // Reverts the last assignment (not reassignment!).
  void Revert();

  // Returns whether the variable is assigned.
  bool Assigned(const int variable) const;

  // Returns whether the assignment is complete, i.e., all variables are
  // assigned.
  bool Complete() const;

  // Returns whether the assignment is consistent, i.e., the assigned variables
  // satisfy all constraints.
  bool Consistent() const;

  // Returns the ids of the violated constraints.
  std::vector<int> violated_constraints() const;

  // Returns the number of violated constraints.
  int num_violated_constraints() const;

  // Returns the next best yet unassigned variable.
  int SelectUnassigned() const;

  // Returns a string representation.
  std::string Str() const;

  // Returns the value assigned to given variable.
  int value(const int variable) const;

  // Returns the total number of variables, which is equal to the number of
  // variables in the constraint network.
  int size() const;

  // Returns the number of assigned variables.
  int num_assigned() const;

 private:
  // Increases all constraint marks of given variable by one.
  void UpdateConstraintMarks(const int variable);

  // Reverts the mark increases for the last assigned variable.
  void RevertConstraintMarks();

  const Network* network_;
  std::vector<int> values_;
  std::vector<int> constraint_marks_;
  std::vector<int> assigned_;
};

}  // namespace ace
#endif  // SRC_ASSIGNMENT_H_

