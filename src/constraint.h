// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_CONSTRAINT_H_
#define SRC_CONSTRAINT_H_

#include <unordered_map>
#include <string>
#include <vector>
#include <utility>

namespace ace {

class Network;

struct DomainsRelationHash {
  size_t operator()(const std::pair<std::vector<std::vector<int> >, int>& pair)
                    const {
    size_t h = 0x5391;
    for (auto it = pair.first.cbegin(), end = pair.first.cend();
         it != end; ++it) {
      const std::vector<int>& domain = *it;
      for (auto it2 = domain.cbegin(), end2 = domain.cend();
           it2 != end2; ++it2) {
        h ^= (h * *it2 << 7) ^ (h * *it2);
      }
    }
    return h ^ pair.second;
  }
};

struct DomainsRelationEqual {
  bool operator()(const std::pair<std::vector<std::vector<int> >, int>& lhs,
                  const std::pair<std::vector<std::vector<int> >, int>& rhs)
                  const {
    if (lhs.second != rhs.second ||
        lhs.first.size() != rhs.first.size()) {
      return false;
    }
    const int size = lhs.first.size();
    for (int i = 0; i < size; ++i) {
      const std::vector<int>& lhs_domain = lhs.first[i];
      const std::vector<int>& rhs_domain = rhs.first[i];
      if (lhs_domain != rhs_domain) {
        return false;
      }
    }
    return true;
  }
};

// A constraint for a constraint network.
class Constraint {
 public:
  // Initialises the constraint given its name, its relation and its scope.
  Constraint(const std::string& name, const int relation_id,
             const std::vector<int>& scope, const Network& network);

  bool Supports(const std::vector<int>& values) const;
  bool Conflicts(const std::vector<int>& values) const;

  // Returns the name of the constraint.
  const std::string& name() const;

  // Returns the scope size of the constraint.
  int arity() const;

  // Returns the variable id at given scope index.
  int scope(const int index) const;

  // Returns a const reference to the scope (involved variables).
  const std::vector<int>& scope() const;

 private:
  void InitMatrix(const Network& network);

  typedef std::unordered_map<std::pair<std::vector<std::vector<int> >, int>,
                             std::vector<bool>,
                             DomainsRelationHash,
                             DomainsRelationEqual> MatrixCache;

  static MatrixCache matrix_cache_;

  std::vector<int> scope_;
  std::vector<bool> matrix_;
  int matrix_offset_;
  int relation_id_;
  std::string name_;
};

}  // namespace ace
#endif  // SRC_CONSTRAINT_H_
