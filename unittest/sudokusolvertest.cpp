// Testing class SudokuSolver
// Copyright (C) 2012-2017 Zettsu Tatsuya
//
// I use CppUnit code on the website.
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
    bool call_fillCells(SudokuSolver& inst);
    bool call_fillCells(SudokuSseSolver& inst);
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

    std::unique_ptr<SudokuOutStream> pSudokuOutStream_;  // destination to write strings
    std::shared_ptr<SudokuSolver>    pInstance_;         // tested object
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
    std::unique_ptr<SudokuOutStream> pSudokuOutStream_;  // destination to write strings
    std::shared_ptr<SudokuSseSolver> pInstance_;         // tested object
    std::unique_ptr<SudokuSolverCommonTest<SudokuSseSolver, SudokuSseElement>> pCommonTester_;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SudokuSseSolverTest);

// Confirms that constants for testing are correct
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

// Confirms candidates of cells are set collect
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

template <>
void SudokuSolverCommonTest<SudokuSolver, SudokuCellCandidates>::CheckCells(SudokuSolver *pInst, const SudokuIndex* expectedIndexes) {
    checkCells(pInst, expectedIndexes);
    return;
}

// Call before running a test
void SudokuSolverTest::setUp() {
    pSudokuOutStream_ = decltype(pSudokuOutStream_)(new SudokuOutStream());
    pInstance_ = decltype(pInstance_)(new SudokuSolver(SudokuTestPattern::NoBacktrackString, 0, pSudokuOutStream_.get()));
    pCommonTester_ = decltype(pCommonTester_)(new SudokuSolverCommonTest<SudokuSolver, SudokuCellCandidates>(pInstance_));

    CPPUNIT_ASSERT_EQUAL(static_cast<int>(0), pInstance_->count_);
    CPPUNIT_ASSERT(pSudokuOutStream_.get() == pInstance_->pSudokuOutStream_);
    return;
}

// Call after running a test
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

// Test cases
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
        // Destroy after destructing an instance which uses 'sudokuOutStream'
        SudokuOutStream sudokuOutStream;
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

    // This cast will never fail when SetUp() selected the right class.
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
        // Destroy after destructing an instance which uses 'sudokuOutStream'
        SudokuOutStream sudokuOutStream;
        {
            TestedT inst(test.presetStr, 0, &sudokuOutStream, 0);
            CPPUNIT_ASSERT_EQUAL(test.result, inst.solve(inst.map_, true, false, true));
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
    // Always returns true
    return true;
}


template <class TestedT, class CandidatesT>
bool SudokuSolverCommonTest<TestedT, CandidatesT>::call_fillCells(SudokuSolver& inst) {
    return inst.fillCells(inst.map_, true, false);
}

template <class TestedT, class CandidatesT>
bool SudokuSolverCommonTest<TestedT, CandidatesT>::call_fillCells(SudokuSseSolver& inst) {
    SudokuSseMapResult result;
    return inst.fillCells(inst.map_, true, false, result);
}

template <class TestedT, class CandidatesT>
void SudokuSolverCommonTest<TestedT, CandidatesT>::test_fillCells() {
    for(const auto& test : SudokuTestPattern::testSet) {
        // Destroy after destructing an instance which uses 'sudokuOutStream'
        SudokuOutStream sudokuOutStream;
        {
            TestedT inst(test.presetStr, 0, &sudokuOutStream, 0);
            CPPUNIT_ASSERT_EQUAL(filterRetvalFillCells(test.result), call_fillCells(inst));
            CPPUNIT_ASSERT_EQUAL(static_cast<int>(1), inst.count_);
        }
    }

    return;
}

void SudokuSolverTest::test_Constructor() {
    verifyTestVector();
    for(size_t i=0; i<arraySizeof(SudokuTestPattern::testSet); ++i) {
        // Destroy after destructing an instance which uses 'sudokuOutStream'
        SudokuOutStream sudokuOutStream;
        {
            // Confirm before cast
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

// Call before running a test
void SudokuSseSolverTest::setUp() {
    pSudokuOutStream_ = decltype(pSudokuOutStream_)(new SudokuOutStream());
    pInstance_ = decltype(pInstance_)(new SudokuSseSolver(SudokuTestPattern::NoBacktrackString, pSudokuOutStream_.get(), 0));
    pCommonTester_ = decltype(pCommonTester_)(new SudokuSolverCommonTest<SudokuSseSolver, SudokuSseElement>(pInstance_));

    CPPUNIT_ASSERT_EQUAL(static_cast<int>(0), pInstance_->count_);
    CPPUNIT_ASSERT(pSudokuOutStream_.get() == pInstance_->pSudokuOutStream_);
    return;
}

// Call after running a test
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
    // If all test for SudokuSseMap succeed, the constructor of SudokuSseSolver works well.
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

/*
Local Variables:
mode: c++
coding: utf-8-unix
tab-width: nil
c-file-style: "stroustrup"
End:
*/
