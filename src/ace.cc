// Copyright 2012 Eugen Sawin <esawin@me73.com>
#include <gflags/gflags.h>
#include <cassert>
#include <iostream>
#include <vector>
#include "./parser.h"
#include "./network.h"
#include "./network-factory.h"
#include "./backtrack-solver.h"
#include "./backjump-solver.h"
#include "./random-walk-solver.h"
#include "./ac3.h"
#include "./clock.h"
#include "./profiler.h"
#include "./max-cardinality-ordering.h"
#include "./min-width-ordering.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::min;
using base::Clock;
using ace::parse::Parser;
using ace::Solver;
using ace::BacktrackSolver;
using ace::BackjumpSolver;
using ace::RandomWalkSolver;

// Flag for the automatic selection of solving procedures.
DEFINE_bool(auto, false, "Automatic selection of solving procedures.");

// Flag for verbose output.
DEFINE_bool(verbose, false, "Verbose output");

// Flag for Gaschnig's backjumping algorithm.
DEFINE_bool(backjumping, false, "Use Gaschnig's backjumping algorithm.");

// Flag and help text for random walk algorithm.
static const std::string randomwalk_help = string("Use random walk algorithm") +
  " with given parameters MAXTRIES,MAXFLIPS,Z, where p=Z/100.";
DEFINE_string(randomwalk, "", randomwalk_help.c_str());

// Flag for consistency preprocessing.
DEFINE_string(consistency, "ac3",
              "Preprocessing consistency algorithms (none, ac3, ac2001, pc2)");

// Flag for variable ordering heuristic.
DEFINE_string(heuristic, "unspecified",
              "Variable selection heuristic\
               (unspecified, minwidth, maxcardinality)");

// Flag for execution time limit.
DEFINE_int64(timelimit, Solver::kDefTimeLimit * Clock::kSecInMicro,
             "Time limit in seconds.");

// Flag for maximum number of solutions to be found.
DEFINE_int32(maxnumsolutions, 1,
             "Maximum number of solutions to be found (0 for unlimited).");

// The command-line usage text.
static const string kUsage =  // NOLINT
  string("Usage:\n") +
  "  $ ace input.xml\n" +
  "  input.xml is a CSP instance in the XCSP 2.1 format";

namespace ace {

// The main Ace procedure.
void Main(const string& input_path);

// Executes the given preprocessor and logs results and duration.
// Returns whether the network was found to be consistent.
bool Preprocess(Preprocessor* pre, Clock::Diff* duration);

// Selects a solver based on provided flags.
// Returns the solver when successful, nullptr otherwise.
Solver* SelectSolver(Network* network);

}  // namespace ace

int main(int argc, char* argv[]) {
  google::SetUsageMessage(kUsage);
  // Parse command line flags and remove them from the argc and argv.
  google::ParseCommandLineFlags(&argc, &argv, true);
  if (argc != 2) {
    cout << "Wrong argument number provided, use -help for help.\n"
         << kUsage << "\n";
    return 1;
  } else if (!Parser::FileSize(argv[1])) {
    cout << "File " << argv[1] << " is empty or does not exist.\n";
    return 1;
  }
  const string input_path = argv[1];
  ace::Main(input_path);
  return 0;
}

namespace ace {

using base::Profiler;
using parse::Instance;

void Main(const string& input_path) {
  const Clock beg;
  Parser parser(input_path);
  Instance instance = parser.ParseInstance();
  NetworkFactory factory;
  Profiler::Start("log/network-factory.prof");
  Network network = factory.Create(instance);
  Profiler::Stop();
  // Gaschnig's backjumping overrides consistency options.
  if (FLAGS_backjumping || FLAGS_randomwalk.size()) {
    FLAGS_consistency = "none";
  }
  if (FLAGS_auto) {
    FLAGS_consistency = "ac3";
    FLAGS_randomwalk.clear();
    FLAGS_backjumping = false;
    FLAGS_heuristic = "minwidth";
    FLAGS_timelimit = min(FLAGS_timelimit, 5 * Clock::kSecInMin) / 2;
  }
  // Output problem and parsing stats.
  if (FLAGS_verbose) {
    cout << "File: " << input_path << "\n";
  }

  bool consistent = true;
  Clock::Diff preprocess_time = 0;
  // Consistency preprocessing.
  if (consistent && FLAGS_consistency.find("ac3") != string::npos) {
    Ac3 pre(&network);
    Clock::Diff duration = 0;
    consistent = Preprocess(&pre, &duration);
    preprocess_time += duration;
  }
  Clock::Diff solver_time = 0;
  Solver* const solver = SelectSolver(&network);
  bool sat = false;
  if (solver) {
    // Solve.
    solver->time_limit(FLAGS_timelimit * Clock::kMicroInSec - (Clock() - beg));
    solver->max_num_solutions(FLAGS_maxnumsolutions);
    Profiler::Start("log/solve.prof");
    sat = solver->Solve();
    Profiler::Stop();
    solver_time = solver->duration();
  }

  // Output results.
  if (!consistent) {
    // Inconsistent problems are unsatisfiable.
    cout << "UNSAT\n";
  } else if (!solver) {
    cout << "INVALID FLAGS\n";
  } else if (sat) {
    cout << "SAT\n"
         << solver->solutions().back().Str() << "\n";
  } else if (solver_time >= solver->time_limit()) {
    MaxCardinalityOrdering var_ordering(network);
    BacktrackSolver* backtrack_solver = static_cast<BacktrackSolver*>(solver);
    backtrack_solver->variable_ordering(var_ordering);
    if (backtrack_solver->Solve()) {
      cout << "SAT\n"
           << solver->solutions().back().Str() << "\n";
    } else {
      solver_time = solver->duration();
      cout << (solver_time > solver->time_limit() ? "INDETERMINATE\n" :
                                                    "UNSAT\n");
    }
  } else {
    cout << (solver_time > solver->time_limit() ? "INDETERMINATE\n" :
                                                  "UNSAT\n");
  }
  if (consistent && FLAGS_randomwalk.size()) {
    // Exploration stats of the random walk.
    RandomWalkSolver* random_walk_solver =
      static_cast<RandomWalkSolver*>(solver);
    cout << "\nRandom steps: " << random_walk_solver->num_random_steps()
         << "\nGreedy steps: " << random_walk_solver->num_greedy_steps();
  } else if (consistent && FLAGS_verbose) {
    // Exploration stats of backtrack-based algorithms.
    double explored = solver->num_explored_states();
    cout << "\nExplored: " << explored << " of "
         <<  network.num_states()
         << " (" << explored / network.num_states() * 100.0 << "%)"
         << "\nBacktracks: " << solver->num_backtracks();
  }
  if (FLAGS_verbose) {
    const Clock::Diff parse_time = parser.duration();
    const Clock::Diff construction_time = factory.duration();
    cout << "\nParse time: " << Clock::DiffStr(parse_time)
         << "\nConstruction time: " << Clock::DiffStr(construction_time)
         << "\nPreprocess time: " << Clock::DiffStr(preprocess_time)
         << "\nSolve time: " << Clock::DiffStr(solver_time)
         << "\nTotal time: " << Clock::DiffStr(parse_time + construction_time +
                                               preprocess_time + solver_time)
         << endl;
  }

  delete solver;
}

bool Preprocess(Preprocessor* pre, Clock::Diff* duration) {
  Profiler::Start("log/" + pre->type() + ".prof");
  const bool consistent = pre->Preprocess();
  Profiler::Stop();
  *duration = pre->duration();
  if (FLAGS_verbose) {
    cout << pre->type() << ": " << (consistent ? "consistent" : "inconsistent")
         << " (" << pre->num_processed() << " removed)\n"
         << pre->type() << " iterations: " << pre->num_iterations() << "\n"
         << pre->type() << " time: " << Clock::DiffStr(*duration) << "\n";
  }
  return consistent;
}

Solver* SelectSolver(Network* network) {
  // Choose solving algorithm.
  if (FLAGS_backjumping) {
    // Prepare solver for the basic version of Gaschnig's backjumping.
    BackjumpSolver* backjump_solver = new BackjumpSolver(network);
    return backjump_solver;
  }
  if (FLAGS_randomwalk.size()) {
    // Prepare solver for random-walk.
    const vector<int> parameters = Parser::CollectInts(FLAGS_randomwalk, ",");
    if (parameters.size() != 3) {
      cout << "Wrong parameter size for randomwalk flag.\n";
      return nullptr;
    }
    RandomWalkSolver* random_walk_solver = new RandomWalkSolver(network);
    random_walk_solver->max_num_tries(parameters[0]);
    random_walk_solver->max_num_flips(parameters[1]);
    random_walk_solver->random_probability(parameters[2]);
    cout << "Random walk max tries: " << random_walk_solver->max_num_tries()
         << "\nRandom walk max flips: " << random_walk_solver->max_num_flips()
         << "\nRandom step probability: "
         << random_walk_solver->random_probability() << "%\n";
    return random_walk_solver;
  }

  // Prepare solver for backtracking with arc-consistency look-ahead.
  BacktrackSolver* backtrack_solver = new BacktrackSolver(network);
  // Choose variable ordering.
  if (FLAGS_heuristic == "maxcardinality") {
    MaxCardinalityOrdering var_ordering(*network);
    backtrack_solver->variable_ordering(var_ordering);
  } else if (FLAGS_heuristic == "minwidth") {
    MinWidthOrdering var_ordering(*network);
    backtrack_solver->variable_ordering(var_ordering);
  }
  return backtrack_solver;
}

}  // namespace ace
