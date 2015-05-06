# Makefile
#
# Available Cygwin only (MinGW with CppUnit not supported)
# Add path /local/... to CppUnit 1.13.1 for MinGW-w64 + Msys
INCLUDES=-I. -I.. -I/local/include
LIBPATH=-L/local/lib

# Only a DLL is available for CppUnit on Cygwin
LIBCFLAGS=
LIBS= -lcppunit
CPPFLAGS= -std=c++11 -g -Wall -masm=intel -m64 -msse4.2 -DUNITTEST $(INCLUDES) $(LIBCFLAGS)
LDFLAGS= $(LIBCFLAGS) -mcmodel=small

CELLS_UNPACKED_TARGET= unittest.exe
CELLS_PACKED_TARGET= unittest_cells_packed.exe
TARGETS = $(CELLS_UNPACKED_TARGET) $(CELLS_PACKED_TARGET)

TESTEROBJS= main.o sudokussetest.o sudokutest.o sudokucelltest.o \
            sudokumaptest.o sudokusolvertest.o sudokuloadertest.o

TESTEDOBJS= sudoku.o sudokuxmmreg.o

CELLS_UNPACKED_OBJS= $(TESTEROBJS) $(TESTEDOBJS) sudokusse_cells_unpacked.o
CELLS_PACKED_OBJS=   $(TESTEROBJS) $(TESTEDOBJS) sudokusse_cells_packed.o

HEADERS = ../sudoku.h sudokutest.h
GENERATED_CODES = sudokuConstAll.h
GENERATOR_SCRIPT= ../sudokumap.rb

.PHONY: all clean

all: $(TARGETS)

$(CELLS_UNPACKED_TARGET): $(CELLS_UNPACKED_OBJS)
	$(CXX) $(LIBPATH) -o $@ $(LDFLAGS) $^ $(LIBS)
	./$(CELLS_UNPACKED_TARGET)

$(CELLS_PACKED_TARGET): $(CELLS_PACKED_OBJS)
	$(CXX) $(LIBPATH) -o $@ $(LDFLAGS) $^ $(LIBS)
	./$(CELLS_PACKED_TARGET)

# no inlining
main.o : main.cpp $(HEADERS) $(GENERATED_CODES)
	$(CXX) -c $(CPPFLAGS) $< -o $@

sudokussetest.o : sudokussetest.cpp $(HEADERS) $(GENERATED_CODES)
	$(CXX) -c $(CPPFLAGS) $< -o $@

sudokutest.o : sudokutest.cpp $(HEADERS) $(GENERATED_CODES)
	$(CXX) -c $(CPPFLAGS) $< -o $@

sudokucelltest.o : sudokucelltest.cpp $(HEADERS) $(GENERATED_CODES)
	$(CXX) -c $(CPPFLAGS) $< -o $@

sudokumaptest.o : sudokumaptest.cpp $(HEADERS) $(GENERATED_CODES)
	$(CXX) -c $(CPPFLAGS) $< -o $@

sudokusolvertest.o : sudokusolvertest.cpp $(HEADERS) $(GENERATED_CODES)
	$(CXX) -c $(CPPFLAGS) $< -o $@

sudokuloadertest.o : sudokuloadertest.cpp $(HEADERS) $(GENERATED_CODES)
	$(CXX) -c $(CPPFLAGS) $< -o $@

sudoku.o : ../sudoku.cpp $(HEADERS) $(GENERATED_CODES)
	$(CXX) -c $(CPPFLAGS) $< -o $@

sudokuxmmreg.o : ../sudokuxmmreg.cpp $(HEADERS) $(GENERATED_CODES)
	$(CXX) -c $(CPPFLAGS) $< -o $@

sudokusse_cells_unpacked.o : ../sudokusse.s
	$(AS) -defsym CellsPacked=0 -o $@ $<

sudokusse_cells_packed.o : ../sudokusse.s
	$(AS) -defsym CellsPacked=1 -o $@ $<

$(GENERATED_CODES) : $(GENERATOR_SCRIPT)
	ruby $(GENERATOR_SCRIPT)

$(HEADERS):

clean:
	$(RM) $(TARGETS) $(CELLS_UNPACKED_OBJS) $(CELLS_PACKED_OBJS) $(GENERATED_CODES)
