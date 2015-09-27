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

class SudokuLoaderTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(SudokuLoaderTest);
    CPPUNIT_TEST(test_CanLaunch);
    CPPUNIT_TEST(test_getMeasureCount);
    CPPUNIT_TEST(test_Constructor);
    CPPUNIT_TEST(test_Exec);
    CPPUNIT_TEST(test_solveSudoku);
    CPPUNIT_TEST(test_enumerateSudoku);
    CPPUNIT_TEST(test_printTime);
    CPPUNIT_TEST(test_convertTimeToNum);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();
protected:
    void test_Constructor();
    void test_CanLaunch();
    void test_getMeasureCount();
    void test_Exec();
    void test_solveSudoku();
    void test_enumerateSudoku();
    void test_printTime();
    void test_convertTimeToNum();
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

// これ以降はテスト・ケースの実装内容
void SudokuLoaderTest::test_Constructor() {
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

    // ストリームを設定しない
    auto pSudokuInStream = createSudokuStream(SudokuTestPattern::testSet[streamIndex].presetStr);
    SudokuLoader instNoOut(SudokuTestPattern::testArgs[0].argc,
                           SudokuTestPattern::testArgs[0].argv, pSudokuInStream.get(), 0);
    SudokuLoader instNoIn(SudokuTestPattern::testArgs[0].argc,
                          SudokuTestPattern::testArgs[0].argv, 0, pSudokuOutStream_.get());

    return;
}

void SudokuLoaderTest::test_Exec() {
    size_t streamIndex = 0;
    for(const auto& test : SudokuTestPattern::testArgs) {
        if (test.measureCount == 0) {
            continue;
        }
        auto pSudokuInStream = createSudokuStream(SudokuTestPattern::testSet[streamIndex].presetStr);
        SudokuLoader inst(test.argc, test.argv, pSudokuInStream.get(), pSudokuOutStream_.get());
        constexpr int expected = 0;
        CPPUNIT_ASSERT_EQUAL(expected, inst.Exec());
        streamIndex = (streamIndex + 1) % arraySizeof(SudokuTestPattern::testSet);
    }

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

void SudokuLoaderTest::test_printTime() {
    constexpr FILETIME startTimeShort {0x3e8UL, 0UL};  // 1000
    constexpr FILETIME stopTimeShort  {0xbb8UL, 0UL};  // 3000 * 100nsec
    constexpr FILETIME startClockShort {0x4b, 0UL};    // 75
    constexpr FILETIME stopClockShort  {0xe1, 0UL};    // 225 : 200/150 = 1.333 usec/clock
    // Total時間は     200 usec(100ns単位->usecになるので1/10)
    // 1回の時間は     66.667 usec
    // 1回の最小時間は 40.000 usec (1.333 * 30 clock)
    pInstance_->printTime(startTimeShort, stopTimeShort, startClockShort, stopClockShort, 3, 30, true);

    constexpr FILETIME startTimeMiddle  {0x0, 0UL};
    constexpr FILETIME stopTimeMiddle   {0x754d4c0, 0UL};  // 123000000 * 100nsec
    constexpr FILETIME startClockMiddle {0, 0UL};          // 75
    constexpr FILETIME stopClockMiddle  {12300, 0UL};      // 225 : 200/150 = 1.333 usec/clock
    // Total時間は     12 300 000 usec(100ns単位->usecになるので1/10)
    // 1回の時間は     66.667 usec
    // 1回の最小時間は 40.000 usec (1.333 * 30 clock)
    pInstance_->printTime(startTimeMiddle, stopTimeMiddle, startClockMiddle, stopClockMiddle, 3, 300, true);

    constexpr FILETIME startTimeLong  {0x4e72a000, 0xfffff918};  //  10000000000000 (0が13個) * 100nsec
    constexpr FILETIME stopTimeLong   {0x5eece003, 0x0000540b};  // 110000000000003 (0が間に12個) * 100nsec
    constexpr FILETIME startClockLong {0x00000000, 0xff000000};  // 0.000123456 usec/clock
    constexpr FILETIME stopClockLong  {0xd60d6015, 0x001fc58a};  // 81000518403317781.233

    // Total時間は 10000000000000 usec (0が13個)
    // 1回の時間は        2000000 usec (0が6個)
    // 1回の最小時間は 12.3456usec
    pInstance_->printTime(startTimeLong, stopTimeLong, startClockLong, stopClockLong, 5000000, 100000, true);
    pInstance_->printTime(startTimeLong, stopTimeLong, startClockLong, stopClockLong, 5000000, 100000, false);

    const std::string actualstr = pSudokuOutStream_->str();

    // 画面に表示するはずの内容を確認する
    std::string expectedstr;
    expectedstr = "Total : 200usec, 150clock\naverage : 66.667usec, 50clock\n";
    expectedstr += "Once least : 40.000usec, 30clock\n\n";
    expectedstr += "Total : 12sec, 12300000usec, 12300clock\naverage : 4100000.000usec, 4100clock\n";
    expectedstr += "Once least : 300000.000usec, 300clock\n\n";
    expectedstr += "Total : 166666min 40sec, 10000000000000usec, 81000518403317781clock\n";
    expectedstr += "average : 2000000.000usec, 16200103680clock\nOnce least : 12.346usec, 100000clock\n\n";
    expectedstr += "Total : 166666min 40sec, 10000000000000usec, 81000518403317781clock\n";
    CPPUNIT_ASSERT(actualstr == expectedstr);

    return;
}

void SudokuLoaderTest::test_convertTimeToNum() {
    struct TestSet {
        FILETIME   filetime; // low, highの順
        SudokuTime sudokutime;
    };

    constexpr TestSet testSet[] {
        {{0UL,0UL},0ULL},
        {{1UL,0UL},1ULL},
        {{0UL,1UL},0x100000000ULL},
        {{2UL,1UL},0x100000002ULL},
        {{0xffffffffUL,1UL},0x1ffffffffULL},
        {{0xffffffffUL,0xffffffffUL},0xffffffffffffffffULL}
    };

    for(const auto& test : testSet) {
        CPPUNIT_ASSERT_EQUAL(test.sudokutime,  pInstance_->convertTimeToNum(test.filetime));
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
