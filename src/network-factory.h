// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_NETWORK_FACTORY_H_
#define SRC_NETWORK_FACTORY_H_

#include <unordered_map>
#include <utility>
#include <string>
#include <vector>
#include "./clock.h"
#include "./relation.h"

namespace ace {

namespace parse { class Instance; }

class Network;

typedef std::unordered_map<std::string, int> NameIdMap;

class NetworkFactory {
 public:
  // Creates a constraint network out of a parsed instance.
  Network Create(const parse::Instance& instance);

  // Returns the duration of the last network creation in microseconds.
  base::Clock::Diff duration() const;

 private:
  void AddDomains(const parse::Instance& instance,
                  NameIdMap* domain_map, Network* network);
  void AddVariables(const parse::Instance& instance,
                    const NameIdMap& domain_map,
                    NameIdMap* variable_map, Network* network);
  void AddRelations(const parse::Instance& instance,
                    NameIdMap* relation_map, Network* network);
  void AddConstraints(const parse::Instance& instance,
                      const NameIdMap& variable_map,
                      const NameIdMap& relation_map,
                      NameIdMap* constraint_map, Network* network);

  Relation CreateConstraintRelation(const Network& network,
                                    const std::vector<int>& scope,
                                    const int relation_id);

  // TupleSetMap tuples_cache_;
  base::Clock::Diff duration_;
};

}  // namespace ace
#endif  // SRC_NETWORK_FACTORY_H_
