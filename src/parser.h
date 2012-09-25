// Copyright 2012 Eugen Sawin <esawin@me73.com>
#ifndef SRC_PARSER_H_
#define SRC_PARSER_H_

#include <cassert>
#include <vector>
#include <set>
#include <string>
#include "./clock.h"

namespace ace { namespace parse {

// XCSP 2.1 domain element container.
struct Domain {
  // Returns a string representation.
  std::string Str() const;

  std::string name;
  std::set<int> values;
};

// XCSP 2.1 variable element container.
struct Variable {
  // Returns a string representation.
  std::string Str() const;

  std::string name;
  std::string domain;
};

// XCSP 2.1 relation element container.
struct Relation {
  enum Semantics { kSupports, kConflicts };

  // Functor used for tuple ordering.
  struct VectorLess {
    bool operator()(const std::vector<int>& lhs,
                    const std::vector<int>& rhs) const;
  };

  typedef std::set<std::vector<int>, VectorLess> TupleSet;

  // Returns a string representation.
  std::string Str() const;

  std::string name;
  int arity;
  Semantics semantics;
  TupleSet tuples;
};

// XCSP 2.1 constraint element container.
struct Constraint {
  // Returns a string representation.
  std::string Str() const;

  std::string name;
  std::string reference;
  int arity;
  std::vector<std::string> scope;
};

// XCSP 2.1 instance element container.
struct Instance {
  static const char* kDefName;
  static const char* kDefType;
  static const int kDefMaxConstraintArity;
  static const int kDefMinViolatedConstraints;
  static const int kDefNumSolutions;

  Instance();

  // Returns a string representation.
  std::string Str() const;

  // Presentation element.
  std::string format;
  std::string name;
  std::string type;
  std::string description;
  int max_constraint_arity;
  int min_violated_constraints;
  int num_solutions;

  // Domains element.
  std::vector<Domain> domains;

  // Variables element.
  std::vector<Variable> variables;

  // Relations element.
  std::vector<Relation> relations;

  // TODO(esawin): Predicates support.
  // Constraints element.
  std::vector<Constraint> constraints;
};

class Parser {
 public:
  static const char* kNumbers;
  static const char* kWhitespace;

  // Converts a value from one type to another.
  template<typename To, typename From>
  static To Convert(const From& from) {
    std::stringstream ss;
    ss << from;
    To to;
    ss >> to;
    return to;
  }

  // Returns the file size of given path in bytes.
  static size_t FileSize(const std::string& path);

  // Collects int sequences (1 3 4) and expanded int intervals (3..7) from given
  // string.
  static void CollectIntValues(std::set<int>* values,
                               const std::string& content);

  // Collects int tuples, which are int sequences separated by | from given
  // string. E.g.: 1 3 | 1 4 | 2 4 --> {[1, 3], [1, 4], [2, 4]}.
  static void CollectIntTuples(Relation::TupleSet* tuples,
                               const std::string& content);

  // Collects ints in given content separated by given delimeters.
  static std::vector<int> CollectInts(const std::string& content,
                                      const std::string& delims);

  // Splits the given string at whitespaces.
  static std::vector<std::string> Split(const std::string& content);

  // Splits the given string at given delimeters.
  static std::vector<std::string> Split(const std::string& content,
                                        const std::string& delims);

  // Strips all whitespace characters at beginning and end of given string.
  static std::string StripWhitespace(const std::string& content);

  // Initialised the parser with given path.
  explicit Parser(const std::string& path);

  // Parses an instance with the given name.
  // Dies if such an instance is not defined.
  Instance ParseInstance(const std::string& name);

  // Parses the next still unparsed instance.
  Instance ParseInstance();

  // Returns the duration of the last instance parsing in microseconds.
  base::Clock::Diff duration() const;

 private:
  // Reads (or dies) a mandatory attribute between pos and end.
  template<typename T>
  void LoadAttribute(const std::string& attribute, T* value,
                     const size_t pos, const size_t end) const {
    std::string value_str = AttributeValue(attribute, pos, end);
    assert(value_str.size());
    *value = Convert<T>(value_str);
  }

  // Reads an optional attribute between pos and end and updates the given value
  // variable if attribute is found.
  template<typename T>
  void LoadOptionalAttribute(const std::string& attribute, T* value,
                             const size_t pos, const size_t end) const {
    std::string value_str = AttributeValue(attribute, pos, end);
    if (value_str.size()) {
      *value = Convert<T>(value_str);
    }
  }

  // Loads the next presentation element.
  void LoadPresentation(Instance* instance, const size_t pos) const;

  // Loads the next domains element.
  void LoadDomains(Instance* instance, const size_t pos) const;

  // Loads the next domain element and returns its end position.
  size_t LoadDomain(Domain* domain, const size_t pos) const;

  // Loads the next variables element.
  void LoadVariables(Instance* instance, const size_t pos) const;

  // Loads the next variable element and treturns its end position.
  size_t LoadVariable(Variable* variable, const size_t pos) const;

  // Loads the next relations element.
  void LoadRelations(Instance* instance, const size_t pos) const;

  // Loads the next relation element and returns its end position.
  size_t LoadRelation(Relation* relation, const size_t pos) const;

  // Loads the next constraints element.
  void LoadConstraints(Instance* instance, const size_t pos) const;

  // Loads the next constraint element and returns its end position.
  size_t LoadConstraint(Constraint* constraint, const size_t pos) const;

  // Returns the content string between pos and end (between > and </).
  std::string Content(const size_t pos, const size_t end) const;

  // Returns (or dies) a mandatory attribute of given name between pos and end.
  std::string AttributeValue(const std::string& attribute, const size_t pos,
                             const size_t end) const;

  // Returns the begin position of given element.
  size_t ElementBegin(const std::string& element, const size_t pos) const;

  // Returns the end position of given element.
  size_t ElementEnd(const std::string& element, const size_t pos) const;

  // Reads the whole file into parser cache.
  void ReadAll();

  std::string path_;
  std::string content_;
  base::Clock::Diff duration_;
};

} }  // namespace ace::parse
#endif  // SRC_PARSER_H_
