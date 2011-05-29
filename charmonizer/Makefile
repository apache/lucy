rm=/bin/rm -f
CC= cc
DEFS=  
PROGNAME= charmonize
INCLUDES=  -I. -Isrc
LIBS=
TESTS= TestDirManip TestFuncMacro TestHeaders TestIntegers TestLargeFiles TestUnusedVars TestVariadicMacros


DEFINES= $(INCLUDES) $(DEFS)
CFLAGS= -g $(DEFINES)

SRCS = charmonize.c src/Charmonizer/Probe.c src/Charmonizer/Core/Compiler.c src/Charmonizer/Core/ConfWriter.c src/Charmonizer/Core/Dir.c src/Charmonizer/Core/HeaderChecker.c src/Charmonizer/Core/OperatingSystem.c src/Charmonizer/Core/Stat.c src/Charmonizer/Core/Util.c src/Charmonizer/Probe/AtomicOps.c src/Charmonizer/Probe/DirManip.c src/Charmonizer/Probe/Floats.c src/Charmonizer/Probe/FuncMacro.c src/Charmonizer/Probe/Headers.c src/Charmonizer/Probe/Integers.c src/Charmonizer/Probe/LargeFiles.c src/Charmonizer/Probe/Memory.c src/Charmonizer/Probe/UnusedVars.c src/Charmonizer/Probe/VariadicMacros.c

TEST_SRCS = src/Charmonizer/Test.c src/Charmonizer/Test/TestDirManip.c src/Charmonizer/Test/TestFuncMacro.c src/Charmonizer/Test/TestHeaders.c src/Charmonizer/Test/TestIntegers.c src/Charmonizer/Test/TestLargeFiles.c src/Charmonizer/Test/TestUnusedVars.c src/Charmonizer/Test/TestVariadicMacros.c  

OBJS = charmonize.o src/Charmonizer/Probe.o src/Charmonizer/Core/Compiler.o src/Charmonizer/Core/ConfWriter.o src/Charmonizer/Core/Dir.o src/Charmonizer/Core/HeaderChecker.o src/Charmonizer/Core/OperatingSystem.o src/Charmonizer/Core/Stat.o src/Charmonizer/Core/Util.o src/Charmonizer/Probe/AtomicOps.o src/Charmonizer/Probe/DirManip.o src/Charmonizer/Probe/Floats.o src/Charmonizer/Probe/FuncMacro.o src/Charmonizer/Probe/Headers.o src/Charmonizer/Probe/Integers.o src/Charmonizer/Probe/LargeFiles.o src/Charmonizer/Probe/Memory.o src/Charmonizer/Probe/UnusedVars.o src/Charmonizer/Probe/VariadicMacros.o

TEST_OBJS = src/Charmonizer/Test.o src/Charmonizer/Test/TestDirManip.o src/Charmonizer/Test/TestFuncMacro.o src/Charmonizer/Test/TestHeaders.o src/Charmonizer/Test/TestIntegers.o src/Charmonizer/Test/TestLargeFiles.o src/Charmonizer/Test/TestUnusedVars.o src/Charmonizer/Test/TestVariadicMacros.o  

.c.o:
	$(CC) $(CFLAGS) -c $*.c -o $@

all: $(PROGNAME)

tests: $(TESTS)

$(PROGNAME) : $(OBJS)
	$(CC) $(CFLAGS) -o $(PROGNAME) $(OBJS) $(LIBS)

TestDirManip:	src/Charmonizer/Test.o src/Charmonizer/Test/TestDirManip.o
	$(CC) $(CFLAGS) -o $@ src/Charmonizer/Test.o src/Charmonizer/Test/TestDirManip.o $(LIBS)

TestFuncMacro:	src/Charmonizer/Test.o src/Charmonizer/Test/TestFuncMacro.o
	$(CC) $(CFLAGS) -o $@ src/Charmonizer/Test.o src/Charmonizer/Test/TestFuncMacro.o $(LIBS)

TestHeaders:	src/Charmonizer/Test.o src/Charmonizer/Test/TestHeaders.o
	$(CC) $(CFLAGS) -o $@ src/Charmonizer/Test.o src/Charmonizer/Test/TestHeaders.o $(LIBS)

TestIntegers:	src/Charmonizer/Test.o src/Charmonizer/Test/TestIntegers.o
	$(CC) $(CFLAGS) -o $@ src/Charmonizer/Test.o src/Charmonizer/Test/TestIntegers.o $(LIBS)

TestLargeFiles:	src/Charmonizer/Test.o src/Charmonizer/Test/TestLargeFiles.o
	$(CC) $(CFLAGS) -o $@ src/Charmonizer/Test.o src/Charmonizer/Test/TestLargeFiles.o $(LIBS)

TestUnusedVars:	src/Charmonizer/Test.o src/Charmonizer/Test/TestUnusedVars.o
	$(CC) $(CFLAGS) -o $@ src/Charmonizer/Test.o src/Charmonizer/Test/TestUnusedVars.o $(LIBS)

TestVariadicMacros:	src/Charmonizer/Test.o src/Charmonizer/Test/TestVariadicMacros.o
	$(CC) $(CFLAGS) -o $@ src/Charmonizer/Test.o src/Charmonizer/Test/TestVariadicMacros.o $(LIBS)


clean:
	$(rm) $(OBJS) $(TEST_OBJS) $(PROGNAME) $(TESTS) core *~

