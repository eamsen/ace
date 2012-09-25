// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_PREPROCESSOR_H_
#define SRC_PREPROCESSOR_H_

#include <string>
#include "./clock.h"

namespace ace {

class Preprocessor {
 public:
  explicit Preprocessor(const std::string& type)
      : type_(type) {}

  virtual ~Preprocessor() {}

  virtual bool Preprocess() = 0;
  virtual base::Clock::Diff duration() const = 0;
  virtual int num_iterations() const = 0;
  virtual int num_processed() const = 0;

  std::string type() const {
    return type_;
  }

 private:
  std::string type_;
};

}  // namespace ace
#endif  // SRC_PREPROCESSOR_H_

