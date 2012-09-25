# Copyright 2012: Eugen Sawin <esawin@me73.com>
SRCDIR:=src
TSTDIR:=src/test
BINDIR:=bin
OBJDIR:=bin/obj
GTESTLIBS:=-lgtest -lgtest_main
GFLAGSDIR:=libs/gflags-2.0/.libs
CXX:=g++ -std=c++0x -Ilibs/gflags-2.0/src
CFLAGS:=-Wall -O3
LIBS:=$(GFLAGSDIR)/libgflags.a -lpthread -lrt
TSTFLAGS:=
TSTLIBS:=$(GTESTLIBS) -lpthread -lrt
BINS:=ace

TSTBINS:=$(notdir $(basename $(wildcard $(TSTDIR)/*.cc)))
TSTOBJS:=$(addsuffix .o, $(notdir $(basename $(wildcard $(TSTDIR)/*.cc))))
OBJS:=$(notdir $(basename $(wildcard $(SRCDIR)/*.cc)))
OBJS:=$(addsuffix .o, $(filter-out $(BINS), $(OBJS)))
OBJS:=$(addprefix $(OBJDIR)/, $(OBJS))
BINS:=$(addprefix $(BINDIR)/, $(BINS))
TSTBINS:=$(addprefix $(BINDIR)/, $(TSTBINS))

compile: makedirs $(BINS)
	@echo compiled all

profile: CFLAGS=-Wall -O3 -DPROFILE
profile: LIBS+=-lprofiler
profile: clean compile

opt: CFLAGS=-Ofast -flto -mtune=native -DNDEBUG
opt: clean compile

debug: CFLAGS=-O0 -g
debug: compile

BENCHMARKS:=normal
ARGS:=-consistency=ac3
LOG:=perf-results.txt
perftest: opt
	@mkdir -p log; rm -f log/$(LOG);
	@echo "test parameters: BENCHMARKS=$(BENCHMARKS) ARGS=$(ARGS) LOG=$(LOG)";
	@for i in benchmarks/$(BENCHMARKS)/*.xml;\
		do echo "testing $$i";\
		./bin/ace $$i $(ARGS) >> log/$(LOG);\
		echo " " >> log/$(LOG);\
	done
	@echo "tested all (results in log/$(LOG))";

depend: gflags 

makedirs:
	@mkdir -p bin/obj

gflags:
	@cd libs/gflags-2.0/; ./configure; make;
	@echo compiled gflags

test: makedirs $(TSTBINS)
	@for t in $(TSTBINS); do ./$$t; done
	@echo completed tests

checkstyle:
	@python libs/cpplint/cpplint.py \
		--filter=-readability/streams,-readability/multiline_string\
		$(SRCDIR)/*.h $(SRCDIR)/*.cc

clean:
	@rm -f $(OBJDIR)/*.o
	@rm -f $(BINS)
	@rm -f $(TSTBINS)
	@echo cleaned

.PRECIOUS: $(OBJS) $(TSTOBJS)
.PHONY: compile profile opt perftest depend makedirs gflags test\
	checkstyle clean

$(BINDIR)/%: $(OBJS) $(SRCDIR)/%.cc
	@$(CXX) $(CFLAGS) -o $(OBJDIR)/$(@F).o -c $(SRCDIR)/$(@F).cc
	@$(CXX) $(CFLAGS) -o $(BINDIR)/$(@F) $(OBJDIR)/$(@F).o $(OBJS) $(LIBS)
	@echo compiled $(BINDIR)/$(@F)

$(BINDIR)/%-test: $(OBJDIR)/%-test.o $(OBJS)
	@$(CXX) $(TSTFLAGS) -o $(BINDIR)/$(@F) $(OBJS) $< $(TSTLIBS)
	@echo compiled $(BINDIR)/$(@F)

$(OBJDIR)/%-test.o: $(TSTDIR)/%-test.cc
	@$(CXX) $(TSTFLAGS) -o $(OBJDIR)/$(@F) -c $<

$(OBJDIR)/%.o: $(SRCDIR)/%.cc $(SRCDIR)/%.h
	@$(CXX) $(CFLAGS) -o $(OBJDIR)/$(@F) -c $<
