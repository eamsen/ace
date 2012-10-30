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
  * gflags (http://code.google.com/p/gflags, also included)
  * gtest (http://code.google.com/p/googletest, only for testing)
  * gperftools (http://code.google.com/p/gperftools, only for profiling)
  * cpplint (included, only for style checking)

# Building gflags
  The repository contains a slightly modified gflags version with less verbose
  help output.
  If you want to use the provided version instead, you need to build gflags
  locally:
  `$ make gflags`
  and then activate the two lines in the makefile, which are commented out.

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

# Testing Ace (depends on gtest)
  To build and run the unit tests use:
  `$ make test`

# Profiling Ace (depends on gperftools)
  To build Ace with profiling turned on use:
  `$ make profile`

# Checking style
  To test code style conformance with the [Google C++ Style Guide](http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml) use:
  `$ make checkstyle`
