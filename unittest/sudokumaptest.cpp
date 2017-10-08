// Testing class Sudokumap
// Copyright (C) 2012-2017 Zettsu Tatsuya
//
// I use CppUnit code on the website.
// http://www.atmarkit.co.jp/fdotnet/cpptest/cpptest02/cpptest02_02.html

#include <cppunit/extensions/HelperMacros.h>
#include <cassert>
#include <cstring>
#include <memory>
#include "sudoku.h"
#include "sudokutest.h"

namespace SudokuTest {
    void SetAllCellsFilled(size_t initialRegisterNum, XmmRegisterSet& xmmRegSet) {
        for(size_t i = initialRegisterNum;
            i < (initialRegisterNum + Sudoku::SizeOfGroupsPerMap); ++i) {
            xmmRegSet.regVal_[i * SudokuSse::RegisterWordCnt] = 0x40404;
            xmmRegSet.regVal_[i * SudokuSse::RegisterWordCnt + 1] = 0x0202020;
            xmmRegSet.regVal_[i * SudokuSse::RegisterWordCnt + 2] = 0x1010100;
            xmmRegSet.regVal_[i * SudokuSse::RegisterWordCnt + 3] = 0;
        }

        return;
    }

    void SetAllCellsFullCandidates(size_t initialRegisterNum, XmmRegisterSet& xmmRegSet) {
        for(size_t i = initialRegisterNum;
            i < (initialRegisterNum + Sudoku::SizeOfGroupsPerMap); ++i) {
            for(size_t j = 0; j < (SudokuSse::RegisterWordCnt - 1); ++j) {
                xmmRegSet.regVal_[i * SudokuSse::RegisterWordCnt + j] = Sudoku::AllThreeCandidates;
            }
            xmmRegSet.regVal_[(i + 1) * SudokuSse::RegisterWordCnt - 1] = 0;
        }

        return;
    }
}

class SudokuMapTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(SudokuMapTest);
    CPPUNIT_TEST(test_Preset);
    CPPUNIT_TEST(test_Print);
    CPPUNIT_TEST(test_IsFilled);
    CPPUNIT_TEST(test_FillCrossing);
    CPPUNIT_TEST(test_CanSetUniqueCell);
    CPPUNIT_TEST(test_SetUniqueCell);
    CPPUNIT_TEST(test_CountFilledCells);
    CPPUNIT_TEST(test_SelectBacktrackedCellIndex);
    CPPUNIT_TEST(test_IsConsistent);
    CPPUNIT_TEST(test_findUnusedCandidate);
    CPPUNIT_TEST(test_findUniqueCandidate);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;
protected:
    void test_Preset();
    void test_Print();
    void test_IsFilled();
    void test_FillCrossing();
    void test_CanSetUniqueCell();
    void test_SetUniqueCell();
    void test_CountFilledCells();
    void test_SelectBacktrackedCellIndex();
    void test_IsConsistent();
    void test_findUnusedCandidate();
    void test_findUniqueCandidate();
private:
    void checkConstructor(void);
    void checkCellIndexes(void);
    SudokuCellCandidates indexToCandidate(SudokuIndex index);
    void setAllCellsFilled(void);
    void setAllCellsMultiCandidates(void);
    void setAllCellsFullCandidates(void);
    void setConsistentCells(SudokuCellCandidates candidates);
    void setTopLineExceptRightest(void);
    SudokuIndex convertCellPosition(SudokuIndex orignal, SudokuIndex boxX, SudokuIndex boxY,
                                    SudokuIndex inboxOfsX, SudokuIndex inboxOfsY);
    void testFillCrossingBox(SudokuIndex boxX, SudokuIndex boxY,
                             SudokuIndex inboxOfsX, SudokuIndex inboxOfsY);
    std::unique_ptr<SudokuMap> pInstance_;   // tested object
};

CPPUNIT_TEST_SUITE_REGISTRATION(SudokuMapTest);

class SudokuSseMapTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(SudokuSseMapTest);
    CPPUNIT_TEST(test_Preset);
    CPPUNIT_TEST(test_Print);
    CPPUNIT_TEST(test_FillCrossing);
    CPPUNIT_TEST(test_GetNextCell);
    CPPUNIT_TEST(test_CanSetUniqueCell);
    CPPUNIT_TEST(test_SetUniqueCell);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

protected:
    void test_Preset();
    void test_Print();
    void test_FillCrossing();
    void test_GetNextCell();
    void test_CanSetUniqueCell();
    void test_SetUniqueCell();

private:
    void checkConstructor(void);
    void setAllCellsFilled(void);
    void setAllCellsFullCandidates(void);
    void checkCandidateCell(const SudokuSseCandidateCell& expected, decltype(SudokuSseCandidateCell::regIndex) regIndex,
                            decltype(SudokuSseCandidateCell::shift) inBoxShift);
    std::unique_ptr<SudokuSseMap> pInstance_;  // tested object
};

CPPUNIT_TEST_SUITE_REGISTRATION(SudokuSseMapTest);

void SudokuMapTest::checkConstructor(void) {
    checkCellIndexes();
    return;
}

// Checks all index of cells are right
void SudokuMapTest::checkCellIndexes(void) {
    decltype(SudokuCell::indexNumber_) i = 0;

    for(const auto& cell : pInstance_->cells_) {
        CPPUNIT_ASSERT(cell.indexNumber_ == i);
        ++i;
    }
    return;
}

// Converts a digit of candidate 1..9 to a bitboard representation
SudokuCellCandidates SudokuMapTest::indexToCandidate(SudokuIndex index) {
    if ((index < 1) || (index > 9)) {
        return SudokuCell::SudokuAllCandidates;
    }

    return SudokuTestCommon::ConvertToCandidate(index);
}

// Sets a unique candidate to cells
void SudokuMapTest::setAllCellsFilled(void) {
    for(SudokuIndex i=0;i<Sudoku::SizeOfAllCells;++i) {
        pInstance_->cells_[i].candidates_ = SudokuTestCandidates::OneOnly;
    }
    return;
}

// Sets multiple and part of 1..9 candidate to cells
void SudokuMapTest::setAllCellsMultiCandidates(void) {
    for(SudokuIndex i=0;i<Sudoku::SizeOfAllCells;++i) {
        pInstance_->cells_[i].candidates_ = SudokuTestCandidates::TwoToNine;
    }

    return;
}

// Sets all 1..9 candidate to cells
void SudokuMapTest::setAllCellsFullCandidates(void) {
    for(SudokuIndex i=0;i<Sudoku::SizeOfAllCells;++i) {
        pInstance_->cells_[i].candidates_ = SudokuTestCandidates::All;
    }

    return;
}

void SudokuMapTest::setConsistentCells(SudokuCellCandidates candidates) {
    // Set a candidate to boxes as below
    // *........
    // ...*.....
    // ......*..
    // .*.......
    // ....*....
    // .......*.
    // ..*......
    // .....*...
    // ........*
    for(SudokuIndex i=0;i<Sudoku::SizeOfCellsPerGroup;++i) {
        const SudokuIndex index = i*9 + i/3 + (i%3)*3;
        pInstance_->cells_[index].candidates_ = candidates;
    }
}

// Sets a candidate to cells in the top row except its rightmost
void SudokuMapTest::setTopLineExceptRightest(void) {
    for(SudokuIndex i=0;i<Sudoku::SizeOfCellsPerGroup - 1;++i) {
        pInstance_->cells_[i].candidates_ = indexToCandidate(i+1);
    }

    return;
}

// Call before running a test
void SudokuMapTest::setUp() {
    pInstance_ = decltype(pInstance_)(new SudokuMap());
    checkConstructor();
    return;
}

// Call after running a test
void SudokuMapTest::tearDown() {
    return;
}

// Test cases
void SudokuMapTest::test_Preset() {
    const std::string presetStr = "1234567890\a\b\n\r\f\t\v0987654321 !\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~.abcdefxyzABCDEFXYZ..";
//                                 12345678912 3 4 5 6 7 8 9123456789123 45678 91234567891234567 89123456789123456789123456789
    const std::string ShortStr = "1234567890\\";
    constexpr SudokuIndex presetNum[] {
        1,2,3,4,5,6,7,8,9, 0,0,0,0,0,0,0,0,0, 9,8,7,6,5,4,3,2,1,
        0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0};
    assert(presetStr.length() == arraySizeof(presetNum));

    pInstance_->Preset(presetStr, 0);
    for(SudokuIndex i=0;i<Sudoku::SizeOfAllCells;++i) {
        auto expected = indexToCandidate(presetNum[i]);
        CPPUNIT_ASSERT_EQUAL(expected, pInstance_->cells_[i].candidates_);
    }

    for(SudokuIndex i=0; i < Sudoku::SizeOfGroupsPerCell * 2; ++i) {
        pInstance_->Preset(presetStr, i);
        SudokuIndex expected = i % Sudoku::SizeOfGroupsPerCell;
        CPPUNIT_ASSERT_EQUAL(expected, pInstance_->backtrackedGroup_);
        CPPUNIT_ASSERT(pInstance_->backtrackedGroup_ < Sudoku::SizeOfGroupsPerCell);
    }

    // An input string is too short but it does not cause a crash.
    pInstance_->Preset(ShortStr, 0);
    return;
}

void SudokuMapTest::test_Print() {
    SudokuCellCandidates candidates = 0;
    SudokuIndex index = 0;
    size_t expected = 0;  // expected length of output strings

    for(SudokuIndex y=0; y < Sudoku::SizeOfCellsPerGroup; ++y) {
        for(SudokuIndex x=0; x < Sudoku::SizeOfCellsPerGroup; ++x) {
            pInstance_->cells_[index++].candidates_ = candidates;
            expected += y+1;  // Count a delimiter
        }
        candidates = (candidates << 1) | SudokuTestCandidates::UniqueBase;
        ++expected;  // Count a linefeed
    }

    // Set all candidates 1..9 to the last cell
    pInstance_->cells_[Sudoku::SizeOfAllCells - 1].candidates_ = SudokuTestCandidates::All;
    ++expected;
    // Count the last linefeed
    ++expected;

    SudokuOutStream sudokuOutStream;
    pInstance_->Print(&sudokuOutStream);
    CPPUNIT_ASSERT_EQUAL(expected, sudokuOutStream.str().length());

    // Missing output stream
    pInstance_->cells_[0].candidates_ = SudokuTestCandidates::CenterLine;
    pInstance_->Print(nullptr);
    return;
}

void SudokuMapTest::test_IsFilled() {
    // All cells are filled at first
    setAllCellsFilled();
    CPPUNIT_ASSERT_EQUAL(true, pInstance_->IsFilled());

    // Make each cell not filled one by one
    SudokuIndex i = Sudoku::SizeOfAllCells;
    do {
        --i;
        pInstance_->cells_[i].candidates_ = SudokuTestCandidates::TwoToNine;
        CPPUNIT_ASSERT_EQUAL(false, pInstance_->IsFilled());
    } while(i != 0);

    return;
}

SudokuIndex SudokuMapTest::convertCellPosition(SudokuIndex orignal, SudokuIndex boxX, SudokuIndex boxY,
                                               SudokuIndex inboxOfsX, SudokuIndex inboxOfsY) {
    const SudokuIndex originalOfsX = orignal % Sudoku::SizeOfCellsOnBoxEdge;
    const SudokuIndex originalOfsY = (orignal / Sudoku::SizeOfCellsPerGroup) % Sudoku::SizeOfCellsOnBoxEdge;
    const SudokuIndex originalBoxX = (orignal % Sudoku::SizeOfCellsPerGroup) / Sudoku::SizeOfCellsOnBoxEdge;
    const SudokuIndex originalBoxY = (orignal / Sudoku::SizeOfCellsPerGroup) / Sudoku::SizeOfCellsOnBoxEdge;
    const SudokuIndex newOfsX = (originalOfsX + Sudoku::SizeOfCellsOnBoxEdge) % Sudoku::SizeOfCellsOnBoxEdge;
    const SudokuIndex newOfsY = (originalOfsY + Sudoku::SizeOfCellsOnBoxEdge) % Sudoku::SizeOfCellsOnBoxEdge;
    const SudokuIndex newBoxX = (originalBoxX + Sudoku::SizeOfCellsOnBoxEdge) % Sudoku::SizeOfCellsOnBoxEdge;
    const SudokuIndex newBoxY = (originalBoxY + Sudoku::SizeOfCellsOnBoxEdge) % Sudoku::SizeOfCellsOnBoxEdge;
    const SudokuIndex newPos = newOfsX + newOfsY * Sudoku::SizeOfCellsPerGroup
        + newBoxX * Sudoku::SizeOfCellsOnBoxEdge
        + newBoxY * Sudoku::SizeOfCellsPerGroup * Sudoku::SizeOfCellsOnBoxEdge;
    return newPos;
}

void SudokuMapTest::testFillCrossingBox(SudokuIndex boxX, SudokuIndex boxY,
                                        SudokuIndex inboxOfsX, SudokuIndex inboxOfsY) {
    // ?73846259
    // 8**..9..@
    // !5^......
    // .2.......
    // .........
    // .........
    // ..2......
    // .........
    // .........
    // ?=1, *=[4,6], !=2, ^=9 are filled.
    // Setting 4 to @ makes this map inconsistent.

    // This test fills these cells in the map and checks the map after calling FillCrossing().
    // We can swap rows, columns, and boxes according to the arguments and
    // we get same results before and after swapping.

    struct TestSet{
        SudokuIndex cellIndex;
        SudokuIndex candidatesIndex;
    };
    constexpr TestSet testSet[] {
        {1,7}, {2,3}, {3,8}, {4,4}, {5,6}, {6,2}, {7,5}, {8,9}, {9,8}, {14,9}, {19,5}, {28,2}, {56,2}
    };
    constexpr TestSet conflictSet[] {{17,4}};

    struct ResultSet{
        SudokuIndex cellIndex;
        SudokuCellCandidates candidates;
    };

    constexpr ResultSet resultSet[] {
        {0,  SudokuTestCandidates::OneOnly},
        {10, SudokuTestCandidates::FourAndSix},
        {11, SudokuTestCandidates::FourAndSix},
        {18, SudokuTestCandidates::TwoOnly},
        {20, SudokuTestCandidates::NineOnly}
    };

    setAllCellsFullCandidates();
    // Swaps rows, columns, and boxes according to the arguments
    for(const auto& test : testSet) {
        const auto target = convertCellPosition(test.cellIndex, boxX, boxY, inboxOfsX, inboxOfsY);
        pInstance_->cells_[target].candidates_ = indexToCandidate(test.candidatesIndex);
    }

    CPPUNIT_ASSERT_EQUAL(false, pInstance_->SudokuMap::FillCrossing());

    // Confirms cells are updated correctly
    for(const auto& result : resultSet) {
        const auto target = convertCellPosition(result.cellIndex, boxX, boxY, inboxOfsX, inboxOfsY);
        CPPUNIT_ASSERT_EQUAL(result.candidates, pInstance_->cells_[target].candidates_);
    }

    // Makes the map inconsistent intentionally
    for(const auto& conflict : conflictSet) {
        const auto target = convertCellPosition(conflict.cellIndex, boxX, boxY, inboxOfsX, inboxOfsY);
        pInstance_->cells_[target].candidates_ = indexToCandidate(conflict.candidatesIndex);
    }

    CPPUNIT_ASSERT_EQUAL(true, pInstance_->SudokuMap::FillCrossing());
    return;
}

void SudokuMapTest::test_FillCrossing() {
    // Boxes
    for(SudokuIndex boxX = 0; boxX < Sudoku::SizeOfBoxesOnEdge; ++boxX) {
        for(SudokuIndex boxY = 0; boxY < Sudoku::SizeOfBoxesOnEdge; ++boxY) {
            // In a box
            for(SudokuIndex ofsX = 0; ofsX < Sudoku::SizeOfCellsOnBoxEdge; ++ofsX) {
                for(SudokuIndex ofsY = 0; ofsY < Sudoku::SizeOfCellsOnBoxEdge; ++ofsY) {
                    testFillCrossingBox(boxX, boxY, ofsX, ofsY);
                }
            }
        }
    }

    return;
}

void SudokuMapTest::test_CanSetUniqueCell() {
    for(SudokuIndex i=0;i<Sudoku::SizeOfAllCells;++i) {
        assert(i < arraySizeof(pInstance_->cells_));
        for(SudokuIndex j=0;j<Sudoku::SizeOfCandidates;++j) {
            const SudokuCellCandidates candidate = 1 << j;
            bool expected = ((j % 2) == 0);

            pInstance_->cells_[i].candidates_ = SudokuTestCandidates::Odds;
            CPPUNIT_ASSERT_EQUAL(expected, pInstance_->CanSetUniqueCell(i, candidate));
            pInstance_->cells_[i].candidates_ = SudokuTestCandidates::Evens;
            CPPUNIT_ASSERT_EQUAL(!expected, pInstance_->CanSetUniqueCell(i, candidate));
        }
    }
}

void SudokuMapTest::test_SetUniqueCell() {
    struct TestSetNotFilled{
        SudokuCellCandidates candidates;
        bool filled;
    };

    constexpr TestSetNotFilled testSet[] = {
        {SudokuTestCandidates::All, true},
        {SudokuTestCandidates::Evens, true},
        {SudokuTestCandidates::Odds, true},
        {SudokuTestCandidates::FourAndSix, true},
        {SudokuTestCandidates::CenterLine, true},
        {SudokuTestCandidates::CenterOnly, false},
        {SudokuTestCandidates::Evens, false},
        {SudokuTestCandidates::Empty, false},
        {SudokuTestCandidates::FourAndSix, false}};

    for(SudokuIndex i=0;i<Sudoku::SizeOfAllCells;++i) {
        const SudokuIndex index = i % Sudoku::SizeOfCandidates;
        assert(index < arraySizeof(testSet));

        const auto expected = testSet[index].filled;
        // 9,8,7,6,5,4,3,2,1
        auto candidate = indexToCandidate(9 - index);
        pInstance_->cells_[i].candidates_ = testSet[index].candidates;
        CPPUNIT_ASSERT_EQUAL(expected, pInstance_->CanSetUniqueCell(i, candidate));

        pInstance_->SetUniqueCell(i, candidate);
        SudokuCellCandidates expectedValue = (expected) ? candidate : 0;
        CPPUNIT_ASSERT_EQUAL(expectedValue, pInstance_->cells_[i].candidates_);

        // 1..9
        candidate = indexToCandidate(index + 1);
        pInstance_->cells_[i].candidates_ = testSet[index].candidates;
        CPPUNIT_ASSERT_EQUAL(expected, pInstance_->CanSetUniqueCell(i, candidate));

        pInstance_->SetUniqueCell(i, candidate);
        expectedValue = (expected) ? candidate : 0;
        CPPUNIT_ASSERT_EQUAL(expectedValue, pInstance_->cells_[i].candidates_);
    }

    return;
}

void SudokuMapTest::test_CountFilledCells() {
    // All cells have no unique candidate
    setAllCellsMultiCandidates();
    CPPUNIT_ASSERT_EQUAL(static_cast<SudokuIndex>(0), pInstance_->CountFilledCells());

    // Sets a unique candidate to each cell one by one
    SudokuIndex expected = 0;
    for(SudokuIndex i=0;i<Sudoku::SizeOfAllCells;++i) {
        ++expected;
        pInstance_->cells_[i].candidates_ = SudokuTestCandidates::OneOnly;
        CPPUNIT_ASSERT_EQUAL(expected, pInstance_->CountFilledCells());
    }

    // All cells have each unique candidate
    setAllCellsFilled();
    CPPUNIT_ASSERT_EQUAL(Sudoku::SizeOfAllCells, pInstance_->CountFilledCells());

    // Sets multiple candidates to each cell one by one
    SudokuIndex i = Sudoku::SizeOfAllCells;
    do {
        --i;
        pInstance_->cells_[i].candidates_ = SudokuTestCandidates::TwoToNine;
        CPPUNIT_ASSERT_EQUAL(i, pInstance_->CountFilledCells());
    } while(i != 0);

    return;
}

void SudokuMapTest::test_SelectBacktrackedCellIndex() {
    // First, all cells have all 1..9 candidates and are consistent.
    setAllCellsFullCandidates();
    // The first (top-left) cell is selected.
    auto expected = SudokuTestPosition::Head;
    CPPUNIT_ASSERT_EQUAL(expected, pInstance_->SelectBacktrackedCellIndex());

    // Decrement a candidate from the last (bottom-right) cell.
    expected = SudokuTestPosition::Last;
    // The last cell is selected because it has least candidates.
    pInstance_->cells_[expected].candidates_ = SudokuTestCandidates::ExceptCenter;
    CPPUNIT_ASSERT_EQUAL(expected, pInstance_->SelectBacktrackedCellIndex());

    // Decrement a candidate from the second cell.
    expected = SudokuTestPosition::HeadNext;
    pInstance_->cells_[expected].candidates_ = SudokuTestCandidates::TwoToNine;
    // The second cell is selected because it has a smaller index than the last cell.
    CPPUNIT_ASSERT_EQUAL(expected, pInstance_->SelectBacktrackedCellIndex());

    // Set two candidates to two cells in a box.
    // A cell is selected in the box because the box has least candidates.
    expected = SudokuTestPosition::Backtracked;
    pInstance_->cells_[expected].candidates_ = SudokuTestCandidates::FourAndSix;
    CPPUNIT_ASSERT_EQUAL(expected, pInstance_->SelectBacktrackedCellIndex());

    // Cells which have no candidates are not selected.
    pInstance_->cells_[SudokuTestPosition::Center].candidates_ = SudokuTestCandidates::Empty;
    CPPUNIT_ASSERT_EQUAL(expected, pInstance_->SelectBacktrackedCellIndex());

    // Cells which have only one candidate are not selected.
    pInstance_->cells_[SudokuTestPosition::Center].candidates_ = SudokuTestCandidates::CenterOnly;
    CPPUNIT_ASSERT_EQUAL(expected, pInstance_->SelectBacktrackedCellIndex());

    return;
}

void SudokuMapTest::test_IsConsistent() {
    // First, all cells have all 1..9 candidates and are consistent.
    setAllCellsFullCandidates();
    CPPUNIT_ASSERT_EQUAL(true, pInstance_->IsConsistent());

    // Set a candidate '1' to cells in each box.
    setConsistentCells(SudokuTestCandidates::OneOnly);
    CPPUNIT_ASSERT_EQUAL(true, pInstance_->IsConsistent());

    // Clear candidates in a cell.
    pInstance_->cells_[SudokuTestPosition::Last].candidates_ = SudokuTestCandidates::Empty;
    CPPUNIT_ASSERT_EQUAL(false, pInstance_->IsConsistent());

    // Set a candidate '1' to cells and it makes the map inconsistent.
    setConsistentCells(SudokuTestCandidates::OneOnly);
    pInstance_->cells_[SudokuTestPosition::Conflict].candidates_ = SudokuTestCandidates::OneOnly;
    CPPUNIT_ASSERT_EQUAL(false, pInstance_->IsConsistent());

    return;
}

void SudokuMapTest::test_findUnusedCandidate() {
    // a row
    setAllCellsFullCandidates();
    setTopLineExceptRightest();
    auto target = Sudoku::SizeOfCellsPerGroup - 1;
    CPPUNIT_ASSERT_EQUAL(false, pInstance_->findUnusedCandidate(pInstance_->cells_[target]));
    auto expected = indexToCandidate(target+1);
    CPPUNIT_ASSERT_EQUAL(expected, pInstance_->cells_[target].candidates_);

    // inconsistent
    setAllCellsFullCandidates();
    setTopLineExceptRightest();
    pInstance_->cells_[Sudoku::SizeOfAllCells - 1].candidates_ = indexToCandidate(Sudoku::SizeOfCellsPerGroup);
    CPPUNIT_ASSERT_EQUAL(true, pInstance_->findUnusedCandidate(pInstance_->cells_[target]));
    expected = SudokuTestCandidates::Empty;
    CPPUNIT_ASSERT_EQUAL(expected, pInstance_->cells_[target].candidates_);

    // a column
    setAllCellsFullCandidates();
    for(SudokuIndex i=0;i<Sudoku::SizeOfCellsPerGroup - 1;++i) {
        pInstance_->cells_[Sudoku::SizeOfCellsPerGroup - 1 + Sudoku::SizeOfCellsPerGroup * i].
            candidates_ = indexToCandidate(i+1);
    }
    target = Sudoku::SizeOfAllCells - 1;
    CPPUNIT_ASSERT_EQUAL(false, pInstance_->findUnusedCandidate(pInstance_->cells_[target]));
    expected = indexToCandidate(Sudoku::SizeOfCellsPerGroup);
    CPPUNIT_ASSERT_EQUAL(expected, pInstance_->cells_[target].candidates_);

    // box (3*3)
    setAllCellsFullCandidates();
    for(SudokuIndex y=0;y<3;++y) {
        for(SudokuIndex x=0;x<3;++x) {
            if ((x != 1) || (y != 1)) {
                const SudokuIndex ofs = SudokuTestPosition::Centerorigin + x + y*Sudoku::SizeOfCellsPerGroup;
                pInstance_->cells_[ofs].candidates_ = indexToCandidate(x + y*3 + 1);
            }
        }
    }
    target = SudokuTestPosition::Center;
    CPPUNIT_ASSERT_EQUAL(false, pInstance_->findUnusedCandidate(pInstance_->cells_[target]));
    expected = SudokuTestCandidates::CenterOnly;
    CPPUNIT_ASSERT_EQUAL(expected, pInstance_->cells_[target].candidates_);

    // complex cases
    struct TestSet {
        SudokuIndex cellPosition;
        SudokuIndex candidateIndex;
    };
    // 9123
    // 47
    // 5.8
    // 6
    constexpr TestSet testSet[] {{1, 1}, {2, 2}, {3, 3}, {9, 4}, {18, 5}, {27, 6}, {10, 7}, {20, 8}};
    setAllCellsFullCandidates();
    for(const auto& test : testSet) {
        pInstance_->cells_[test.cellPosition].candidates_ = indexToCandidate(test.candidateIndex);
    }

    target = SudokuTestPosition::Head;
    CPPUNIT_ASSERT_EQUAL(false, pInstance_->findUnusedCandidate(pInstance_->cells_[target]));
    expected = indexToCandidate(Sudoku::SizeOfCellsPerGroup);
    CPPUNIT_ASSERT_EQUAL(expected, pInstance_->cells_[target].candidates_);

    // [4,6] 123
    // 9     7
    // 5    .8
    constexpr TestSet testSet2[] {{1, 1}, {2, 2}, {3, 3}, {18, 5}, {10, 7}, {20, 8}, {9, 9}};
    setAllCellsFullCandidates();
    for(const auto& test : testSet2) {
        pInstance_->cells_[test.cellPosition].candidates_ = indexToCandidate(test.candidateIndex);
    }
    target = SudokuTestPosition::Head;
    CPPUNIT_ASSERT_EQUAL(false, pInstance_->findUnusedCandidate(pInstance_->cells_[target]));
    expected = SudokuTestCandidates::FourAndSix;
    CPPUNIT_ASSERT_EQUAL(expected, pInstance_->cells_[target].candidates_);

    return;
}

void SudokuMapTest::test_findUniqueCandidate() {
    // a row
    setAllCellsFullCandidates();
    auto target = Sudoku::SizeOfCellsPerGroup - 1;
    for(SudokuIndex i=0;i<target;++i) {
        pInstance_->cells_[i].candidates_ = SudokuTestCandidates::TwoToNine;
    }
    CPPUNIT_ASSERT_EQUAL(false, pInstance_->findUniqueCandidate(pInstance_->cells_[target]));
    auto expected = SudokuTestCandidates::OneOnly;
    CPPUNIT_ASSERT_EQUAL(expected, pInstance_->cells_[target].candidates_);

    // This does not decrease candidates.
    setAllCellsFullCandidates();
    target = Sudoku::SizeOfCellsPerGroup - 1;
    pInstance_->cells_[SudokuTestPosition::Head].candidates_ = SudokuTestCandidates::OneOnly;
    pInstance_->cells_[SudokuTestPosition::HeadNext].candidates_ = SudokuTestCandidates::DoubleLine;
    CPPUNIT_ASSERT_EQUAL(false, pInstance_->findUniqueCandidate(pInstance_->cells_[target]));
    expected = SudokuTestCandidates::All;
    CPPUNIT_ASSERT_EQUAL(expected, pInstance_->cells_[target].candidates_);

    // column
    setAllCellsFullCandidates();
    for(SudokuIndex i=0;i<Sudoku::SizeOfCellsPerGroup - 1;++i) {
        pInstance_->cells_[Sudoku::SizeOfCellsPerGroup - 1 + Sudoku::SizeOfCellsPerGroup * i].
            candidates_ = SudokuTestCandidates::ExceptTwo;
    }
    target = Sudoku::SizeOfAllCells - 1;
    CPPUNIT_ASSERT_EQUAL(false, pInstance_->findUniqueCandidate(pInstance_->cells_[target]));
    expected = SudokuTestCandidates::TwoOnly;
    CPPUNIT_ASSERT_EQUAL(expected, pInstance_->cells_[target].candidates_);

    // box (3*3)
    setAllCellsFullCandidates();
    for(SudokuIndex y=0;y<3;++y) {
        for(SudokuIndex x=0;x<3;++x) {
            if ((x != 1) || (y != 1)) {
                const SudokuIndex ofs = SudokuTestPosition::Centerorigin + x + y*Sudoku::SizeOfCellsPerGroup;
                pInstance_->cells_[ofs].candidates_ = SudokuTestCandidates::ExceptCenter;
            }
        }
    }
    target = SudokuTestPosition::Center;
    CPPUNIT_ASSERT_EQUAL(false, pInstance_->findUniqueCandidate(pInstance_->cells_[target]));
    expected = SudokuTestCandidates::CenterOnly;
    CPPUNIT_ASSERT_EQUAL(expected, pInstance_->cells_[target].candidates_);

    // inconsistent
    setAllCellsFullCandidates();
    target = Sudoku::SizeOfCellsPerGroup - 1;
    for(SudokuIndex i=0;i<target;++i) {
        pInstance_->cells_[i].candidates_ = SudokuTestCandidates::DoubleLine;
    }
    CPPUNIT_ASSERT_EQUAL(true, pInstance_->findUniqueCandidate(pInstance_->cells_[target]));
    expected = SudokuTestCandidates::All;
    CPPUNIT_ASSERT_EQUAL(expected, pInstance_->cells_[target].candidates_);

    return;
}

// Call before running a test
void SudokuSseMapTest::setUp() {
    pInstance_ = decltype(pInstance_)(new SudokuSseMap());
    checkConstructor();
    return;
}

// Call after running a test
void SudokuSseMapTest::tearDown() {
    return;
}

void SudokuSseMapTest::checkConstructor(void) {
    static_assert(SudokuSse::RegisterCnt == 16, "Unexpected SudokuSse::RegisterCnt value");
    static_assert(SudokuSse::RegisterWordCnt == 4, "Unexpected SudokuSse::RegisterWordCnt value");
    assert(pInstance_->InitialRegisterNum == 1);

    constexpr SudokuSseElement expectedRegVal[] {
        0, 0, 0, 0,
        0x7ffffff, 0x7ffffff, 0x7ffffff, 0, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0,
        0x7ffffff, 0x7ffffff, 0x7ffffff, 0, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0,
        0x7ffffff, 0x7ffffff, 0x7ffffff, 0, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    static_assert(sizeof(expectedRegVal) == sizeof(pInstance_->xmmRegSet_), "Unexpected pInstance_->xmmRegSet_ size");

    const auto actual = memcmp(expectedRegVal, &pInstance_->xmmRegSet_, sizeof(expectedRegVal));
    constexpr decltype(actual) expected = 0;
    CPPUNIT_ASSERT_EQUAL(expected, actual);
    return;
}

void SudokuSseMapTest::setAllCellsFilled(void) {
    SudokuTest::SetAllCellsFilled(SudokuSseMap::InitialRegisterNum, pInstance_->xmmRegSet_);
    return;
}

void SudokuSseMapTest::setAllCellsFullCandidates(void) {
    SudokuTest::SetAllCellsFullCandidates(SudokuSseMap::InitialRegisterNum, pInstance_->xmmRegSet_);
    return;
}

void SudokuSseMapTest::checkCandidateCell(const SudokuSseCandidateCell& actual,
                                          decltype(SudokuSseCandidateCell::regIndex) regIndex,
                                          decltype(SudokuSseCandidateCell::shift) inBoxShift) {
    CPPUNIT_ASSERT_EQUAL(regIndex, actual.regIndex);
    const decltype(actual.shift) shift = Sudoku::SizeOfCandidates * inBoxShift;
    CPPUNIT_ASSERT_EQUAL(shift, actual.shift);
    const decltype(actual.mask) mask = Sudoku::AllCandidates << shift;
    CPPUNIT_ASSERT_EQUAL(mask, actual.mask);
    return;
}

void SudokuSseMapTest::test_Preset() {
    const std::string presetStr = "1234567890\a\b\n\r\f\t\v0987654321 !\"#$%&\'(9*+,5./:1<=>?@[\\]^_`{|}~.abcdefxyzAB*D7FXYYZ*";
//                                 12345678912 3 4 5 6 7 8 9123456789123 45678 91234567891234567 89123456789123456789123456789
    constexpr SudokuSseElement expectedRegVal[] {
        0, 0, 0, 0,
        0x1010100, 0x0202020, 0x0040404, 0, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0, 0x0100401, 0x0802008, 0x4010040, 0,
        0x7ffffff, 0x7ffffff, 0x7ffffff, 0, 0x7fffe01, 0x7fc21ff, 0x403ffff, 0, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0,
        0x7ffffff, 0x7ffffff, 0x7ffffff, 0, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0, 0x7ffffff, 0x7ffffff, 0x7fffe40, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    static_assert(sizeof(expectedRegVal) == sizeof(pInstance_->xmmRegSet_), "Unexpected pInstance_->xmmRegSet_ size");

    pInstance_->Preset(presetStr);

    const auto actual = memcmp(expectedRegVal, &pInstance_->xmmRegSet_, sizeof(expectedRegVal));
    constexpr decltype(actual) expected = 0;
    CPPUNIT_ASSERT_EQUAL(expected, actual);

    return;
}

void SudokuSseMapTest::test_Print() {
    SudokuCellCandidates candidates = 0x40201;
    size_t regIndex = pInstance_->InitialRegisterNum * SudokuSse::RegisterWordCnt;
    size_t expected = 0;  // expected length of output strings

    for(SudokuIndex y=0; y < Sudoku::SizeOfCellsPerGroup; ++y) {
        for(SudokuIndex x=0; x < Sudoku::SizeOfBoxesOnEdge; ++x) {
            // Sets three cells at once
            pInstance_->xmmRegSet_.regVal_[regIndex++] = candidates;
            expected += 3 * (y + 1);
        }
        ++regIndex;
        candidates = (candidates << 1) | candidates;  // Sets a unique candidate one by one
        expected += Sudoku::SizeOfCellsPerGroup;      // Count delimiters
        ++expected;  // Count a linefeed
    }

    // Count the last linefeed
    ++expected;

    SudokuOutStream sudokuOutStream;
    pInstance_->Print(&sudokuOutStream);
    CPPUNIT_ASSERT_EQUAL(expected, sudokuOutStream.str().length());

    // Missing output stream
    pInstance_->Print(nullptr);
    return;
}

void SudokuSseMapTest::test_FillCrossing() {}

void SudokuSseMapTest::test_GetNextCell() {
    struct TestSet {
        bool      found;
        gRegister outBoxIndex;
        gRegister inBoxIndex;
        gRegister rowNumber;
        size_t           expectedRegIndex;
        SudokuSseElement expectedShift;
    };

    constexpr TestSet testSet[] = {
        {false, 0xffff, 0xffff, 0xffff, 0, 0},
        {true, 1, 0, 0, SudokuSseMap::InitialRegisterNum * SudokuSse::RegisterWordCnt + 1, 0},
        {true, 0, 1, 0, SudokuSseMap::InitialRegisterNum * SudokuSse::RegisterWordCnt, 1},
        {true, 0, 0, 1, (SudokuSseMap::InitialRegisterNum + 1) * SudokuSse::RegisterWordCnt, 0},
        {true, 1, 2, 7, (SudokuSseMap::InitialRegisterNum + 7) * SudokuSse::RegisterWordCnt + 1, 2},
        {true, 2, 1, 8, (SudokuSseMap::InitialRegisterNum + 8) * SudokuSse::RegisterWordCnt + 2, 1},
    };

    for(const auto& test : testSet) {
        SudokuSseMapResult result;
        result.aborted = 0;
        result.elementCnt = 0;
        result.nextCellFound = (test.found) ?  1 : 0;
        result.nextOutBoxIndex = test.outBoxIndex;
        result.nextInBoxIndex = test.inBoxIndex;
        result.nextRowNumber = test.rowNumber;

        SudokuSseCandidateCell cell = {0, 0, 0};
        CPPUNIT_ASSERT_EQUAL(test.found, pInstance_->GetNextCell(result, cell));
        if (test.found) {
            checkCandidateCell(cell, test.expectedRegIndex, test.expectedShift);
        }
    }
}

void SudokuSseMapTest::test_CanSetUniqueCell() {
    constexpr size_t regIndex = 0;
    SudokuCellCandidates candidate = 1;
    SudokuSseCandidateCell cell = {regIndex, 0, Sudoku::AllCandidates};

    for(SudokuIndex i=0; i < Sudoku::SizeOfCandidates; ++i) {
        pInstance_->xmmRegSet_.regVal_[regIndex] = candidate | (1 << i);
        CPPUNIT_ASSERT_EQUAL(true, pInstance_->CanSetUniqueCell(cell, candidate));
    }

    candidate = 0x100;
    for(SudokuIndex i=0; i < (Sudoku::SizeOfCandidates - 1); ++i) {
        SudokuSseElement value = 3 | (1 << i);
        pInstance_->xmmRegSet_.regVal_[regIndex] = value;
        CPPUNIT_ASSERT_EQUAL(false, pInstance_->CanSetUniqueCell(cell, candidate));
    }

    candidate = 0x80;
    cell = {regIndex, Sudoku::SizeOfCandidates, Sudoku::AllCandidates << Sudoku::SizeOfCandidates};
    pInstance_->xmmRegSet_.regVal_[regIndex] = candidate * 3;
    CPPUNIT_ASSERT_EQUAL(false, pInstance_->CanSetUniqueCell(cell, candidate));
    pInstance_->xmmRegSet_.regVal_[regIndex] <<= Sudoku::SizeOfCandidates;
    CPPUNIT_ASSERT_EQUAL(true, pInstance_->CanSetUniqueCell(cell, candidate));
}

void SudokuSseMapTest::test_SetUniqueCell() {
    constexpr size_t regIndex = 0;
    SudokuCellCandidates candidate = 1;
    SudokuSseCandidateCell cell = {regIndex, 0, Sudoku::AllCandidates};

    for(SudokuIndex i=0; i < Sudoku::SizeOfCandidates; ++i) {
        pInstance_->xmmRegSet_.regVal_[regIndex] = candidate | (1 << i);
        pInstance_->SetUniqueCell(cell, candidate);
        CPPUNIT_ASSERT_EQUAL(candidate, pInstance_->xmmRegSet_.regVal_[regIndex]);
    }

    candidate = 0x100;
    for(SudokuIndex i=0; i < (Sudoku::SizeOfCandidates - 1); ++i) {
        SudokuSseElement value = 3 | (1 << i);
        pInstance_->xmmRegSet_.regVal_[regIndex] = value;
        pInstance_->SetUniqueCell(cell, candidate);
        CPPUNIT_ASSERT_EQUAL(candidate, pInstance_->xmmRegSet_.regVal_[regIndex]);
    }

    candidate = 0x80;
    const SudokuSseCandidateCell filledCell = {regIndex, Sudoku::SizeOfCandidates,
                                               Sudoku::AllCandidates << Sudoku::SizeOfCandidates};

    cell = filledCell;
    SudokuCellCandidates cellValue = candidate * 3;
    SudokuCellCandidates expectedValue = cellValue | (candidate << Sudoku::SizeOfCandidates);
    pInstance_->xmmRegSet_.regVal_[regIndex] = expectedValue;
    pInstance_->SetUniqueCell(cell, candidate);
    CPPUNIT_ASSERT_EQUAL(expectedValue, pInstance_->xmmRegSet_.regVal_[regIndex]);

    cell = filledCell;
    pInstance_->xmmRegSet_.regVal_[regIndex] = cellValue << Sudoku::SizeOfCandidates;
    pInstance_->SetUniqueCell(cell, candidate);
    CPPUNIT_ASSERT_EQUAL(candidate << Sudoku::SizeOfCandidates, pInstance_->xmmRegSet_.regVal_[regIndex]);
}

/*
Local Variables:
mode: c++
coding: utf-8-unix
tab-width: nil
c-file-style: "stroustrup"
End:
*/
