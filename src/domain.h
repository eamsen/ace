// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_DOMAIN_H_
#define SRC_DOMAIN_H_

#include <string>
#include <vector>

namespace ace {

class Domain {
 public:
  typedef std::vector<int>::const_iterator const_iterator;

  Domain(const std::string& name, const std::vector<int>& values);

  const_iterator begin() const;
  const_iterator end() const;
  const int& at(const int index) const;
  const int& back() const;
  const std::vector<int>& values() const;
  int size() const;

 private:
  std::string name_;
  std::vector<int> values_;
};

}  // namespace ace
#endif  // SRC_DOMAIN_H_
