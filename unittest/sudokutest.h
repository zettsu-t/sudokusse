// Testing Sudoku solver with SSE 4.2 / AVX
// Copyright (C) 2012-2017 Zettsu Tatsuya

#include <stdint.h>
#include <cstdlib>
#include <string>

// Define arraySizeof()
#include "sudoku.h"

// Returns number of bits of T
template<typename T>
constexpr size_t bitsOf(T) {
    return sizeof(T) * 8;
}

// Declare conditions for testing here and define them in assembly
extern "C" {
    extern volatile uint64_t testSearchRowPartElementsPreUniqueCandidate;
    extern volatile uint64_t testSearchRowPartElementsPreRowPart;
    extern volatile uint64_t testSearchRowPartElementsPreInBoxShift;
    extern volatile uint64_t testSearchRowPartElementsPreOutBoxShift;
    extern volatile uint64_t testSearchRowElementsPreUniqueCandidate;
    extern volatile uint64_t testSearchRowElementsPreCandidateRow;
    extern volatile uint64_t testSearchRowElementsPreInBoxShift;
    extern volatile uint64_t testSearchRowElementsPreOutBoxShift;
    extern volatile uint64_t testFillUniqueElementPreRowPart;
    extern volatile uint64_t testFillUniqueElementPreColumn;
    extern volatile uint64_t testFillUniqueElementPreBox;
    extern volatile uint64_t testFillUniqueElementPreRow;
    extern volatile uint64_t testFindRowPartCandidatesPreRowPartTarget;
    extern volatile uint64_t testFindRowPartCandidatesPreRowCandidates;
    extern volatile uint64_t testFindRowPartCandidatesPreBox;
    extern volatile uint64_t testFindRowPartCandidatesRowPartTarget;
    extern volatile uint64_t testFindRowPartCandidatesRowCandidates;
    extern volatile uint64_t testFindRowPartCandidatesBox;
    extern volatile uint64_t testFillNineUniqueCandidatesPreRow;
    extern volatile uint64_t testCountRowCellCandidatesMinCount;
    extern volatile uint64_t testCountRowCellCandidatesRowPopCount;
    extern volatile xmmRegister testFillNineUniqueCandidatesRowX;
    extern volatile xmmRegister testFillNineUniqueCandidatesBoxX;
    extern volatile xmmRegister testFillNineUniqueCandidatesColumnX;
    extern volatile xmmRegister testCountRowCellCandidatesRowX;
}

namespace SudokuTestCandidates {
    constexpr SudokuCellCandidates Empty = 0;              // []
    constexpr SudokuCellCandidates UniqueBase = 1;         // [1]
    constexpr SudokuCellCandidates OneOnly = 1;            // [1]
    constexpr SudokuCellCandidates TwoOnly = 1 << 1;       // [2]
    constexpr SudokuCellCandidates CenterOnly = 1 << 4;    // [5]
    constexpr SudokuCellCandidates NineOnly = 1 << 8;      // [9]
    constexpr SudokuCellCandidates CenterLine = 0x038;     // [4,5,6]
    constexpr SudokuCellCandidates FourAndSix  = 0x028;    // [4,6]
    constexpr SudokuCellCandidates DoubleLine = 0x01c7;    // [1,2,3,7,8,9]
    constexpr SudokuCellCandidates TwoToNine = 0x1fe;      // [2,3,4,5,6,7,8,9]
    constexpr SudokuCellCandidates ExceptTwo = 0x1fd;      // [1,3,4,5,6,7,8,9]
    constexpr SudokuCellCandidates ExceptCenter = 0x1ef;   // [1,2,3,4,6,7,8,9]
    constexpr SudokuCellCandidates Odds = 0x155;           // [1,3,5,7,9]
    constexpr SudokuCellCandidates Evens = 0x0aa;          // [2,4,6,8]
    constexpr SudokuCellCandidates All = 0x1ff;            // [1,2,3,4,5,6,7,8,9]
    constexpr SudokuCellCandidates CandidateSetOne[] {OneOnly, TwoToNine, Odds, Evens, All};
    constexpr SudokuCellCandidates CandidateSetTwo[] {TwoOnly, ExceptTwo, Odds, Evens, All};
    constexpr SudokuCellCandidates CandidateSetCenter[] {CenterOnly, ExceptCenter, Odds, Evens, All};
    constexpr size_t candidateSetSize = arraySizeof(CandidateSetOne);
}

// Index of cells for testing
// *........
// ...*.....
// ......*..
// .*.^.....
// ....*....
// .......*.
// ..*.....!
// .....*.b.
// ........*
namespace SudokuTestPosition {
    constexpr SudokuIndex Head = 0;          // top-left corner
    constexpr SudokuIndex HeadNext = 1;      // top and next-to-left
    constexpr SudokuIndex Center = 40;       // center
    constexpr SudokuIndex Centerorigin = 30; // top-left corner of the center box
    constexpr SudokuIndex Conflict = 62;     // the cell marked ! above
    constexpr SudokuIndex Backtracked = 70;  // the cell marked b above (selected for backtracking)
    constexpr SudokuIndex Last = Sudoku::SizeOfAllCells - 1;  // last and bottom-righr corner
};

// These cases are based on the book;
// Dennis E. Shasha (May 2007), "Puzzles for Programmers and Pros", Wrox
// (I read its Japanese translation published by Ohmsha).
namespace SudokuTestPattern {
    constexpr SudokuIndex ConflictedCell = 0xfe;  // unspecified value for an inconsistent cell
    const std::string NoBacktrackString = "........77.4...893..68.2.....75286...8...67.19.34...8....7.49..6...9....459...1.8";
    const std::string NoBacktrackStringSolution = "815349267724651893396872415147528639582936741963417582231784956678195324459263178";
    const std::string BacktrackString =   ".3.....4..1..97.5...25.86....3...8..9....43....76....4..98.54...7.....2..5..71.8.";
    const std::string BacktrackString2 =  ".30_0_.40_10_97.50_.25.860_0_30_.80_90_0_430_0_760_0_40_98.540_.70_0_.20_50_71.8.";
    const std::string ConflictString =    "123456789123456789123456789123456789123456789123456789123456789123456789123456...";
    constexpr SudokuIndex NoBacktrackPreset[] {0,0,0,0,0,0,0,0,7,7,0,4,0,0,0,8,9,3,0,0,6,8,0,2,0,0,0,0,0,7,5,2,8,6,0,0,0,8,0,0,0,6,7,0,1,9,0,3,4,0,0,0,8,0,0,0,0,7,0,4,9,0,0,6,0,0,0,9,0,0,0,0,4,5,9,0,0,0,1,0,8};
    constexpr SudokuIndex BacktrackPreset[]   {0,3,0,0,0,0,0,4,0,0,1,0,0,9,7,0,5,0,0,0,2,5,0,8,6,0,0,0,0,3,0,0,0,8,0,0,9,0,0,0,0,4,3,0,0,0,0,7,6,0,0,0,0,4,0,0,9,8,0,5,4,0,0,0,7,0,0,0,0,0,2,0,0,5,0,0,7,1,0,8,0};
    constexpr SudokuIndex ConflictPreset[]    {1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,0,0,0};

    constexpr SudokuIndex NoBacktrackResult[] {8,1,5,3,4,9,2,6,7, 7,2,4,6,5,1,8,9,3, 3,9,6,8,7,2,4,1,5, 1,4,7,5,2,8,6,3,9, 5,8,2,9,3,6,7,4,1 ,9,6,3,4,1,7,5,8,2, 2,3,1,7,8,4,9,5,6, 6,7,8,1,9,5,3,2,4, 4,5,9,2,6,3,1,7,8};
    constexpr SudokuIndex BacktrackResult[]   {8,3,5,1,2,6,7,4,9, 4,1,6,3,9,7,2,5,8, 7,9,2,5,4,8,6,3,1, 6,4,3,9,1,2,8,7,5, 9,8,1,7,5,4,3,6,2, 5,2,7,6,8,3,1,9,4, 2,6,9,8,3,5,4,1,7, 1,7,8,4,6,9,5,2,3, 3,5,4,2,7,1,9,8,6};
    constexpr SudokuIndex ConflictResult[]    {1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,ConflictedCell,ConflictedCell,ConflictedCell};

    struct TestSet {
        const std::string& presetStr;
        const SudokuIndex* presetNum;
        const SudokuIndex* resultNum;
        const bool    result;
    };

    constexpr TestSet testSet[] {
        {NoBacktrackString, NoBacktrackPreset, NoBacktrackResult, true},
        {BacktrackString,   BacktrackPreset,   BacktrackResult,   true},
        {BacktrackString2,  BacktrackPreset,   BacktrackResult,   true},
        {ConflictString,    ConflictPreset,    ConflictResult,    false},
    };

    struct TestArgs {
        int    argc;
        const char* argv[4];
        bool   isBenchmark;   // true when it runs for benchmarking
        bool   verbose;       // true if printing steps to solving a puzzle
        int    measureCount;  // how many times it solves a puzzle
        SudokuPatternCount printAllCandidate;  // number of candicates
    };

    constexpr TestArgs testArgs[] {
        // Solving a solution. Specify how many times test cases solve.
        {1, {"sudoku", 0,      0, 0}, false, true,  1,   0},  // not specified
        {2, {"sudoku", "1",    0, 0}, true,  false, 1,   0},  // solve once
        {2, {"sudoku", "100",  0, 0}, true,  false, 100, 0},  // 100 times
        {2, {"sudoku", "-1",   0, 0}, false, true,  1,   0},  // once
        {2, {"sudoku", "-200", 0, 0}, false, true,  200, 0},  // once
        // Counting solutions
        {2, {"sudoku", "0", 0,    0}, false, true, 0, 0},  // none of solutions
        {3, {"sudoku", "0", "",   0}, false, true, 0, 0},  // none
        {3, {"sudoku", "0", "0",  0}, false, true, 0, 0},  // none
        {3, {"sudoku", "0", "2",  0}, false, true, 0, 2},  // 2
        {3, {"sudoku", "0", "-1", 0}, false, true, 0, 0},  // none
    };
}

using SudokuInStream = std::istringstream;
using SudokuOutStream = std::ostringstream;

// Utilities for testing
class SudokuTestCommon {
public:
    static SudokuCellCandidates ConvertToCandidate(char index);
    static SudokuCellCandidates ConvertToCandidate(SudokuIndex index);
private:
    static SudokuCellCandidates convertToCandidate(SudokuIndex index);
};

namespace SudokuTest {
    extern void SetAllCellsFilled(size_t initialRegisterNum, XmmRegisterSet& xmmRegSet);
    extern void SetAllCellsFullCandidates(size_t initialRegisterNum, XmmRegisterSet& xmmRegSet);
}

/*
Local Variables:
mode: c++
coding: utf-8-unix
tab-width: nil
c-file-style: "stroustrup"
End:
*/
