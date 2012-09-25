// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fstream>
#include <vector>
#include <set>
#include "../parser.h"

using ace::parse::Relation;
using ace::parse::Instance;
using ace::parse::Parser;

using std::vector;
using std::set;
using std::string;
using std::cout;
using std::endl;
using std::ofstream;

using ::testing::ElementsAre;
using ::testing::Contains;
using ::testing::Not;
// using ::testing::WhenSortedBy;

class ParserTest : public ::testing::Test {
 public:
  void SetUp() {
    xml1 =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
    <instance>\n\
      <presentation\
       name=\"xml1\"\
       maxConstraintArity=\"2\"\
       format=\"XCSP 2.1\"/>\n\
      <domains nbDomains=\"3\">\n\
        <domain name=\"D0\" nbValues=\"2\">1 2</domain>\n\
        <domain name=\"D1\" nbValues=\"4\">1..4</domain>\n\
        <domain name=\"D2\" nbValues=\"10\">1..4 7 9..13</domain>\n\
      </domains>\n\
      <variables nbVariables=\"4\">\n\
        <variable name=\"c1\" domain=\"D0\"/>\n\
        <variable name=\"c2\" domain=\"D1\"/>\n\
        <variable name=\"c3\" domain=\"D2\"/>\n\
        <variable name=\"c4\" domain=\"D2\"/>\n\
      </variables>\n\
      <relations nbRelations=\"3\">\n\
        <relation name=\"e1\" arity=\"2\" nbTuples=\"6\"\
         semantics=\"supports\">\
          1 3 | 1 4 | 2 4 | 3 1 | 4 1 | 4 2\
        </relation>\n\
        <relation name=\"e2\" arity=\"2\" nbTuples=\"8\"\
         semantics=\"supports\">\
           1 2 | 1 4 | 2 1 | 2 3 | 3 2 | 3 4 | 4 3 | 4 1\
        </relation>\n\
        <relation name=\"e3\" arity=\"2\" nbTuples=\"10\"\
         semantics=\"supports\">\
           1 2 | 1 3 | 2 1 | 2 3 | 2 4 | 3 1 | 3 2 | 3 4 | 4 2 | 4 3\
        </relation>\n\
      </relations>\n\
      <constraints nbConstraints=\"6\">\n\
        <constraint name=\"r12\" arity=\"2\" scope=\"c1 c2\"\
         reference=\"e1\"/>\n\
        <constraint name=\"r23\" arity=\"2\" scope=\"c2 c3\"\
         reference=\"e1\"/>\n\
        <constraint name=\"r34\" arity=\"2\" scope=\"c3 c4\"\
         reference=\"e1\"/>\n\
        <constraint name=\"r13\" arity=\"2\" scope=\"c1 c3\"\
         reference=\"e2\"/>\n\
        <constraint name=\"r24\" arity=\"2\" scope=\"c2 c4\"\
         reference=\"e2\"/>\n\
        <constraint name=\"r14\" arity=\"2\" scope=\"c1 c4\"\
         reference=\"e3\"/>\n\
      </constraints>\n\
    </instance>";
    xml1_path = "/tmp/ace-parser-test-xml1.xml";
    ofstream xml1_stream(xml1_path.c_str());
    xml1_stream.write(xml1.c_str(), xml1.size());
    xml1_stream.close();

    xml2 =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
    <instance>\n\
      <presentation\
       name=\"xml2\"\
       format=\"XCSP 2.1\">\n\
       xml2 description\n\
      </presentation>\n\
      <domains nbDomains=\"1\">\n\
        <domain name=\"D0\" nbValues=\"2\">1 2</domain>\n\
      </domains>\n\
      <variables nbVariables=\"1\">\n\
        <variable name=\"c1\" domain=\"D0\"/>\n\
      </variables>\n\
      <relations nbRelations=\"1\">\n\
        <relation name=\"e1\" arity=\"2\" nbTuples=\"6\"\
         semantics=\"supports\">\
          1 3 | 1 4 | 2 4 | 3 1 | 4 1 | 4 2\
        </relation>\n\
      </relations>\n\
      <constraints nbConstraints=\"1\">\n\
        <constraint name=\"r12\" arity=\"2\" scope=\"c1 c2\"\
         reference=\"e1\"/>\n\
      </constraints>\n\
    </instance>";
    xml2_path = "/tmp/ace-parser-test-xml2.xml";
    ofstream xml2_stream(xml2_path.c_str());
    xml2_stream.write(xml2.c_str(), xml2.size());
    xml2_stream.close();
  }
  void TearDown() {
    // /tmp/* is cleared automatically on reboot.
  }

  string xml1;
  string xml1_path;
  string xml2;
  string xml2_path;
};

TEST_F(ParserTest, CollectIntValues) {
  {
    string value_str = "";
    set<int> values;
    Parser::CollectIntValues(&values, value_str);
    EXPECT_THAT(values, ElementsAre());
  }
  {
    string value_str = "1..7";
    set<int> values;
    Parser::CollectIntValues(&values, value_str);
    EXPECT_THAT(values, ElementsAre(1, 2, 3, 4, 5, 6, 7));
  }
  {
    string value_str = "1 5 10";
    set<int> values;
    Parser::CollectIntValues(&values, value_str);
    EXPECT_THAT(values, ElementsAre(1, 5, 10));
  }
  {
    string value_str = "0 1..3 7 10..13 17";
    set<int> values;
    Parser::CollectIntValues(&values, value_str);
    EXPECT_THAT(values, ElementsAre(0, 1, 2, 3, 7, 10, 11, 12, 13, 17));
  }
  {
    string value_str = "1..3 10..12 17 18..20";
    set<int> values;
    Parser::CollectIntValues(&values, value_str);
    EXPECT_THAT(values, ElementsAre(1, 2, 3, 10, 11, 12, 17, 18, 19, 20));
  }
}

TEST_F(ParserTest, Split) {
  {
    string s = "";
    vector<string> items = Parser::Split(s);
    EXPECT_EQ(vector<string>(), items);
  } 
  {
    string s = "a";
    vector<string> items = Parser::Split(s);
    EXPECT_EQ(vector<string>({"a"}), items);
  } 
  {
    string s = "c1 c2";
    vector<string> items = Parser::Split(s);
    EXPECT_EQ(vector<string>({"c1", "c2"}), items);
  } 
  {
    string s = "a b c d";
    vector<string> items = Parser::Split(s);
    EXPECT_EQ(vector<string>({"a", "b", "c", "d"}), items);
  }
  {
    string s = " a b c d ";
    vector<string> items = Parser::Split(s);
    EXPECT_EQ(vector<string>({"a", "b", "c", "d"}), items);
  }
}
  
  
TEST_F(ParserTest, ParseInstance_xml1) {
  Parser parser(xml1_path);
  Instance instance = parser.ParseInstance();

  // Check presentation values.
  // These are set in xml1.
  EXPECT_EQ("xml1", instance.name);
  EXPECT_EQ(2, instance.max_constraint_arity);
  EXPECT_EQ("XCSP 2.1", instance.format);
  // These are not set in xml1.
  EXPECT_EQ("", instance.description);
  EXPECT_EQ(Instance::kDefMinViolatedConstraints,
            instance.min_violated_constraints);
  EXPECT_EQ(Instance::kDefNumSolutions,
            instance.num_solutions);
  EXPECT_EQ(Instance::kDefType,
            instance.type);

  // Check domains.
  ASSERT_EQ(3, instance.domains.size());
  // D0: 1 2
  EXPECT_EQ("D0", instance.domains.at(0).name);
  EXPECT_EQ(2, instance.domains.at(0).values.size());
  EXPECT_EQ("(D0 (1 2))", instance.domains.at(0).Str());
  // D1: 1..4
  EXPECT_EQ("D1", instance.domains.at(1).name);
  EXPECT_EQ(4, instance.domains.at(1).values.size());
  EXPECT_EQ("(D1 (1 2 3 4))", instance.domains.at(1).Str());
  // D2: 1..4 7 9..13
  EXPECT_EQ("D2", instance.domains.at(2).name);
  EXPECT_EQ(10, instance.domains.at(2).values.size());
  EXPECT_EQ("(D2 (1 2 3 4 7 9 10 11 12 13))", instance.domains.at(2).Str());

  // Check variables.
  ASSERT_EQ(4, instance.variables.size());
  // c1: D0
  EXPECT_EQ("c1", instance.variables.at(0).name);
  EXPECT_EQ("D0", instance.variables.at(0).domain);
  // c2: D1
  EXPECT_EQ("c2", instance.variables.at(1).name);
  EXPECT_EQ("D1", instance.variables.at(1).domain);
  // c4: D2
  EXPECT_EQ("c3", instance.variables.at(2).name);
  EXPECT_EQ("D2", instance.variables.at(2).domain);
  // c4: D2
  EXPECT_EQ("c4", instance.variables.at(3).name);
  EXPECT_EQ("D2", instance.variables.at(3).domain);

  // Check relations.
  ASSERT_EQ(3, instance.relations.size());
  // e1: supports 1 3 | 1 4 | 2 4 | 3 1 | 4 1 | 4 2
  EXPECT_EQ("e1", instance.relations.at(0).name);
  EXPECT_EQ(2, instance.relations.at(0).arity);
  EXPECT_EQ(Relation::kSupports, instance.relations.at(0).semantics);
  EXPECT_EQ(6, instance.relations.at(0).tuples.size());
  EXPECT_EQ("(e1 2 s ((1 3) (1 4) (2 4) (3 1) (4 1) (4 2)))",
            instance.relations.at(0).Str());
  // e2: supports 1 2 | 1 4 | 2 1 | 2 3 | 3 2 | 3 4 | 4 1 | 4 3
  EXPECT_EQ("e2", instance.relations.at(1).name);
  EXPECT_EQ(2, instance.relations.at(1).arity);
  EXPECT_EQ(Relation::kSupports, instance.relations.at(1).semantics);
  EXPECT_EQ(8, instance.relations.at(1).tuples.size());
  EXPECT_EQ("(e2 2 s ((1 2) (1 4) (2 1) (2 3) (3 2) (3 4) (4 1) (4 3)))",
            instance.relations.at(1).Str());
  // e3: supports 1 2 | 1 3 | 2 1 | 2 3 | 2 4 | 3 1 | 3 2 | 3 4 | 4 2 | 4 3
  EXPECT_EQ("e3", instance.relations.at(2).name);
  EXPECT_EQ(2, instance.relations.at(2).arity);
  EXPECT_EQ(Relation::kSupports, instance.relations.at(2).semantics);
  EXPECT_EQ(10, instance.relations.at(2).tuples.size());
  EXPECT_EQ("(e3 2 s ((1 2) (1 3) (2 1) (2 3) (2 4) (3 1) (3 2) (3 4) (4 2)" +
            string(" (4 3)))"),
            instance.relations.at(2).Str());

  // Check constraints.
  ASSERT_EQ(6, instance.constraints.size());
  // r12: e1(c1, c2)
  EXPECT_EQ("r12", instance.constraints.at(0).name);
  EXPECT_EQ(2, instance.constraints.at(0).arity);
  EXPECT_EQ(vector<string>({"c1", "c2"}), instance.constraints.at(0).scope);
  EXPECT_EQ("e1", instance.constraints.at(0).reference);
  // r23: e1(c2 c3)
  EXPECT_EQ("r23", instance.constraints.at(1).name);
  EXPECT_EQ(2, instance.constraints.at(1).arity);
  EXPECT_EQ(vector<string>({"c2", "c3"}), instance.constraints.at(1).scope);
  EXPECT_EQ("e1", instance.constraints.at(1).reference);
  // r34: e1(c3 c4)
  EXPECT_EQ("r34", instance.constraints.at(2).name);
  EXPECT_EQ(2, instance.constraints.at(2).arity);
  EXPECT_EQ(vector<string>({"c3", "c4"}), instance.constraints.at(2).scope);
  EXPECT_EQ("e1", instance.constraints.at(2).reference);
  // r13: e2(c1 c3)
  EXPECT_EQ("r13", instance.constraints.at(3).name);
  EXPECT_EQ(2, instance.constraints.at(3).arity);
  EXPECT_EQ(vector<string>({"c1", "c3"}), instance.constraints.at(3).scope);
  EXPECT_EQ("e2", instance.constraints.at(3).reference);
  // r24: e2(c2 c4)
  EXPECT_EQ("r24", instance.constraints.at(4).name);
  EXPECT_EQ(2, instance.constraints.at(4).arity);
  EXPECT_EQ(vector<string>({"c2", "c4"}), instance.constraints.at(4).scope);
  EXPECT_EQ("e2", instance.constraints.at(4).reference);
  // r14: e3(c1 c4)
  EXPECT_EQ("r14", instance.constraints.at(5).name);
  EXPECT_EQ(2, instance.constraints.at(5).arity);
  EXPECT_EQ(vector<string>({"c1", "c4"}), instance.constraints.at(5).scope);
  EXPECT_EQ("e3", instance.constraints.at(5).reference);
}

TEST_F(ParserTest, ParseInstance_xml2) {
  Parser parser(xml2_path);
  Instance instance = parser.ParseInstance();

  // Check presentation values.
  // These are set in xml1.
  EXPECT_EQ("xml2", instance.name);
  EXPECT_EQ("XCSP 2.1", instance.format);
  EXPECT_EQ("xml2 description", instance.description);
  // These are not set in xml1.
  EXPECT_EQ(Instance::kDefMaxConstraintArity,
            instance.max_constraint_arity);
  EXPECT_EQ(Instance::kDefMinViolatedConstraints,
            instance.min_violated_constraints);
  EXPECT_EQ(Instance::kDefNumSolutions,
            instance.num_solutions);
  EXPECT_EQ(Instance::kDefType,
            instance.type);

  // Check domains.
  ASSERT_EQ(1, instance.domains.size());
  // D0: 1 2
  EXPECT_EQ("D0", instance.domains.at(0).name);
  EXPECT_EQ(2, instance.domains.at(0).values.size());
  EXPECT_EQ("(D0 (1 2))", instance.domains.at(0).Str());

  // Check variables.
  ASSERT_EQ(1, instance.variables.size());
  // c1: D0
  EXPECT_EQ("c1", instance.variables.at(0).name);
  EXPECT_EQ("D0", instance.variables.at(0).domain);

  // Check relations.
  ASSERT_EQ(1, instance.relations.size());
  // e1: supports 1 3 | 1 4 | 2 4 | 3 1 | 4 1 | 4 2
  EXPECT_EQ("e1", instance.relations.at(0).name);
  EXPECT_EQ(2, instance.relations.at(0).arity);
  EXPECT_EQ(Relation::kSupports, instance.relations.at(0).semantics);
  EXPECT_EQ(6, instance.relations.at(0).tuples.size());
  EXPECT_EQ("(e1 2 s ((1 3) (1 4) (2 4) (3 1) (4 1) (4 2)))",
            instance.relations.at(0).Str());
}
