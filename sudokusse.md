# Sudoku asm solver with SSE/AVX SIMD instructions

_SudokuSSE_ solves Sudoku puzzles with SIMD instructions and C++ template metaprogramming.

## Platform

SudokuSSE is a C++ and assembly program that runs on Windows 64bit Edition. Cygwin, Ubuntu on WSL2 or MinGW-w64 with tools shown below are required.

|Tool|Cygwin 64bit|Ubuntu 18.04 LTS on WSL2|MinGW-w64|
|:------|:------|:------|:------|
|GCC (g++)|10.2.0|10.1.0|9.2.0|
|LLVM (clang++)|8.0.1|10.0.0|11.0.0|
|GNU assembler (as)|2.36.1|2.30|2.33.1|
|GNU Make|4.3|4.1|4.2.1|
|Ruby|2.6.4p104|3.0.1p64|3.0.1p64|
|Perl|5.32.1|5.26.1|Cygwin perl|
|CppUnit|1.13.2|1.14.0|-|

* SudokuSSE requires C++ compilers that support C++11 and GNU-style inline assembly.
* Inline assembly of `clang++` is not compatible with GNU-style inline assembly and my unit tests for assembly do not run on `clang++`.
* Building SudokuSSE on MinGW requires Cygwin _/usr/bin_ tools such as `perl` and `rm`. Set the _PATH_ environment variable to find them.
* I have not tried to build unit tests with CppUnit on MinGW.

## How to build

### Create executables

Launch a terminal and change its working directory to a directory that contains _sudoku.cpp_.

```bash
cd path/to/sudokusse
```

And execute `make` without arguments to build.

```bash
make
```

After build successfully, it creates these two executable files.

* sudokusse.exe (standard)
* sudokusse_cells_packed.exe (special ; explained later)

### Use LLVM and C++17

Set the environment variable _CXX_ to `clang++` and you can compile C++ source files of SudokuSSE with clang++ instead of g++.

```bash
# Cygwin bash
export CXX=clang++ ; make

# Windows cmd
set CXX=clang++
make
```

The _Makefile_ in SudokuSSE compiles C++ source files as C++17 (with -std=c++17 option) if the compilers support C++17. It is hard-coded in the _Makefile_vars_ that GCC 7.2<= and LLVM 5.0<= support C++17.

### Switch to using SSE4.2 and AVX

Set the environment _EnableAvx_ in _sudokusse.s_ passed by _Makefile_vars_ to 0 for SSE4.2 and 1 for AVX. This is designated at compile-time, not in runtime.

SudokuSSE with AVX uses ANDN instruction of BMI1 (Bit Manipulation Instructions), which is available on Haswell and newer microarchitectures. If you cannot run on such processors, set _EnableAvx_ to 0 or an invalid opcode exception occurs.

### Solve parallel

Compiling sudoku.cpp with std::future fails on MinGW. To avoid it, add `-DNO_PARALLEL` to _CPPFLAGS_PARALLEL_ in _Makefile_vars_. If you can use boost::thread instead of it, set the _USE_BOOST_THREAD_ environment variable to "yes" so that the _Makefile_ passes `-DSOLVE_PARALLEL_WITH_BOOST_THREAD` to C++ compilers.

Checking Sudoku solutions on Cygwin may be very slow (at the time of this writing). I applied these items below to improve this issue. I guess the false sharing issue or other overhead occurs on heap memory holding string buffers. This issue does not occur on Bash on Ubuntu on Windows.

* Use std::array instead of std::vector if available
* Call std::string::reserve(N) to separate string buffers on heap memory. N is larger than a size of a cache line in bytes.

## Prepare Sudoku puzzles

SudokuSSE accepts Sudoku puzzles in text files.

* To place a cell with an initial number, write the number (1 to 9).
* To place a cell that is blank and solvers are going to fill, write a printable character except 1 to 9 such as a period, 0, or white space.

SudokuSSE accepts two formats. Redundant lines are ignored in both formats and you can write anything there as comments. Every Sudoku puzzle must have at least one cell with an initial number.

### Format 1 : 9 characters x 9 lines

```text
.3.....4.
.1..97.5.
..25.86..
..3...8..
9....43..
..76....4
..98.54..
.7.....2.
.5..71.8.
```

SudokuSSE reads the first nine lines and the first nine characters in each line of an input file. Indents are not allowed.

### Format 2 : 81 characters in one line

```text
.3.....4..1..97.5...25.86....3...8..9....43....76....4..98.54...7.....2..5..71.8.
```

SudokuSSE reads the first 81 characters of an input text.

## Run SudokuSSE

### Solve a Sudoku puzzle

Execute below in a terminal.

```bash
bin/sudokusse times_to_solve < puzzle_text_filename
```

When `times_to_solve` is a positive integer number, SudokuSSE solves the input puzzle `times_to_solve` times repeatedly and prints execution time to solve once.

When `times_to_solve` is a negative integer number, SudokuSSE solves the input puzzle absolute `times_to_solve` times (5 times for -5) and prints steps to solve it. Usually, specify -1 to check the steps.

_bin/sudokusse_cells_packed.exe_ prints "Cannot solve" error message because it assumes cells are packed and misses blank cells.

#### Display execution time

SudokuSSE prints its execution time in usec (that means microsecond = 1/1,000,000 second). The execution time excludes reading an input file and includes printing time in a terminal.

* Total : elapsed time in N-times execution
* Average : total / N
* Once least : the least elapsed time in one execution of all

Even when SudokuSSE solves a puzzle once, Once least is shorter than Total because their checkpoints of timestamps are different.

I assume the average becomes much longer than the least for these reasons.

* Other processes including terminals consume a CPU.
* Cache hit rate varies. Especially after a process preempted SudokuSSE, SudokuSSE gets a cold cache that causes a cache miss. Calling Win32 `SetProcessAffinityMask()` or Linux `sched_setaffinity()` improves the cold cache problem.
* A CPU reaches its thermal limit and slows down. Changing the Windows power option may relax this situation.

Process affinity must not set in using std::future because it prevents running threads of a process on multi-core.

#### Print steps to solve a Sudoku puzzle

When SudokuSSE prints steps in solving a Sudoku puzzle, each line of the output matches a row of the input puzzle and `:` splits cells in a row. Numbers in a cell mean candidates of the cell at a step.

For example `:123:` means the cell can contain 1 or 2 or 3 but cannot contain 4 to 9 (SudokuSSE judges so at the step). When the puzzle is solved completely, each cell has a unique number.

### Solve Sudoku puzzles in one step

Prepare a file that contains Sudoku puzzles at each line in the format 2 (81 characters in one line). SudokuSSE solves all lines of the file and checks whether their solutions are valid.

```bash
bin/sudokusse filename
```

When the second argument is "0" or "c++" or omitted, SudokuSSE solves Sudoku puzzles without SSE/AVX instructions. Set the second argument to "1" or "sse" and SudokuSSE uses SSE/AVX instructions.

```bash
bin/sudokusse filename 1
```

SudokuSSE solves the hardest 49151 puzzles [sudoku17 (broken link)](http://staffhome.ecm.uwa.edu.au/~00013890/sudoku17) within 10 seconds.

When the third argument is "1" or "off", SudokuSSE does not check whether their solutions are valid and it takes less time. When the third argument is "2" or "print", SudokuSSE checks whether their solutions are valid and prints the solutions.

```bash
bin/sudokusse filename sse off
bin/sudokusse filename sse print
```

When you place an argument "-Nnumber" or "-N" following a filename, SudokuSSE solves in Sudoku puzzles of the file with _number_ of threads. If you omit the number, SudokuSSE sets the number to the number of threads of a processor on which SudokuSSE runs (this comes from std::thread::hardware_concurrency()). My CPU (Intel Core i3 4160) has 4 threads (2 cores with hyper-threading).

```bash
bin/sudokusse filename -N8 sse
bin/sudokusse filename -N sse
```

### Count how many solutions a Sudoku puzzle has

A well-posed Sudoku puzzle has a unique solution but an ill-posed Sudoku puzzle has many solutions. SudokuSSE counts all solutions of a puzzle.

Execute in a terminal

```bash
bin/sudokusse 0 < puzzle_text_filename
```

and SudokuSSE prints the number of solutions of `puzzle_text_filename` and its execution time.

Set a non-zero number as a second argument

```bash
bin/sudokusse 0 100 < puzzle_text_filename
```

and SudokuSSE prints 100 solutions and exits after finding all solutions.

_bin/sudokusse_cells_packed.exe_ assumes Sudoku cells are packed at the top left corner of a puzzle. This means the puzzle comprises nine lines from its top to bottom ordered by

1. nine filled cells (0 or more lines)
1. filled cell(s) followed by blank cell(s) (none or one line)
1. nine blank cells (0 or more lines)

### Solve Sudoku-X puzzles

If you define the C++ macro `DIAGONAL_SUDOKU=1` and the assembly macro `DiagonalSudoku=1`, the executable solves diagonal Sudoku puzzles instead of original Sudoku puzzles.

It is hard-coded whether executables solve original or diagonal Sudoku puzzles to avoid run-time overhead.

Its usage is the same as the solver for the original Sudoku. You can check whether the solutions are correct with the script `solve_sudoku_x.py`.

```bash
# Solve a puzzle
bin/sudokusse_diagonal 10000 < data/sudoku_x_example1.txt
# Solve puzzles in one file
bin/sudokusse_diagonal data/sudoku-x-12-7193.sdm sse print > solutions.txt
# Check whether solutions are correct
python3 solve_sudoku_x.py --log ./solutions.txt
```

### Helper scripts

#### Measure time to solve puzzles

1. Download [sudoku17](http://staffhome.ecm.uwa.edu.au/~00013890/sudoku17) into data/ directory.
1. Execute `make time` to measure how long it takes to solve _sudoku17_. `make check` solves Sudoku puzzle examples in data/ before solves _sudoku17_.

To solve other Sudoku puzzle files instead of _sudoku17_, launch the ruby script directly.

```bash
ruby sudoku_check.rb sudoku_puzzle_filename
```

#### Measure time to count solutions

Execute

```bash
perl sudoku_count.pl puzzle_text_filename
```

and this script launches _sudoku*_ executables, solves `puzzle_text_filename`, and writes results to a log file which is named with current time such as SudokuTime_2013_09_27_21_34_56.log.

This repeats infinitely and you need to hit ctrl-c many times to abort (hitting once can break _sudoku*.exe_ and may not break the script.) In some platforms, hitting ctrl-z and `kill %1` are needed.

#### Search minimum execution time to count solutions from the log file

Execute

```bash
perl sudoku_search_timelog.pl SudokuTime_2013_09_27_21_34_56.log
```

and this script parses the log `SudokuTime_2013_09_27_21_34_56.log` and prints minimum execution time to solve.

## Test SudokuSSE

### Check if solving Sudoku puzzles correctly

Execute

```bash
perl sudoku_solve_all.pl
```

and the script solves puzzles in _data/*.txt_ .  When the script solves all the puzzles correctly, it prints execution time elapsed. The script aborts when it cannot solve any of the puzzles correctly.

### Check C++ functions and assembly macros

Change your working directory to _unittest/_ and execute

```bash
make
```

If the `make` command completes successfully, it created _unittest*_.exe executables, ran them, and reported a testing result that shows all tests passed.

## The inside of SudokuSSE

SudokuSSE uses bitboards to represent Sudoku puzzles.

### C++ data structures to solve Sudoku puzzles

#### Class SudokuCell

A cell in a Sudoku puzzle.

The member `candidates_` contains a candidate number set. A Sudoku puzzle contains 81 cells.

#### Integer SudokuCellCandidates

A candidate number set as a bitmap.

Set bit 0 (0x01) of the bitmap when 1 is a candidate and reset bit 0 when 1 is not a candidate. Set and reset in the manner 2 for bit 1 (0x02) ...  9 for bit 8 (0x100). If all numbers (1 to 9) are candidates, the bitmap has 0x1ff.

I define this type as unsigned int (uint32_t). Unsigned short (uint16_t) can hold the bitmap but is slow to calculate.

#### Class SudokuMap

All cells in a Sudoku puzzle.

The cells are numbered left to right, top to bottom as shown below and the numbers are in `SudokuCell::indexNumber_`.

```text
 0,  1, ... ,  9
 9, 10, ... , 17
  ...
72, 73, ... , 80
```

#### SudokuMap::Group_ and SudokuMap::ReverseGroup_

A lookup table that contains each cell is in which row, column, or 3x3 box.  `Group_` looks up forward (a 9-cells group to cells) and `ReverseGroup_` looks up reverse (a cell to 9-cells groups).

Rows are numbered top to bottom, columns are numbered left to right, and boxes are placed as shown below.

```text
0, 1, 2
3, 4, 5
6, 7, 8
```

_sudokumap.rb_ generates these tables as _sudokuConstAll.h_.

#### Class SudokuCellLookUp

SudokuCellLookUp checks properties of SudokuCellCandidates to avoid bit operation (C++ lacks population count until C++20 and I wrote assembly version with SSE `popcnt`).

SudokuCellLookUp has 512 (=2^9) entries * 4byte = 2Kbyte and L1 data cache (32Kbyte) can hold it whole.

#### Class SudokuSolver

SudokuSolver receives a string, extracts 81 cells from its characters, and solves it. In extracting, it parses input lines and detects in which format a Sudoku puzzle is described.

#### Class SudokuLoader

SudokuLoader parses command line options, reads a Sudoku puzzle file, solves it, and measures its execution time.

### SSE4.2/AVX data structures to solve Sudoku puzzles

Classes _SudokuSse*_ are data structures working with SIMD instructions.

XMM1..9 registers hold rows in a Sudoku puzzle. XMM-N register (128bit) holds the Nth row that contains four 32bit parts; 0 (32bit), left 3 cells, middle 3 cells, and right 3 cells. Each part (32bit) comprises 0s (5bit), left cell (9bit), middle cell (9bit), and right cell (9bit).

The cell has a candidate bitmap as in the form of SudokuCellCandidates. Each bit in the bitmap indicates whether each of 1..9 is a candidate of the cell or not.

Note that x86 uses little-endian so copying an XMM register to memory looks like bytes are flipped.

* 32bit dump : right 3 cells (32bit), middle (32bit), left (32bit), 0 (32bit)
* 64bit dump : ((middle 3 cells:32bit << 32) | right:32bit) , (left:32bit)

I define _unique candidate_ as below.

* if the cell has a unique candidate, the bitmask for a candidate (1 << (candidate number - 1))
* zero if the cell has multiple candidates

Other XMM registers hold data described below.

|Register|Value|
|:-------|:----|
|XMM0 |logical OR of unique candidates in each column|
|XMM10|logical OR of unique candidates in 1st to 3rd rows|
|XMM11|logical OR of unique candidates in 4th to 6th rows|
|XMM12|logical OR of unique candidates in 7th to 9th rows|
|XMM15|the number of cells that have a unique candidate|
|XMM13, XMM14|work area|

General-purpose registers are used for

|Register|Value|
|:-------|:----|
|r15|a constant for mask bits of candidates|
|rbp, rsp|not changed|
|others|work area|

### SSE4.2/AVX data structures to count Sudoku solutions

XMM registers hold cells described below.

|Register|Cells|
|:-------|:----|
|XMM1|(0,0), ... , (7,0)|
|XMM-N|(0,N-1), ... , (7,N-1)|
|XMM9|(0,8), ... , (7,8)|
|XMM10|(8,0), ... , (8,7)|

XMM1..9 registers hold left 8 cells of a row and XMM-N register holds the Nth row. Each word (16bit * 8) low to high in an XMM register contains the 1st to the 8th column.

```text
XMM1..9 : Cells in 8th, 7th, 6th, 5th, 4th, 3rd, 2nd, 1st column in a row
```

XMM10 register holds rightmost cells of 1st to 8th rows. Each word low to high (16bit * 8) in XMM10 register contains the 1st to the 8th row.

```text
XMM10 : Cells at 8th, 7th, 6th, 5th, 4th, 3rd, 2nd, 1st rows in the rightmost column
```

_sudokuXmmRightBottomElement_ holds a preset number (written in an input file) of the right bottom corner cell. _sudokuXmmRightBottomSolved_ holds a solved (blank in an input file) number of the right bottom corner cell.

Each word (16bit) holds a bitmap in the form of SudokuCellCandidates as described. None or one bit in the word is set and multiple bits are never set simultaneously.

### Notice to writing assembly code

All memory accesses in _sudokusse.s_ require RIP (instruction pointer) relative addressing.

```as
movdqa xmm0, xmmword ptr [sudokuXmmToPrint]
```

is not allowed and

```as
movdqa xmm0, xmmword ptr [rip + sudokuXmmToPrint]
```

is right. MinGW-w64 may accept non-RIP-relative addressing but Cygwin causes link errors.

g++ accepts assembly code in Intel syntax but it causes errors when you mix up Intel and AT&T syntax. This occurs when your inline assembly code is in Intel syntax and inline assembly code in header files is in AT&T syntax. I found this issue in using boost::future and do not find it in std::future.

Its workarounds are

* Writing a compact file that contains inline assembly and includes fewer header files
* Using the compile option -masm=intel if it is required

### Footprints

Here is a size (41,163 bytes) of core code solving Sudoku puzzles in a version. This is 26% larger than the L1 I-cache size (32 KBytes per core) in an x86 processor.

```bash
$ objdump -x --section=.text bin/sudokusse | sort
```

|Label|Base Address [hex]|Size [bytes, dec]|Macro|
|:------|:------|------:|:------|
|solveSudokuAsm|0x423251|3,044|CollectUniqueCandidatesInThreeLine|
|countFilledElementsLabel|0x423e35|59|CountFilledElements|
|exitFilling|0x423e70|1,334|CheckConsistency|
|searchNextCandidateLabel|0x4243a6|5,692|SearchNextCandidate|
|exitFillingCells|0x4259e2|13,685|CollectUniqueCandidates|
|findCandidatesLabel|0x428f57|17,349|FindCandidates|
|loadXmmRegisters|0x42d31c|-|-|

### Algorithm to solve Sudoku puzzles

I apply the steps described in _Shasha[2007]_.

1. Fill a candidate of a cell if any. Apply this to all 81 (9x9) cells.
1. Find a candidate of a cell if you cannot use the candidate in other cells. Apply this to all 81 (9x9) cells.
1. Repeat 1 and 2. If you cannot decrease the number of candidates more, start backtracking. i.e. choose a candidate and repeat 1 and 2 again.

#### Step 1

To find a unique candidate of a cell, collect numbers in cells of a row, column, and box that the cell belongs to. For example, the size of the collected numbers is 8, the rest is the unique candidate. This is commonly called _naked single_.

When a row has cells `*23456789`, set `*` to 1.

When a row, column, and box have cells as shown below,

```text
*..123...
456
7
8
```

set `*` to 9.

Now we can extend this rule. To find a unique candidate of a cell, collect numbers not used in cells of a row, column, and box that the cell belongs to. For example,

```text
*..12....
3456
7
```

We apply `*` to logical AND {8,9}. In other words, we mask `*` by the complementary set of {1..7}.

#### Step 2

For a cell, if there is a number that is exclusive for a row, column, and box that the cell belongs to, we can fill the cell with the number. This is commonly called _hidden single_.

Consider an example here.

```text
*12......
???4..7..
!??...4..
.7.
...
...
..7
```

The six cells marked `?` and `!` cannot hold 4 because 4 is on the same rows. The cells filled by 1 and 2 also cannot hold 4. This leads to the conclusion that the cell `*` in the top left box only can hold 4.

We can apply this rule to columns and boxes. After filling 4, we apply the rule to 7 and set the cell marked `!` to 7.

#### Step 3 : backtracking

SudokuSSE does not use the locked candidates method. Before starting backtracking, SudokuSSE chooses a candidate in a cell of an ongoing Sudoku map.

1. Select a cell that has the least size (2 or more) of candidates from the cells.
1. Select a row that has the least size of candidates in the cells of the row. Column and boxes can be used instead of rows.
1. If the row has multiple cells that have the least and same size of candidates, use a cell found first.

When every cell in the ongoing Sudoku map has a unique candidate, SudokuSSE checks whether all rows, columns, and boxes in the map are correct and preset numbers are unchanged. If it is true, it is the solution to the map.

Guessing a candidate sometimes leads to inconsistent cells and an incorrect solution. When the guess is wrong, SudokuSSE filters it out and continues to backtracking.

Before starting backtracking, SudokuSSE makes a copy of the Sudoku map to rewind backtracking. The map has only primitives therefore we can use compiler-generated copying (trivial copy) and avoid object aliasing.

### Algorithm to count solutions of Sudoku puzzles

SudokuSSE uses backtracking only. It sets a cell to one of candidates 1..9 if it is not a blank, and recursively sets other cells.

Assume that cells are packed top left and count fast if the assumption is true.

```text
123456789
456789123
789123456
2958.....
.........
.........
.........
.........
.........
```

Note that rightmost and bottom cells always have only one candidate in backtracking and we can avoid setting candidates to them in the recursion.

Counting solutions can run on a single thread only. To run on multiple threads, it needs to eliminate global variables.

### Make SudokuSSE faster

I have no quantitative analysis of these items because I have not used a profiler to SudokuSSE.

* Use the `inline` keyword and turn it on or off via a macro. SudokuSSE enables inlining, and unit tests disable inlining. Inlining causes link errors in unit tests.
* Eliminate virtual function calls. This also prohibits virtual destructors. If you would like to define virtual destructors as a good practice, undefine the `NO_DESTRUCTOR_AND_VTABLE` macro.
* Declare aliases for all integer types to switch their bit widths easily. Performance depends on the bit widths and I hope the current combination of bit widths makes SudokuSSE the fastest. Different processors and compilers may need different bit widths.
* Use const and constexpr as much as possible. This is useful to fix branch directions in if-statements. Constant propagation removes constant expressions and unused blocks in an if-statement (C++17 will support this with _if-constexpr_ officially). _SudokuCell::CountCandidatesIfMultiple_ and _SudokuCell::MaskCandidatesUnlessMultiple_ eliminates if-statements.
* Unroll loops if appropriate. If a loop has complex branch conditions, unrolling the loop can disturb branch prediction of a processor and make it run slower.
* Unroll loops with member function templates and recursive call. I hope compilers expand the recursive call and there is no overhead in runtime.
* Use macros instead of function templates if really needed. C++ code cannot break to exit nested loops and use the macro `#define func { ... return; }` to do it.
* Change `#define FAST_MODE true` to false and run code that is easy to read but slow.
* Set optimization level -O2. -O3 makes SudokuSSE slower.

To write fast code in x86_64 assembly, we need to read the x86_64 manual closely. The manual tells us not only general optimization guidelines but slight differences in performance such as CMOV instructions.

* Use the x86_64 32-bit registers instead of the 64-bit registers if possible. Output to a 32-bit register clears its upper 32 bits and removes redundant bit masking. This rule surely works fine on the _using-32bit-registers_ branch.
* To return from a function in assembly, `pop + jmp` is faster than `ret`.

I replaced assembly macro parameters as 64-bit registers with 32-bit registers manually. If you know how to convert a 64-bit register to its 32-bit register alias, for example, RAX to EAX register, I would like to share your solution on [the Stack Overflow Community](http://stackoverflow.com/questions/41107642/how-to-convert-x86-64-64-bit-register-names-to-their-corresponding-32-bit-regist).

## Solving Sudoku puzzles with NumPy

I write a Python script to solve 9x9 and 16x16 Sudoku puzzles with NumPy. This script accepts files in the format 1 (one row in one line) and guesses whether input puzzles are 9x9 or 16x16. Notice that its backtracking is very slow.

```bash
$ python3 sudoku_numpy.py puzzle_text_filename
```

## Solving Sudoku puzzles with Rust

You can build a [Rust Sudoku solver](sudoku_rust/src/main.rs), measure how long it takes to solve puzzles, and check whether their solutions are correct. The `make` command below builds C++ and Rust executable, runs them, and compares their solutions.

```bash
$ make all rust
```

The Rust solver is built on `cargo`.

```bash
$ cd sudoku_rust/
$ cargo build
$ target/debug/sudoku_rust ../data/sudoku_example.txt
```

If you specify one or more Sudoku puzzle files, `sudoku_rust` solves puzzles in the files. Otherwise, `sudoku_rust` takes puzzles from stdin. Some options are available to compare execution time with the C++/SSE solvers.

* -n Number : solve only the head Number of puzzles (equivalent to ```head -n Number ../data/sudoku_example.txt | sudoku_rust```)
* -s : do not print solutions to save time in writing the solutions into stdout
* -1 : run on a single thread instead of multi-threads
* -v : verify solutions and panic if it finds a wrong solution

The cargo profiler works with `sudoku_rust` on some platforms. If the profiler does not take command line arguments followed by --, its workaround is putting them to the environment variable SUDOKU_RUST as a whitespace-separated string.

```bash
$ cargo profiler callgrind -n 10 --bin target/release/sudoku_rust -- -n 200 ../data/sudoku17.txt
$ export SUDOKU_RUST="-n 200 ../data/sudoku17.txt"
$ cargo profiler callgrind -n 10 --bin target/release/sudoku_rust
```

## Bibliography and acknowledgments

1. I cite the Sudoku solver algorithm and puzzle examples from the book.

  Dennis E. Shasha (May 2007), "Puzzles for Programmers and Pros", Wrox
  (I read its Japanese translation published by Ohmsha. See my Japanese
  [howtobuild.txt](howtobuild.txt).)

2. I use CppUnit code on the website.

  _http://www.atmarkit.co.jp/fdotnet/cpptest/cpptest02/cpptest02_02.html_

3. I adopt an idea of Mr. Kawai Hidemi to count how many solutions in a Sudoku puzzle and compare execution time with his program on the article.

  _http://developer.cybozu.co.jp/tech/?p=1692_

4. I check instructions of x86_64 and their latency on the manual.

  "Intel 64 and IA-32 Architectures Optimization Reference Manual"

  _http://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-optimization-manual.html_

5. I use these Sudoku puzzles as test cases.

  http://staffhome.ecm.uwa.edu.au/~00013890/sudoku17

  http://logicmastersindia.com/BeginnersSudoku/Types/?test=B201312

  http://www.sudocue.net/minx.php

  http://www.sudocue.net/files/sudoku-x-12-7193.sdm

6. I posted a question for assembly and received some useful advice.

  http://stackoverflow.com/questions/41107642/how-to-convert-x86-64-64-bit-register-names-to-their-corresponding-32-bit-regist

7. I learned best practices in C++ from the books listed below.

  https://github.com/zettsu-t/zettsu-t.github.io/wiki/Books-English
