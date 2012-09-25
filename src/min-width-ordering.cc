// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./min-width-ordering.h"
#include <cassert>
#include <queue>
#include <vector>
#include <algorithm>
#include "./network.h"

using std::priority_queue;
using std::pair;
using std::make_pair;
using std::greater;
using std::vector;
using std::reverse;

namespace ace {

struct PairGreater {
  bool operator()(const pair<int, int>& lhs, const pair<int, int>& rhs) const {
    return lhs.first > rhs.first ||
           (lhs.first == rhs.first && lhs.second > rhs.second);
  }
};

MinWidthOrdering::MinWidthOrdering(const Network& network)
    : network_(network) {}

vector<int> MinWidthOrdering::CreateOrdering() const {
  typedef priority_queue<pair<int, int>,
                         vector<pair<int, int> >,
                         PairGreater> Queue;

  const int num_variables = network_.num_variables();
  vector<int> ordering;
  ordering.reserve(num_variables);
  vector<int> degrees(num_variables, 0);

  Queue q;
  for (int i = 0; i < num_variables; ++i) {
    const vector<int>& cons = network_.constraints(i);
    for (auto it = cons.begin(), end = cons.end(); it != end; ++it) {
      const Constraint& con = network_.constraint(*it);
      degrees[i] += con.arity() - 1;
    }
    q.push(make_pair(degrees[i], i));
  }

  vector<bool> ordered(num_variables, false);
  while (static_cast<int>(ordering.size()) < num_variables) {
    const int degree = q.top().first;
    const int var_id = q.top().second;
    q.pop();
    assert(degree >= 0);
    if (ordered[var_id]) {
      assert(degrees[var_id] <= degree);
      continue;
    }
    ordering.push_back(var_id);
    ordered[var_id] = true;
    const vector<int>& cons = network_.constraints(var_id);
    for (auto it = cons.cbegin(), end = cons.cend(); it != end; ++it) {
      const Constraint& con = network_.constraint(*it);
      const int arity = con.arity();
      for (int i = 0; i < arity; ++i) {
        const int neigh_id = con.scope(i);
        if (neigh_id == var_id) {
          continue;
        }
        q.push(make_pair(--degrees[neigh_id], neigh_id));
      }
    }
  }
  reverse(ordering.begin(), ordering.end());
  return ordering;
}

}  // namespace ace
