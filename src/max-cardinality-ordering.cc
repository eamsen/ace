// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./max-cardinality-ordering.h"
#include <cassert>
#include <queue>
#include <vector>
#include <set>
#include "./network.h"

using std::priority_queue;
using std::pair;
using std::make_pair;
using std::vector;
using std::set;

namespace ace {

struct PairLess {
  bool operator()(const pair<int, int>& lhs, const pair<int, int>& rhs) const {
    return lhs.first < rhs.first ||
           (lhs.first == rhs.first && lhs.second > rhs.second);
  }
};

MaxCardinalityOrdering::MaxCardinalityOrdering(const Network& network)
    : network_(network) {}

vector<int> MaxCardinalityOrdering::CreateOrdering() const {
  typedef priority_queue<pair<int, int>,
                         vector<pair<int, int> >,
                         PairLess> Queue;

  const int num_variables = network_.num_variables();
  vector<int> ordering;
  ordering.reserve(num_variables);

  Queue q;
  for (int i = 0; i < num_variables; ++i) {
    q.push(make_pair(0, i));
  }

  vector<int> cardinalities(num_variables, 0);
  vector<bool> ordered(num_variables, false);
  while (static_cast<int>(ordering.size()) < num_variables) {
    // const int cardinality = q.top().first;
    const int var_id = q.top().second;
    q.pop();
    if (ordered[var_id]) {
      continue;
    }
    ordering.push_back(var_id);
    ordered[var_id] = true;
    const vector<int>& cons = network_.constraints(var_id);
    for (auto it = cons.cbegin(), end = cons.cend(); it != end; ++it) {
      const Constraint& constraint = network_.constraint(*it);
      const vector<int>& scope = constraint.scope();
      for (auto it2 = scope.cbegin(), end2 = scope.cend();
           it2 != end2; ++it2) {
        ++cardinalities[*it2];
      }
    }
    for (auto it = cons.cbegin(), end = cons.cend(); it != end; ++it) {
      const Constraint& constraint = network_.constraint(*it);
      const vector<int>& scope = constraint.scope();
      for (auto it2 = scope.cbegin(), end2 = scope.cend();
           it2 != end2; ++it2) {
        q.push(make_pair(cardinalities[*it2], *it2));
      }
    }
  }
  return ordering;
}

}  // namespace ace
