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

## Solve a sudoku puzzle example

Execute the generated binary from a terminal. _sudokusse.exe_ solves a
sudoku puzzle example 10,000 times and shows elapsed time to solve it.

```bash
bin/sudokusse 10000 < data/sudoku_example1.txt
```

## And more

Please read [sudokusse.md](sudokusse.md) to know more details.
