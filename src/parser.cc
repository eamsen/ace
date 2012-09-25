// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include "./parser.h"
#include <cassert>
#include <fstream>
#include <sstream>

using std::string;
using std::ifstream;
using std::stringstream;
using std::vector;
using std::set;
using std::min;
using base::Clock;

namespace ace { namespace parse {

string Domain::Str() const {
  stringstream ss;
  ss << "(" << name << " (";
  for (auto it = values.begin(); it != values.end(); ++it) {
    if (it != values.begin()) {
      ss << " ";
    }
    ss << *it;
  }
  ss << "))";
  return ss.str();
}

string Variable::Str() const {
  stringstream ss;
  ss << "(" << name << " " << domain << ")";
  return ss.str();
}

string Relation::Str() const {
  stringstream ss;
  ss << "(" << name << " " << arity << (semantics == kSupports ? " s " : " c ")
     << "(";
  for (auto it = tuples.begin(); it != tuples.end(); ++it) {
    const vector<int>& tuple = *it;
    if (it != tuples.begin()) {
      ss << " ";
    }
    ss << "(";
    for (auto it2 = tuple.begin(); it2 != tuple.end(); ++it2) {
      if (it2 != tuple.begin()) {
        ss << " ";
      }
      ss << *it2;
    }
    ss << ")";
  }
  ss << "))";
  return ss.str();
}

bool Relation::VectorLess::operator()(const std::vector<int>& lhs,
                                      const std::vector<int>& rhs) const {
  assert(lhs.size() == rhs.size());
  size_t i = 0;
  while (i < lhs.size() && lhs[i] == rhs[i]) {
    ++i;
  }
  return i < lhs.size() && lhs[i] < rhs[i];
}

string Constraint::Str() const {
  stringstream ss;
  ss << "(" << name << " " << reference << " (";
  for (auto it = scope.begin(); it != scope.end(); ++it) {
    if (it != scope.begin()) {
      ss << " ";
    }
    ss << *it;
  }
  ss << "))";
  return ss.str();
}

const char* Instance::kDefName = "";
const char* Instance::kDefType = "";
const int Instance::kDefMaxConstraintArity = 0;
const int Instance::kDefMinViolatedConstraints = 0;
const int Instance::kDefNumSolutions = 0;

Instance::Instance()
    : format(""),
      name(kDefName),
      type(kDefType),
      max_constraint_arity(kDefMaxConstraintArity),
      min_violated_constraints(kDefMinViolatedConstraints),
      num_solutions(kDefNumSolutions) {}

string Instance::Str() const {
  stringstream ss;
  ss << "(" << name;
  ss << "\n  (domains ";
  for (auto it = domains.begin(); it != domains.end(); ++it) {
    const Domain& d = *it;
    ss << "\n    " << d.Str();
  }
  ss << ")\n  (variables ";
  for (auto it = variables.begin(); it != variables.end(); ++it) {
    const Variable& v = *it;
    ss << "\n    " << v.Str();
  }
  ss << ")\n  (relations ";
  for (auto it = relations.begin(); it != relations.end(); ++it) {
    const Relation& r = *it;
    ss << "\n    " << r.Str();
  }
  ss << ")\n  (constraints ";
  for (auto it = constraints.begin(); it != constraints.end(); ++it) {
    const Constraint& c = *it;
    ss << "\n    " << c.Str();
  }
  ss << "))\n";
  return ss.str();
}

const char* Parser::kNumbers = "0123456789";
const char* Parser::kWhitespace = "\n\r\t ";

size_t Parser::FileSize(const string& path) {
  ifstream stream(path.c_str());
  size_t size = 0;
  if (stream.good()) {
    stream.seekg(0, std::ios::end);
    size = stream.tellg();
    stream.seekg(0, std::ios::beg);
  }
  stream.close();
  return size;
}

void Parser::CollectIntValues(set<int>* values, const string& content) {
  assert(values);
  size_t pos = content.find_first_of(kNumbers);
  while (pos != string::npos) {
    pos = content.find_first_of(kNumbers, pos);
    assert(pos != string::npos);
    size_t value_end = content.find_first_not_of(kNumbers, pos);
    if (value_end == string::npos) {
      // Last single value found.
      values->insert(Convert<int>(content.substr(pos)));
    } else if (content[value_end] == ' ') {
      // Single value found.
      values->insert(Convert<int>(content.substr(pos, value_end - pos)));
    } else if (content[value_end] == '.') {
      // Value interval found.
      int interval_beg = Convert<int>(content.substr(pos, value_end - pos));
      size_t next_value_beg = value_end + 2;
      value_end = content.find_first_not_of(kNumbers, next_value_beg);
      int interval_end = interval_beg;
      if (value_end == string::npos) {
        interval_end = Convert<int>(content.substr(next_value_beg));
      } else {
        interval_end = Convert<int>(content.substr(next_value_beg,
                                                   value_end - next_value_beg));
      }
      // Expand interval.
      for (int i = interval_beg; i <= interval_end; ++i) {
        values->insert(i);
      }
    }
    pos = value_end;
  }
}

void Parser::CollectIntTuples(Relation::TupleSet* tuples,
                              const string& content) {
  assert(tuples);
  size_t pos = content.find_first_of(kNumbers);
  while (pos != string::npos) {
    pos = content.find_first_of(kNumbers, pos);
    assert(pos != string::npos);
    size_t value_end = content.find_first_of("|", pos);
    if (value_end == string::npos) {
      // Last tuple found.
      vector<int> tuple;
      stringstream ss;
      ss << content.substr(pos);
      while (ss.good()) {
        int value = 0;
        ss >> value;
        if (!ss.fail()) {
          tuple.push_back(value);
        }
      }
      tuples->insert(tuple);
    } else {
      // Tuple found.
      vector<int> tuple;
      stringstream ss;
      ss << content.substr(pos, value_end - pos);
      while (ss.good()) {
        int value = 0;
        ss >> value;
        if (!ss.fail()) {
          tuple.push_back(value);
        }
      }
      tuples->insert(tuple);
    }
    pos = value_end;
  }
}

vector<int> Parser::CollectInts(const string& content, const string& delims) {
  vector<string> strings = Split(content, delims);
  vector<int> items;
  items.reserve(strings.size());
  for (auto it = strings.cbegin(), end = strings.cend(); it != end; ++it) {
    items.push_back(Convert<int>(*it));
  }
  return items;
}


vector<string> Parser::Split(const string& content) {
  return Split(content, kWhitespace);
}

vector<string> Parser::Split(const string& content, const string& delims) {
  vector<string> items;
  size_t pos = content.find_first_not_of(delims);
  while (pos != string::npos) {
    size_t end = content.find_first_of(delims, pos);
    if (end == string::npos) {
      // Last item found.
      items.push_back(content.substr(pos));
    } else {
      // Item found.
      items.push_back(content.substr(pos, end - pos));
    }
    pos = content.find_first_not_of(delims, end);
  }
  return items;
}

string Parser::StripWhitespace(const string& content) {
  size_t beg = content.find_first_not_of(kWhitespace);
  size_t end = content.find_last_not_of(kWhitespace);
  if (beg == end) {
    return "";
  }
  return content.substr(beg, end - beg + 1);
}

Parser::Parser(const string& path)
    : path_(path) {}

Instance Parser::ParseInstance(const string& name) {
  Instance instance = ParseInstance();
  assert(instance.name == name);
  return instance;
}

Instance Parser::ParseInstance() {
  const Clock beg;
  if (content_.size() == 0) {
    ReadAll();
  }
  assert(content_.size());
  size_t instance_beg = ElementBegin("instance", 0);
  assert(instance_beg != string::npos);
  instance_beg += 10u;
  size_t instance_end = ElementEnd("instance", instance_beg);
  assert(instance_end != string::npos);
  Instance instance;
  LoadPresentation(&instance, instance_beg);
  LoadDomains(&instance, instance_beg);
  LoadVariables(&instance, instance_beg);
  LoadRelations(&instance, instance_beg);
  LoadConstraints(&instance, instance_beg);

  duration_ = Clock() - beg;
  return instance;
}

Clock::Diff Parser::duration() const {
  return duration_;
}

void Parser::LoadPresentation(Instance* instance, const size_t pos) const {
  size_t pres_beg = ElementBegin("presentation", pos);
  assert(pres_beg != string::npos);
  pres_beg += 13u;
  size_t pres_end = ElementEnd("presentation", pres_beg);
  assert(pres_end != string::npos);
  // Mandatory attribute.
  instance->format = AttributeValue("format", pres_beg, pres_end);
  // Optional attributes.
  instance->name = AttributeValue("name", pres_beg, pres_end);
  LoadOptionalAttribute("maxConstraintArity",
                        &instance->max_constraint_arity,
                        pres_beg, pres_end);
  LoadOptionalAttribute("minViolatedConstraints",
                        &instance->min_violated_constraints,
                        pres_beg, pres_end);
  LoadOptionalAttribute("nbSolutions",
                        &instance->num_solutions,
                        pres_beg, pres_end);
  LoadOptionalAttribute("type",
                        &instance->type,
                        pres_beg, pres_end);
  instance->description = StripWhitespace(Content(pres_beg, pres_end));
  // TODO(esawin): Load solutions attribute.
}

void Parser::LoadDomains(Instance* instance, const size_t pos) const {
  size_t domains_beg = ElementBegin("domains", pos);
  assert(domains_beg != string::npos);
  domains_beg += 8u;
  size_t domains_end = ElementEnd("domains", domains_beg);
  assert(domains_end != string::npos);
  int num_domains = 0;
  LoadAttribute("nbDomains", &num_domains, domains_beg, domains_end);
  assert(num_domains > 0);
  instance->domains.resize(num_domains);
  for (int d = 0; d < num_domains; ++d) {
    domains_beg = LoadDomain(&instance->domains[d], domains_beg);
  }
}

size_t Parser::LoadDomain(Domain* domain, const size_t pos) const {
  size_t domain_beg = ElementBegin("domain", pos);
  assert(domain_beg != string::npos);
  domain_beg += 7u;
  size_t domain_end = ElementEnd("domain", domain_beg);
  assert(domain_end != string::npos);
  LoadAttribute("name", &domain->name, domain_beg, domain_end);
  size_t num_values = 0;
  LoadAttribute("nbValues", &num_values, domain_beg, domain_end);
  assert(num_values > 0);
  string content_str = Content(domain_beg, domain_end);
  CollectIntValues(&domain->values, content_str);
  assert(domain->values.size() == num_values);
  return domain_end;
}

void Parser::LoadVariables(Instance* instance, const size_t pos) const {
  size_t variables_beg = ElementBegin("variables", pos);
  assert(variables_beg != string::npos);
  variables_beg += 10u;
  size_t variables_end = ElementEnd("variables", variables_beg);
  assert(variables_end != string::npos);
  int num_variables = 0;
  LoadAttribute("nbVariables", &num_variables, variables_beg, variables_end);
  assert(num_variables > 0);
  instance->variables.resize(num_variables);
  for (int v = 0; v < num_variables; ++v) {
    variables_beg = LoadVariable(&instance->variables[v], variables_beg);
  }
}

size_t Parser::LoadVariable(Variable* variable, const size_t pos) const {
  size_t variable_beg = ElementBegin("variable", pos);
  assert(variable_beg != string::npos);
  variable_beg += 9u;
  size_t variable_end = ElementEnd("variable", variable_beg);
  assert(variable_end != string::npos);
  LoadAttribute("name", &variable->name, variable_beg, variable_end);
  LoadAttribute("domain", &variable->domain, variable_beg, variable_end);
  return variable_end;
}

void Parser::LoadRelations(Instance* instance, const size_t pos) const {
  size_t relations_beg = ElementBegin("relations", pos);
  assert(relations_beg != string::npos);
  relations_beg += 10u;
  size_t relations_end = ElementEnd("relations", relations_beg);
  assert(relations_end != string::npos);
  int num_relations = 0;
  LoadAttribute("nbRelations", &num_relations, relations_beg, relations_end);
  assert(num_relations > 0);
  instance->relations.resize(num_relations);
  for (int r = 0; r < num_relations; ++r) {
    relations_beg = LoadRelation(&instance->relations[r], relations_beg);
  }
}

size_t Parser::LoadRelation(Relation* relation, const size_t pos) const {
  size_t relation_beg = ElementBegin("relation", pos);
  assert(relation_beg != string::npos);
  relation_beg += 9u;
  size_t relation_end = ElementEnd("relation", relation_beg);
  assert(relation_end != string::npos);
  LoadAttribute("name", &relation->name, relation_beg, relation_end);
  LoadAttribute("arity", &relation->arity, relation_beg, relation_end);
  string semantics_str;
  LoadAttribute("semantics", &semantics_str, relation_beg, relation_end);
  assert(semantics_str == "supports" || semantics_str == "conflicts");
  relation->semantics = semantics_str == "supports" ? Relation::kSupports :
                                                      Relation::kConflicts;
  size_t num_tuples = 0;
  LoadAttribute("nbTuples", &num_tuples, relation_beg, relation_end);
  string content_str = Content(relation_beg, relation_end);
  CollectIntTuples(&relation->tuples, content_str);
  assert(relation->tuples.size() == num_tuples);
  return relation_end;
}

void Parser::LoadConstraints(Instance* instance, const size_t pos) const {
  size_t constraints_beg = ElementBegin("constraints", pos);
  assert(constraints_beg != string::npos);
  constraints_beg += 12u;
  size_t constraints_end = ElementEnd("constraints", constraints_beg);
  assert(constraints_end != string::npos);
  int num_constraints = 0;
  LoadAttribute("nbConstraints", &num_constraints, constraints_beg,
                constraints_end);
  assert(num_constraints > 0);
  instance->constraints.resize(num_constraints);
  for (int c = 0; c < num_constraints; ++c) {
    constraints_beg = LoadConstraint(&instance->constraints[c],
                                     constraints_beg);
  }
}

size_t Parser::LoadConstraint(Constraint* constraint, const size_t pos) const {
  size_t constraint_beg = ElementBegin("constraint", pos);
  assert(constraint_beg != string::npos);  // nbConstraints is wrong
  constraint_beg += 11u;
  size_t constraint_end = ElementEnd("constraint", pos);
  assert(constraint_end != string::npos);
  LoadAttribute("name", &constraint->name, constraint_beg, constraint_end);
  LoadAttribute("arity", &constraint->arity, constraint_beg, constraint_end);
  assert(constraint->arity > 0);
  LoadAttribute("reference", &constraint->reference, constraint_beg,
                constraint_end);
  string scope_str = AttributeValue("scope", constraint_beg, constraint_end);
  assert(scope_str.size());
  constraint->scope = Split(StripWhitespace(scope_str));
  assert(static_cast<int>(constraint->scope.size()) == constraint->arity);
  return constraint_end;
}

string Parser::Content(const size_t pos, size_t end) const {
  assert(end > pos);
  const string substr = content_.substr(pos, end - pos);
  size_t content_beg = substr.find(">");
  if (content_beg == string::npos) {
    // The element was closed by />.
    return "";
  }
  assert(content_beg != string::npos);
  content_beg += 1;
  size_t content_end = substr.find("<", content_beg);
  assert(content_end != string::npos);
  return substr.substr(content_beg, content_end - content_beg);
}

string Parser::AttributeValue(const string& attribute,
                              const size_t pos, const size_t end) const {
  assert(end > pos);
  const string substr = content_.substr(pos, end - pos);
  size_t value_beg = substr.find(attribute);
  if (value_beg == string::npos) {
    return string();
  }
  value_beg = substr.find("\"", value_beg + attribute.size() + 1);
  assert(value_beg != string::npos);
  value_beg += 1;
  size_t value_end = substr.find("\"", value_beg + 1);
  assert(value_end != string::npos);
  return substr.substr(value_beg, value_end - value_beg);
}

size_t Parser::ElementBegin(const string& element, const size_t pos) const {
  size_t begin = content_.find("<" + element, pos);
  return begin;
}

size_t Parser::ElementEnd(const string& element, const size_t pos) const {
  size_t end1 = content_.find("/>", pos);
  size_t end2 = content_.find("</" + element, pos);
  assert(end1 != string::npos || end2 != string::npos);
  size_t end = min(end1 == string::npos ? end2 + 1 : end1,
                   end2 == string::npos ? end1 + 1 : end2);
  end = content_.find(">", end);
  return end;
}

void Parser::ReadAll() {
  size_t file_size = FileSize(path_);
  char* buffer = new char[file_size + 1];
  buffer[file_size] = '\0';
  ifstream stream(path_);
  assert(stream.good());
  stream.read(buffer, file_size);
  assert(static_cast<size_t>(stream.gcount()) == file_size);
  content_ = string(buffer);
}

} }  // namespace ace::parse

