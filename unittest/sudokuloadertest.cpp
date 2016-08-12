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
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;
protected:
    void test_Constructor();
    void test_CanLaunch();
    void test_getMeasureCount();
    void test_Exec();
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

/*
Local Variables:
mode: c++
coding: utf-8-unix
tab-width: nil
c-file-style: "stroustrup"
End:
*/
