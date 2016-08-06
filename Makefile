# Makefile

CXX?=g++
RUBY?=ruby

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

ifneq (,$(findstring linux,$(shell gcc -dumpmachine)))
OS_DEPENDENT_SRC=sudoku_linux.cpp
else
OS_DEPENDENT_SRC=sudoku_windows.cpp
endif
OS_DEPENDENT_OBJ=$(patsubst %.cpp,%.o,$(OS_DEPENDENT_SRC))

CELLS_UNPACKED_OBJS=sudokumain.o sudoku.o sudokuxmmreg.o sudokusse_cells_unpacked.o $(OS_DEPENDENT_OBJ)
CELLS_PACKED_OBJS=sudokumain.o sudoku.o sudokuxmmreg.o sudokusse_cells_packed.o $(OS_DEPENDENT_OBJ)

HEADERS=sudoku.h sudoku_os_dependent.h
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

$(OS_DEPENDENT_OBJ) : $(OS_DEPENDENT_SRC) $(HEADERS) $(GENERATED_CODES)
	$(CXX) -c $(CPPFLAGS) $< -o $@

sudokusse_cells_unpacked.o : sudokusse.s
	$(AS) -defsym CellsPacked=0 $(ASFLAGS_SSE_AVX) -o $@ $<

sudokusse_cells_packed.o : sudokusse.s
	$(AS) -defsym CellsPacked=1 $(ASFLAGS_SSE_AVX) -o $@ $<

$(GENERATED_CODES) : $(GENERATOR_SCRIPT)
	$(RUBY) $(GENERATOR_SCRIPT)

$(HEADERS):

clean:
	$(RM) $(TARGETS) $(CELLS_UNPACKED_OBJS) $(CELLS_PACKED_OBJS) $(GENERATED_CODES) ./*.o
