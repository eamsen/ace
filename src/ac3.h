// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_AC3_H_
#define SRC_AC3_H_

#include <queue>
#include "./preprocessor.h"
#include "./clock.h"

namespace ace {

class Network;
class Assignment;

class Ac3 : public Preprocessor {
 public:
  explicit Ac3(Network* network);

  bool Propagate(const int var_id);
  bool Preprocess();
  void Reset();

  base::Clock::Diff duration() const;
  int num_iterations() const;
  int num_processed() const;

 private:
  struct ReviseItem {
    ReviseItem(const int constraint, const int var_index)
        : constraint(constraint),
          var_index(var_index) {}
    int constraint;
    int var_index;
  };

  typedef std::queue<ReviseItem> Queue;

  bool Enforce(Queue* queue);
  bool Revise(const ReviseItem& item);

  Network* network_;
  int num_iterations_;
  int num_removed_;
  base::Clock::Diff duration_;
};

}  // namespace ace
#endif  // SRC_AC3_H_
