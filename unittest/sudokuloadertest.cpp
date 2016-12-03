// SudokuLoaderクラスをテストする
// Copyright (C) 2012-2015 Zettsu Tatsuya
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
    CPPUNIT_TEST(test_parse);
    CPPUNIT_TEST(test_checkRowSet);
    CPPUNIT_TEST(test_checkColumnSet);
    CPPUNIT_TEST(test_checkBoxSet);
    CPPUNIT_TEST(test_checkUnique);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;
protected:
    void test_parse();
    void test_checkRowSet();
    void test_checkColumnSet();
    void test_checkBoxSet();
    void test_checkUnique();
    SudokuChecker::Grid grid_;
};

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

void SudokuCheckerTest::test_parse() {
    const std::string solution = "1:2:3:4:5:6:7:8:9\n4:5:6:7:8:9:1:2:3\n7:8:9:1:2:3:4:5:6\n2:9:5:8:7:4:6:3:1\n3:1:4:2:6:5:8:9:7\n6:7:8:3:9:1:2:4:5\n5:3:1:6:4:2:9:7:8\n8:6:2:9:3:7:5:1:4\n9:4:7:5:1:8:3:6:2\n";
    SudokuOutStream os;
    SudokuChecker checker(solution, &os);
    CPPUNIT_ASSERT_EQUAL(true, checker.valid());
    CPPUNIT_ASSERT_EQUAL(true, os.str().empty());

    SudokuChecker checkerNull(solution, nullptr);
    CPPUNIT_ASSERT_EQUAL(true, checkerNull.valid());

    auto len = solution.size();
    for(decltype(len) i = 0; i < len; ++i) {
        auto arg = solution;
        auto c = arg.at(i);
        if (!::isdigit(c)) {
            continue;
        }

        arg.at(i) = (c == arg.at(0)) ? arg.at(2) : arg.at(0);
        SudokuOutStream os;
        SudokuChecker checker(arg, &os);
        CPPUNIT_ASSERT_EQUAL(false, checker.valid());
        CPPUNIT_ASSERT_EQUAL(false, os.str().empty());
        SudokuChecker checkerNull(arg, nullptr);
        CPPUNIT_ASSERT_EQUAL(false, checkerNull.valid());
    }

    SudokuChecker checkerEmpty("", nullptr);
    CPPUNIT_ASSERT_EQUAL(false, checkerEmpty.valid());

    SudokuChecker checkerShortColumn("1:2:3:4:5:6:7:8:9\n4:5:6:7:8:9:1:2:3\n7:8:9:1:2:3:4:5:6\n2:9:5:8:7:4:6:3:1\n3:1:4:2:6:5:8:9:7\n6:7:8:3:9:1:2:4:5\n5:3:1:6:4:2:9:7:8\n8:6:2:9:3:7:5:1:4\n", nullptr);
    CPPUNIT_ASSERT_EQUAL(false, checkerShortColumn.valid());

    SudokuChecker checkerShortRow("1:2:3:4:5:6:7:89\n", nullptr);
    CPPUNIT_ASSERT_EQUAL(false, checkerShortRow.valid());

    SudokuChecker checkerVoid("1::3:4:5:6:7:8:9\n", nullptr);
    CPPUNIT_ASSERT_EQUAL(false, checkerVoid.valid());

    SudokuChecker checkerTooManyCandidates("1:234:4:5:6:7:8:9\n4:5:6:7:8:9:1:2:3\n7:8:9:1:2:3:4:5:6\n2:9:5:8:7:4:6:3:1\n3:1:4:2:6:5:8:9:7\n6:7:8:3:9:1:2:4:5\n5:3:1:6:4:2:9:7:8\n8:6:2:9:3:7:5:1:4\n9:4:7:5:1:8:3:6:2\n", nullptr);
    CPPUNIT_ASSERT_EQUAL(false, checkerTooManyCandidates.valid());
}

void SudokuCheckerTest::test_checkRowSet() {
    SudokuOutStream os;
    SudokuChecker checker("", &os);
    CPPUNIT_ASSERT_EQUAL(true, checker.check(grid_, &os));
    CPPUNIT_ASSERT_EQUAL(true, os.str().empty());
    CPPUNIT_ASSERT_EQUAL(true, checker.check(grid_, nullptr));

    CPPUNIT_ASSERT_EQUAL(true, checker.checkRowSet(grid_, &os));
    CPPUNIT_ASSERT_EQUAL(true, os.str().empty());
    CPPUNIT_ASSERT_EQUAL(true, checker.checkRowSet(grid_, nullptr));

    for(SudokuIndex row = 0; row < Sudoku::SizeOfGroupsPerMap; ++row) {
        for(SudokuIndex column = 0; column < Sudoku::SizeOfCellsPerGroup; ++column) {
            SudokuChecker::Grid grid = grid_;
            auto fromColumn = ((column + 1) == Sudoku::SizeOfCellsPerGroup) ? (column - 1) : (column + 1);
            grid.at(row).at(column) = grid.at(row).at(fromColumn);

            std::string expected = "Error in row ";
            expected += '1' + row;
            expected += "\n";
            {
                SudokuOutStream os;
                SudokuChecker checker("", &os);
                CPPUNIT_ASSERT_EQUAL(false, checker.checkRowSet(grid, &os));
                CPPUNIT_ASSERT_EQUAL(expected, os.str());
            }
            CPPUNIT_ASSERT_EQUAL(false, checker.checkRowSet(grid, nullptr));
            {
                SudokuOutStream os;
                SudokuChecker checker("", &os);
                CPPUNIT_ASSERT_EQUAL(false, checker.check(grid, &os));
                CPPUNIT_ASSERT_EQUAL(expected, os.str());
            }
        }
    }
}

void SudokuCheckerTest::test_checkColumnSet() {
    SudokuOutStream os;
    SudokuChecker checker("", &os);
    CPPUNIT_ASSERT_EQUAL(true, checker.checkColumnSet(grid_, &os));
    CPPUNIT_ASSERT_EQUAL(true, os.str().empty());
    CPPUNIT_ASSERT_EQUAL(true, checker.checkColumnSet(grid_, nullptr));

    for(SudokuIndex column = 0; column < Sudoku::SizeOfGroupsPerMap; ++column) {
        for(SudokuIndex row = 0; row < Sudoku::SizeOfCellsPerGroup; ++row) {
            SudokuChecker::Grid grid = grid_;
            auto fromRow = ((row + 1) == Sudoku::SizeOfCellsPerGroup) ? (row - 1) : (row + 1);
            grid.at(row).at(column) = grid.at(fromRow).at(column);
            {
                std::string expected = "Error in column ";
                expected += '1' + column;
                expected += "\n";
                SudokuOutStream os;
                SudokuChecker checker("", &os);
                CPPUNIT_ASSERT_EQUAL(false, checker.checkColumnSet(grid, &os));
                CPPUNIT_ASSERT_EQUAL(expected, os.str());
            }
            CPPUNIT_ASSERT_EQUAL(false, checker.checkColumnSet(grid, nullptr));
            {
                std::string expected = "Error in row ";
                expected += '1' + row;
                expected += "\n";
                SudokuOutStream os;
                SudokuChecker checker("", &os);
                CPPUNIT_ASSERT_EQUAL(false, checker.check(grid, &os));
                CPPUNIT_ASSERT_EQUAL(expected, os.str());
            }
        }
    }
}

void SudokuCheckerTest::test_checkBoxSet() {
    SudokuOutStream os;
    SudokuChecker checker("", &os);
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
            grid.at(row).at(column) = grid.at(fromRow).at(fromColumn);
            {
                std::string expected = "Error in a box with row ";
                expected += '1' + (row / Sudoku::SizeOfCellsOnBoxEdge) * Sudoku::SizeOfCellsOnBoxEdge;
                expected += " column ";
                expected += '1' + (column / Sudoku::SizeOfCellsOnBoxEdge) * Sudoku::SizeOfCellsOnBoxEdge;
                expected += "\n";
                SudokuOutStream os;
                SudokuChecker checker("", &os);
                CPPUNIT_ASSERT_EQUAL(false, checker.checkBoxSet(grid, &os));
                CPPUNIT_ASSERT_EQUAL(expected, os.str());
            }
            CPPUNIT_ASSERT_EQUAL(false, checker.checkBoxSet(grid, nullptr));
            {
                std::string expected = "Error in row ";
                expected += '1' + row;
                expected += "\n";
                SudokuOutStream os;
                SudokuChecker checker("", &os);
                CPPUNIT_ASSERT_EQUAL(false, checker.check(grid, &os));
                CPPUNIT_ASSERT_EQUAL(expected, os.str());
            }
        }
    }
}

void SudokuCheckerTest::test_checkUnique() {
    {
        SudokuChecker::Group line {1, 2, 3, 4, 5, 6, 7, 8, 9};
        SudokuChecker checker("", nullptr);
        CPPUNIT_ASSERT_EQUAL(true, checker.checkUnique(line));
    }
    {
        SudokuChecker::Group line {9, 8, 7, 6, 5, 4, 3, 2, 1};
        SudokuChecker checker("", nullptr);
        CPPUNIT_ASSERT_EQUAL(true, checker.checkUnique(line));
    }
    {
        SudokuChecker::Group line {1, 9, 2, 8, 3, 7, 4, 6, 5};
        SudokuChecker checker("", nullptr);
        CPPUNIT_ASSERT_EQUAL(true, checker.checkUnique(line));
    }
    {
        SudokuChecker::Group line {1, 2, 3, 4, 5, 6, 7, 8, 1};
        SudokuChecker checker("", nullptr);
        CPPUNIT_ASSERT_EQUAL(false, checker.checkUnique(line));
    }
    {
        SudokuChecker::Group line {1, 9, 2, 8, 3, 7, 4, 1, 9};
        SudokuChecker checker("", nullptr);
        CPPUNIT_ASSERT_EQUAL(false, checker.checkUnique(line));
    }
    {
        SudokuChecker::Group line {1, 2, 3, 4, 5, 6, 7, 8};
        SudokuChecker checker("", nullptr);
        CPPUNIT_ASSERT_EQUAL(false, checker.checkUnique(line));
    }
}

class SudokuLoaderTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(SudokuLoaderTest);
    CPPUNIT_TEST(test_Constructor);
    CPPUNIT_TEST(test_CanLaunch);
    CPPUNIT_TEST(test_setSingleMode);
    CPPUNIT_TEST(test_setMultiMode);
    CPPUNIT_TEST(test_getMeasureCount);
    CPPUNIT_TEST(test_execSingle);
    CPPUNIT_TEST(test_execMultiPassedCpp);
    CPPUNIT_TEST(test_execMultiPassedSse);
    CPPUNIT_TEST(test_execMultiFailed);
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
    void test_CanLaunch();
    void test_getMeasureCount();
    void test_execSingle();
    void test_execMultiPassedCpp();
    void test_execMultiPassedSse();
    void test_execMultiFailed();
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
    CPPUNIT_ASSERT(inst.printAllCadidate_ == 0);
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
        CPPUNIT_ASSERT_EQUAL(test.printAllCadidate, inst.printAllCadidate_);
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
        CPPUNIT_ASSERT_EQUAL(test.printAllCadidate, inst.printAllCadidate_);
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

namespace {
    struct TestArgsMultiMode {
        int    argc;
        const char* argv[4];
        bool   expected;
        SudokuSolverType solverType;
        SudokuSolverCheck check;
    };

    constexpr TestArgsMultiMode testArgsMultiMode[] {
        // 解を一つ求める
        {1, {"sudoku", nullptr, nullptr, nullptr},
                true, SudokuSolverType::SOLVER_GENERAL, SudokuSolverCheck::CHECK},
        {2, {"sudoku", "../data/sudoku_example1.txt", nullptr, nullptr},
                false, SudokuSolverType::SOLVER_GENERAL, SudokuSolverCheck::CHECK},
        {2, {"sudoku", "FileNotExists", nullptr, nullptr},
                true, SudokuSolverType::SOLVER_GENERAL, SudokuSolverCheck::CHECK},
        {3, {"sudoku", "../data/sudoku_example1.txt", "0", nullptr},
                false, SudokuSolverType::SOLVER_GENERAL, SudokuSolverCheck::CHECK},
        {3, {"sudoku", "../data/sudoku_example1.txt", "1", nullptr},
                false, SudokuSolverType::SOLVER_SSE_4_2, SudokuSolverCheck::CHECK},
        {4, {"sudoku", "../data/sudoku_example1.txt", "c++", "0"},
                false, SudokuSolverType::SOLVER_GENERAL, SudokuSolverCheck::CHECK},
        {4, {"sudoku", "../data/sudoku_example1.txt", "c++", "1"},
                false, SudokuSolverType::SOLVER_GENERAL, SudokuSolverCheck::DO_NOT_CHECK},
        {4, {"sudoku", "../data/sudoku_example1.txt", "sse", "0"},
                false, SudokuSolverType::SOLVER_SSE_4_2, SudokuSolverCheck::CHECK},
        {4, {"sudoku", "../data/sudoku_example1.txt", "avx", "off"},
                false, SudokuSolverType::SOLVER_SSE_4_2, SudokuSolverCheck::DO_NOT_CHECK}
    };
}

void SudokuLoaderTest::test_setMultiMode() {
    for(const auto& test : testArgsMultiMode) {
        auto pSudokuInStream = createSudokuStream(SudokuTestPattern::BacktrackString);
        SudokuLoader inst(test.argc, test.argv, nullptr, pSudokuOutStream_.get());
        CPPUNIT_ASSERT_EQUAL(test.expected, inst.multiLineFilename_.empty());
        CPPUNIT_ASSERT_EQUAL(static_cast<int>(test.solverType), static_cast<int>(inst.solverType_));
    }

    for(const auto& test : testArgsMultiMode) {
        auto pSudokuInStream = createSudokuStream(SudokuTestPattern::BacktrackString);
        SudokuLoader inst(0, nullptr, nullptr, nullptr);
        CPPUNIT_ASSERT_EQUAL(!test.expected, inst.setMultiMode(test.argc, test.argv));
        CPPUNIT_ASSERT_EQUAL(static_cast<int>(test.solverType), static_cast<int>(inst.solverType_));
        if (!test.expected) {
            decltype(inst.multiLineFilename_) str = test.argv[1];
            CPPUNIT_ASSERT_EQUAL(str, inst.multiLineFilename_);
        }
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

    std::string expected = "Solving in C++\nError in case 1";
    std::string actual = pSudokuOutStream_->str();
    actual.resize(expected.size());
    CPPUNIT_ASSERT_EQUAL(expected, actual);
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
