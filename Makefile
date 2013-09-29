# Makefile
#
# build on
# Windows 7 64-bit Edition Service Pack 1
# - MinGW-w64 for 64 bit Windows (ver. 4.5)
# - MSYS 1.0.11
# - GNU make 3.81
# - g++ 4.5.2
# - GNU assembler 2.20.51
# Ruby 1.9.2dev

# set link options to link c/c++ libraries statically
LIBCFLAGS=

# -O2 optimization is better than -O3
CPPFLAGS= -O2 -Wall -masm=intel -m64 -msse4.2 $(LIBCFLAGS)
LDFLAGS= $(LIBCFLAGS)

CELLS_UNPACKED_TARGET= bin/sudokusse.exe
CELLS_PACKED_TARGET= bin/sudokusse_cells_packed.exe
TARGETS = $(CELLS_UNPACKED_TARGET) $(CELLS_PACKED_TARGET)

CELLS_UNPACKED_OBJS= sudoku.o sudokumain.o sudokumap.o sudokuxmmreg.o sudokusse_cells_unpacked.o
CELLS_PACKED_OBJS=   sudoku.o sudokumain.o sudokumap.o sudokuxmmreg.o sudokusse_cells_packed.o

HEADERS= sudoku.h

GENERATED_CODE=sudokumap.cpp
GENERATOR_SCRIPT=sudokumap.rb

.PHONY: all clean

all: $(TARGETS)

$(CELLS_UNPACKED_TARGET): $(CELLS_UNPACKED_OBJS)
	$(CXX) -o $@ $(LDFLAGS) $^

$(CELLS_PACKED_TARGET): $(CELLS_PACKED_OBJS)
	$(CXX) -o $@ $(LDFLAGS) $^

$(CELLS_UNPACKED_OBJS): $(HEADERS) Makefile

$(CELLS_PACKED_OBJS): $(HEADERS) Makefile

sudokusse_cells_unpacked.o : sudokusse.s
	$(AS) -defsym CellsPacked=0 -o $@ $<

sudokusse_cells_packed.o : sudokusse.s
	$(AS) -defsym CellsPacked=1 -o $@ $<

$(GENERATED_CODE): $(GENERATOR_SCRIPT) Makefile
	ruby $< > $@

clean:
	rm -f $(TARGETS) $(CELLS_UNPACKED_OBJS) $(CELLS_PACKED_OBJS) $(GENERATED_CODE)
