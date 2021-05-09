# Sudoku asm solver with SSE/AVX SIMD instructions

Solving Sudoku puzzles with SIMD instructions faster than C++ template metaprogramming.

## Platform

This Sudoku solver is built on and runs on Cygwin and Ubuntu on WSL2 with tools shown below. MinGW-w64 with Cygwin /usr/bin/ tools is also available.

* Windows 10
* Cygwin 64bit version or Ubuntu on WSL2
* GNU Make
* g++ (GCC)
* clang++ (LLVM)
* GNU assembler
* Ruby
* Perl
* Python
* CppUnit

## How to build

Launch a terminal and change its working directory to a directory
that contains _sudoku.cpp_.

```bash
cd path/to/sudokusse
```

And execute `make` without arguments to build.

```bash
make
```

After built successfully, executable files _bin/sudokusse*.exe_ are created.

SudokuSSE with AVX uses ANDN instruction of BMI1 (Bit Manipulation
Instructions), which is available on Haswell and newer
microarchitectures. If you cannot run on such processors, set
_EnableAvx_ in _Makefile_vars_ to 0 or an invalid opcode
exception occurs.

## Solve a Sudoku puzzle example

Execute the created binary in a terminal. _sudokusse.exe_ solves a
sudoku puzzle example 10,000 times and shows elapsed time to solve it.

```bash
bin/sudokusse 10000 < data/sudoku_example1.txt
```

## Solve the hardest Sudoku puzzles

SudokuSSE solves the hardest (17-clue) 49151 puzzles
[sudoku17 (broken link)](http://staffhome.ecm.uwa.edu.au/~00013890/sudoku17)
within 10 seconds. You can replace _sudoku17_ with other files in the
same format.

```bash
bin/sudokusse sudoku17
```

When you place an argument "-N" following a filename, SudokuSSE
solves puzzles using multiple threads.

```bash
bin/sudokusse sudoku17 -N
```

## Solve Sudoku-X puzzles

_bin/sudokusse_diagonal_ is a solver for Sudoku-X (diagonal Sudoku)
puzzles. I tested with puzzles cited from these websites (these
puzzles are not included in this repository).

* http://logicmastersindia.com/BeginnersSudoku/Types/?test=B201312 (I cite this puzzle in my unit tests.)
* [The hardest Sudoku-X puzzle](http://www.sudocue.net/minx.php) (I solved all of these 7193 puzzles in [sudoku-x-12-7193.sdm](http://www.sudocue.net/files/sudoku-x-12-7193.sdm).)

Execute the created binary similarly to the solver for the original Sudoku.

```bash
# Solve a puzzle
bin/sudokusse_diagonal 10000 < data/sudoku_x_example1.txt
# Solve puzzles in one file
bin/sudokusse_diagonal data/sudoku-x-12-7193.sdm sse print > solutions.txt
```

You can check whether the solutions are correct as below.

```bash
python3 solve_sudoku_x.py --log ./solutions.txt
```

## License

Copyright (c) 2016-2021 Zettsu Tatsuya

This software is released under the MIT License, see [LICENSE.txt](LICENSE.txt).

## And more

SudokuSSE uses bitboards to represent Sudoku puzzles.
I would like you to read [sudokusse.md](sudokusse.md) to know more details.
