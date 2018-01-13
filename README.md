# Sudoku asm solver with SSE/AVX SIMD instructions

Solving sudoku puzzles with SIMD instructions faster than C++ template metaprogramming.

## Platform

This sudoku solver is built on and runs on Cygwin with tools shown
below. Bash on Ubuntu on Windows is available on Windows 10
Anniversary Update.  MinGW-w64 is also available with Cygwin /usr/bin/
tools.

* Windows 10 Anniversary Update 64bit Edition
* Cygwin 64bit version
* GNU Make
* g++
* clang++ (LLVM)
* GNU assembler
* Ruby
* Perl
* CppUnit

## How to build

Launch a terminal and change its current directory to a directory
that contains _sudoku.cpp_.

```bash
cd .../sudokusse
```

And execute _make_ without arguments to build.

```bash
make
```

After built successfully, executable files _bin/sudokusse*.exe_ are generated.

SudokuSSE with AVX uses ANDN instruction of BMI1 (Bit Manipulation
Instructions), which is available on Haswell and newer
microarchitectures. If you cannot run on such processors, set
_EnableAvx_ in _Makefile_vars_ to 0 or an invalid opcode
exception occurs.

## Solve a sudoku puzzle example

Execute the generated binary from a terminal. _sudokusse.exe_ solves a
sudoku puzzle example 10,000 times and shows elapsed time to solve it.

```bash
bin/sudokusse 10000 < data/sudoku_example1.txt
```

## Solve the hardest sudoku puzzles

SudokuSSE solves the hardest 49151 puzzles
[sudoku17](http://staffhome.ecm.uwa.edu.au/~00013890/sudoku17) within
10 seconds. You can replace _sudoku17_ with other files in the same
format.

```bash
bin/sudokusse.exe sudoku17
```

When you place an argument "-N" following a filename, SudokuSSE
solves puzzles with multi-threading.

```bash
bin/sudokusse.exe sudoku17 -N
```

## License

Copyright (c) 2016, 2017 Zettsu Tatsuya

This software is released under the MIT License, see [LICENSE.txt](LICENSE.txt).

## And more

SudokuSSE uses bitboards to represent sudoku puzzles.
I would like you to read [sudokusse.md](sudokusse.md) to know more details.
