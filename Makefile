# Makefile
#
# Build on
# Windows 8.1 64-bit Edition
# - Cygwin 64 (1.7.34)
#  - GNU make 4.0
#  - g++ 4.9.2
#  - GNU assembler 2.25
#  - Ruby 2.0.0p598
#  - Perl 5.14.4

# Set link options to link c/c++ libraries statically
# libstdc++-6.dll in MinGW-w64 cannot be loaded on Cygwin
# LIBCFLAGS= -static
LIBCFLAGS=

# -O2 optimization is better than -O3 in this program
CPPFLAGS= -std=c++11 -O2 -Wall -masm=intel -m64 -msse4.2 $(LIBCFLAGS)
LDFLAGS= $(LIBCFLAGS) -mcmodel=small

CELLS_UNPACKED_TARGET= bin/sudokusse.exe
CELLS_PACKED_TARGET= bin/sudokusse_cells_packed.exe
TARGETS= $(CELLS_UNPACKED_TARGET) $(CELLS_PACKED_TARGET)

CELLS_UNPACKED_OBJS= sudokumain.o sudoku.o sudokuxmmreg.o sudokusse_cells_unpacked.o
CELLS_PACKED_OBJS=   sudokumain.o sudoku.o sudokuxmmreg.o sudokusse_cells_packed.o

HEADERS= sudoku.h
GENERATED_CODES = sudokuConstAll.h
GENERATOR_SCRIPT= sudokumap.rb

.PHONY: all clean

all: $(TARGETS)

$(CELLS_UNPACKED_TARGET): $(CELLS_UNPACKED_OBJS)
	$(CXX) -o $@ $(LDFLAGS) $^

$(CELLS_PACKED_TARGET): $(CELLS_PACKED_OBJS)
	$(CXX) -o $@ $(LDFLAGS) $^

sudokumain.o : sudokumain.cpp $(HEADERS) $(GENERATED_CODES)
	$(CXX) -c $(CPPFLAGS) $< -o $@

sudoku.o : sudoku.cpp $(HEADERS) $(GENERATED_CODES)
	$(CXX) -c $(CPPFLAGS) $< -o $@

sudokuxmmreg.o : sudokuxmmreg.cpp $(HEADERS) $(GENERATED_CODES)
	$(CXX) -c $(CPPFLAGS) $< -o $@

sudokusse_cells_unpacked.o : sudokusse.s
	$(AS) -defsym CellsPacked=0 -o $@ $<

sudokusse_cells_packed.o : sudokusse.s
	$(AS) -defsym CellsPacked=1 -o $@ $<

$(GENERATED_CODES) : $(GENERATOR_SCRIPT)
	ruby $(GENERATOR_SCRIPT)

$(HEADERS):

clean:
	$(RM) $(TARGETS) $(CELLS_UNPACKED_OBJS) $(CELLS_PACKED_OBJS) $(GENERATED_CODES)
