# Makefile

POSTFIX_UNPACKED=_unpacked
POSTFIX_PACKED=_packed
POSTFIX_SSE=_sse
POSTFIX_AVX=_avx

# Available Cygwin only (MinGW with CppUnit not supported)
INCLUDES=-I. -I..
LIBPATH=
LIBCFLAGS=
LIBS=-lcppunit
LDFLAGS=$(LIBCFLAGS) -mcmodel=small

CPPFLAGS_COMMON=-std=c++11 -O0 -g -Wall -masm=intel -m64 -DUNITTEST $(INCLUDES) $(LIBCFLAGS)
CPPFLAGS_SSE=$(CPPFLAGS_COMMON) -msse4.2 -DSUDOKU_USE_AVX=0
CPPFLAGS_AVX=$(CPPFLAGS_COMMON) -mavx2 -DSUDOKU_USE_AVX=1

ASFLAGS_UNPACKED=-defsym CellsPacked=0
ASFLAGS_PACKED=-defsym CellsPacked=1
ASFLAGS_SSE=-defsym EnableAvx=0
ASFLAGS_AVX=-defsym EnableAvx=1

CELLS_UNPACKED_TARGET_SSE=unittest_cells$(POSTFIX_UNPACKED)$(POSTFIX_SSE)
CELLS_PACKED_TARGET_SSE=unittest_cells$(POSTFIX_PACKED)$(POSTFIX_SSE)
CELLS_UNPACKED_TARGET_AVX=unittest_cells$(POSTFIX_UNPACKED)$(POSTFIX_AVX)
CELLS_PACKED_TARGET_AVX=unittest_cells$(POSTFIX_PACKED)$(POSTFIX_AVX)

TARGETS=$(CELLS_UNPACKED_TARGET_SSE) $(CELLS_PACKED_TARGET_SSE) \
        $(CELLS_UNPACKED_TARGET_AVX) $(CELLS_PACKED_TARGET_AVX)

TESTER_OBJS=main.o sudokussetest.o sudokutest.o sudokucelltest.o \
            sudokumaptest.o sudokusolvertest.o sudokuloadertest.o
TESTED_OBJS=sudoku.o sudokuxmmreg.o
CPP_OBJS=$(TESTER_OBJS) $(TESTED_OBJS)

ASM_UNPACKED_OBJS=sudokusse_cells$(POSTFIX_UNPACKED).o
ASM_PACKED_OBJS=sudokusse_cells$(POSTFIX_PACKED).o

CELLS_UNPACKED_OBJS_SSE=$(patsubst %.o,%$(POSTFIX_SSE).o,$(CPP_OBJS) $(ASM_UNPACKED_OBJS))
CELLS_PACKED_OBJS_SSE=$(patsubst %.o,%$(POSTFIX_SSE).o,$(CPP_OBJS) $(ASM_PACKED_OBJS))
CELLS_UNPACKED_OBJS_AVX=$(patsubst %.o,%$(POSTFIX_AVX).o,$(CPP_OBJS) $(ASM_UNPACKED_OBJS))
CELLS_PACKED_OBJS_AVX=$(patsubst %.o,%$(POSTFIX_AVX).o,$(CPP_OBJS) $(ASM_PACKED_OBJS))

HEADERS=../sudoku.h sudokutest.h
GENERATED_CODES=sudokuConstAll.h
GENERATOR_SCRIPT=../sudokumap.rb

.PHONY: all clean

all: $(TARGETS)

$(CELLS_UNPACKED_TARGET_SSE): $(CELLS_UNPACKED_OBJS_SSE)
	$(CXX) $(LIBPATH) -o $@ $(LDFLAGS) $^ $(LIBS)
	./$@

$(CELLS_PACKED_TARGET_SSE): $(CELLS_PACKED_OBJS_SSE)
	$(CXX) $(LIBPATH) -o $@ $(LDFLAGS) $^ $(LIBS)
	./$@

$(CELLS_UNPACKED_TARGET_AVX): $(CELLS_UNPACKED_OBJS_AVX)
	$(CXX) $(LIBPATH) -o $@ $(LDFLAGS) $^ $(LIBS)
	./$@

$(CELLS_PACKED_TARGET_AVX): $(CELLS_PACKED_OBJS_AVX)
	$(CXX) $(LIBPATH) -o $@ $(LDFLAGS) $^ $(LIBS)
	./$@

# no inlining
main$(POSTFIX_SSE).o : main.cpp $(HEADERS) $(GENERATED_CODES)
	$(CXX) -c $(CPPFLAGS_SSE) $< -o $@

sudokussetest$(POSTFIX_SSE).o : sudokussetest.cpp $(HEADERS) $(GENERATED_CODES)
	$(CXX) -c $(CPPFLAGS_SSE) $< -o $@

sudokutest$(POSTFIX_SSE).o : sudokutest.cpp $(HEADERS) $(GENERATED_CODES)
	$(CXX) -c $(CPPFLAGS_SSE) $< -o $@

sudokucelltest$(POSTFIX_SSE).o : sudokucelltest.cpp $(HEADERS) $(GENERATED_CODES)
	$(CXX) -c $(CPPFLAGS_SSE) $< -o $@

sudokumaptest$(POSTFIX_SSE).o : sudokumaptest.cpp $(HEADERS) $(GENERATED_CODES)
	$(CXX) -c $(CPPFLAGS_SSE) $< -o $@

sudokusolvertest$(POSTFIX_SSE).o : sudokusolvertest.cpp $(HEADERS) $(GENERATED_CODES)
	$(CXX) -c $(CPPFLAGS_SSE) $< -o $@

sudokuloadertest$(POSTFIX_SSE).o : sudokuloadertest.cpp $(HEADERS) $(GENERATED_CODES)
	$(CXX) -c $(CPPFLAGS_SSE) $< -o $@

sudoku$(POSTFIX_SSE).o : ../sudoku.cpp $(HEADERS) $(GENERATED_CODES)
	$(CXX) -c $(CPPFLAGS_SSE) $< -o $@

sudokuxmmreg$(POSTFIX_SSE).o : ../sudokuxmmreg.cpp $(HEADERS) $(GENERATED_CODES)
	$(CXX) -c $(CPPFLAGS_SSE) $< -o $@


main$(POSTFIX_AVX).o : main.cpp $(HEADERS) $(GENERATED_CODES)
	$(CXX) -c $(CPPFLAGS_AVX) $< -o $@

sudokussetest$(POSTFIX_AVX).o : sudokussetest.cpp $(HEADERS) $(GENERATED_CODES)
	$(CXX) -c $(CPPFLAGS_AVX) $< -o $@

sudokutest$(POSTFIX_AVX).o : sudokutest.cpp $(HEADERS) $(GENERATED_CODES)
	$(CXX) -c $(CPPFLAGS_AVX) $< -o $@

sudokucelltest$(POSTFIX_AVX).o : sudokucelltest.cpp $(HEADERS) $(GENERATED_CODES)
	$(CXX) -c $(CPPFLAGS_AVX) $< -o $@

sudokumaptest$(POSTFIX_AVX).o : sudokumaptest.cpp $(HEADERS) $(GENERATED_CODES)
	$(CXX) -c $(CPPFLAGS_AVX) $< -o $@

sudokusolvertest$(POSTFIX_AVX).o : sudokusolvertest.cpp $(HEADERS) $(GENERATED_CODES)
	$(CXX) -c $(CPPFLAGS_AVX) $< -o $@

sudokuloadertest$(POSTFIX_AVX).o : sudokuloadertest.cpp $(HEADERS) $(GENERATED_CODES)
	$(CXX) -c $(CPPFLAGS_AVX) $< -o $@

sudoku$(POSTFIX_AVX).o : ../sudoku.cpp $(HEADERS) $(GENERATED_CODES)
	$(CXX) -c $(CPPFLAGS_AVX) $< -o $@

sudokuxmmreg$(POSTFIX_AVX).o : ../sudokuxmmreg.cpp $(HEADERS) $(GENERATED_CODES)
	$(CXX) -c $(CPPFLAGS_AVX) $< -o $@


sudokusse_cells$(POSTFIX_UNPACKED)$(POSTFIX_SSE).o : ../sudokusse.s
	$(AS) $(ASFLAGS_UNPACKED) $(ASFLAGS_SSE) -o $@ $<

sudokusse_cells$(POSTFIX_PACKED)$(POSTFIX_SSE).o : ../sudokusse.s
	$(AS) $(ASFLAGS_PACKED) $(ASFLAGS_SSE) -o $@ $<

sudokusse_cells$(POSTFIX_UNPACKED)$(POSTFIX_AVX).o : ../sudokusse.s
	$(AS) $(ASFLAGS_UNPACKED) $(ASFLAGS_AVX) -o $@ $<

sudokusse_cells$(POSTFIX_PACKED)$(POSTFIX_AVX).o : ../sudokusse.s
	$(AS) $(ASFLAGS_PACKED) $(ASFLAGS_AVX) -o $@ $<


$(GENERATED_CODES) : $(GENERATOR_SCRIPT)
	ruby $(GENERATOR_SCRIPT)

$(HEADERS):

clean:
	$(RM) $(TARGETS) $(GENERATED_CODES)
	$(RM) $(CELLS_UNPACKED_OBJS_SSE)
	$(RM) $(CELLS_PACKED_OBJS_SSE)
	$(RM) $(CELLS_UNPACKED_OBJS_AVX)
	$(RM) $(CELLS_PACKED_OBJS_AVX)
