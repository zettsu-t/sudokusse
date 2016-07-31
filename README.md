# Sudoku asm solver with SSE4.2 / AVX

Solving sudoku puzzles with SIMD instructions faster than C++ template metaprogramming.

## Platform

This sudoku solver is built on and runs on Cygwin with tools shown
below. MinGW-w64 is also available with Cygwin /usr/bin/ tools.

* Windows 10 64bit Edition
* Cygwin 64bit version (2.5.2)
* GNU Make (4.2.1)
* g++ (5.4.0)
* GNU assembler (2.25.2)
* Ruby (2.2.5p319)
* Perl (5.22.2)
* CppUnit (1.12.1)

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

Please read [notes_english.txt](notes_english.txt) to know more details.
