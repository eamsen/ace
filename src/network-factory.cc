// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./network-factory.h"
#include <cassert>
#include "./parser.h"
#include "./network.h"

using std::string;
using std::vector;
using std::set;
using std::unordered_map;
using base::Clock;
using ace::parse::Parser;
using ace::parse::Instance;

namespace ace {

Network NetworkFactory::Create(const Instance& instance) {
  const Clock beg;

  Network network;
  network.name(instance.name);
  NameIdMap domain_map;
  AddDomains(instance, &domain_map, &network);
  NameIdMap variable_map;
  AddVariables(instance, domain_map, &variable_map, &network);
  NameIdMap relation_map;
  AddRelations(instance, &relation_map, &network);
  NameIdMap constraint_map;
  AddConstraints(instance, variable_map, relation_map,
                 &constraint_map, &network);
  network.Finalise();
  // tuples_cache_.clear();

  duration_ = Clock() - beg;
  return network;
}

Clock::Diff NetworkFactory::duration() const {
  return duration_;
}

void NetworkFactory::AddDomains(const Instance& instance,
                                NameIdMap* domain_map,
                                Network* network) {
  assert(domain_map->empty());
  const vector<parse::Domain>& domains = instance.domains;
  for (auto it = domains.cbegin(), end = domains.cend(); it != end; ++it) {
    const parse::Domain& d = *it;
    assert(domain_map->find(d.name) == domain_map->end());
    Domain domain(d.name, vector<int>(d.values.begin(), d.values.end()));
    (*domain_map)[d.name] = network->AddDomain(domain);
  }
  assert(domain_map->size() == domains.size());
}

void NetworkFactory::AddVariables(const Instance& instance,
                                  const NameIdMap& domain_map,
                                  NameIdMap* variable_map,
                                  Network* network) {
  assert(variable_map->empty());
  const vector<parse::Variable>& variables = instance.variables;
  for (auto it = variables.cbegin(), end = variables.cend(); it != end; ++it) {
    const parse::Variable& v = *it;
    assert(variable_map->find(v.name) == variable_map->end());
    auto domain_id = domain_map.find(v.domain);
    assert(domain_id != domain_map.end());
    const parse::Domain& d = instance.domains[domain_id->second];
    const set<int>& domain_values = d.values;
    Variable variable(v.name, domain_id->second, domain_values);
    (*variable_map)[v.name] = network->AddVariable(variable);
  }
  assert(variable_map->size() == variables.size());
}

void NetworkFactory::AddRelations(const Instance& instance,
                                  NameIdMap* relation_map,
                                  Network* network) {
  assert(relation_map->empty());
  const vector<parse::Relation>& relations = instance.relations;
  for (auto it = relations.begin(), end = relations.end(); it != end; ++it) {
    const parse::Relation& r = *it;
    assert(relation_map->find(r.name) == relation_map->end());
    Relation::Semantics semantics =
      r.semantics == ace::parse::Relation::kSupports ? Relation::kSupports :
                                                       Relation::kConflicts;
    Relation::TupleSet tuples(r.tuples.begin(), r.tuples.end());
    Relation relation(r.name, semantics, tuples);
    (*relation_map)[r.name] = network->AddRelation(relation);
  }
  assert(relation_map->size() == relations.size());
}

void NetworkFactory::AddConstraints(const Instance& instance,
                                    const NameIdMap& variable_map,
                                    const NameIdMap& relation_map,
                                    NameIdMap* constraint_map,
                                    Network* network) {
  assert(constraint_map->empty());
  const vector<parse::Constraint>& constraints = instance.constraints;
  for (auto it = constraints.cbegin(), end = constraints.cend();
       it != end; ++it) {
    const parse::Constraint& c = *it;
    assert(constraint_map->find(c.name) == constraint_map->end());
    auto relation_id = relation_map.find(c.reference);
    assert(relation_id != relation_map.end());
    vector<int> scope;
    scope.reserve(c.scope.size());
    for (auto it2 = c.scope.cbegin(), end2 = c.scope.cend();
         it2 != end2; ++it2) {
      auto variable_id = variable_map.find(*it2);
      assert(variable_id != variable_map.end());
      scope.push_back(variable_id->second);
    }
    Constraint constraint(c.name, relation_id->second, scope, *network);
    (*constraint_map)[c.name] = network->AddConstraint(constraint);
  }
  assert(constraint_map->size() == instance.constraints.size());
}

Relation NetworkFactory::CreateConstraintRelation(const Network& network,
                                                  const vector<int>& scope,
                                                  const int relation_id) {
  // Construct the domains for the variables in the scope.
  // const int num_vars = scope.size();
  // vector<vector<int> > domains(num_vars);
  // for (int i = 0; i < num_vars; ++i) {
  //   domains[i] = network.variable(scope[i]).domain();
  // }
  const Relation& org_relation = network.relation(relation_id);
  // Search for cached relation.
  // DomainsRelationHash hasher;
  // auto const key = make_pair(domains, relation_id);
  // assert(hasher(key) == hasher(make_pair(domains, relation_id)));
  // auto cached_tuples = tuples_cache_.find(key);
  // if (cached_tuples == tuples_cache_.end()) {
  //   // Tuples are not cached, create them.
  //   tuples_cache_[key] = org_relation.Supporting(domains);
  //   cached_tuples = tuples_cache_.find(key);
  //   assert(cached_tuples != tuples_cache_.end());
  // }
  // return Relation(org_relation.name(), Relation::kSupports,
  //                 cached_tuples->second);
  //
  return org_relation;
}

}  // namespace ace
