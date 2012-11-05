# Ace CSP Solver
by Eugen Sawin <esawin@me73.com>

## Current version
This is the version that was used to win the competition at the CSPs course at
the University of Freiburg.   
It is a stripped version of the original solver missing a lot of
(suboptimal/not optimized) techniques and supporting only binary constraints.  
At some point, I will merge the original features back into this repository, as
long as they provide good results and don't break the efficiency of the general
approach.

## License
The MIT License

## Requirements
  * POSIX.1b-compliant operating system (librt)
  * GNU GCC 4.6 or newer
  * GNU Make
  * Python 2.7 or newer (only for style checking)

## Dependencies
  * gflags (http://code.google.com/p/gflags or `$ make gflags`)
  * gtest (http://code.google.com/p/googletest, only for testing)
  * gperftools (http://code.google.com/p/gperftools, only for profiling)
  * cpplint (`$ make cppling`, only for style checking)

## Building gflags
  The repository contains a slightly modified gflags version with less verbose
  help output.
  If you want to use the provided version instead, you need to build gflags
  locally:
  `$ make gflags`
  and then activate the two lines in the makefile, which are commented out.

  Alternatively you can build all dependencies at once:
  `$ make depend`

## Building Ace (depends on gflags)
  To build Ace use:
  `$ make`

  For performance measuring use the more optimised version:
  `$ make opt`

## Using Ace
  `$ ace xcsp-input.xml`

  To show the full usage and flags help use:
  `$ ace -help`

## Running Ace performance tests
  To run performance tests on some benchmark problems use:
  `$ make perftest`

  All test parameters can be changed on invocation like this:
  `$ make perftest ARGS="-consistency=ac2001 -verbose"`

## Testing Ace (depends on gtest)
  To build and run the unit tests use:
  `$ make check`

## Profiling Ace (depends on gperftools)
  To build Ace with profiling turned on use:
  `$ make profile`

## Getting cpplint
  Code style checking depends on a modified version of Google's cpplint.  
  Get it via `$ make cpplint`.

## Checking style (depends on cpplint)
  To test code style conformance with the [Google C++ Style Guide](http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml) use:
  `$ make checkstyle`
