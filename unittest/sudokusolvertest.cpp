// SudokuSolverクラスをテストする
// Copyright (C) 2012-2015 Zettsu Tatsuya
//
// クラス定義は下記から流用
// http://www.atmarkit.co.jp/fdotnet/cpptest/cpptest02/cpptest02_02.html

#include <cppunit/extensions/HelperMacros.h>
#include <cassert>
#include <limits>
#include <memory>
#include "sudoku.h"
#include "sudokutest.h"

template <class TestedT, class CandidatesT>
class SudokuSolverCommonTest {
public:
    SudokuSolverCommonTest(std::shared_ptr<TestedT>& pInst);
    virtual ~SudokuSolverCommonTest();
    void test_Exec();
    void test_PrintType(const char *pExpectedStr);
    void test_solve();
    void test_fillCells();
    void CheckCells(SudokuSolver *pInst, const SudokuIndex* expectedIndexes);

private:
    void checkCells(TestedT *pInst, const SudokuIndex* expectedIndexes);
    bool filterRetvalFillCells(bool original);

    std::shared_ptr<TestedT> pInstance_;
};

class SudokuSolverTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(SudokuSolverTest);
    CPPUNIT_TEST(test_Constructor);
    CPPUNIT_TEST(test_Exec);
    CPPUNIT_TEST(test_PrintType);
    CPPUNIT_TEST(test_solve);
    CPPUNIT_TEST(test_fillCells);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;
protected:
    void test_Constructor();
    void test_Exec();
    void test_PrintType();
    void test_solve();
    void test_fillCells();
private:
    void verifyTestVector(void);

    std::unique_ptr<SudokuOutStream> pSudokuOutStream_;  // 結果出力先
    std::shared_ptr<SudokuSolver>    pInstance_;         // インスタンス
    std::unique_ptr<SudokuSolverCommonTest<SudokuSolver, SudokuCellCandidates>> pCommonTester_;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SudokuSolverTest);

class SudokuSseSolverTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(SudokuSseSolverTest);
    CPPUNIT_TEST(test_Constructor);
    CPPUNIT_TEST(test_Exec);
    CPPUNIT_TEST(test_Enumerate);
    CPPUNIT_TEST(test_PrintType);
    CPPUNIT_TEST(test_solve);
    CPPUNIT_TEST(test_fillCells);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

protected:
    void test_Constructor();
    void test_Exec();
    void test_Enumerate();
    void test_PrintType();
    void test_solve();
    void test_fillCells();

private:
    std::unique_ptr<SudokuOutStream> pSudokuOutStream_;  // 結果出力先
    std::shared_ptr<SudokuSseSolver> pInstance_;         // インスタンス
    std::unique_ptr<SudokuSolverCommonTest<SudokuSseSolver, SudokuSseElement>> pCommonTester_;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SudokuSseSolverTest);

class SudokuSseSearchStateTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(SudokuSseSearchStateTest);
    CPPUNIT_TEST(test_Print);
    CPPUNIT_TEST_SUITE_END();

protected:
    std::unique_ptr<SudokuSseSearchState> pInstance_;    // インスタンス
    std::unique_ptr<SudokuOutStream> pSudokuOutStream_;  // 結果出力先
public:
    void setUp() override;
    void tearDown() override;
protected:
    void test_Print();

};

CPPUNIT_TEST_SUITE_REGISTRATION(SudokuSseSearchStateTest);

// テスト用の値が正しいかどうか検算する
void SudokuSolverTest::verifyTestVector(void) {
    assert(SudokuTestPattern::NoBacktrackString.length() >= Sudoku::SizeOfAllCells);
    assert(SudokuTestPattern::BacktrackString.length() >= Sudoku::SizeOfAllCells);
    assert(SudokuTestPattern::BacktrackString2.length() >= Sudoku::SizeOfAllCells);
    assert(SudokuTestPattern::ConflictString.length() >= Sudoku::SizeOfAllCells);
    assert(arraySizeof(SudokuTestPattern::NoBacktrackPreset) >= Sudoku::SizeOfAllCells);
    assert(arraySizeof(SudokuTestPattern::BacktrackPreset) >= Sudoku::SizeOfAllCells);
    assert(arraySizeof(SudokuTestPattern::ConflictPreset) >= Sudoku::SizeOfAllCells);
    assert(arraySizeof(SudokuTestPattern::NoBacktrackResult) >= Sudoku::SizeOfAllCells);
    assert(arraySizeof(SudokuTestPattern::BacktrackResult) >= Sudoku::SizeOfAllCells);
    assert(arraySizeof(SudokuTestPattern::ConflictResult) >= Sudoku::SizeOfAllCells);
    return;
}

// マスの候補が正しく設定されたかどうか調べる
template <>
void SudokuSolverCommonTest<SudokuSolver, SudokuCellCandidates>::checkCells(SudokuSolver *pInst, const SudokuIndex* expectedIndexes) {
    for(size_t i=0;i<arraySizeof(pInst->map_.cells_);++i) {
        const auto index = expectedIndexes[i];
        if (index == SudokuTestPattern::ConflictedCell) {
            continue;
        }

        auto expected = SudokuCell::SudokuAllCandidates;
        if ((index >= 1) && (index <= 9)) {
            expected = SudokuTestCommon::ConvertToCandidate(index);
        }
        CPPUNIT_ASSERT_EQUAL(expected, pInst->map_.cells_[i].candidates_);
    }

    return;
}

template <>
void SudokuSolverCommonTest<SudokuSseSolver, SudokuSseElement>::checkCells(SudokuSseSolver *pInst, const SudokuIndex* expectedIndexes) {
    size_t regIndex = pInstance_->map_.InitialRegisterNum * SudokuSse::RegisterWordCnt + 2;
    size_t elementIndex = 0;

    for(SudokuIndex y=0; y < Sudoku::SizeOfCellsPerGroup; ++y) {
        for(SudokuIndex x=0; x < Sudoku::SizeOfBoxesOnEdge; ++x) {
            SudokuSseElement regval = pInst->map_.xmmRegSet_.regVal_[regIndex--];
            auto i=Sudoku::SizeOfCellsOnBoxEdge;
            do {
                --i;
                SudokuSseElement actual = (regval >> (i * 9)) & 0x1ff;
                auto index = expectedIndexes[elementIndex++];
                assert(elementIndex <= Sudoku::SizeOfAllCells);
                if (index != SudokuTestPattern::ConflictedCell) {
                    auto expected = SudokuCell::SudokuAllCandidates;
                    if ((index >= 1) && (index <= 9)) {
                        expected = 1 << (index - 1);
                    }
                    CPPUNIT_ASSERT_EQUAL(expected, actual);
                }
            } while (i != 0);
        }
        regIndex += 7;
        assert(regIndex <= arraySizeof(pInst->map_.xmmRegSet_.regVal_));
    }

    return;
}

// マスの候補が正しく設定されたかどうか調べる
template <>
void SudokuSolverCommonTest<SudokuSolver, SudokuCellCandidates>::CheckCells(SudokuSolver *pInst, const SudokuIndex* expectedIndexes) {
    checkCells(pInst, expectedIndexes);
    return;
}

// 各テスト・ケースの実行直前に呼ばれる
void SudokuSolverTest::setUp() {
    pSudokuOutStream_ = decltype(pSudokuOutStream_)(new SudokuOutStream());
    pInstance_ = decltype(pInstance_)(new SudokuSolver(SudokuTestPattern::NoBacktrackString, 0, pSudokuOutStream_.get()));
    pCommonTester_ = decltype(pCommonTester_)(new SudokuSolverCommonTest<SudokuSolver, SudokuCellCandidates>(pInstance_));

    CPPUNIT_ASSERT_EQUAL(static_cast<int>(0), pInstance_->count_);
    CPPUNIT_ASSERT(pSudokuOutStream_.get() == pInstance_->pSudokuOutStream_);
    return;
}

// 各テスト・ケースの実行直後に呼ばれる
void SudokuSolverTest::tearDown() {
    assert(pCommonTester_);
    assert(pInstance_);
    assert(pSudokuOutStream_);
    pCommonTester_.reset();

    assert(pInstance_);
    assert(pSudokuOutStream_);
    pInstance_.reset();
    pSudokuOutStream_.reset();
    return;
}

// これ以降はテスト・ケースの実装内容
template <class TestedT, class CandidatesT>
SudokuSolverCommonTest<TestedT, CandidatesT>::SudokuSolverCommonTest(std::shared_ptr<TestedT>& pInst)
    : pInstance_(pInst) {
    return;
}

template <class TestedT, class CandidatesT>
SudokuSolverCommonTest<TestedT, CandidatesT>::~SudokuSolverCommonTest() {
    return;
}

template <class TestedT, class CandidatesT>
void SudokuSolverCommonTest<TestedT, CandidatesT>::test_Exec() {
    for(const auto& test : SudokuTestPattern::testSet) {
        SudokuOutStream sudokuOutStream;  // SudokuSolverインスタンスの解体後に解体する
        {
            TestedT inst(test.presetStr, 0, &sudokuOutStream, 0);
            CPPUNIT_ASSERT_EQUAL(test.result, inst.Exec(true, false));
            checkCells(&inst, test.resultNum);
            CPPUNIT_ASSERT(inst.count_>=1);
        }
    }

    return;
}

template <class TestedT, class CandidatesT>
void SudokuSolverCommonTest<TestedT, CandidatesT>::test_PrintType(const char *pExpectedStr) {
    pInstance_->PrintType();

    // SetUpで設定したのだから成功するはず
    SudokuOutStream* pOstream = dynamic_cast<SudokuOutStream*>(pInstance_->pSudokuOutStream_);
    assert(pOstream != nullptr);
    const std::string actualstr = pOstream->str();
    const std::string expectedstr(pExpectedStr);
    CPPUNIT_ASSERT(actualstr == expectedstr);
    return;
}

template <class TestedT, class CandidatesT>
void SudokuSolverCommonTest<TestedT, CandidatesT>::test_solve() {
    for(const auto& test : SudokuTestPattern::testSet) {
        SudokuOutStream sudokuOutStream;  // SudokuSolverインスタンスの解体後に解体する
        {
            TestedT inst(test.presetStr, 0, &sudokuOutStream, 0);
            CPPUNIT_ASSERT_EQUAL(test.result, inst.solve(inst.map_, true, false));
            CPPUNIT_ASSERT(inst.count_>=1);
        }
    }
    return;
}

template <>
bool SudokuSolverCommonTest<SudokuSolver, SudokuCellCandidates>::filterRetvalFillCells(bool original) {
    return original;
}

template <>
bool SudokuSolverCommonTest<SudokuSseSolver, SudokuSseElement>::filterRetvalFillCells(bool original) {
    // 常に同じ値
    return true;
}

template <class TestedT, class CandidatesT>
void SudokuSolverCommonTest<TestedT, CandidatesT>::test_fillCells() {
    for(const auto& test : SudokuTestPattern::testSet) {
        SudokuOutStream sudokuOutStream;  // SudokuSolverインスタンスの解体後に解体する
        {
            TestedT inst(test.presetStr, 0, &sudokuOutStream, 0);
            CPPUNIT_ASSERT_EQUAL(filterRetvalFillCells(test.result), inst.fillCells(inst.map_, true, false));
            CPPUNIT_ASSERT_EQUAL(static_cast<int>(1), inst.count_);
        }
    }

    return;
}

void SudokuSolverTest::test_Constructor() {
    verifyTestVector();
    for(size_t i=0; i<arraySizeof(SudokuTestPattern::testSet); ++i) {
        SudokuOutStream sudokuOutStream;  // SudokuSolverインスタンスの解体後に解体する
        {
            // キャストする前に確認する
            assert(arraySizeof(SudokuTestPattern::testSet) < std::numeric_limits<SudokuIndex>::max());
            SudokuSolver inst(SudokuTestPattern::testSet[i].presetStr, static_cast<SudokuIndex>(i), &sudokuOutStream);
            pCommonTester_->CheckCells(&inst, SudokuTestPattern::testSet[i].presetNum);
            CPPUNIT_ASSERT(&sudokuOutStream == inst.pSudokuOutStream_);
        }
    }

    SudokuOutStream sudokuOutStream;
    const std::string ShortStr =  "123456789\n987...32";
    SudokuSolver inst(ShortStr, 0, &sudokuOutStream);
    return;
}

void SudokuSolverTest::test_Exec() {
    pCommonTester_->test_Exec();
    return;
}

void SudokuSolverTest::test_PrintType() {
    pCommonTester_->test_PrintType("[C++]\n");
    return;
}

void SudokuSolverTest::test_solve() {
    pCommonTester_->test_solve();
    return;
}

void SudokuSolverTest::test_fillCells() {
    pCommonTester_->test_fillCells();
    return;
}

// 各テスト・ケースの実行直前に呼ばれる
void SudokuSseSolverTest::setUp() {
    pSudokuOutStream_ = decltype(pSudokuOutStream_)(new SudokuOutStream());
    pInstance_ = decltype(pInstance_)(new SudokuSseSolver(SudokuTestPattern::NoBacktrackString, pSudokuOutStream_.get(), 0));
    pCommonTester_ = decltype(pCommonTester_)(new SudokuSolverCommonTest<SudokuSseSolver, SudokuSseElement>(pInstance_));

    CPPUNIT_ASSERT_EQUAL(static_cast<int>(0), pInstance_->count_);
    CPPUNIT_ASSERT(pSudokuOutStream_.get() == pInstance_->pSudokuOutStream_);
    return;
}

// 各テスト・ケースの実行直後に呼ばれる
void SudokuSseSolverTest::tearDown() {
    assert(pCommonTester_);
    assert(pInstance_);
    assert(pSudokuOutStream_);
    pCommonTester_.reset();

    assert(pInstance_);
    assert(pSudokuOutStream_);
    pInstance_.reset();
    pSudokuOutStream_.reset();
    return;
}

void SudokuSseSolverTest::test_Constructor() {
    // SudokuSseMapのテストをもって替える
    return;
}

void SudokuSseSolverTest::test_Exec() {
    pCommonTester_->test_Exec();
    return;
}

void SudokuSseSolverTest::test_Enumerate() {
    const std::string presetStr = "123456789456789123789123456295874631.............................................";
    SudokuSseSolver solver(presetStr, pSudokuOutStream_.get(), 1);
    static_assert(sizeof(sudokuXmmPrintFunc) == sizeof(&PrintPattern), "Unexpected uintptr_t size");
    sudokuXmmPrintFunc = reinterpret_cast<uintptr_t>(&PrintPattern);
    solver.Enumerate();

    std::string expected;
    expected += "1:2:3:4:5:6:7:8:9\n";
    expected += "4:5:6:7:8:9:1:2:3\n";
    expected += "7:8:9:1:2:3:4:5:6\n";
    expected += "2:9:5:8:7:4:6:3:1\n";
    expected += "0:0:0:0:0:0:0:0:0\n";
    expected += "0:0:0:0:0:0:0:0:0\n";
    expected += "0:0:0:0:0:0:0:0:0\n";
    expected += "0:0:0:0:0:0:0:0:0\n";
    expected += "0:0:0:0:0:0:0:0:0\n";
    expected += "[Solution 1]\n";
    expected += "1:2:3:4:5:6:7:8:9\n";
    expected += "4:5:6:7:8:9:1:2:3\n";
    expected += "7:8:9:1:2:3:4:5:6\n";
    expected += "2:9:5:8:7:4:6:3:1\n";
    expected += "3:1:4:2:6:5:8:9:7\n";
    expected += "6:7:8:3:9:1:2:4:5\n";
    expected += "5:3:1:6:4:2:9:7:8\n";
    expected += "8:6:2:9:3:7:5:1:4\n";
    expected += "9:4:7:5:1:8:3:6:2\n";
    expected += "Number of solutions : 570000\n";

    CPPUNIT_ASSERT_EQUAL(expected, pSudokuOutStream_->str());
}

void SudokuSseSolverTest::test_PrintType() {
    pCommonTester_->test_PrintType("[SSE 4.2]\n");
    return;
}

void SudokuSseSolverTest::test_solve() {
    pCommonTester_->test_solve();
    return;
}

void SudokuSseSolverTest::test_fillCells() {
    pCommonTester_->test_fillCells();
    return;
}

// 各テスト・ケースの実行直前に呼ばれる
void SudokuSseSearchStateTest::setUp() {
    pSudokuOutStream_ = decltype(pSudokuOutStream_)(new SudokuOutStream());
    pInstance_ = decltype(pInstance_)(new SudokuSseSearchState());

    CPPUNIT_ASSERT_EQUAL(static_cast<uint64_t>(0), pInstance_->member_.uniqueCandidate_);
    CPPUNIT_ASSERT_EQUAL(static_cast<uint64_t>(0), pInstance_->member_.candidateCnt_);
    CPPUNIT_ASSERT_EQUAL(static_cast<uint64_t>(0), pInstance_->member_.candidateRow_);
    CPPUNIT_ASSERT_EQUAL(static_cast<uint64_t>(0), pInstance_->member_.candidateInBoxShift_);
    CPPUNIT_ASSERT_EQUAL(static_cast<uint64_t>(0), pInstance_->member_.candidateOutBoxShift_);
    return;
}

// 各テスト・ケースの実行直後に呼ばれる
void SudokuSseSearchStateTest::tearDown() {
    assert(pInstance_);
    assert(pSudokuOutStream_);
    pInstance_.reset();
    pSudokuOutStream_.reset();
    return;
}

void SudokuSseSearchStateTest::test_Print() {
    constexpr SudokuSseSearchStateMember member {64, 4, 3, 2, 1};
    pInstance_->member_ = member;
    pInstance_->Print(pSudokuOutStream_.get());

    // SetUpで設定したのだから成功するはず
    SudokuOutStream* pOstream = dynamic_cast<SudokuOutStream*>(pSudokuOutStream_.get());
    assert(pOstream != nullptr);
    const std::string actualstr = pOstream->str();
    std::string expectedstr("fill unique candidate 64, cnt 4, row 3, in 2, out 1\n");
    CPPUNIT_ASSERT(expectedstr == actualstr);
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
