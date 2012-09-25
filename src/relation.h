// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_RELATION_H_
#define SRC_RELATION_H_

#include <unordered_set>
#include <string>
#include <vector>
#include <set>

namespace ace {

class Domain;

class Relation {
 public:
  enum Semantics { kSupports, kConflicts };

  typedef std::vector<int> Tuple;

  // Functor used for tuple ordering.
  struct VectorLess {
    bool operator()(const Tuple& lhs, const Tuple& rhs) const;
  };

  struct TupleHash {
    size_t operator()(const Tuple& tuple) const;
  };

  // typedef std::set<Tuple, VectorLess> TupleSet;
  typedef std::unordered_set<Tuple, TupleHash> TupleSet;

  static std::string UniStr(const TupleSet& tuples);

  Relation(const std::string& name, const Semantics semantics,
           const TupleSet& tuples);

  bool Supports(const std::vector<int>& values) const;
  bool Conflicts(const std::vector<int>& values) const;

  void AddConflicting(const Tuple& tuple);

  const TupleSet& tuples() const;
  TupleSet& tuples();

  TupleSet Supporting(const std::vector<const std::vector<int>*>& domains)
                                                                    const;
  TupleSet Conflicting(const std::vector<const std::vector<int>*>& domains)
                                                                     const;
  TupleSet Supporting(const std::vector<std::vector<int> >& domains) const;
  TupleSet Conflicting(const std::vector<std::vector<int> >& domains) const;
  Semantics semantics() const;

  const std::string& name() const;

 private:
  bool supporting_;
  TupleSet tuples_;
  std::string name_;
};

}  // namespace ace
#endif  // SRC_RELATION_H_
