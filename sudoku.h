// Sudoku solver with SSE 4.2 / AVX
// Copyright (C) 2012-2018 Zettsu Tatsuya

#ifndef SUDOKU_H_INCLUDED
#define SUDOKU_H_INCLUDED

#include <stdint.h>
#include <array>
#include <string>
#include <vector>
#include <utility>
#include <nmmintrin.h>

// Undefine this macro if we use virtual functions.
// Normally we avoid virtual functions to make this solver run fast.
#define NO_DESTRUCTOR_AND_VTABLE (1)

// Set this macro to use fast but complicated code
#define FAST_MODE (true)

// Set this macro to solve Sudoku-X (Set 1..0 to diagonal bars)
#define SUDOKU_X_MODE (true)
#if defined(UNITTEST)
extern bool SudokuXmode;
#endif

// if-constexpr {} in C++17
#if __cplusplus >= 201703L
#define CPP17_IF_CONSTEXPR constexpr
#else
#define CPP17_IF_CONSTEXPR
#endif

// Do not use inlining in unit tests to prevent link errors.
#if !defined(UNITTEST)
  #define INLINE inline
#else
  #define INLINE
#endif

#ifdef NO_DESTRUCTOR_AND_VTABLE
  #define ALLOW_VIRTUAL
  #define NO_DESTRUCTOR (1)
#else
  #define ALLOW_VIRTUAL virtual
#endif

// Declare an interface to support multiple platforms:
// Windows/Linux and C++11/Boost C++ Libraries.
#include "sudoku_os_dependent.h"

// This function calculates the size of an array in compilation time.
// We have to take a const reference to an array as an argument to reject pointers.
// This function may cause compilation errors if it takes arrays of
// function-internally defined structs in C++98 (not in C++11).
template<typename T, size_t n>
constexpr size_t arraySizeof(const T (&)[n]) {
    return n;
}

// Command line arguments
namespace SudokuOption {
    const char * const CommandLineArgParallel = "-N";
    const char * const CommandLineArgSseSolver[] = {"1", "sse", "avx"};
    const char * const CommandLineNoChecking[] = {"1", "off"};
    const char * const CommandLinePrint[] = {"2", "print"};

    // This function sets a value of a command line argument to arg 'target' if it is valid.
    template <typename T, size_t n>
    void setMode(int argc, const char * const argv[], int argIndex, const char * const (&pOptionSet)[n], T& target, T value) {
        if (argc <= argIndex) {
            return;
        }

        std::string param = argv[argIndex];
        for(auto& pOpt : pOptionSet) {
            std::string opt = pOpt;
            if (param == opt) {
                target = value;
            }
        }

        return;
    }
}

// Primitive type aliases
// We have to choose appropriate types to run this solver faster.
// Built-in short is faster than int in some cases.
// Notice all data in this solver should fit in 32Kbyte L1 Data Cache.
using SudokuIndex = unsigned short;          // indexes of cells and groups of cells (short is faster)
using SudokuLoopIndex = unsigned int;        // indexes of loops for cells and groups of cells (int is faster)
using SudokuCellCandidates = unsigned int;   // candidates [1..9] of a cell
using SudokuNumber = int;                    // a preset number of a cell
using SudokuSseElement = uint32_t;           // three adjacent cells to solve with SSE4.2 instructions
using gRegister = uint64_t;                  // a number that a general purpose register holds (must be unsigned)
using xmmRegister = __m128;                  // a number that an XMM register holds
using SudokuPatternCount = uint64_t;         // a number of solutions of a puzzle
using SudokuPuzzleCount = size_t;            // a number of puzzles in an input file

static_assert(sizeof(SudokuSseElement) == 4, "Unexpected SudokuSseElement size");
static_assert(sizeof(xmmRegister) == 16, "Unexpected xmmRegister size");
static_assert((alignof(xmmRegister) % 16) == 0, "Unexpected xmmRegister alignment");

// Size for SSE4.2 instructions
namespace SudokuSse {
    constexpr size_t RegisterCnt = 16;    // the number of XMM registers
    constexpr size_t RegisterWordCnt = 4; // the number of words in an XMM register
}

// All XMM registers (128-bit * 16 registers)
union XmmRegisterSet {
    SudokuSseElement regVal_[SudokuSse::RegisterCnt * SudokuSse::RegisterWordCnt];
    xmmRegister      regXmmVal_[SudokuSse::RegisterCnt];
};

// Constants for Sudoku
namespace Sudoku {
    constexpr SudokuIndex SizeOfCellsPerGroup = 9;    // the number of cells in a column, row, and box (square)
    constexpr SudokuIndex SizeOfGroupsPerMap = 9;     // the number of columns, rows, and boxes in a puzzle
    constexpr SudokuIndex SizeOfDiagonalBarsPerMap = 2;  // the number of diagonal bars in a puzzle
    constexpr SudokuIndex SizeOfAllCells = 81;        // the number of cells in a puzzle
    constexpr SudokuIndex SizeOfCandidates = 9;       // the maximum number of candidates in a cell
    constexpr SudokuIndex SizeOfUniqueCandidate = 1;  // the minimum number of candidates in a cell
    constexpr SudokuIndex OutOfRangeCandidates = 0x10;  // a special number of candidates if a cell has none of or one candidate (must be 2^n)
    constexpr SudokuIndex OutOfRangeMask = OutOfRangeCandidates - 1;  // a bitmask for a number of candidates when a cell has none of or one candidate
    constexpr SudokuIndex SizeOfGroupsPerCell = 3;    // indicates that a cell belongs to a column, row, and box.
    constexpr SudokuIndex SizeOfLookUpCell = 512;     // the number of elements in a cell look-up table
    constexpr SudokuIndex SizeOfBoxesOnEdge = 3;      // indicates that a row and column have three boxes.
    constexpr SudokuIndex SizeOfCellsOnBoxEdge = 3;   // indicates that a box has three rows and columns.
    // Hold these constants in their narrowest bit width needed. Compilers expand them if needed.
    constexpr unsigned short EmptyCandidates = 0;     // indicates that a cell holds no candidates.
    constexpr unsigned short UniqueCandidates = 1;    // indicates that a cell holds one candidate.
    constexpr unsigned short AllCandidates = 0x1ff;   // indicates that a cell holds all 9 candidates.
    constexpr SudokuSseElement AllThreeCandidates = 0x7ffffff;  // indicates that all three adjacent cells hold all 27 candidates.
    constexpr short MinCandidatesNumber = 1;          // a minimum preset number of a cell
    constexpr short MaxCandidatesNumber = 9;          // a maximum preset number of a cell
    // Allocate larger memory than the cache line size to prevent false sharing in heap memory.
    constexpr uint32_t CacheGuardSize = 128;
}

// Common functions
namespace Sudoku {
    void LoadXmmRegistersFromMem(const xmmRegister *pData);  // loads XMM registers from main memory
    void SaveXmmRegistersToMem(xmmRegister *pData);          // saves XMM registers to main memory

    // Sets a number to a cell if valid
#if __cplusplus >= 201703L
    template <typename SudokuNumberType>
    std::pair<bool, int> ConvertCharToSudokuCandidate(SudokuNumberType minNum, SudokuNumberType maxNum, char c);
#else
    template <typename SudokuNumberType>
    bool ConvertCharToSudokuCandidate(SudokuNumberType minNum, SudokuNumberType maxNum, char c, int& num);
#endif
    // Prints all candidates in a cell
    template <typename SudokuElementType>
    void PrintSudokuElement(SudokuElementType candidates, SudokuElementType uniqueCandidates,
                            SudokuElementType emptyCandidates, std::ostream* pSudokuOutStream);
}

// Selecting how to solve
enum class SudokuSolverType {
    SOLVER_GENERAL,  // C++ template metaprogramming without assembly
    SOLVER_SSE_4_2,  // SSE4.2 or AVX assembly
};

// Selecting whether to check solutions
enum class SudokuSolverCheck {
    CHECK,         // Check solutions
    DO_NOT_CHECK,  // Do not check solutions
};

// Selecting whether to print solutions when solving puzzles
enum class SudokuSolverPrint {
    DO_NOT_PRINT,  // Do not print solutions
    PRINT,         // Print solutions
};

// A base sudoku solver class
class SudokuBaseSolver {
public:
    virtual bool Exec(bool silent, bool verbose) = 0;
    virtual void PrintType(void) = 0;
protected:
    SudokuBaseSolver(std::ostream* pSudokuOutStream);
    SudokuBaseSolver(const SudokuBaseSolver&) = delete;
    SudokuBaseSolver& operator =(const SudokuBaseSolver&) = delete;
    virtual void printType(const std::string& presetStr, std::ostream* pSudokuOutStream);
    int            count_;             // counts how many times it repeats to solve a puzzle
    std::ostream*  pSudokuOutStream_;  // destination to print results
};

// An element of a look-up table to get attributes of a cell from a cell candidate bitboard.
class SudokuCellLookUp {
public:
    bool        IsUnique;           // indicates that this cell has a unique candidate
    bool        IsMultiple;         // indicates that this cell has multiple candidates
    SudokuIndex NumberOfCandidates; // presents a number of candidates this cell has
};

// A cell in a Sudoku puzzle
class SudokuCell {
    // unit tests
    friend class SudokuCellTest;
    friend class SudokuMapTest;
    friend class SudokuSolverTest;
    template <class TestedT, class CandidatesT> friend class SudokuCellCommonTest;
    template <class TestedT, class CandidatesT> friend class SudokuSolverCommonTest;

public:
    SudokuCell(void);
#ifndef NO_DESTRUCTOR
    // If destructors do nothing but compiler-generated code,
    // it is a good practice to define them with =default or not to define them.
    // Giving implementations to destructors may disturb move constructors.
    ALLOW_VIRTUAL ~SudokuCell() = default;
#endif
    // Initializing and output a cell
    void Preset(char c);
    void SetIndex(SudokuIndex indexNumber);
    void Print(std::ostream* pSudokuOutStream) const;
    INLINE SudokuIndex GetIndex(void) const;
    // Manipulating a cell while solving a puzzle
    INLINE bool IsFilled(void) const;
    INLINE bool HasMultipleCandidates(void) const;
    INLINE bool IsConsistent(SudokuCellCandidates candidates) const;
    INLINE bool HasCandidate(SudokuCellCandidates candidate) const;
    INLINE bool HasNoCandidates(void) const;
    INLINE void SetCandidates(SudokuCellCandidates candidates);
    INLINE SudokuCellCandidates GetCandidates(void) const;
    INLINE SudokuCellCandidates GetUniqueCandidate(void) const;
    INLINE SudokuIndex CountCandidates(void) const;
    INLINE SudokuIndex CountCandidatesIfMultiple(void) const;

    INLINE static SudokuIndex MaskCandidatesUnlessMultiple(SudokuIndex numberOfCandidates);
    INLINE static bool IsEmptyCandidates(SudokuCellCandidates candidates);
    INLINE static bool IsUniqueCandidate(SudokuCellCandidates candidates);
    INLINE static SudokuCellCandidates GetEmptyCandidates(void);
    INLINE static SudokuCellCandidates FlipCandidates(SudokuCellCandidates candidates);
    INLINE static SudokuCellCandidates MergeCandidates(SudokuCellCandidates candidatesA, SudokuCellCandidates candidatesB);
    INLINE static SudokuCellCandidates GetInitialCandidate();
    INLINE static SudokuCellCandidates GetNextCandidate(SudokuCellCandidates candidate);
private:
    INLINE void updateState(void);
    // All members are trivially copyable.
    SudokuIndex          indexNumber_;  // a serial number in all cells
    SudokuCellCandidates candidates_;   // candidates (each of 1..9 matches bit 0..8)
    // A look-up table to get attributes of cells from their bitboard.
    static const SudokuCellLookUp CellLookUp_[Sudoku::SizeOfLookUpCell];

    static constexpr SudokuCellCandidates SudokuEmptyCandidates = Sudoku::EmptyCandidates;
    static constexpr SudokuCellCandidates SudokuUniqueCandidates = Sudoku::UniqueCandidates;
    // All bits[8..0] are set to 1.
    static constexpr SudokuCellCandidates SudokuAllCandidates = Sudoku::AllCandidates;
    static constexpr SudokuNumber SudokuMinCandidatesNumber = Sudoku::MinCandidatesNumber;
    static constexpr SudokuNumber SudokuMaxCandidatesNumber = Sudoku::MaxCandidatesNumber;
};

// All cells in solving C++ template metaprogramming without assembly
class SudokuMap {
    // unit tests
    friend class SudokuMapTest;
    friend class SudokuSolverTest;
    template <class TestedT, class CandidatesT> friend class SudokuSolverCommonTest;

public:
    SudokuMap(void);
#ifndef NO_DESTRUCTOR
    ALLOW_VIRTUAL ~SudokuMap() = default;
#endif
    // Initializing and output
    void Preset(const std::string& presetStr, SudokuIndex seed);
    void Print(std::ostream* pSudokuOutStream) const;
    // Solving a puzzle
    INLINE bool IsFilled(void) const;
    bool FillCrossing(void);
    INLINE bool CanSetUniqueCell(SudokuIndex cellIndex, SudokuCellCandidates candidate) const;
    INLINE void SetUniqueCell(SudokuIndex cellIndex, SudokuCellCandidates candidate);
    INLINE SudokuIndex CountFilledCells(void) const;
    INLINE SudokuIndex SelectBacktrackedCellIndex(void) const;
    bool IsConsistent(void) const;
private:
    bool findUnusedCandidate(SudokuCell& targetCell) const;
    bool findUniqueCandidate(SudokuCell& targetCell) const;
    // All cells in a puzzle
    SudokuCell cells_[Sudoku::SizeOfAllCells];
    // determines which cell in columns, rows, or boxes do we select in backtracking.
    SudokuIndex backtrackedGroup_;

    // Cells in all columns, all rows, and all boxes
    static const SudokuIndex Group_[Sudoku::SizeOfGroupsPerCell][Sudoku::SizeOfGroupsPerMap][Sudoku::SizeOfCellsPerGroup];

    // Columns, rows, and boxes which all cells belong to
    static const SudokuIndex ReverseGroup_[Sudoku::SizeOfAllCells][Sudoku::SizeOfGroupsPerCell];

    // Cells in diagonal bars
    static const SudokuIndex DiagonalBarGroup_[Sudoku::SizeOfDiagonalBarsPerMap][Sudoku::SizeOfCellsPerGroup];

    // The serial number of a box in 9-cells groups {rows:0, columns:1, box:3}.
    static constexpr SudokuIndex SudokuBoxGroupId = 2;

    // Use inlining and unrolling to solve puzzles faster.
    template <SudokuIndex index> INLINE SudokuIndex unrolledCountFilledCells
        (SudokuIndex accumCount) const;
    template <SudokuIndex innerIndex> INLINE SudokuCellCandidates unrolledFindUnusedCandidateInner
        (SudokuIndex targetCellIndex, SudokuIndex outerIndex, SudokuIndex groupIndex, SudokuCellCandidates candidates) const;
    template <SudokuIndex outerIndex> INLINE SudokuCellCandidates unrolledFindUnusedCandidateOuter
        (SudokuIndex targetCellIndex, SudokuCellCandidates candidates) const;
    template <SudokuIndex innerIndex> INLINE SudokuCellCandidates unrolledFindUniqueCandidateInner
        (SudokuIndex targetCellIndex, SudokuIndex outerIndex, SudokuIndex groupIndex, SudokuCellCandidates candidates) const;
    template <SudokuIndex innerIndex> INLINE SudokuIndex unrolledSelectBacktrackedCellIndexInner
        (SudokuIndex outerIndex, SudokuIndex& leastCountOfGroup, SudokuIndex& candidateCellIndex) const;
    INLINE SudokuCellCandidates unrolledFindUnusedCandidateInnerCommon
        (SudokuIndex targetCellIndex, SudokuIndex outerIndex, SudokuIndex groupIndex, SudokuIndex innerIndex,
         SudokuCellCandidates candidates) const;
    INLINE SudokuCellCandidates unrolledFindUnusedCandidateOuterCommon
        (SudokuIndex targetCellIndex, SudokuIndex outerIndex, SudokuCellCandidates candidates) const;
    INLINE SudokuCellCandidates unrolledFindUniqueCandidateInnerCommon
        (SudokuIndex targetCellIndex, SudokuIndex outerIndex, SudokuIndex groupIndex, SudokuIndex innerIndex,
         SudokuCellCandidates candidates) const;
    INLINE SudokuIndex unrolledSelectBacktrackedCellIndexInnerCommon
        (SudokuIndex outerIndex, SudokuIndex innerIndex,
         SudokuIndex& leastCountOfGroup, SudokuIndex& candidateCellIndex) const;
};

// A Sudoku solver with C++ template metaprogramming without assembly
class SudokuSolver : public SudokuBaseSolver {
    // unit tests
    friend class SudokuSolverTest;
    template <class TestedT, class CandidatesT> friend class SudokuSolverCommonTest;
public:
    SudokuSolver(const std::string& presetStr, SudokuIndex seed, std::ostream* pSudokuOutStream);
    SudokuSolver(const std::string& presetStr, SudokuIndex seed, std::ostream* pSudokuOutStream, SudokuPatternCount printAllCandidate);
    virtual ~SudokuSolver() = default;
    virtual bool Exec(bool silent, bool verbose) override;
    virtual void PrintType(void) override;
private:
    bool solve(SudokuMap& map, bool topLevel, bool verbose);
    bool fillCells(SudokuMap& map, bool topLevel, bool verbose);

    SudokuMap map_;  // A sudoku puzzle (we allocate copies of this in backtracking)
};

// A cell in solving assembly
class SudokuSseCell {
    // unit tests
    friend class SudokuSseCellTest;
    template <class TestedT, class CandidatesT> friend class SudokuCellCommonTest;
    template <class TestedT, class CandidatesT> friend class SudokuSolverCommonTest;

public:
    SudokuSseCell(void);
#ifndef NO_DESTRUCTOR
    ALLOW_VIRTUAL ~SudokuSseCell() = default;
#endif
    void Preset(char c);
    void Print(std::ostream* pSudokuOutStream) const;
    void SetCandidates(SudokuSseElement candidates);
    SudokuSseElement GetCandidates(void);
    static constexpr SudokuSseElement AllCandidates = Sudoku::AllCandidates;
private:
    static constexpr SudokuSseElement SudokuEmptyCandidates = Sudoku::EmptyCandidates;
    static constexpr SudokuSseElement SudokuUniqueCandidates = Sudoku::UniqueCandidates;
    static constexpr SudokuNumber SudokuMinCandidatesNumber = Sudoku::MinCandidatesNumber;
    static constexpr SudokuNumber SudokuMaxCandidatesNumber = Sudoku::MaxCandidatesNumber;
    SudokuSseElement candidates_;   // Candidates (each of 1..9 matches bit 0..8)
};

// Define these variables in assembly .s files.
extern "C" {
    // Member functions of SudokuSseEnumeratorMap
    // These are variables global and are not for multi-threading.
    extern volatile uint64_t sudokuXmmPrintAllCandidate;
    extern volatile uint64_t sudokuXmmRightBottomElement;
    extern volatile uint64_t sudokuXmmRightBottomSolved;
    extern volatile uint64_t sudokuXmmAllPatternCnt;
    // PrintPattern() -> SudokuSseEnumeratorMap::PrintFromAsm
    extern XmmRegisterSet sudokuXmmToPrint;

    // These variables are set before running on multi-threading and
    // are read-only from threads. So it is allowed they are non-thread-local variables.
    extern volatile uint64_t sudokuXmmPrintFunc;  // main()
    extern volatile uint64_t sudokuXmmAssumeCellsPacked; // SudokuLoader::CanLaunch()
    extern volatile uint64_t sudokuXmmUseAvx;     // unused

    // sudokuXmmDebug is used for debugging only and not suitable for multi-threading.
    extern volatile uint64_t sudokuXmmDebug;
}

// Candidates for backtracking
struct SudokuSseCandidateCell {
    size_t           regIndex;  // a general purpose register number of this cell
    SudokuSseElement shift;     // a bit position from LSB in a register of this cell
    SudokuSseElement mask;      // a bit mask in a register that holds this cell
};

// Results for solving in sudokusse.s
struct SudokuSseMapResult {
    gRegister aborted;
    gRegister elementCnt;
    gRegister nextCellFound;
    gRegister nextOutBoxIndex;
    gRegister nextInBoxIndex;
    gRegister nextRowNumber;
};

// All cells in solving assembly
class SudokuSseMap {
    // unit tests
    friend class SudokuSseMapTest;
    template <class TestedT, class CandidatesT> friend class SudokuSolverCommonTest;
private:
    static constexpr size_t InitialRegisterNum = 1;  // The number of an XMM register which holds the top row
    XmmRegisterSet xmmRegSet_;
public:
    SudokuSseMap(void);
#ifndef NO_DESTRUCTOR
    ALLOW_VIRTUAL ~SudokuSseMap() = default;
#endif
    void Preset(const std::string& presetStr);
    void Print(std::ostream* pSudokuOutStream) const;
    void FillCrossing(bool loadXmm, SudokuSseMapResult& result);
    INLINE bool GetNextCell(const SudokuSseMapResult& result, SudokuSseCandidateCell& cell);
    INLINE bool CanSetUniqueCell(const SudokuSseCandidateCell& cell, SudokuCellCandidates candidate) const;
    INLINE void SetUniqueCell(const SudokuSseCandidateCell& cell, SudokuCellCandidates candidate);
};

// All cells to counting solutions of a puzzle in assembly
class SudokuSseEnumeratorMap {
    // unit tests
    friend class SudokuSseEnumeratorMapTest;
private:
    static constexpr size_t InitialRegisterNum = 1;      // the number of an XMM register which holds the top row.
    static constexpr size_t RightColumnRegisterNum = 10; // the number of an XMM register which holds the rightmost column.
    static constexpr size_t CellBitWidth = 16;           // the number of bits for a cell
    static constexpr size_t BitsPerByte = 8;             // bits per byte
    SudokuCellCandidates rightBottomElement_;
    gRegister firstCell_;
    SudokuPatternCount patternNumber_;
    static SudokuSseEnumeratorMap* pInstance_;
    XmmRegisterSet xmmRegSet_;
public:
    SudokuSseEnumeratorMap(std::ostream* pSudokuOutStream);
    virtual ~SudokuSseEnumeratorMap();
    // Initializing and output
    void SetToPrint(SudokuPatternCount printAllCandidate);
    void Preset(const std::string& presetStr);
    void Print(void) const;
    void Print(bool solved, const XmmRegisterSet& xmmRegSet) const;
    void PrintFromAsm(const XmmRegisterSet& xmmRegSet);
    SudokuPatternCount Enumerate(void);
    static SudokuSseEnumeratorMap* GetInstance(void);
private:
    void presetCell(SudokuLoopIndex index, int num);
    size_t powerOfTwoPlusOne(SudokuSseElement regValue) const;
    std::ostream* pSudokuOutStream_;
};

// A Sudoku solver with assembly
class SudokuSseSolver : public SudokuBaseSolver {
    // unit tests
    friend class SudokuSseSolverTest;
    template <class TestedT, class CandidatesT> friend class SudokuSolverCommonTest;

public:
    SudokuSseSolver(const std::string& presetStr, std::ostream* pSudokuOutStream, SudokuPatternCount printAllCandidate);
    SudokuSseSolver(const std::string& presetStr, SudokuIndex seed, std::ostream* pSudokuOutStream, SudokuPatternCount printAllCandidate);
    virtual ~SudokuSseSolver() = default;
    virtual bool Exec(bool silent, bool verbose) override;
    virtual void Enumerate(void);
    virtual void PrintType(void) override;
private:
    void initialize(const std::string& presetStr, std::ostream* pSudokuOutStream);
    bool solve(SudokuSseMap& map, bool topLevel, bool verbose);
    bool fillCells(SudokuSseMap& map, bool topLevel, bool verbose, SudokuSseMapResult& result);

    SudokuSseMap map_;    // a sudoku puzzle (we allocate copies of this in backtracking)
    SudokuSseEnumeratorMap enumeratorMap_;
    SudokuPatternCount printAllCandidate_;
};

// Checking solutions are correct and meet constraints for Sudoku
class SudokuCheckerTest;
class SudokuChecker {
    // unit tests
    friend class SudokuCheckerTest;
public:
    SudokuChecker(const std::string& puzzle, const std::string& solution, SudokuSolverPrint printSolution, std::ostream* pSudokuOutStream);
    virtual ~SudokuChecker() = default;
    SudokuChecker(const SudokuChecker&) = delete;
    SudokuChecker& operator =(const SudokuChecker&) = delete;
    bool Valid() const;  // returns true if solved correctly
private:
    // Allocate these arrays on stack to avoid false sharing on heap.
    using Group = std::array<SudokuNumber, Sudoku::SizeOfCellsPerGroup>;
    using Grid = std::array<Group, Sudoku::SizeOfGroupsPerMap>;
    bool parse(const std::string& puzzle, const std::string& solution, SudokuSolverPrint printSolution, std::ostream* pSudokuOutStream);
    bool parseRow(SudokuIndex row, const std::string& rowLine, Grid& grid, std::string& solutionLine);
    bool compare(const std::string& puzzle, const std::string& solution, std::ostream* pSudokuOutStream);
    bool check(const Grid& grid, std::ostream* pSudokuOutStream);
    bool checkRowSet(const Grid& grid, std::ostream* pSudokuOutStream);
    bool checkColumnSet(const Grid& grid, std::ostream* pSudokuOutStream);
    bool checkBoxSet(const Grid& grid, std::ostream* pSudokuOutStream);
    bool checkDiagonal(const Grid& grid, std::ostream* pSudokuOutStream);
    bool checkUnique(const Group& line);
    bool valid_;
};

class SudokuDispatcherTest;
class SudokuMultiDispatcherTest;

// Reading and solving a puzzle in a thread.
// This class is movable and cannot be inherited to be pushed to a vector.
class SudokuDispatcher final {
    // unit tests
    friend class SudokuDispatcherTest;
    friend class SudokuMultiDispatcherTest;
    friend class SudokuLoaderTest;
public:
    SudokuDispatcher(SudokuSolverType solverType, SudokuSolverCheck check, SudokuSolverPrint print,
                     SudokuPatternCount printAllCandidate, SudokuPuzzleCount puzzleNum, const std::string& puzzleLine);
    // Do not define a destructor to make this movable.
    bool Exec(void);
    const std::string& GetMessage(void) const;
private:
    bool exec(SudokuBaseSolver& solver, std::ostringstream& ss);
    SudokuSolverType   solverType_;   // How to solve Sudoku puzzles in lines
    SudokuSolverCheck  check_;        // Whether or not checking solutions
    SudokuSolverPrint  print_;        // Whether or not printing results
    SudokuPatternCount printAllCandidate_;
    SudokuPuzzleCount  puzzleNum_;
    std::string puzzleLine_;
    std::string message_;  // written to an output stream
};

// Reading and solving puzzles in a thread
class SudokuMultiDispatcher {
public:
    // unit tests
    friend class SudokuMultiDispatcherTest;
    friend class SudokuLoaderTest;

    SudokuMultiDispatcher(SudokuSolverType solverType, SudokuSolverCheck check, SudokuSolverPrint print, SudokuPatternCount printAllCandidate);
    virtual ~SudokuMultiDispatcher() = default;
    SudokuMultiDispatcher(const SudokuMultiDispatcher&) = delete;
    SudokuMultiDispatcher& operator =(const SudokuMultiDispatcher&) = delete;
    virtual void AddPuzzle(SudokuPuzzleCount puzzleNum, const std::string& puzzleLine);
    virtual bool ExecAll(void);
    virtual const std::string& GetMessage(size_t index) const;
private:
    std::vector<SudokuDispatcher> dipatcherSet_;
    // Same as SudokuDispatcher
    SudokuSolverType   solverType_;
    SudokuSolverCheck  check_;
    SudokuSolverPrint  print_;
    SudokuPatternCount printAllCandidate_;
};

// Reading puzzles and measuring how long does it take to solve them.
class SudokuLoader {
    // unit tests
    friend class SudokuLoaderTest;
    friend struct SudokuTestArgsMultiMode;

public:
    SudokuLoader(int argc, const char * const argv[], std::istream* pSudokuInStream, std::ostream* pSudokuOutStream);

#ifndef NO_DESTRUCTOR
    ALLOW_VIRTUAL ~SudokuLoader() = default;
#endif
    SudokuLoader(const SudokuLoader&) = delete;
    SudokuLoader& operator =(const SudokuLoader&) = delete;
    int Exec(void);
    static bool CanLaunch(int argc, const char * const argv[]);
private:
    using ExitStatusCode = int;
    using NumberOfCores = Sudoku::BaseParallelRunner::NumberOfCores;
    using DispatcherPtr = std::unique_ptr<SudokuMultiDispatcher>;
    using DispatcherPtrSet = std::vector<DispatcherPtr>;

    static int getMeasureCount(const char *arg);
    void setSingleMode(int argc, const char * const argv[], std::istream* pSudokuInStream);
    bool setMultiMode(int argc, const char * const argv[]);
    bool setNumberOfThreads(int argc, const char * const argv[], int argIndex);
    ExitStatusCode execSingle(void);
    ExitStatusCode execMulti(void);
    ExitStatusCode execMulti(std::istream* pSudokuInStream);
    void printHeader(SudokuSolverType solverType, std::ostream* pSudokuOutStream);
    SudokuPuzzleCount readLines(NumberOfCores numberOfCores, std::istream* pSudokuInStream, DispatcherPtrSet& dispatcherSet);
    ExitStatusCode execAll(NumberOfCores numberOfCores, DispatcherPtrSet& dispatcherSet);
    void writeMessage(NumberOfCores numberOfCores, SudokuPuzzleCount sizeOfPuzzle, DispatcherPtrSet& dispatcherSet, std::ostream* pSudokuOutStream);
    void measureTimeToSolve(SudokuSolverType solverType);
    SudokuTime solveSudoku(SudokuSolverType solverType, int count, bool warmup);
    SudokuTime enumerateSudoku(void);

    std::string sudokuStr_;  // represents a puzzle (set of initial numbers)
    std::string multiLineFilename_;      // name of a file that holds Sudoku puzzles in lines.
    std::unique_ptr<Sudoku::BaseParallelRunner> pParallelRunner_;  // set of parallel runners of solvers
    NumberOfCores     numberOfThreads_;  // How many threads solving puzzles
    SudokuSolverType  solverType_;  // How to solve Sudoku puzzles
    SudokuSolverCheck check_;       // Whether or not checking solutions
    SudokuSolverPrint print_;       // Whether or not printing results
    bool   isBenchmark_;    // true when it runs for benchmarking
    bool   verbose_;        // true if printing steps to solving a puzzle
    int    measureCount_;   // how many times it solves a puzzle
    SudokuPatternCount printAllCandidate_;
    std::ostream* pSudokuOutStream_;  // receives strings to write
    static constexpr NumberOfCores DefaultNumberOfThreads = 1;  // the default number of threads
    // Exit status set that is passed to make
    static const ExitStatusCode ExitStatusPassed;
    static const ExitStatusCode ExitStatusFailed;
};

extern "C" {
    void PrintPattern(void);
}

#endif // SUDOKU_H_INCLUDED

/*
Local Variables:
mode: c++
coding: utf-8-unix
tab-width: nil
c-file-style: "stroustrup"
End:
*/
