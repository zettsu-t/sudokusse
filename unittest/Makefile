# Makefile
#
# add path to CppUnit 1.13.1 for MinGW-w64 + Msys
INCLUDES=-I. -I.. -I/local/include
LIBPATH=-L/local/lib

CELLS_UNPACKED_TARGET= unittest.exe
CELLS_PACKED_TARGET= unittest_cells_packed.exe
TARGETS = $(CELLS_UNPACKED_TARGET) $(CELLS_PACKED_TARGET)
CPPFLAGS=-g -Wall -masm=intel -DUNITTEST $(INCLUDES)

TESTEROBJS= \
	main.o \
	sudokussetest.o \
	sudokutest.o \
	sudokucelltest.o \
	sudokumaptest.o \
	sudokusolvertest.o \
	sudokuloadertest.o \

TESTEDOBJS= \
	sudoku.o \
	sudokuxmmreg.o \
	sudokumap.o \

CELLS_UNPACKED_OBJS= $(TESTEROBJS) $(TESTEDOBJS) sudokusse_cells_unpacked.o
CELLS_PACKED_OBJS=   $(TESTEROBJS) $(TESTEDOBJS) sudokusse_cells_packed.o

HEADERS = \
	../sudoku.h \
	sudokutest.h

GENERATED_CODE=sudokumap.cpp
GENERATOR_SCRIPT=../sudokumap.rb

.PHONY: all clean

all: $(TARGETS)

$(CELLS_UNPACKED_TARGET): $(CELLS_UNPACKED_OBJS)
	$(CXX) $(LIBPATH) -o $@ $(LDFLAGS) $^ -lcppunit
	./$(CELLS_UNPACKED_TARGET)

$(CELLS_PACKED_TARGET): $(CELLS_PACKED_OBJS)
	$(CXX) $(LIBPATH) -o $@ $(LDFLAGS) $^ -lcppunit
	./$(CELLS_PACKED_TARGET)

$(CELLS_UNPACKED_OBJS): $(HEADERS) Makefile

$(CELLS_PACKED_OBJS): $(HEADERS) Makefile

# no inlining
sudoku.o: ../sudoku.cpp
	$(CXX) $(CPPFLAGS) -c $<

sudokuxmmreg.o: ../sudokuxmmreg.cpp
	$(CXX) $(CPPFLAGS) -c $<

sudokumap.o: sudokumap.cpp
	$(CXX) $(CPPFLAGS) -c $<

sudokusse_cells_unpacked.o : ../sudokusse.s
	$(AS) -defsym CellsPacked=0 -o $@ $<

sudokusse_cells_packed.o : ../sudokusse.s
	$(AS) -defsym CellsPacked=1 -o $@ $<

$(GENERATED_CODE): $(GENERATOR_SCRIPT) Makefile
	ruby $< > $@

clean:
	rm -f $(TARGETS) $(CELLS_UNPACKED_OBJS) $(CELLS_PACKED_OBJS) $(GENERATED_CODE)
