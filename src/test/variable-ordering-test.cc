// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fstream>
#include <vector>
#include <set>
#include "../parser.h"
#include "../network.h"
#include "../network-factory.h"
#include "../min-width-ordering.h"
#include "../max-cardinality-ordering.h"

using std::vector;
using std::set;
using std::string;
using std::cout;
using std::endl;
using std::ofstream;

using ace::parse::Parser;
using ace::parse::Instance;
using ace::Network;
using ace::NetworkFactory;

using ::testing::ElementsAre;
using ::testing::Contains;
using ::testing::Not;
// using ::testing::WhenSortedBy;

class VariableOrderingTest : public ::testing::Test {
 public:
  void SetUp() {
    string xml1 =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
    <instance>\n\
      <presentation\
       name=\"ex7.2\"\
       maxConstraintArity=\"2\"\
       format=\"XCSP 2.1\"/>\n\
      <domains nbDomains=\"1\">\n\
        <domain name=\"D0\" nbValues=\"2\">1 2</domain>\n\
      </domains>\n\
      <variables nbVariables=\"9\">\n\
        <variable name=\"v1\" domain=\"D0\"/>\n\
        <variable name=\"v2\" domain=\"D0\"/>\n\
        <variable name=\"v3\" domain=\"D0\"/>\n\
        <variable name=\"v4\" domain=\"D0\"/>\n\
        <variable name=\"v5\" domain=\"D0\"/>\n\
        <variable name=\"v6\" domain=\"D0\"/>\n\
        <variable name=\"v7\" domain=\"D0\"/>\n\
        <variable name=\"v8\" domain=\"D0\"/>\n\
        <variable name=\"v9\" domain=\"D0\"/>\n\
      </variables>\n\
      <relations nbRelations=\"1\">\n\
        <relation name=\"e1\" arity=\"2\" nbTuples=\"0\"\
         semantics=\"conflicts\">\
        </relation>\n\
      </relations>\n\
      <constraints nbConstraints=\"13\">\n\
        <constraint name=\"c12\" arity=\"2\" scope=\"v1 v2\"\
         reference=\"e1\"/>\n\
        <constraint name=\"c14\" arity=\"2\" scope=\"v1 v4\"\
         reference=\"e1\"/>\n\
        <constraint name=\"c18\" arity=\"2\" scope=\"v1 v8\"\
         reference=\"e1\"/>\n\
        <constraint name=\"c23\" arity=\"2\" scope=\"v2 v3\"\
         reference=\"e1\"/>\n\
        <constraint name=\"c25\" arity=\"2\" scope=\"v2 v5\"\
         reference=\"e1\"/>\n\
        <constraint name=\"c35\" arity=\"2\" scope=\"v3 v5\"\
         reference=\"e1\"/>\n\
        <constraint name=\"c36\" arity=\"2\" scope=\"v3 v6\"\
         reference=\"e1\"/>\n\
        <constraint name=\"c45\" arity=\"2\" scope=\"v4 v5\"\
         reference=\"e1\"/>\n\
        <constraint name=\"c47\" arity=\"2\" scope=\"v4 v7\"\
         reference=\"e1\"/>\n\
        <constraint name=\"c56\" arity=\"2\" scope=\"v5 v6\"\
         reference=\"e1\"/>\n\
        <constraint name=\"c58\" arity=\"2\" scope=\"v5 v8\"\
         reference=\"e1\"/>\n\
        <constraint name=\"c78\" arity=\"2\" scope=\"v7 v8\"\
         reference=\"e1\"/>\n\
        <constraint name=\"c89\" arity=\"2\" scope=\"v8 v9\"\
         reference=\"e1\"/>\n\
      </constraints>\n\
    </instance>";
    string xml1_path = "/tmp/ace-variable-ordering-test-xml1.xml";
    ofstream xml1_stream(xml1_path.c_str());
    xml1_stream.write(xml1.c_str(), xml1.size());
    xml1_stream.close();
    Parser parser(xml1_path);
    Instance instance = parser.ParseInstance();
    NetworkFactory factory;
    network = factory.Create(instance);

    ASSERT_EQ("v1", network.variable(0).name());
    ASSERT_EQ("v5", network.variable(4).name());
    ASSERT_EQ("v9", network.variable(8).name());
  }

  void TearDown() {
  }

  Network network;
};

TEST_F(VariableOrderingTest, MinWidth) {
  using ace::MinWidthOrdering;
  MinWidthOrdering ordering(network);
  vector<int> expected({7, 6, 4, 3, 0, 1, 2, 5, 8});
  EXPECT_EQ(expected, ordering.CreateOrdering());
}

TEST_F(VariableOrderingTest, MaxCardinality) {
  using ace::MaxCardinalityOrdering;
  MaxCardinalityOrdering ordering(network);
  vector<int> expected({0, 1, 2, 4, 3, 5, 7, 6, 8});
  EXPECT_EQ(expected, ordering.CreateOrdering());
}
