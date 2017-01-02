# Sudoku asm solver with SSE/AVX SIMD instructions

_SudokuSSE_ solves sudoku puzzles with SIMD instructions and C++
template metaprogramming.

## Platform

SudokuSSE is a C++ and assembly program that runs on Windows 64bit
Edition.  Cygwin, Bash on Ubuntu on Windows or MinGW-w64 with tools
shown below are required.

|Tool|Cygwin 64bit|Bash on Ubuntu on Windows|MinGW-w64|
|:------|:------|:------|:------|
|GCC (g++)|5.4.0|4.8.4|6.1.0|
|GNU assembler (as)|2.25.2|2.24|2.26|
|GNU Make|4.2.1|3.81|4.2|
|Ruby|2.2.5p319|1.9.3p484|ActiveScriptRuby 2.3.1p112|
|Perl|5.22.2|5.18.2|Cygwin perl|
|CppUnit|1.12.1|1.13.0|-|

* SudokuSSE requires C++ compilers that support C++11 and GNU style
  inline assembly.
* Building SudokuSSE on MinGW requires Cygwin _/usr/bin_ tools such as
  perl and rm. Set environment variable _PATH_ to find them.
* I have not tried to build unittests with CppUnit on MinGW.

## How to build

### Generate executables

1. Launch a terminal and change its current directory to a directory
  that contains _sudoku.cpp_.

  ```bash
  cd .../sudokusse
  ```

1. Execute make without arguments to build.

  ```bash
  make
  ```

After built successfully, these two executable files are generated.

* sudokusse.exe (standard)
* sudokusse_cells_packed.exe (special ; explained later)

### Switch to using SSE4.2 and AVX

Set the environment _EnableAvx_ in _sudokusse.s_ passed by
_Makefile_vars_ to 0 for SSE4.2 and 1 for AVX. This is designated at
compile-time, not in runtime.

SudokuSSE with AVX uses ANDN instruction of BMI1 (Bit Manipulation
Instructions), which is available on Haswell and newer
microarchitecture. If you cannot run on such processors, set
_EnableAvx_ to 0 or an invalid opcode exception occurs.

### Solve parallel

Compiling sudoku.cpp with std::future fails on MinGW. To avoid it, add
`-DNO_SOLVE_PARALLEL` to _CPPFLAGS_PARALLEL_ in _Makefile_vars_.

Checking sudoku solutions on Cygwin may be very slow. I applied these
items below to improve this issue. I guess the false sharing issue
occurs on heap memory holding string buffers. This issue does not
occur on Bash on Ubuntu on Windows.

* Use std::array instead of std::vector if available
* Call std::string::reserve(N) to separate string buffers on heap
  memory. N is larger than a size of a cache line in bytes.

## Prepare sudoku puzzles

SudokuSSE accepts sudoku puzzles in text files.

* For cells with an initial number, write the number (1 to 9).
* For cells which are blank and going to be filled by solvers, write a
  printable non-number character such as a period or white space.

SudokuSSE accepts two formats. Redundant lines are not parsed in both
formats and you can write anything there as comments. Every sudoku
puzzle must have at least one cell with an initial number.

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

SudokuSSE reads first nine lines and first nine characters in each
line of an input file. No indents are allowed.

### Format 2 : 81 characters in one line

```text
.3.....4..1..97.5...25.86....3...8..9....43....76....4..98.54...7.....2..5..71.8.
```

SudokuSSE reads first 81 characters of an input text.

## Run SudokuSSE

### Solve a sudoku puzzle

Execute from a terminal

```bash
bin/sudokusse.exe times_to_solve < puzzle_text_filename
```

When `times_to_solve` is a positive integer number, SudokuSSE solves
the same input puzzle `times_to_solve` times and prints execution time
to solve once.

When `times_to_solve` is a negative integer number, SudokuSSE solves
absolute `times_to_solve` times (5 times for -5) and prints steps to
solve it. Usually, specify -1 to check the steps.

_bin/sudokusse_cells_packed.exe_ prints "Cannot solve" error message
because it assumes cells are packed and misses blank cells.

#### Display execution time

SudokuSSE prints its execution time in usec (that means microsecond =
1/1,000,000 second). The execution time excludes reading an input file
and includes printing time to a terminal.

* Total : all execution time in repetition
* Average : total / number of repetition
* Once least : least execution time in repetition

Even when SudokuSSE solves a puzzle once, Once least is shorter than
Total because checkpoints of time stamp are different.

I assume the average is much longer than the least for these reasons.

1. Other processes including terminals consume a CPU.
1. Cache hit rate varies. Especially after a process preempted
  SudokuSSE, SudokuSSE gets a cold cache that causes a cache miss.
  Calling Win32 `SetProcessAffinityMask()` or Linux
  `sched_setaffinity()` improves the cold cache problem.
1. A CPU reaches its thermal limit and slows down. Setting of the
  Windows power option may relax this situation.

Process affinity must not set in using std::future because it prevents
running a process on multi-core.

#### Print steps to solve a sudoku puzzle

When SudokuSSE prints steps to solve a sudoku puzzle, each line of the
output matches a line of the input puzzle and `:` splits cells in a
line. Numbers in a cell mean candidates of the cell at a step.

For example `:123:` means the cell can contain 1 or 2 or 3 but cannot
contain 4 to 9 (SudokuSSE judges so at the step). When the puzzle
solved completely, each cell has a unique number.

### Solve sudoku puzzles in one step

Prepare a file which contains sudoku puzzles at each line in the
format 2 (81 characters in one line). SudokuSSE solves all lines of
the file and checks whether their solutions are valid.

```bash
bin/sudokusse.exe filename
```

When the second argument is "0" or "c++" or omitted, SudokuSSE solves
sudoku puzzles without SSE/AVX instructions.  Set "1" or "sse" to the
second argument and SudokuSSE uses SSE/AVX instructions.

```bash
bin/sudokusse.exe filename 1
```

SudokuSSE solves the hardest 49151 puzzles
[sudoku17](http://staffhome.ecm.uwa.edu.au/~00013890/sudoku17)
within 10 seconds.

When the third argument is "1" or "off", SudokuSSE does not check
whether their solutions are valid and it takes less time.  When the
third argument is "2" or "print", SudokuSSE checks whether their
solutions are valid and prints the solutions.

```bash
bin/sudokusse.exe filename sse off
bin/sudokusse.exe filename sse print
```

When you place an argument "-Nnumber" or "-N" following a filename,
SudokuSSE solves in sudoku puzzles of the file with _number_ of
threads. If you omit the number, the number is set to the number of
threads of a processor on which SudokuSSE runs (actually this is
std::thread::hardware_concurrency()). My CPU (Intel Core i3 4160)
has 4 threads (2 cores with hyper-threading).

```bash
bin/sudokusse.exe filename -N8 sse
bin/sudokusse.exe filename -N sse
```

### Count how many solutions a sudoku puzzle has

A well-posed sudoku puzzle has a unique solution but a not well-posed
sudoku puzzle may have many solutions. SudokuSSE counts all solutions
of a puzzle.

Execute from a terminal

```bash
bin/sudokusse.exe 0 < puzzle_text_filename
```

and SudokuSSE prints the number of solutions of `puzzle_text_filename`
and its execution time.

Set a non-zero number as a second argument

```bash
bin/sudokusse.exe 0 100 < puzzle_text_filename
```

and SudokuSSE prints 100 solutions and exits after all solutions
counted.

_bin/sudokusse_cells_packed.exe_ assumes sudoku cells are packed at
the top left corner of a puzzle. This means the puzzle has such nine
lines from its top to bottom ordered by

1. filled nine cells (0 or more lines)
1. filled cell(s) followed by blank cell(s) (none or one line)
1. blank nine cells (0 or more lines)

### Helper scripts

#### Measure time to count solutions

Execute

```bash
perl sudoku_count.pl puzzle_text_filename
```

and this script launches _sudoku*_ executables, solves
`puzzle_text_filename` and writes results to a log file which is named
with current time such as SudokuTime_2013_09_27_21_34_56.log.

This repeats infinitely and you need to hit ctrl-c many times to abort
(hitting once can break _sudoku*.exe_ and may not break the script.)
In some platform, hitting ctrl-z and `kill %1` are needed.

#### Search minimum execution time to count solutions from the log file

Execute

```bash
perl sudoku_search_timelog.pl SudokuTime_2013_09_27_21_34_56.log
```

and this script parses the log `SudokuTime_2013_09_27_21_34_56.log`
(or other specified file name) and print minimum execution time to
solve.

## Test SudokuSSE

### Check to solve sudoku puzzles correctly

Execute

```bash
perl sudoku_solve_all.pl
```

and the script solves puzzles in _data/*.txt_ . When the puzzles are
solved correctly, the script prints execution time elapsed. When one
of the puzzles cannot be solved correctly, the script aborts
immediately.

### Check C++ functions and assembly macros

Change current directory to _unittest/_ and execute

```bash
make
```

If the make command completes successfully, it generated
_unittest*_.exe executables, ran them and reported a testing result
which indicates all tests passed.

## The inside of SudokuSSE

SudokuSSE uses bitboards to represent sudoku puzzles.

### C++ data structures to solve sudoku puzzles

#### Class SudokuCell

A cell in a sudoku puzzle. The puzzle contains 81 cells.  The member
`candidates_` contains candidate number set.

#### Integer SudokuCellCandidates

Candidate number set as a bitmap.

Set bit 0 (0x01) of the bitmap when 1 is a candidate and reset bit 0
when 1 is not a candidate. Set and reset in the manner 2 for bit 1
(0x02) ...  9 for bit 8 (0x100). If all numbers (1 to 9) are
candidates, the bitmap has 0x1ff.

I define this type as unsigned int (uint32_t). Unsigned short
(uint16_t) can hold the bitmap but is slow to calculate.

#### Class SudokuMap

All cells in a sudoku puzzle. The cells are numbered left to right,
top to bottom as shown below and set in `SudokuCell::indexNumber_`.

```text
 0,  1, ... ,  9
 9, 10, ... , 17
  ...
72, 73, ... , 80
```

#### SudokuMap::Group_ and SudokuMap::ReverseGroup_

Look up table in which each cell is in which row, column, or 3x3 box.
`Group_` looks up forward (a 9 cells group to cells) and
`ReverseGroup_` looks up reverse (a cell to 9 cells groups).

Rows are numbered top to bottom, columns are numbered left to right,
and boxes are placed as shown below.

```text
0, 1, 2
3, 4, 5
6, 7, 8
```

_sudokumap.rb_ generates these tables as _sudokuConstAll.h_.

#### Class SudokuCellLookUp

SudokuCellLookUp checks properties of SudokuCellCandidates to avoid
bit operation (standard C++ lacks population count and I wrote
assembly version with SSE popcnt).

SudokuCellLookUp has 512 (=2^9) entries * 4byte = 2Kbyte and L1 data
cache (32Kbyte) can hold it whole.

#### Class SudokuSolver

SudokuSolver receives a given string, extracts 81 characters as cells
and solves it. In extracting, it parses input lines and detects in
which format a sudoku puzzle is described.

#### Class SudokuLoader

SudokuLoader understands command line options, read a sudoku puzzle
file, solve it and measure its execution time.

### SSE4.2/AVX data structures to solve sudoku puzzles

Classes _SudokuSse*_ are data structures using SIMD instructions.

XMM1..9 registers hold rows in a sudoku puzzle. XMM-N register
(128bit) holds the Nth row that contains four 32bit parts; 0(32bit),
left 3 cells, middle 3 cells, right 3 cells. The part (32bit) consists
of 0(5bit), left cell(9bit), middle cell(9bit), right cell(9bit).

The cell has candidate bitmap as in the form of SudokuCellCandidates.
Each bit in the bitmap indicates each of 1..9 is a candidate of the
cell or not.

Note that x86 uses little endian so copying an XMM register to memory
looks like bytes are flipped.

* 32bit dump : right 3 cells (32bit), middle(32bit), left(32bit), 0(32bit)
* 64bit dump : ((middle 3 cells:32bit << 32) | right:32bit) , (left:32bit)

I define _unique candidate_ means here.

* if the cell has a unique candidate, the bit mask for a candidate
  (i.e. 1 << (candidate number - 1))
* zero if the cell has multiple candidates

Other XMM registers hold data described below.

|Register|Value|
|:-------|:----|
|XMM0 |Logical OR of unique candidates in each column|
|XMM10|Logical OR of unique candidates in 1st to 3rd rows|
|XMM11|Logical OR of unique candidates in 4th to 6th rows|
|XMM12|Logical OR of unique candidates in 7th to 9th rows|
|XMM15|Number of cells that have a unique candidate|
|XMM13, XMM14 | work area|

General purpose registers are used for

|Register|Value|
|:-------|:----|
|r15|a constant for mask bits of candidates|
|rbp, rsp|not changed|
|others|work area|

### SSE4.2/AVX data structures to count sudoku solutions

XMM registers hold cells described below.

|Register|Cells|
|:-------|:----|
|XMM1|(0,0), ... , (7,0)|
|XMM-N|(0,N-1), ... , (7,N-1)|
|XMM9|(0,8), ... , (7,8)|
|XMM10|(8,0), ... , (8,7)|

XMM1..9 registers hold left 8 cells of a row and XMM-N register holds
the Nth row. Each word (16bit * 8) low to high in an XMM register
contains the 1st to the 8th column.

```text
XMM1..9 : Cells in 8th, 7th, 6th, 5th, 4th, 3rd, 2nd, 1st column in a row
```

XMM10 register holds rightest cells of 1st to 8th rows.  Each word low
to high (16bit * 8) in XMM10 register contains the 1st to the 8th row.

```text
XMM10 : Cells at 8th, 7th, 6th, 5th, 4th, 3rd, 2nd, 1st rows in the rightest column
```

_sudokuXmmRightBottomElement_ holds a preset (written in a given file)
number of the right bottom corner cell. _sudokuXmmRightBottomSolved_
holds solved (blank in a given file) number of the right bottom corner
cell.

Each word (16bit) holds a bitmap in the form of SudokuCellCandidates
as described. None or one bit in the word is set and multiple bits are
never set simultaneously.

### Notice to write assembly code

All memory accesses in _sudokusse.s_ require RIP (instruction pointer)
relative addressing.

```as
movdqa xmm0, xmmword ptr [sudokuXmmToPrint]
```

is not allowed and

```as
movdqa xmm0, xmmword ptr [rip + sudokuXmmToPrint]
```

is right. MinGW-w64 may accept non-RIP-relative addressing but Cygwin
causes link errors.

### Footprints

Here is a size (41,163 bytes) of core code solving sudoku puzzles in
assembly. This is 26% larger than the L1 I-cache size (32 KBytes per
core) in my CPU.

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

### Algorithm to solve sudoku puzzles

I apply steps described in _Shasha[2007]_.

1. Fill a candidate of a cell if any. Apply this to all 81 (9x9) cells.
1. Find a candidate of a cell if the candidate cannot be set to other
  cells.  Apply this to all 81 (9x9) cells.
1. Repeat 1 and 2. If a decrease of the candidates stops, start
  backtracking. i.e. choose a candidate and repeat 1 and 2 again.

#### Step 1

To find a unique candidate of a cell, collect numbers in cells of a
row, column, and box that the cell belongs. For example, the size of
the number is 8, the rest is the unique candidate. This is commonly
called _naked single_.

When a row has cells `*23456789`, set 1 to `*`.

When a row, column, and box have cells as shown below,

```text
*..123...
456
7
8
```

set 9 to `*`.

Now we can extend this rule. To find a unique candidate of a cell,
collect numbers not used in cells of a row, column and box that the
cell belongs. For example,

```text
*..12....
3456
7
```

we apply `*` to logical AND {8,9}. In other words, we mask `*` by the
complementary set of {1..7}.

#### Step 2

For a cell, if there is a number that cannot be set in a row, column
and box that the cell belongs, we can fill the cell with the number.
This is commonly called _hidden single_.

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

The 6 cells marked `?` and `!` cannot hold 4 because 4 is on the same
rows.  The cells filled by 1 and 2 also cannot hold 4. This leads a
conclusion that the cell `*` only can hold 4.

We can apply this rule for columns and boxes. After filling 4, we
apply the rule for 7 and set 7 to the cell marked `!`.

#### Step 3 : backtracking

SudokuSSE does not use the locked candidates method. Before starting
backtracking, SudokuSSE chooses a candidate in a cell in an ongoing
sudoku map.

1. Select a cell that has least size (2 or more) of candidates in the
  cells.
1. If multiple cells are selected, select a row that has the least
  size of candidates in the cells of the row. (Column and boxes can be
  used the step 2 instead of rows.)
1. If the row has multiple cells that have least and same size of
  candidates, use a cell found first.

When every cell in the ongoing sudoku map has unique candidate,
SudokuSSE checks whether all rows, columns, and boxes in the map are
correct. If it is correct, it is a solution of the map. Guessing a
candidate sometimes leads an incorrect solution when the guess is
wrong and SudokuSSE filters it out.

Before starting backtracking, SudokuSSE makes a copy of the sudoku map
to rewind backtracking. The map has only primitives so we can use
compiler-generated copying (trivial copy) and avoid object aliasing.

### Algorithm to count solutions of sudoku puzzles

Backtracking only. Set a cell to candidate 1 to 9 if not blank and
recursively set other cells.

Assume that cells are packed top left and count fast if the assumption
is true.

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

Note that rightmost and bottom cells always have only one candidate in
backtracking and we can avoid setting candidates to them in the
recursion.

Counting solutions can run on a single thread only. To run on multiple
threads, it needs to eliminate global variables.

### Make SudokuSSE faster

I have no quantitative analysis of these items because I have not used
a profiler to SudokuSSE.

* Use the inline keyword and switch it via a macro. SudokuSSE enables
  inlining and unit tests disable inlining. Inlining causes link
  errors in unit test.

* Eliminate virtual function calls. This also prohibits virtual
  destructor. If you can define virtual destructor as a good practice,
  undefine `NO_DESTRUCTOR_AND_VTABLE` macro.

* Declare aliases all integer primitives to switch their bit width
  easily. Performance depends on the bit width and I hope the current
  combination of bit width makes SudokuSSE fastest. Different
  processors and compilers may need different bit width.

* Use const and constexpr as much as possible. This is useful to set a
  constant expression in an if-statement. Constant propagation removes
  constant expressions and unused blocks in an if-statement.
  _SudokuCell::CountCandidatesIfMultiple_ and
  _SudokuCell::MaskCandidatesUnlessMultiple_ eliminates if-statements.

* Unroll loops if appropriate. If a loop has complex branch
  conditions, unrolling the loop can disturb branch prediction of a
  processor and makes it run slower.

* Unroll loops with member function templates and recursive call.  I
  hope compilers expand the recursive call and there is no overhead in
  runtime.

* Use macros instead of function templates if really needed.  C++ code
  cannot break to exit nested loops and use macro `#define func {
  ... return; }` to do it.

* Change `#define FAST_MODE true` to false and run code that is easy
  to read but slow.

* Set optimization level -O2. -O3 makes SudokuSSE slower.

To write fast code in x86_64 assembly, we need to read the x86_64
manual closely. The manual tells us not only general optimization
guidelines but slight differences on performance such as CMOV
instructions.

* Use the x86_64 32-bit registers instead of the 64-bit registers if
possible.  Output to a 32-bit register clears its upper 32 bits and
removes redundant bit masking. This rule surely works fine on the
_using-32bit-registers_ branch.

* To return from a function in assembly, pop + jmp is faster than ret.

I replaced assembly macro parameters as 64-bit registers with 32-bit
registers manually. If you know how to convert a 64-bit register to
its 32-bit register alias, for example RAX to EAX register, I would
like to share your solution on
[the Stack Overflow Community](http://stackoverflow.com/questions/41107642/how-to-convert-x86-64-64-bit-register-names-to-their-corresponding-32-bit-regist).

## Bibliography and acknowledgments

1. I cite the sudoku solver algorithm and puzzle examples from the
  book.

  Dennis E. Shasha (May 2007), "Puzzles for Programmers and Pros", Wrox
  (I read its Japanese translation published by Ohmsha. See my Japanese
  [howtobuild.txt](howtobuild.txt).)

2. I use CppUnit code on the website.

  _http://www.atmarkit.co.jp/fdotnet/cpptest/cpptest02/cpptest02_02.html_

3. I adopt an idea of Mr. Kawai Hidemi to count how many solutions in
  a sudoku puzzle and compare execution time with his program on the
  article.

  _http://developer.cybozu.co.jp/tech/?p=1692_

4. I check instruction set and latency of x86_64 on the manual.

  "Intel 64 and IA-32 Architectures Optimization Reference Manual"

  _http://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-optimization-manual.html_

5. I use these sudoku puzzles as test cases.

  http://staffhome.ecm.uwa.edu.au/~00013890/sudoku17

6. I posted a question for assembly and received some useful advice.

  http://stackoverflow.com/questions/41107642/how-to-convert-x86-64-64-bit-register-names-to-their-corresponding-32-bit-regist
