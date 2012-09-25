// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./domain.h"

using std::string;
using std::vector;

namespace ace {

Domain::Domain(const string& name, const vector<int>& values)
    : name_(name),
      values_(values) {}

Domain::const_iterator Domain::begin() const {
  return values_.begin();
}

Domain::const_iterator Domain::end() const {
  return values_.end();
}

const int& Domain::at(const int index) const {
  return values_[index];
}

const int& Domain::back() const {
  return values_.back();
}

const vector<int>& Domain::values() const {
  return values_;
}

int Domain::size() const {
  return values_.size();
}

}  // namespace ace
