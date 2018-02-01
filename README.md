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
* g++ (GCC)
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

## Solve a Sudoku-X puzzle

This branch is a solver for Sudoku-X (diagonal Sudoku) puzzles. I tested with puzzles cited from these websites (these puzzles are not included in this repository).

* http://logicmastersindia.com/BeginnersSudoku/Types/?test=B201312 (I cite this puzzle in my unit tests.)
* [The hardest Sudoku-X puzzle](http://www.sudocue.net/minx.php) (I passed all of these 7193 puzzles.)

Execute the generated binary from a terminal. _sudokusse.exe_ solves a
sudoku puzzle example 10,000 times and shows elapsed time to solve it.

```bash
bin/sudokusse 10000 < data/sudoku_x_example1.txt
```

These commands solve puzzles in data/puzzle.txt, write their solutions into ./solution.txt and check whether their solutions are correct.

```bash
time bin/sudokusse data/puzzle.txt sse print > ./solution.txt
python3 solve_sudoku_x.py --log ./solution.txt
```

## License

Copyright (c) 2016-2018 Zettsu Tatsuya

This software is released under the MIT License, see [LICENSE.txt](LICENSE.txt).
