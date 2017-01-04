// SudokuLoaderクラスをテストする
// Copyright (C) 2012-2017 Zettsu Tatsuya
//
// クラス定義は下記から流用
// http://www.atmarkit.co.jp/fdotnet/cpptest/cpptest02/cpptest02_02.html

#include <cppunit/extensions/HelperMacros.h>
#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>
#include "sudoku.h"
#include "sudokutest.h"

class SudokuCheckerTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(SudokuCheckerTest);
    CPPUNIT_TEST(test_initializeGrid);
    CPPUNIT_TEST(test_parse);
    CPPUNIT_TEST(test_parseRow);
    CPPUNIT_TEST(test_compare);
    CPPUNIT_TEST(test_checkRowSet);
    CPPUNIT_TEST(test_checkColumnSet);
    CPPUNIT_TEST(test_checkBoxSet);
    CPPUNIT_TEST(test_checkUnique);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;
protected:
    void test_initializeGrid();
    void test_parse();
    void test_parseRow();
    void test_compare();
    void test_checkRowSet();
    void test_checkColumnSet();
    void test_checkBoxSet();
    void test_checkUnique();

    SudokuChecker::Grid grid_;
    static const SudokuSolverPrint solutionPrint_ = SudokuSolverPrint::DO_NOT_PRINT;
    static const std::string puzzle_;
    static const std::string solutionOneLine_;
    static const std::string solutionLineSet_;
    static const std::string solutionMissingLine_;
    static const std::string solutionMultiCandidates_;
    static const std::string solutionOverwritten_;
    static const std::string solutionWrong_;
};

const std::string SudokuCheckerTest::puzzle_          = "123456780456789120789123450295874630314265890678391240531642970862937510947518360";
const std::string SudokuCheckerTest::solutionOneLine_ = "123456789456789123789123456295874631314265897678391245531642978862937514947518362";
const std::string SudokuCheckerTest::solutionLineSet_         = "1:2:3:4:5:6:7:8:9\n4:5:6:7:8:9:1:2:3\n7:8:9:1:2:3:4:5:6\n2:9:5:8:7:4:6:3:1\n3:1:4:2:6:5:8:9:7\n6:7:8:3:9:1:2:4:5\n5:3:1:6:4:2:9:7:8\n8:6:2:9:3:7:5:1:4\n9:4:7:5:1:8:3:6:2\n";
const std::string SudokuCheckerTest::solutionMissingLine_     = "1:2:3:4:5:6:7:8:9\n4:5:6:7:8:9:1:2:3\n7:8:9:1:2:3:4:5:6\n2:9:5:8:7:4:6:3:1\n3:1:4:2:6:5:8:9:7\n6:7:8:3:9:1:2:4:5\n5:3:1:6:4:2:9:7:8\n8:6:2:9:3:7:5:1:4\n";
const std::string SudokuCheckerTest::solutionMultiCandidates_ = "1:234:4:5:6:7:8:9\n4:5:6:7:8:9:1:2:3\n7:8:9:1:2:3:4:5:6\n2:9:5:8:7:4:6:3:1\n3:1:4:2:6:5:8:9:7\n6:7:8:3:9:1:2:4:5\n5:3:1:6:4:2:9:7:8\n8:6:2:9:3:7:5:1:4\n9:4:7:5:1:8:3:6:2\n";
const std::string SudokuCheckerTest::solutionOverwritten_     = "1:2:3:4:5:6:7:8:9\n4:5:6:7:8:9:1:2:3\n7:8:9:1:2:3:4:5:6\n2:9:5:8:7:4:6:3:1\n3:1:4:2:6:5:8:9:7\n6:7:8:3:9:1:2:4:5\n5:3:1:6:4:2:9:7:8\n8:6:2:9:3:7:5:1:4\n9:4:7:5:1:8:3:7:2\n";
const std::string SudokuCheckerTest::solutionWrong_           = "1:2:3:4:5:6:7:8:9\n4:5:6:7:8:9:1:2:3\n7:8:9:1:2:3:4:5:6\n2:9:5:8:7:4:6:3:1\n3:1:4:2:6:5:8:9:7\n6:7:8:3:9:1:2:4:5\n5:3:1:6:4:2:9:7:8\n8:6:2:9:3:7:5:1:4\n9:4:7:5:1:8:3:6:7\n";

CPPUNIT_TEST_SUITE_REGISTRATION(SudokuCheckerTest);

void SudokuCheckerTest::setUp() {
    SudokuChecker::Group row1 {1,2,3,4,5,6,7,8,9};
    SudokuChecker::Group row2 {4,5,6,7,8,9,1,2,3};
    SudokuChecker::Group row3 {7,8,9,1,2,3,4,5,6};
    SudokuChecker::Group row4 {2,9,5,8,7,4,6,3,1};
    SudokuChecker::Group row5 {3,1,4,2,6,5,8,9,7};
    SudokuChecker::Group row6 {6,7,8,3,9,1,2,4,5};
    SudokuChecker::Group row7 {5,3,1,6,4,2,9,7,8};
    SudokuChecker::Group row8 {8,6,2,9,3,7,5,1,4};
    SudokuChecker::Group row9 {9,4,7,5,1,8,3,6,2};
    SudokuChecker::Grid grid {row1, row2, row3, row4, row5, row6, row7, row8, row9};
    grid_ = grid;
    return;
}

void SudokuCheckerTest::tearDown() {
    return;
}

void SudokuCheckerTest::test_initializeGrid() {
    SudokuChecker::Group group {{0}};
    for(auto e : group) {
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(e)>(0), e);
    }
}

void SudokuCheckerTest::test_parse() {
    constexpr SudokuSolverPrint printSet[] = {SudokuSolverPrint::DO_NOT_PRINT, SudokuSolverPrint::PRINT};

    for(auto solutionPrint : printSet) {
        SudokuOutStream os;
        SudokuChecker checker(puzzle_, solutionLineSet_, solutionPrint, &os);
        CPPUNIT_ASSERT_EQUAL(true, checker.Valid());

        const std::string expected = (solutionPrint == SudokuSolverPrint::DO_NOT_PRINT) ? "" : (solutionOneLine_ + "\n");
        CPPUNIT_ASSERT_EQUAL(expected, os.str());

        SudokuChecker checkerNull(puzzle_, solutionLineSet_, solutionPrint, nullptr);
        CPPUNIT_ASSERT_EQUAL(true, checkerNull.Valid());

        auto len = solutionLineSet_.size();
        for(decltype(len) i = 0; i < len; ++i) {
            auto solution = solutionLineSet_;
            auto c = solution.at(i);
            if (!::isdigit(c)) {
                continue;
            }

            solution.at(i) = (c == solution.at(0)) ? solution.at(2) : solution.at(0);
            SudokuOutStream os;
            SudokuChecker checker(puzzle_, solution, solutionPrint, &os);
            CPPUNIT_ASSERT_EQUAL(false, checker.Valid());
            CPPUNIT_ASSERT_EQUAL(false, os.str().empty());

            SudokuChecker checkerNull(puzzle_, solution, solutionPrint, nullptr);
            CPPUNIT_ASSERT_EQUAL(false, checkerNull.Valid());
        }
    }

    struct Test {
        std::string line;
        std::string expected;
    };

    const Test testSet[] = {
        {solutionMissingLine_, "Invalid cell arrangement\n"},
        {solutionMultiCandidates_, "Invalid cell arrangement\n"},
        {solutionOverwritten_, "Cell 79 overwritten\n"},
        {solutionWrong_, "Error in row 9\n"}
    };

    for(auto& test : testSet) {
        SudokuOutStream os;
        SudokuChecker checker(puzzle_, test.line, solutionPrint_, &os);
        CPPUNIT_ASSERT_EQUAL(false, checker.Valid());
        CPPUNIT_ASSERT_EQUAL(test.expected, os.str());
    }
}

void SudokuCheckerTest::test_parseRow() {
    for(SudokuIndex row = 0; row < Sudoku::SizeOfCellsPerGroup; ++row) {
        std::string rowLine = "9:8:7:6:5:4:3:2:1";
        switch (row % 4) {
        case 1:
            rowLine += ":";
            break;
        case 2:
            rowLine += "\n";
            break;
        case 3:
            rowLine += ":\n";
            break;
        default:
            break;
        }

        SudokuChecker::Grid grid = grid_;
        std::string solutionLine;
        std::string expected = "987654321";
        SudokuChecker checkerParseRow("", "", solutionPrint_, nullptr);

        CPPUNIT_ASSERT_EQUAL(true, checkerParseRow.parseRow(row, rowLine, grid, solutionLine));
        CPPUNIT_ASSERT_EQUAL(expected, solutionLine);

        for(SudokuIndex column = 0; column < Sudoku::SizeOfCellsPerGroup; ++column) {
            SudokuNumber num = Sudoku::SizeOfCellsPerGroup - column;
            CPPUNIT_ASSERT(grid.size() > row);
            CPPUNIT_ASSERT_EQUAL(num, grid.at(row).at(column));
        }
    }

    std::vector<std::string> rowLineSet = {"", "1:2:3:4:5:6:7:89\n", "1:2:3:4:5:6:7:8:89",
                                           "1:2:3:4:5:6:7:789", "1:::3:4:5:6:7:8:9\n"};
    for(auto rowLine : rowLineSet) {
        SudokuChecker checkerParseRow("", "", solutionPrint_, nullptr);
        SudokuChecker::Grid grid = grid_;
        std::string solutionLine;
        CPPUNIT_ASSERT_EQUAL(false, checkerParseRow.parseRow(0, rowLine, grid, solutionLine));

        SudokuChecker checker(puzzle_, rowLine, solutionPrint_, nullptr);
        CPPUNIT_ASSERT_EQUAL(false, checker.Valid());
    }
}

void SudokuCheckerTest::test_compare() {
    SudokuOutStream os;
    SudokuChecker checker("", "", solutionPrint_, nullptr);
    CPPUNIT_ASSERT_EQUAL(true, checker.compare(puzzle_, solutionOneLine_, &os));
    CPPUNIT_ASSERT_EQUAL(true, os.str().empty());

    std::string solution = solutionOneLine_ + "\n";
    CPPUNIT_ASSERT_EQUAL(true, checker.compare(puzzle_, solution, &os));
    CPPUNIT_ASSERT_EQUAL(true, os.str().empty());

    std::string puzzle = puzzle_;
    while(!puzzle.empty()) {
        puzzle.resize(puzzle.size() - 1);
        CPPUNIT_ASSERT_EQUAL(true, checker.compare(puzzle, solutionOneLine_, &os));
        CPPUNIT_ASSERT_EQUAL(true, os.str().empty());
    }

    {
        SudokuOutStream os;
        SudokuChecker checker("", "", solutionPrint_, nullptr);
        solution = solutionOneLine_;
        std::string expected = "Invalid solution size\n";
        solution.resize(solution.size() - 1);
        CPPUNIT_ASSERT_EQUAL(false, checker.compare(puzzle_, solution, &os));
        CPPUNIT_ASSERT_EQUAL(expected, os.str());
    }

    {
        SudokuOutStream os;
        SudokuChecker checker("", "", solutionPrint_, nullptr);
        solution = solutionOneLine_;
        solution.at(solution.size() - 1) = solution.at(solution.size() - 2);
        CPPUNIT_ASSERT_EQUAL(false, checker.compare(solutionOneLine_, solution, &os));
        std::string expected = "Cell 80 overwritten\n";
        CPPUNIT_ASSERT_EQUAL(expected, os.str());
    }

    auto length = puzzle_.size();
    for(decltype(length) i = 0; i < puzzle_.size(); ++i) {
        char c = puzzle_.at(i);
        if (c == '0') {
            continue;
        }

        SudokuOutStream os;
        SudokuChecker checker("", "", solutionPrint_, nullptr);
        std::string solution = solutionOneLine_;
        solution.at(i) = (c == '1') ? '2' : '1';

        CPPUNIT_ASSERT_EQUAL(false, checker.compare(puzzle_, solution, &os));
        SudokuOutStream expected;
        expected << "Cell " << i << " overwritten\n";
        CPPUNIT_ASSERT_EQUAL(expected.str(), os.str());
    }
}

void SudokuCheckerTest::test_checkRowSet() {
    SudokuOutStream os;
    SudokuChecker checker("", "", solutionPrint_, nullptr);
    CPPUNIT_ASSERT_EQUAL(true, checker.checkRowSet(grid_, &os));
    CPPUNIT_ASSERT_EQUAL(true, os.str().empty());
    CPPUNIT_ASSERT_EQUAL(true, checker.checkRowSet(grid_, nullptr));

    CPPUNIT_ASSERT_EQUAL(true, checker.check(grid_, &os));
    CPPUNIT_ASSERT_EQUAL(true, os.str().empty());
    CPPUNIT_ASSERT_EQUAL(true, checker.check(grid_, nullptr));

    for(SudokuIndex row = 0; row < Sudoku::SizeOfGroupsPerMap; ++row) {
        for(SudokuIndex column = 0; column < Sudoku::SizeOfCellsPerGroup; ++column) {
            SudokuChecker::Grid grid = grid_;
            auto fromColumn = ((column + 1) == Sudoku::SizeOfCellsPerGroup) ? (column - 1) : (column + 1);
            CPPUNIT_ASSERT(grid.size() > row);
            CPPUNIT_ASSERT(grid.at(0).size() > column);
            grid.at(row).at(column) = grid.at(row).at(fromColumn);

            std::string expected = "Error in row ";
            expected += '1' + row;
            expected += "\n";
            {
                SudokuOutStream os;
                SudokuChecker checker("", "", solutionPrint_, nullptr);
                CPPUNIT_ASSERT_EQUAL(false, checker.checkRowSet(grid, &os));
                CPPUNIT_ASSERT_EQUAL(expected, os.str());
            }
            CPPUNIT_ASSERT_EQUAL(false, checker.checkRowSet(grid, nullptr));
            CPPUNIT_ASSERT_EQUAL(false, checker.check(grid, nullptr));
            {
                SudokuOutStream os;
                SudokuChecker checker("", "", solutionPrint_, nullptr);
                CPPUNIT_ASSERT_EQUAL(false, checker.check(grid, &os));
                CPPUNIT_ASSERT_EQUAL(expected, os.str());
            }
        }
    }
}

void SudokuCheckerTest::test_checkColumnSet() {
    SudokuOutStream os;
    SudokuChecker checker("", "", solutionPrint_, nullptr);
    CPPUNIT_ASSERT_EQUAL(true, checker.checkColumnSet(grid_, &os));
    CPPUNIT_ASSERT_EQUAL(true, os.str().empty());
    CPPUNIT_ASSERT_EQUAL(true, checker.checkColumnSet(grid_, nullptr));

    for(SudokuIndex column = 0; column < Sudoku::SizeOfGroupsPerMap; ++column) {
        for(SudokuIndex row = 0; row < Sudoku::SizeOfCellsPerGroup; ++row) {
            SudokuChecker::Grid grid = grid_;
            auto fromRow = ((row + 1) == Sudoku::SizeOfCellsPerGroup) ? (row - 1) : (row + 1);
            CPPUNIT_ASSERT(grid.size() > row);
            CPPUNIT_ASSERT(grid.at(0).size() > column);
            grid.at(row).at(column) = grid.at(fromRow).at(column);
            {
                std::string expected = "Error in column ";
                expected += '1' + column;
                expected += "\n";
                SudokuOutStream os;
                SudokuChecker checker("", "", solutionPrint_, nullptr);
                CPPUNIT_ASSERT_EQUAL(false, checker.checkColumnSet(grid, &os));
                CPPUNIT_ASSERT_EQUAL(expected, os.str());
            }
            CPPUNIT_ASSERT_EQUAL(false, checker.checkColumnSet(grid, nullptr));
            {
                std::string expected = "Error in row ";
                expected += '1' + row;
                expected += "\n";
                SudokuOutStream os;
                SudokuChecker checker("", "", solutionPrint_, nullptr);
                CPPUNIT_ASSERT_EQUAL(false, checker.check(grid, &os));
                CPPUNIT_ASSERT_EQUAL(expected, os.str());
            }
        }
    }
}

void SudokuCheckerTest::test_checkBoxSet() {
    SudokuOutStream os;
    SudokuChecker checker("", "", solutionPrint_, nullptr);
    CPPUNIT_ASSERT_EQUAL(true, checker.checkBoxSet(grid_, &os));
    CPPUNIT_ASSERT_EQUAL(true, os.str().empty());
    CPPUNIT_ASSERT_EQUAL(true, checker.checkBoxSet(grid_, nullptr));

    for(SudokuIndex row = 0; row < Sudoku::SizeOfCellsPerGroup; ++row) {
        for(SudokuIndex column = 0; column < Sudoku::SizeOfCellsPerGroup; ++column) {
            SudokuChecker::Grid grid = grid_;
            auto fromColumn = ((column % Sudoku::SizeOfCellsOnBoxEdge) == 0) ?
                (column + Sudoku::SizeOfCellsOnBoxEdge - 1) : (column - 1);
            auto fromRow = ((row % Sudoku::SizeOfCellsOnBoxEdge) == 0) ?
                (row + Sudoku::SizeOfCellsOnBoxEdge - 1) : (row - 1);

            CPPUNIT_ASSERT(grid.size() > row);
            CPPUNIT_ASSERT(grid.at(0).size() > column);
            grid.at(row).at(column) = grid.at(fromRow).at(fromColumn);
            {
                std::string expected = "Error in a box with row ";
                expected += '1' + (row / Sudoku::SizeOfCellsOnBoxEdge) * Sudoku::SizeOfCellsOnBoxEdge;
                expected += " column ";
                expected += '1' + (column / Sudoku::SizeOfCellsOnBoxEdge) * Sudoku::SizeOfCellsOnBoxEdge;
                expected += "\n";
                SudokuOutStream os;
                SudokuChecker checker("", "", solutionPrint_, nullptr);
                CPPUNIT_ASSERT_EQUAL(false, checker.checkBoxSet(grid, &os));
                CPPUNIT_ASSERT_EQUAL(expected, os.str());
            }
            CPPUNIT_ASSERT_EQUAL(false, checker.checkBoxSet(grid, nullptr));
            {
                std::string expected = "Error in row ";
                expected += '1' + row;
                expected += "\n";
                SudokuOutStream os;
                SudokuChecker checker("", "", solutionPrint_, nullptr);
                CPPUNIT_ASSERT_EQUAL(false, checker.check(grid, &os));
                CPPUNIT_ASSERT_EQUAL(expected, os.str());
            }
        }
    }
}

void SudokuCheckerTest::test_checkUnique() {
    struct Test {
        SudokuChecker::Group line;
        bool expected;
    };

    const Test testSet[] = {
        {{1, 2, 3, 4, 5, 6, 7, 8, 9}, true},
        {{9, 8, 7, 6, 5, 4, 3, 2, 1}, true},
        {{1, 9, 2, 8, 3, 7, 4, 6, 5}, true},
        {{1, 2, 3, 4, 5, 6, 7, 8, 1}, false},
        {{1, 9, 2, 8, 3, 7, 4, 1, 9}, false},
        {{1, 2, 3, 4, 5, 6, 7, 8}, false}
    };

    for(const auto& test : testSet) {
        SudokuChecker checker("", "", solutionPrint_, nullptr);
        CPPUNIT_ASSERT_EQUAL(test.expected, checker.checkUnique(test.line));
    }
}

class SudokuDispatcherTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(SudokuDispatcherTest);
    CPPUNIT_TEST(test_Constructor);
    CPPUNIT_TEST(test_Exec);
    CPPUNIT_TEST(test_exec);
    CPPUNIT_TEST(test_execError);
    CPPUNIT_TEST(test_GetMessage);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;
protected:
    void test_Constructor();
    void test_Exec();
    void test_exec();
    void test_execError();
    void test_GetMessage();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SudokuDispatcherTest);

namespace {
    void CheckDispatcherErrorMessage(bool printLineInError, bool failed, SudokuSolverPrint print,
                                     const std::string& message, const std::string& caseNumberStr) {
        std::string errorMessage = "Error in case ";
        errorMessage += caseNumberStr;
        auto resultPos = message.find(errorMessage);
        auto solutionPos = message.find("123456789123");

        if (failed) {
            CPPUNIT_ASSERT(std::string::npos != resultPos);
        } else {
            CPPUNIT_ASSERT_EQUAL(std::string::npos, resultPos);
        }

        if ((failed && printLineInError) || (print == SudokuSolverPrint::PRINT)) {
            CPPUNIT_ASSERT(std::string::npos != solutionPos);
        } else {
            CPPUNIT_ASSERT_EQUAL(std::string::npos, solutionPos);
        }

        return;
    }
}

void SudokuDispatcherTest::setUp() {
    return;
}

void SudokuDispatcherTest::tearDown() {
    return;
}

void SudokuDispatcherTest::test_Constructor() {
    struct Test {
        SudokuSolverType solverType;
        SudokuSolverCheck check;
        SudokuSolverPrint print;
        SudokuPatternCount printAllCandidate;
        SudokuPuzzleCount puzzleNum;
        std::string puzzleLine;
    };

    const Test testSet[] = {
        {SudokuSolverType::SOLVER_GENERAL, SudokuSolverCheck::CHECK,
         SudokuSolverPrint::DO_NOT_PRINT, 1, 30, "12345678"},
        {SudokuSolverType::SOLVER_SSE_4_2, SudokuSolverCheck::DO_NOT_CHECK,
         SudokuSolverPrint::PRINT, 2, 40, "98765432"}
    };

    for(const auto& test : testSet) {
        SudokuDispatcher inst(test.solverType, test.check, test.print, test.printAllCandidate,
                              test.puzzleNum, test.puzzleLine);
        CPPUNIT_ASSERT(test.solverType == inst.solverType_);
        CPPUNIT_ASSERT(test.check == inst.check_);
        CPPUNIT_ASSERT(test.print == inst.print_);
        CPPUNIT_ASSERT_EQUAL(test.printAllCandidate, inst.printAllCandidate_);
        CPPUNIT_ASSERT_EQUAL(test.puzzleNum, inst.puzzleNum_);
        CPPUNIT_ASSERT_EQUAL(test.puzzleLine, inst.puzzleLine_);
    }

    return;
}

void SudokuDispatcherTest::test_Exec() {
    struct Test {
        SudokuSolverCheck check;
        SudokuSolverPrint print;
        SudokuPatternCount printAllCandidate;
        SudokuPuzzleCount puzzleNum;
    };

    constexpr Test testSet[] = {
        {SudokuSolverCheck::CHECK, SudokuSolverPrint::DO_NOT_PRINT, 0, 1},
        {SudokuSolverCheck::CHECK, SudokuSolverPrint::PRINT, 0, 1},
        {SudokuSolverCheck::DO_NOT_CHECK, SudokuSolverPrint::DO_NOT_PRINT, 0, 1},
        {SudokuSolverCheck::DO_NOT_CHECK, SudokuSolverPrint::DO_NOT_PRINT, 1, 1}
    };

    constexpr SudokuSolverType solverTypeSet[] = {SudokuSolverType::SOLVER_GENERAL, SudokuSolverType::SOLVER_SSE_4_2};
    for(const auto& test : testSet) {
        for(const auto solverType : solverTypeSet) {
            SudokuDispatcher inst(solverType, test.check, test.print, test.printAllCandidate, test.puzzleNum,
                                  SudokuTestPattern::NoBacktrackString);
            CPPUNIT_ASSERT(!inst.Exec());

            if (test.print == SudokuSolverPrint::PRINT) {
                std::string expected = SudokuTestPattern::NoBacktrackStringSolution;
                expected += "\n";
                CPPUNIT_ASSERT_EQUAL(expected, inst.GetMessage());
            } else {
                CPPUNIT_ASSERT(inst.GetMessage().empty());
            }
        }
    }

    return;
}

void SudokuDispatcherTest::test_exec() {
    struct Test {
        SudokuSolverCheck check;
        SudokuSolverPrint print;
        std::string expected;
    };

    const Test testSet[] = {
        {SudokuSolverCheck::DO_NOT_CHECK, SudokuSolverPrint::DO_NOT_PRINT, ""},
        {SudokuSolverCheck::CHECK, SudokuSolverPrint::DO_NOT_PRINT, ""},
        {SudokuSolverCheck::CHECK, SudokuSolverPrint::PRINT, SudokuTestPattern::NoBacktrackStringSolution}
    };

    for(const auto& test : testSet) {
        SudokuDispatcher inst(SudokuSolverType::SOLVER_GENERAL, test.check, test.print, 0, 0, "");
        std::ostringstream ss;
        SudokuSolver solver(SudokuTestPattern::NoBacktrackString, 0, &ss, 0);

        CPPUNIT_ASSERT(!inst.exec(solver, ss));
        std::string expected = test.expected;
        if (!expected.empty()) {
            expected += "\n";
            CPPUNIT_ASSERT_EQUAL(expected, inst.GetMessage());
        } else {
            CPPUNIT_ASSERT(inst.GetMessage().empty());
        }
    }

    return;
}

void SudokuDispatcherTest::test_execError() {
    struct TestConflict {
        SudokuSolverCheck check;
        SudokuSolverPrint print;
        bool expected;
    };

    constexpr TestConflict testSetConflict[] = {
        {SudokuSolverCheck::DO_NOT_CHECK, SudokuSolverPrint::DO_NOT_PRINT, false},
        {SudokuSolverCheck::CHECK, SudokuSolverPrint::DO_NOT_PRINT, true},
        {SudokuSolverCheck::CHECK, SudokuSolverPrint::PRINT, true}
    };

    for(const auto& test : testSetConflict) {
        const std::string puzzleLine = "Puzzle Line";
        SudokuDispatcher inst(SudokuSolverType::SOLVER_GENERAL, test.check, test.print, 0, 12, puzzleLine);
        std::ostringstream ss;

        SudokuSolver solver(SudokuTestPattern::ConflictString, 0, &ss, 0);
        CPPUNIT_ASSERT_EQUAL(test.expected, inst.exec(solver, ss));

        auto message = inst.GetMessage();
        auto pos = message.find(puzzleLine);
        CPPUNIT_ASSERT(!test.expected || (std::string::npos != pos));
        CheckDispatcherErrorMessage(false, test.expected, test.print, message, "12");
    }

    return;
}

void SudokuDispatcherTest::test_GetMessage() {
    SudokuDispatcher inst(SudokuSolverType::SOLVER_GENERAL, SudokuSolverCheck::CHECK,
                          SudokuSolverPrint::DO_NOT_PRINT, 0, 0, "");
    const std::string expected = "Message string";
    inst.message_ = expected;
    CPPUNIT_ASSERT_EQUAL(expected, inst.GetMessage());
    return;
}

class SudokuMultiDispatcherTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(SudokuMultiDispatcherTest);
    CPPUNIT_TEST(test_Constructor);
    CPPUNIT_TEST(test_AddPuzzle);
    CPPUNIT_TEST(test_ExecAll);
    CPPUNIT_TEST(test_GetMessage);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;
protected:
    void test_Constructor();
    void test_AddPuzzle();
    void test_ExecAll();
    void test_GetMessage();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SudokuMultiDispatcherTest);

void SudokuMultiDispatcherTest::setUp() {
    return;
}

void SudokuMultiDispatcherTest::tearDown() {
    return;
}

void SudokuMultiDispatcherTest::test_Constructor() {
    struct Test {
        SudokuSolverType solverType;
        SudokuSolverCheck check;
        SudokuSolverPrint print;
        SudokuPatternCount printAllCandidate;
    };

    constexpr Test testSet[] = {
        {SudokuSolverType::SOLVER_GENERAL, SudokuSolverCheck::CHECK, SudokuSolverPrint::DO_NOT_PRINT, 1},
        {SudokuSolverType::SOLVER_SSE_4_2, SudokuSolverCheck::DO_NOT_CHECK, SudokuSolverPrint::PRINT, 2}
    };

    for(const auto& test : testSet) {
        SudokuMultiDispatcher inst(test.solverType, test.check, test.print, test.printAllCandidate);
        CPPUNIT_ASSERT(test.solverType == inst.solverType_);
        CPPUNIT_ASSERT(test.check == inst.check_);
        CPPUNIT_ASSERT(test.print == inst.print_);
        CPPUNIT_ASSERT_EQUAL(test.printAllCandidate, inst.printAllCandidate_);
    }

    return;
}

void SudokuMultiDispatcherTest::test_AddPuzzle() {
    SudokuPuzzleCount puzzleNum = 21;
    const std::string puzzleLine = "123456789";
    SudokuMultiDispatcher inst(SudokuSolverType::SOLVER_GENERAL, SudokuSolverCheck::DO_NOT_CHECK,
                               SudokuSolverPrint::DO_NOT_PRINT, 0);

    inst.AddPuzzle(puzzleNum, puzzleLine);
    CPPUNIT_ASSERT(!inst.dipatcherSet_.empty());
    CPPUNIT_ASSERT_EQUAL(puzzleNum, inst.dipatcherSet_.at(0).puzzleNum_);
    CPPUNIT_ASSERT_EQUAL(puzzleLine, inst.dipatcherSet_.at(0).puzzleLine_);
    return;
}

void SudokuMultiDispatcherTest::test_ExecAll() {
    {
        SudokuMultiDispatcher inst(SudokuSolverType::SOLVER_GENERAL, SudokuSolverCheck::CHECK,
                                   SudokuSolverPrint::DO_NOT_PRINT, 0);
        CPPUNIT_ASSERT(!inst.ExecAll());
    }
    {
        SudokuMultiDispatcher inst(SudokuSolverType::SOLVER_GENERAL, SudokuSolverCheck::CHECK,
                                   SudokuSolverPrint::DO_NOT_PRINT, 0);
        inst.AddPuzzle(1, SudokuTestPattern::NoBacktrackString);
        CPPUNIT_ASSERT(!inst.ExecAll());
        CPPUNIT_ASSERT(inst.GetMessage(0).empty());
    }
    {
        SudokuMultiDispatcher inst(SudokuSolverType::SOLVER_GENERAL, SudokuSolverCheck::CHECK,
                                   SudokuSolverPrint::DO_NOT_PRINT, 0);
        inst.AddPuzzle(1, SudokuTestPattern::NoBacktrackString);
        inst.AddPuzzle(2, SudokuTestPattern::ConflictString);
        CPPUNIT_ASSERT(inst.ExecAll());
        CPPUNIT_ASSERT(inst.GetMessage(0).empty());
        CPPUNIT_ASSERT(!inst.GetMessage(1).empty());
    }
    return;
}

void SudokuMultiDispatcherTest::test_GetMessage() {
    struct Test {
        SudokuSolverCheck check;
        SudokuSolverPrint print;
        bool failed;
    };

    constexpr Test testSet[] = {
        {SudokuSolverCheck::DO_NOT_CHECK, SudokuSolverPrint::DO_NOT_PRINT, false},
        {SudokuSolverCheck::CHECK, SudokuSolverPrint::DO_NOT_PRINT, true},
        {SudokuSolverCheck::CHECK, SudokuSolverPrint::PRINT, true}
    };

    for(const auto& test : testSet) {
        SudokuMultiDispatcher inst(SudokuSolverType::SOLVER_GENERAL, test.check, test.print, 0);
        inst.AddPuzzle(23, SudokuTestPattern::NoBacktrackString);
        CPPUNIT_ASSERT(!inst.ExecAll());

        std::string expected = SudokuTestPattern::NoBacktrackStringSolution;
        expected += "\n";
        auto message = inst.GetMessage(0);
        auto pos = message.find(expected);

        if (test.print == SudokuSolverPrint::PRINT) {
            CPPUNIT_ASSERT(std::string::npos != pos);
        } else {
            CPPUNIT_ASSERT(message.empty());
        }
    }

    for(const auto& test : testSet) {
        SudokuMultiDispatcher inst(SudokuSolverType::SOLVER_GENERAL, test.check, test.print, 0);
        inst.AddPuzzle(23, SudokuTestPattern::ConflictString);
        CPPUNIT_ASSERT_EQUAL(test.failed, inst.ExecAll());
        CheckDispatcherErrorMessage(true, test.failed, test.print, inst.GetMessage(0), "23");
    }

    return;
}

class SudokuLoaderTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(SudokuLoaderTest);
    CPPUNIT_TEST(test_Constructor);
    CPPUNIT_TEST(test_CanLaunch);
    CPPUNIT_TEST(test_setSingleMode);
    CPPUNIT_TEST(test_setMultiMode);
    CPPUNIT_TEST(test_setNumberOfThreads);
    CPPUNIT_TEST(test_getMeasureCount);
    CPPUNIT_TEST(test_execSingle);
    CPPUNIT_TEST(test_execMultiPassedCpp);
    CPPUNIT_TEST(test_execMultiPassedSse);
    CPPUNIT_TEST(test_execMultiFailed);
    CPPUNIT_TEST(test_printHeader);
    CPPUNIT_TEST(test_readLines);
    CPPUNIT_TEST(test_execAll);
    CPPUNIT_TEST(test_writeMessage);
    CPPUNIT_TEST(test_solveSudoku);
    CPPUNIT_TEST(test_enumerateSudoku);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;
protected:
    void test_Constructor();
    void test_setSingleMode();
    void test_setMultiMode();
    void test_setNumberOfThreads();
    void test_CanLaunch();
    void test_getMeasureCount();
    void test_execSingle();
    void test_execMultiPassedCpp();
    void test_execMultiPassedSse();
    void test_execMultiFailed();
    void test_printHeader();
    void test_readLines();
    void test_execAll();
    void test_writeMessage();
    void test_solveSudoku();
    void test_enumerateSudoku();
private:
    std::unique_ptr<SudokuInStream> createSudokuStream(const std::string& str);
    // istringstreamの代わりにistrstreamを使うと、istrstreamが正しく初期化されない
    std::unique_ptr<SudokuInStream>  pSudokuInStream_;   // 初期マップ入力元
    std::unique_ptr<SudokuOutStream> pSudokuOutStream_;  // 結果出力先
    std::unique_ptr<SudokuLoader>    pInstance_;         // インスタンス
    // コマンドライン引数
    static const int argc_ = 1;
    static const char* const argv_[2];
};

const char* const SudokuLoaderTest::argv_[] {"sudoku", 0};

CPPUNIT_TEST_SUITE_REGISTRATION(SudokuLoaderTest);

std::unique_ptr<SudokuInStream> SudokuLoaderTest::createSudokuStream(const std::string& str) {
    // 一行ごとに分ける
    std::string lines;

    for(SudokuIndex lineCount = 0; lineCount<Sudoku::SizeOfGroupsPerMap; ++lineCount) {
        lines += str.substr(lineCount * Sudoku::SizeOfGroupsPerMap, Sudoku::SizeOfGroupsPerMap);
        lines += '\n';
    }

    return std::unique_ptr<SudokuInStream>(new SudokuInStream(lines.c_str()));
}

// 各テスト・ケースの実行直前に呼ばれる
void SudokuLoaderTest::setUp() {
    pSudokuInStream_ = createSudokuStream(SudokuTestPattern::NoBacktrackString);
    pSudokuOutStream_ = decltype(pSudokuOutStream_)(new SudokuOutStream());
    pInstance_ = decltype(pInstance_)(new SudokuLoader(argc_, argv_, pSudokuInStream_.get(), pSudokuOutStream_.get()));
    return;
}

// 各テスト・ケースの実行直後に呼ばれる
void SudokuLoaderTest::tearDown() {
    assert(pInstance_);
    assert(pSudokuOutStream_);
    assert(pSudokuInStream_);
    pInstance_.reset();

    assert(pSudokuOutStream_);
    assert(pSudokuInStream_);
    pSudokuOutStream_.reset();
    pSudokuInStream_.reset();
    return;
}

void SudokuLoaderTest::test_Constructor() {
    SudokuLoader inst(0, nullptr, nullptr, nullptr);
    CPPUNIT_ASSERT(inst.solverType_ == SudokuSolverType::SOLVER_GENERAL);
    CPPUNIT_ASSERT(inst.check_ == SudokuSolverCheck::CHECK);
    CPPUNIT_ASSERT(inst.isBenchmark_ == false);
    CPPUNIT_ASSERT(inst.verbose_ == true);
    CPPUNIT_ASSERT(inst.measureCount_ == 1);
    CPPUNIT_ASSERT(inst.printAllCandidate_ == 0);
}

// これ以降はテスト・ケースの実装内容
void SudokuLoaderTest::test_setSingleMode() {
    size_t streamIndex = 0;
    for(const auto& test : SudokuTestPattern::testArgs) {
        auto pSudokuInStream = createSudokuStream(SudokuTestPattern::testSet[streamIndex].presetStr);
        SudokuLoader inst(test.argc, test.argv, pSudokuInStream.get(), pSudokuOutStream_.get());

        CPPUNIT_ASSERT(SudokuTestPattern::testSet[streamIndex].presetStr.compare(inst.sudokuStr_) == 0);
        CPPUNIT_ASSERT_EQUAL(test.isBenchmark, inst.isBenchmark_);
        CPPUNIT_ASSERT_EQUAL(test.verbose, inst.verbose_);
        CPPUNIT_ASSERT_EQUAL(test.measureCount, inst.measureCount_);
        CPPUNIT_ASSERT_EQUAL(test.printAllCandidate, inst.printAllCandidate_);
        CPPUNIT_ASSERT(pSudokuOutStream_.get() == pInstance_->pSudokuOutStream_);
        streamIndex = (streamIndex + 1) % arraySizeof(SudokuTestPattern::testSet);
    }

    streamIndex = 0;
    for(const auto& test : SudokuTestPattern::testArgs) {
        auto pSudokuInStream = createSudokuStream(SudokuTestPattern::testSet[streamIndex].presetStr);
        SudokuLoader inst(0, nullptr, nullptr, nullptr);
        inst.setSingleMode(test.argc, test.argv, pSudokuInStream.get());

        CPPUNIT_ASSERT(SudokuTestPattern::testSet[streamIndex].presetStr.compare(inst.sudokuStr_) == 0);
        CPPUNIT_ASSERT_EQUAL(test.isBenchmark, inst.isBenchmark_);
        CPPUNIT_ASSERT_EQUAL(test.verbose, inst.verbose_);
        CPPUNIT_ASSERT_EQUAL(test.measureCount, inst.measureCount_);
        CPPUNIT_ASSERT_EQUAL(test.printAllCandidate, inst.printAllCandidate_);
        streamIndex = (streamIndex + 1) % arraySizeof(SudokuTestPattern::testSet);
    }

    // ストリームを設定しない
    auto pSudokuInStream = createSudokuStream(SudokuTestPattern::testSet[streamIndex].presetStr);
    SudokuLoader instNoOut(SudokuTestPattern::testArgs[0].argc,
                           SudokuTestPattern::testArgs[0].argv, pSudokuInStream.get(), 0);
    SudokuLoader instNoIn(SudokuTestPattern::testArgs[0].argc,
                          SudokuTestPattern::testArgs[0].argv, 0, pSudokuOutStream_.get());

    return;
}

struct SudokuTestArgsMultiMode {
    int    argc;
    const char* argv[5];
    bool   expected;
    SudokuSolverType solverType;
    SudokuSolverCheck check;
    SudokuSolverPrint print;
    SudokuLoader::NumberOfCores numberOfThreads;
};

namespace {
    constexpr SudokuTestArgsMultiMode testArgsMultiMode[] {
        // 解を一つ求める
        {1, {"sudoku", nullptr, nullptr, nullptr, nullptr},
                true, SudokuSolverType::SOLVER_GENERAL, SudokuSolverCheck::CHECK,
                    SudokuSolverPrint::DO_NOT_PRINT, 1},
        {2, {"sudoku", "../data/sudoku_example1.txt", nullptr, nullptr, nullptr},
                false, SudokuSolverType::SOLVER_GENERAL, SudokuSolverCheck::CHECK,
                    SudokuSolverPrint::DO_NOT_PRINT, 1},
        {2, {"sudoku", "FileNotExists", nullptr, nullptr, nullptr},
                true, SudokuSolverType::SOLVER_GENERAL, SudokuSolverCheck::CHECK,
                    SudokuSolverPrint::DO_NOT_PRINT, 1},
        {3, {"sudoku", "../data/sudoku_example1.txt", "INVALID", nullptr, nullptr},
                false, SudokuSolverType::SOLVER_GENERAL, SudokuSolverCheck::CHECK,
                    SudokuSolverPrint::DO_NOT_PRINT, 1},
        {3, {"sudoku", "../data/sudoku_example1.txt", "0", nullptr, nullptr},
                false, SudokuSolverType::SOLVER_GENERAL, SudokuSolverCheck::CHECK,
                    SudokuSolverPrint::DO_NOT_PRINT, 1},
        {3, {"sudoku", "../data/sudoku_example1.txt", "1", nullptr, nullptr},
                false, SudokuSolverType::SOLVER_SSE_4_2, SudokuSolverCheck::CHECK,
                    SudokuSolverPrint::DO_NOT_PRINT, 1},
        {4, {"sudoku", "../data/sudoku_example1.txt", "c++", "0", nullptr},
                false, SudokuSolverType::SOLVER_GENERAL, SudokuSolverCheck::CHECK,
                    SudokuSolverPrint::DO_NOT_PRINT, 1},
        {4, {"sudoku", "../data/sudoku_example1.txt", "c++", "1", nullptr},
                false, SudokuSolverType::SOLVER_GENERAL, SudokuSolverCheck::DO_NOT_CHECK,
                    SudokuSolverPrint::DO_NOT_PRINT, 1},
        {4, {"sudoku", "../data/sudoku_example1.txt", "sse", "0", nullptr},
                false, SudokuSolverType::SOLVER_SSE_4_2, SudokuSolverCheck::CHECK,
                    SudokuSolverPrint::DO_NOT_PRINT, 1},
        {4, {"sudoku", "../data/sudoku_example1.txt", "avx", "off", nullptr},
                false, SudokuSolverType::SOLVER_SSE_4_2, SudokuSolverCheck::DO_NOT_CHECK,
                    SudokuSolverPrint::DO_NOT_PRINT, 1},
        {4, {"sudoku", "../data/sudoku_example1.txt", "0", "2", nullptr},
                false, SudokuSolverType::SOLVER_GENERAL, SudokuSolverCheck::CHECK,
                    SudokuSolverPrint::PRINT, 1},
        {4, {"sudoku", "../data/sudoku_example1.txt", "avx", "print", nullptr},
                false, SudokuSolverType::SOLVER_SSE_4_2, SudokuSolverCheck::CHECK,
                    SudokuSolverPrint::PRINT, 1},
        {3, {"sudoku", "../data/sudoku_example1.txt", "-N3", nullptr, nullptr},
                false, SudokuSolverType::SOLVER_GENERAL, SudokuSolverCheck::CHECK,
                    SudokuSolverPrint::DO_NOT_PRINT, 3},
        {4, {"sudoku", "../data/sudoku_example1.txt", "sse", "-N3", nullptr},
                false, SudokuSolverType::SOLVER_SSE_4_2, SudokuSolverCheck::CHECK,
                    SudokuSolverPrint::DO_NOT_PRINT, 3},
        {4, {"sudoku", "../data/sudoku_example1.txt", "-N3", "sse", nullptr},
                false, SudokuSolverType::SOLVER_SSE_4_2, SudokuSolverCheck::CHECK,
                    SudokuSolverPrint::DO_NOT_PRINT, 3},
        {5, {"sudoku", "../data/sudoku_example1.txt", "avx", "print", "-N5"},
                false, SudokuSolverType::SOLVER_SSE_4_2, SudokuSolverCheck::CHECK,
                    SudokuSolverPrint::PRINT, 5},
        {5, {"sudoku", "../data/sudoku_example1.txt", "avx", "-N5", "print"},
                false, SudokuSolverType::SOLVER_SSE_4_2, SudokuSolverCheck::CHECK,
                    SudokuSolverPrint::PRINT, 5},
        {5, {"sudoku", "../data/sudoku_example1.txt", "-N5", "avx", "print"},
                false, SudokuSolverType::SOLVER_SSE_4_2, SudokuSolverCheck::CHECK,
                    SudokuSolverPrint::PRINT, 5},
        {5, {"sudoku", "../data/sudoku_example1.txt", "-N5", "-N3", "sse"},
                false, SudokuSolverType::SOLVER_SSE_4_2, SudokuSolverCheck::CHECK,
                    SudokuSolverPrint::DO_NOT_PRINT, 3},
        {5, {"sudoku", "../data/sudoku_example1.txt", "-N", "avx", "print"},
                false, SudokuSolverType::SOLVER_SSE_4_2, SudokuSolverCheck::CHECK,
                    SudokuSolverPrint::PRINT, 0},
        {5, {"sudoku", "../data/sudoku_example1.txt", "-Nx", "avx", "print"},
                false, SudokuSolverType::SOLVER_SSE_4_2, SudokuSolverCheck::CHECK,
                    SudokuSolverPrint::PRINT, 0},
        {5, {"sudoku", "../data/sudoku_example1.txt", "-N-1", "avx", "print"},
                false, SudokuSolverType::SOLVER_SSE_4_2, SudokuSolverCheck::CHECK,
                    SudokuSolverPrint::PRINT, 0},
        {4, {"sudoku", "../data/sudoku_example1.txt", "sse", " -N3", nullptr},
                false, SudokuSolverType::SOLVER_SSE_4_2, SudokuSolverCheck::CHECK,
                    SudokuSolverPrint::DO_NOT_PRINT, 1},
        {4, {"sudoku", "../data/sudoku_example1.txt", "sse", "pre-N", nullptr},
                false, SudokuSolverType::SOLVER_SSE_4_2, SudokuSolverCheck::CHECK,
                    SudokuSolverPrint::DO_NOT_PRINT, 1},
    };
}

void SudokuLoaderTest::test_setMultiMode() {
    for(const auto& test : testArgsMultiMode) {
        auto pSudokuInStream = createSudokuStream(SudokuTestPattern::BacktrackString);
        SudokuLoader inst(test.argc, test.argv, nullptr, pSudokuOutStream_.get());
        CPPUNIT_ASSERT_EQUAL(test.expected, inst.multiLineFilename_.empty());
        CPPUNIT_ASSERT_EQUAL(static_cast<int>(test.solverType), static_cast<int>(inst.solverType_));

        auto numberOfCores = Sudoku::CreateParallelRunner()->GetHardwareConcurrency();
        auto expected = (test.numberOfThreads == 0) ? numberOfCores : test.numberOfThreads;
        CPPUNIT_ASSERT_EQUAL(expected, inst.numberOfThreads_);
    }

    for(const auto& test : testArgsMultiMode) {
        auto pSudokuInStream = createSudokuStream(SudokuTestPattern::BacktrackString);
        SudokuLoader inst(0, nullptr, nullptr, nullptr);
        CPPUNIT_ASSERT_EQUAL(!test.expected, inst.setMultiMode(test.argc, test.argv));
        CPPUNIT_ASSERT_EQUAL(static_cast<int>(test.solverType), static_cast<int>(inst.solverType_));

        auto numberOfCores = Sudoku::CreateParallelRunner()->GetHardwareConcurrency();
        auto expected = (test.numberOfThreads == 0) ? numberOfCores : test.numberOfThreads;
        CPPUNIT_ASSERT_EQUAL(expected, inst.numberOfThreads_);
        if (!test.expected) {
            decltype(inst.multiLineFilename_) str = test.argv[1];
            CPPUNIT_ASSERT_EQUAL(str, inst.multiLineFilename_);
        }
    }
}

void SudokuLoaderTest::test_setNumberOfThreads() {
    struct Test {
        int argc;
        const char * const argv[3];
        int argIndex;
        bool expected;
        SudokuLoader::NumberOfCores numberOfThreads;
    };

    constexpr Test testSet[] = {
        {0, {nullptr, nullptr, nullptr},   0, false, 1},
        {1, {nullptr, nullptr, nullptr},   0, false, 1},
        {1, {"command", nullptr, nullptr}, 0, false, 1},
        {1, {"command", nullptr, nullptr}, 1, false, 1},
        {1, {"command", nullptr, nullptr}, 2, false, 1},
        {2, {"......", "file-N", nullptr}, 1, false, 1},
        {2, {".....", "file-N3", nullptr}, 1, false, 1},
        {2, {"command", "-N1", nullptr},  1, true, 1},
        {2, {"command", "-N2", nullptr},  1, true, 2},
        {2, {"command", "-N32", nullptr}, 1, true, 32},
        {2, {"command", "-N", nullptr},   1, true, 0},
        {2, {"command", "-Nx", nullptr},  1, true, 0},
        {2, {"command", "-N-1", nullptr}, 1, true, 0}
    };

    for(const auto& test : testSet) {
        SudokuLoader inst(0, nullptr, nullptr, pSudokuOutStream_.get());
        CPPUNIT_ASSERT_EQUAL(test.expected, inst.setNumberOfThreads(test.argc, test.argv, test.argIndex));

        auto numberOfCores = Sudoku::CreateParallelRunner()->GetHardwareConcurrency();
        auto expected = (test.numberOfThreads == 0) ? numberOfCores : test.numberOfThreads;
        CPPUNIT_ASSERT_EQUAL(expected, inst.numberOfThreads_);
    }
}

namespace {
    enum class SudokuLoaderExec {
        EXEC_PUBLIC,
        EXEC_PRIVATE
    };
    const SudokuLoaderExec SudokuLoaderExecSet[] = {SudokuLoaderExec::EXEC_PUBLIC, SudokuLoaderExec::EXEC_PRIVATE};
}

void SudokuLoaderTest::test_execSingle() {
    size_t streamIndex = 0;
    for(auto mode : SudokuLoaderExecSet) {
        for(const auto& test : SudokuTestPattern::testArgs) {
            if (test.measureCount == 0) {
                continue;
            }
            auto pSudokuInStream = createSudokuStream(SudokuTestPattern::testSet[streamIndex].presetStr);
            SudokuLoader inst(test.argc, test.argv, pSudokuInStream.get(), pSudokuOutStream_.get());
            const auto expected = SudokuLoader::ExitStatusPassed;
            switch(mode) {
            case SudokuLoaderExec::EXEC_PUBLIC:
                CPPUNIT_ASSERT_EQUAL(expected, inst.Exec());
                break;
            case SudokuLoaderExec::EXEC_PRIVATE:
            default:
                CPPUNIT_ASSERT_EQUAL(expected, inst.execSingle());
                break;
            }
            streamIndex = (streamIndex + 1) % arraySizeof(SudokuTestPattern::testSet);
        }
    }

    return;
}

void SudokuLoaderTest::test_execMultiPassedCpp() {
    struct Test {
        SudokuSolverCheck check;
        const char* expected;
    };

    const Test testSet[] = {
        {SudokuSolverCheck::CHECK, "Solving in C++\nAll 3 cases passed.\n"},
        {SudokuSolverCheck::DO_NOT_CHECK, "Solving in C++\nAll 3 cases solved.\n"}
    };

    for(const auto& test : testSet) {
        std::string pattern = SudokuTestPattern::NoBacktrackString;
        pattern += "\n" + SudokuTestPattern::BacktrackString;
        pattern += "\n" + SudokuTestPattern::BacktrackString2;

        SudokuInStream is(pattern);
        SudokuLoader inst(0, nullptr, nullptr, pSudokuOutStream_.get());
        inst.check_ = test.check;
        CPPUNIT_ASSERT_EQUAL(SudokuLoader::ExitStatusPassed, inst.execMulti(&is));
        CPPUNIT_ASSERT_EQUAL(SudokuLoader::ExitStatusFailed, inst.execMulti(nullptr));

        std::string expected = "Solving in C++\nAll 3 cases passed.\n";
        std::string actual = pSudokuOutStream_->str();
        actual.resize(expected.size());
        CPPUNIT_ASSERT_EQUAL(expected, actual);
    }
}

void SudokuLoaderTest::test_execMultiPassedSse() {
    std::string pattern = SudokuTestPattern::NoBacktrackString;
    pattern += "\n" + SudokuTestPattern::BacktrackString;
    SudokuInStream is(pattern);

    SudokuLoader inst(0, nullptr, nullptr, pSudokuOutStream_.get());
    inst.solverType_ = SudokuSolverType::SOLVER_SSE_4_2;
    CPPUNIT_ASSERT_EQUAL(SudokuLoader::ExitStatusPassed, inst.execMulti(&is));

    std::string expected = "Solving with SSE/AVX\nAll 2 cases passed.\n";
    std::string actual = pSudokuOutStream_->str();
    actual.resize(expected.size());
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void SudokuLoaderTest::test_execMultiFailed() {
    const std::string pattern = "111111111........................................................................";
    SudokuInStream is(pattern);
    SudokuLoader inst(0, nullptr, nullptr, pSudokuOutStream_.get());
    CPPUNIT_ASSERT_EQUAL(SudokuLoader::ExitStatusFailed, inst.execMulti(&is));

    std::string expected = "Solving in C++\nInvalid cell arrangement\n";
    std::string actual = pSudokuOutStream_->str();
    actual.resize(expected.size());
    CPPUNIT_ASSERT_EQUAL(expected, actual);
    return;
}

void SudokuLoaderTest::test_printHeader() {
    struct Test {
        SudokuSolverType solverType;
        std::string expected;
    };

    const Test testSet[] = {
        {SudokuSolverType::SOLVER_GENERAL, "C++"},
        {SudokuSolverType::SOLVER_SSE_4_2, "SSE"}
    };

    SudokuLoader inst(0, nullptr, nullptr, pSudokuOutStream_.get());
    inst.printHeader(SudokuSolverType::SOLVER_GENERAL, nullptr);
    for(const auto& test : testSet) {
        std::ostringstream os;
        inst.printHeader(test.solverType, &os);
        auto pos = os.str().find(test.expected);
        CPPUNIT_ASSERT(std::string::npos != pos);
    }

    return;
}

void SudokuLoaderTest::test_readLines() {
    constexpr SudokuPuzzleCount numberOfPuzzles = 3;

    struct Test {
        SudokuLoader::NumberOfCores numberOfCores;
        std::vector<SudokuLoader::DispatcherPtrSet::size_type> sizeSet;
        std::vector<SudokuPuzzleCount> numSet;
    };

    const Test testSet[] = {
        {1, {3, 0, 0}, {1, 2, 3}},
        {2, {2, 1, 0}, {1, 3, 0}},
        {3, {1, 1, 1}, {1, 0, 0}}
    };

    std::string pattern = SudokuTestPattern::NoBacktrackString;
    pattern += "\n" + SudokuTestPattern::BacktrackString;
    pattern += "\n" + SudokuTestPattern::BacktrackString2;

    for(const auto& test : testSet) {
        SudokuLoader::DispatcherPtrSet dispatcherSet;
        for(SudokuPuzzleCount i=0; i<numberOfPuzzles; ++i) {
            dispatcherSet.push_back(
                std::move(SudokuLoader::DispatcherPtr(
                              new SudokuMultiDispatcher(
                                  SudokuSolverType::SOLVER_GENERAL, SudokuSolverCheck::DO_NOT_CHECK,
                                  SudokuSolverPrint::DO_NOT_PRINT, 0))));
        }

        SudokuInStream is(pattern);
        SudokuLoader inst(0, nullptr, nullptr, pSudokuOutStream_.get());
        CPPUNIT_ASSERT_EQUAL(numberOfPuzzles, inst.readLines(test.numberOfCores, &is, dispatcherSet));

        for(SudokuPuzzleCount i = 0; i < numberOfPuzzles; ++i) {
            CPPUNIT_ASSERT_EQUAL(test.sizeSet.at(i), dispatcherSet.at(i)->dipatcherSet_.size());
        }

        for(SudokuPuzzleCount i = 0; i < numberOfPuzzles; ++i) {
            if (i < dispatcherSet.at(0)->dipatcherSet_.size()) {
                CPPUNIT_ASSERT_EQUAL(test.numSet.at(i), dispatcherSet.at(0)->dipatcherSet_.at(i).puzzleNum_);
            }
        }
    }

    return;
}

void SudokuLoaderTest::test_execAll() {
    const int resultSet[] = {SudokuLoader::ExitStatusPassed, SudokuLoader::ExitStatusFailed};
    for(auto result : resultSet) {
        constexpr SudokuPuzzleCount sizeOfPuzzles = 32;
        std::string pattern;
        for(SudokuPuzzleCount i=0; i<sizeOfPuzzles; ++i) {
            if (((i + 1) == sizeOfPuzzles) && (result == SudokuLoader::ExitStatusFailed)) {
                pattern += SudokuTestPattern::ConflictString;
            } else {
                pattern += SudokuTestPattern::NoBacktrackString;
            }
            pattern += "\n";
        }

        for (SudokuLoader::NumberOfCores numberOfCores=1; numberOfCores<=16; ++numberOfCores) {
            SudokuLoader::DispatcherPtrSet dispatcherSet;

            for(SudokuLoader::NumberOfCores i=0; i<numberOfCores; ++i) {
                dispatcherSet.push_back(
                    std::move(SudokuLoader::DispatcherPtr(
                                  new SudokuMultiDispatcher(
                                      SudokuSolverType::SOLVER_GENERAL, SudokuSolverCheck::CHECK,
                                      SudokuSolverPrint::DO_NOT_PRINT, 0))));
            }

            SudokuInStream is(pattern);
            SudokuLoader inst(0, nullptr, nullptr, pSudokuOutStream_.get());
            inst.readLines(numberOfCores, &is, dispatcherSet);
            CPPUNIT_ASSERT_EQUAL(result, inst.execAll(numberOfCores, dispatcherSet));
        }
    }

    return;
}

void SudokuLoaderTest::test_writeMessage() {
    SudokuLoader::DispatcherPtrSet dispatcherSet;
    constexpr SudokuLoader::NumberOfCores numberOfCores = 3;
    for(SudokuLoader::NumberOfCores i=0; i<numberOfCores; ++i) {
        dispatcherSet.push_back(
            std::move(SudokuLoader::DispatcherPtr(
                          new SudokuMultiDispatcher(
                              SudokuSolverType::SOLVER_GENERAL, SudokuSolverCheck::DO_NOT_CHECK,
                              SudokuSolverPrint::DO_NOT_PRINT, 0))));
    }

    constexpr SudokuPuzzleCount sizeOfPuzzles = 7;
    std::string pattern;
    for(SudokuPuzzleCount i=0; i<sizeOfPuzzles; ++i) {
        pattern += SudokuTestPattern::NoBacktrackString;
        pattern += "\n";
    }
    SudokuInStream is(pattern);

    SudokuLoader inst(0, nullptr, nullptr, pSudokuOutStream_.get());
    CPPUNIT_ASSERT_EQUAL(sizeOfPuzzles, inst.readLines(numberOfCores, &is, dispatcherSet));

    dispatcherSet.at(0)->dipatcherSet_.at(0).message_ = "A";
    dispatcherSet.at(0)->dipatcherSet_.at(1).message_ = "D";
    dispatcherSet.at(0)->dipatcherSet_.at(2).message_ = "G";
    dispatcherSet.at(1)->dipatcherSet_.at(0).message_ = "B";
    dispatcherSet.at(1)->dipatcherSet_.at(1).message_ = "E";
    dispatcherSet.at(2)->dipatcherSet_.at(0).message_ = "C";
    dispatcherSet.at(2)->dipatcherSet_.at(1).message_ = "F";

    std::ostringstream os;
    inst.writeMessage(0, 0, dispatcherSet, nullptr);
    inst.writeMessage(numberOfCores, sizeOfPuzzles, dispatcherSet, &os);
    std::string expected = "ABCDEFG";
    CPPUNIT_ASSERT_EQUAL(expected, os.str());
    return;
}

void SudokuLoaderTest::test_CanLaunch() {
    struct TestSet {
        int   argc;
        const char* argv[2];
        bool  expected;
    };

    constexpr TestSet testSet[] {
        {0, {0, 0}, false},
        {1, {"commandName", 0},    false},
        {2, {"commandName", "0"},  true},
        {2, {"commandName", "1"},  false},
        {2, {"commandName", "-1"}, false},
        {2, {"commandName", "a"},  true}};

    for(const auto& test : testSet) {
        const auto expected = (sudokuXmmAssumeCellsPacked) ? test.expected : true;
        CPPUNIT_ASSERT_EQUAL(expected, SudokuLoader::CanLaunch(test.argc, test.argv));
    }
    return;
}

void SudokuLoaderTest::test_getMeasureCount() {
    struct TestSet {
        const char* arg;
        int expected;
    };
    constexpr TestSet testSet[] {{"0", 0}, {"1", 1}, {"123", 123}, {"-1", -1}, {"-123", -123}};

    CPPUNIT_ASSERT_EQUAL(0, SudokuLoader::getMeasureCount(0));
    for(const auto& test : testSet) {
        CPPUNIT_ASSERT_EQUAL(test.expected, SudokuLoader::getMeasureCount(test.arg));
    }
    return;
}

void SudokuLoaderTest::test_solveSudoku() {
    size_t streamIndex = 0;
    int seed = 0;
    for(const auto& test : SudokuTestPattern::testArgs) {
        auto pSudokuInStream = createSudokuStream(SudokuTestPattern::testSet[streamIndex].presetStr);
        SudokuLoader inst(test.argc, test.argv, pSudokuInStream.get(), pSudokuOutStream_.get());
        inst.solveSudoku(SudokuSolverType::SOLVER_GENERAL, seed++, 0);
        streamIndex = (streamIndex + 1) % arraySizeof(SudokuTestPattern::testSet);
    }

    return;
}

void SudokuLoaderTest::test_enumerateSudoku() {
    constexpr int argc = 2;
    const char* argv[2] {"sudoku", "0"};
    const char* presetStrSet[] {"123456789456789123789123456295874................................................",
                                ".............................................295874...789123456456789123123456789"};

    for(const auto& test : presetStrSet) {
        if ((sudokuXmmAssumeCellsPacked != 0) && (!isdigit(*test))) {
            continue;
        }
        auto pSudokuInStream = createSudokuStream(test);
        SudokuLoader inst(argc, argv, pSudokuInStream.get(), pSudokuOutStream_.get());

        inst.enumerateSudoku();
        const auto actual = sudokuXmmAllPatternCnt;
        constexpr decltype(actual) expected = 1140000ull;
        CPPUNIT_ASSERT_EQUAL(expected, actual);
    }
    return;
}

/*
Local Variables:
mode: c++
coding: utf-8-unix
tab-width: nil
c-file-style: "stroustrup"
End:
*/
