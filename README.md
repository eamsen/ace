# Ace CSP Solver
by Eugen Sawin <esawin@me73.com>

# License
The MIT License

# Requirements
  * POSIX.1b-compliant operating system (librt)
  * GNU GCC 4.6 or newer
  * GNU Make
  * Python 2.7 or newer (only for style checking)

# Dependencies
  * gtest (not included, only required for testing)
  * gperftools (not included, only required for profiling)
  * gflags (included)
  * cpplint (included, only required for style checking)

# Building gflags
  Before building ace, you need to build gflags locally once:
  `$ make gflags`

  Alternatively you can build all dependencies at once:
  `$ make depend`

# Building Ace (depends on gflags)
  To build Ace use:
  `$ make`

  For performance measuring use the more optimised version:
  `$ make opt`

# Using Ace
  `$ ace xcsp-input.xml`

  To show the full usage and flags help use:
  `$ ace -help`

# Running Ace performance tests
  To run performance tests on some benchmark problems use:
  `$ make perftest`

  All test parameters can be changed on invocation like this:
  `$ make perftest ARGS="-consistency=ac2001 -verbose"`

# Building gtest
  Before building the tests, you need to build gtest locally once:
  `$ make gtest`

# Testing Ace (depends on gtest)
  To build and run the unit tests use:
  `$ make test`

# Profiling Ace (depends on gperftools)
  To build Ace with profiling turned on use:
  `$ make profile`

# Checking style
  To test code style conformance with the [Google C++ Style Guide](http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml) use:
  `$ make checkstyle`
