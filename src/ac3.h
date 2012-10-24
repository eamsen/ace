// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_AC3_H_
#define SRC_AC3_H_

#include <queue>
#include "./preprocessor.h"
#include "./clock.h"

namespace ace {

class Network;
class Assignment;

// AC3 (Arc Consistency 3) Preprocessor. Also used for MAP iterations.
class Ac3 : public Preprocessor {
 public:
  // Initializes preprocessor with given network.
  explicit Ac3(Network* network);

  // Propagates arc-consistency for given variable id.
  bool Propagate(const int var_id);
  
  // Makes network fully arc-consistent.
  bool Preprocess();

  // Resets temporary data stored between propagations.
  void Reset();

  // Returns the duration in microseconds of the last call to propagate or
  // preprocess.
  base::Clock::Diff duration() const;

  // Returns the number of iterations used for the last call to propagate or
  // preprocess.
  int num_iterations() const;
  
  // Returns the number over domain value reductions in the last call to
  // propagate or preprocess.
  int num_processed() const;

 private:
  // Intermediate structure for revise operations.
  struct ReviseItem {
    // Initializes revice item with given constraint id and variable scope
    // index.
    ReviseItem(const int constraint, const int var_index)
        : constraint(constraint),
          var_index(var_index) {}
    int constraint;
    int var_index;
  };

  // Queue type used for domain change propagation.
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
