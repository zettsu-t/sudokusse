# Makefile
#
# Build on
# Windows 8.1 64-bit Edition
# - Cygwin 64 (2.2.1)
#  - GNU make 4.1
#  - g++ 4.9.3
#  - GNU assembler 2.25
#  - Ruby 2.2.2p95
#  - Perl 5.22.0

# Set link options to link c/c++ libraries statically
# LIBCFLAGS=-static
LIBCFLAGS=

# Set 0 to use SSE and 1 to AVX
USE_AVX_VALUE=1
ASFLAGS_SSE_AVX=-defsym EnableAvx=$(USE_AVX_VALUE)

ifeq ($(USE_AVX_VALUE),0)
CPPFLAGS_SSE_AVX=-msse4.2
else
CPPFLAGS_SSE_AVX=-mavx2
endif

# -O2 optimization is better than -O3 in this program
CPPFLAGS=-std=c++11 -O2 -Wall -masm=intel -m64 $(CPPFLAGS_SSE_AVX) $(LIBCFLAGS)
LDFLAGS=$(LIBCFLAGS) -mcmodel=small

CELLS_UNPACKED_TARGET=bin/sudokusse
CELLS_PACKED_TARGET=bin/sudokusse_cells_packed
TARGETS=$(CELLS_UNPACKED_TARGET) $(CELLS_PACKED_TARGET)

CELLS_UNPACKED_OBJS=sudokumain.o sudoku.o sudokuxmmreg.o sudokusse_cells_unpacked.o
CELLS_PACKED_OBJS=sudokumain.o sudoku.o sudokuxmmreg.o sudokusse_cells_packed.o

HEADERS=sudoku.h
GENERATED_CODES=sudokuConstAll.h
GENERATOR_SCRIPT=sudokumap.rb

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
	$(AS) -defsym CellsPacked=0 $(ASFLAGS_SSE_AVX) -o $@ $<

sudokusse_cells_packed.o : sudokusse.s
	$(AS) -defsym CellsPacked=1 $(ASFLAGS_SSE_AVX) -o $@ $<

$(GENERATED_CODES) : $(GENERATOR_SCRIPT)
	ruby $(GENERATOR_SCRIPT)

$(HEADERS):

clean:
	$(RM) $(TARGETS) $(CELLS_UNPACKED_OBJS) $(CELLS_PACKED_OBJS) $(GENERATED_CODES)
