// Testing assmebly code
// Copyright (C) 2012-2018 Zettsu Tatsuya
//
// I use CppUnit code on the website.
// http://www.atmarkit.co.jp/fdotnet/cpptest/cpptest02/cpptest02_03.html

#include <cppunit/extensions/HelperMacros.h>
#include <cassert>
#include <cstring>
#include <memory>
#include "sudoku.h"
#include "sudokutest.h"

class SudokuSseEnumeratorMapTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(SudokuSseEnumeratorMapTest);
    CPPUNIT_TEST(test_Constructor);
    CPPUNIT_TEST(test_SetToPrint);
    CPPUNIT_TEST(test_presetCell);
    CPPUNIT_TEST(test_Preset);
    CPPUNIT_TEST(test_Print);
    CPPUNIT_TEST(test_PrintFromAsm);
    CPPUNIT_TEST(test_Enumerate);
    CPPUNIT_TEST(test_GetInstance);
    CPPUNIT_TEST(test_powerOfTwoPlusOne);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;
protected:
    void test_Constructor();
    void test_SetToPrint();
    void test_presetCell();
    void test_Preset();
    void test_Print();
    void test_PrintFromAsm();
    void test_Enumerate();
    void test_GetInstance();
    void test_powerOfTwoPlusOne();
private:
    std::unique_ptr<SudokuSseEnumeratorMap> pInstance_;
    std::unique_ptr<SudokuOutStream> pSudokuOutStream_;
};

CPPUNIT_TEST_SUITE_REGISTRATION( SudokuSseEnumeratorMapTest );

class SudokuSseTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(SudokuSseTest);
    CPPUNIT_TEST(test_Compare);
    CPPUNIT_TEST(test_PowerOf2);
    CPPUNIT_TEST(test_MergeThreeElements);
    CPPUNIT_TEST(test_OrThreeXmmRegs);
    CPPUNIT_TEST(test_FilterUniqueCandidatesInRowPartSub);
    CPPUNIT_TEST(test_FilterUniqueCandidatesInRowPart);
    CPPUNIT_TEST(test_CollectUniqueCandidatesInRowPart);
    CPPUNIT_TEST(test_CollectUniqueCandidatesInLine);
    CPPUNIT_TEST(test_CollectUniqueCandidatesInThreeLine);
    CPPUNIT_TEST(test_SelectRowParts);
    CPPUNIT_TEST(test_SelectElementInRowParts);
    CPPUNIT_TEST(test_FillUniqueElement);
    CPPUNIT_TEST(test_FillOneUniqueCandidates);
    CPPUNIT_TEST(test_SaveLineSet);
    CPPUNIT_TEST(test_MaskElementCandidates);
    CPPUNIT_TEST(test_MergeTwoElements);
    CPPUNIT_TEST(test_FindElementCandidates);
    CPPUNIT_TEST(test_FindThreePartsCandidates);
    CPPUNIT_TEST(test_FillThreePartsUniqueCandidates);
    CPPUNIT_TEST(test_FindRowPartCandidates);
    CPPUNIT_TEST(test_MergeThreeDiagonalElements);
    CPPUNIT_TEST(test_MergeNineDiagonalElements);
    CPPUNIT_TEST(test_FindUnusedOneThreeDiagonalElements);
    CPPUNIT_TEST(test_FindUnusedNineDiagonalElements);
    CPPUNIT_TEST(test_CountFilledElements);
    CPPUNIT_TEST(test_PopCountOrPowerOf2);
    CPPUNIT_TEST(test_CompareRegisterSet);
    CPPUNIT_TEST(test_CountCellCandidates);
    CPPUNIT_TEST(test_CountThreeCellCandidates);
    CPPUNIT_TEST(test_CountRowCellCandidatesSub);
    CPPUNIT_TEST(test_CountRowCellCandidates);
    CPPUNIT_TEST(test_SearchNextCandidate);
    CPPUNIT_TEST(test_SearchNextCandidateParam);
    CPPUNIT_TEST(test_FoldRowParts);
    CPPUNIT_TEST(test_CheckRow);
    CPPUNIT_TEST(test_CheckRowSet);
    CPPUNIT_TEST(test_CheckColumn);
    CPPUNIT_TEST(test_CheckBox);
    CPPUNIT_TEST(test_CheckSetBox);
    CPPUNIT_TEST(test_CheckDiagonalThreeCells);
    CPPUNIT_TEST(test_CheckDiagonalNineCells);
    CPPUNIT_TEST(test_CheckConsistency);
    CPPUNIT_TEST(test_FastCollectCandidatesAtRow);
    CPPUNIT_TEST(test_FastCollectCandidatesAtBox);
    CPPUNIT_TEST(test_FastCollectCandidatesAtColumn);
    CPPUNIT_TEST(test_FastSetUniqueCandidatesAtCellSub);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;
protected:
    void test_Compare();
    void test_PowerOf2();
    void test_MergeThreeElements();
    void test_OrThreeXmmRegs();
    void test_FilterUniqueCandidatesInRowPartSub();
    void test_FilterUniqueCandidatesInRowPart();
    void test_CollectUniqueCandidatesInRowPart();
    void test_CollectUniqueCandidatesInLine();
    void test_CollectUniqueCandidatesInThreeLine();
    void test_SelectRowParts();
    void test_SelectElementInRowParts();
    void test_FillUniqueElement();
    void test_FillOneUniqueCandidates();
    void test_SaveLineSet();
    void test_MaskElementCandidates();
    void test_MergeTwoElements();
    void test_FindElementCandidates();
    void test_FindThreePartsCandidates();
    void test_FillThreePartsUniqueCandidates();
    void test_FindRowPartCandidates();
    void test_MergeThreeDiagonalElements();
    void test_MergeNineDiagonalElements();
    void test_FindUnusedOneThreeDiagonalElements();
    void test_FindUnusedNineDiagonalElements();
    void test_CountFilledElements();
    void test_PopCountOrPowerOf2();
    void test_CompareRegisterSet();
    void test_CountCellCandidates();
    void test_CountThreeCellCandidates();
    void test_CountRowCellCandidatesSub();
    void test_CountRowCellCandidates();
    void test_SearchNextCandidate();
    void test_SearchNextCandidateParam();
    void test_FoldRowParts();
    void test_CheckRow();
    void test_CheckRowSet();
    void test_CheckColumn();
    void test_CheckBox();
    void test_CheckSetBox();
    void test_CheckDiagonalThreeCells();
    void test_CheckDiagonalNineCells();
    void test_CheckConsistency();
    void test_FastCollectCandidatesAtRow();
    void test_FastCollectCandidatesAtBox();
    void test_FastCollectCandidatesAtColumn();
    void test_FastSetUniqueCandidatesAtCellSub();
private:
    static constexpr size_t SizeOfRowSet = Sudoku::SizeOfGroupsPerMap;
    union RowRegisterSet {
        SudokuSseElement regVal[SizeOfRowSet * 4];
        xmmRegister regXmm[SizeOfRowSet];
    };
    static_assert(sizeof(RowRegisterSet) == sizeof(RowRegisterSet::regXmm), "Wrong size");

    void callIsPowerOf2ToFlags(gRegister& arg, gRegister& result);
    void callPowerOf2or0_64(gRegister& arg, gRegister& result);
    void callPowerOf2or0_32(gRegister& arg, gRegister& result);
    void callPowerOf2orAll1_64(gRegister& arg, gRegister& result);
    void callPowerOf2orAll1_32(gRegister& arg, gRegister& result);
    gRegister rotateRowPart(gRegister arg);
    void rotateRow(uint64_t row[2]);
    void checkSearchNextCandidate(XmmRegisterSet& xmmRegSet, gRegister expectedFound,
                                  gRegister expectedOutBoxShift, gRegister expectedInBoxShift,
                                  gRegister expectedRowNumber);

    // 111111111 111111110 111111100
    // 111 1111 1111 1111 1101 1111 1100
    static constexpr gRegister CellSet987 = 0x7fffdfc;

    // 111111000 111110000 111100000
    // 111 1110 0011 1110 0001 1110 0000
    static constexpr gRegister CellSet654 = 0x7e3e1e0;

    // 111110000 111100000 111111000
    // 111 1100 0011 1100 0001 1111 1000
    static constexpr gRegister CellSet546 = 0x7c3c1f8;

    // 111100000 111111000 111110000
    // 111 1000 0011 1111 0001 1111 0000
    static constexpr gRegister CellSet456 = 0x783f1f0;
    static constexpr gRegister CellSet401 = 0x7800001;

    static constexpr gRegister InvalidRowPopCount = Sudoku::SizeOfCellsPerGroup * Sudoku::SizeOfCandidates + 1;
    static constexpr gRegister InvalidShift = 0xffff;
    static constexpr gRegister InvalidRowNumber = 0xfffe;
    static constexpr gRegister CountCellCandidatesRowNumber = 5;

    // Number of 32-bit registers in a XMM register
    static constexpr size_t RegIndexStep = sizeof(xmmRegister) / sizeof(SudokuSseElement);
};

CPPUNIT_TEST_SUITE_REGISTRATION( SudokuSseTest );

void SudokuSseEnumeratorMapTest::setUp() {
    pSudokuOutStream_ = decltype(pSudokuOutStream_)(new SudokuOutStream());
    pInstance_ = decltype(pInstance_)(new SudokuSseEnumeratorMap(pSudokuOutStream_.get()));
}

void SudokuSseEnumeratorMapTest::tearDown() {
    assert(pInstance_);
    assert(pSudokuOutStream_);
    pInstance_.reset();
    pSudokuOutStream_.reset();
}

void SudokuSseEnumeratorMapTest::test_Constructor() {
    do {
        CPPUNIT_ASSERT_EQUAL(static_cast<SudokuCellCandidates>(0), pInstance_->rightBottomElement_);
        CPPUNIT_ASSERT_EQUAL(static_cast<gRegister>(0), pInstance_->firstCell_);
        CPPUNIT_ASSERT_EQUAL(static_cast<SudokuPatternCount>(0), pInstance_->patternNumber_);
        CPPUNIT_ASSERT_EQUAL(pInstance_.get(), pInstance_->pInstance_);

        for(const auto& regVal : pInstance_->xmmRegSet_.regVal_) {
            CPPUNIT_ASSERT_EQUAL(static_cast<SudokuSseElement>(0), regVal);
        }

        const uint64_t actual = sudokuXmmPrintAllCandidate;
        constexpr decltype(actual) expected = 0ull;
        CPPUNIT_ASSERT_EQUAL(expected, actual);
    } while(0);

    CPPUNIT_ASSERT_EQUAL(pInstance_.get(), SudokuSseEnumeratorMap::pInstance_);
}

void SudokuSseEnumeratorMapTest::test_SetToPrint() {
    constexpr SudokuPatternCount testSet[] {0, 1, 0xffffffff, 0x1000000000ull, 0xffffffffffffffffull};

    for(const auto& expected : testSet) {
        pInstance_->SetToPrint(expected);
        const auto actual = sudokuXmmPrintAllCandidate;
        CPPUNIT_ASSERT_EQUAL(expected, actual);
    }
}

void SudokuSseEnumeratorMapTest::test_presetCell() {
    struct TestSet {
        size_t pos;
        size_t shift;
    };

    constexpr TestSet testSet[] {
        {4,  0}, {4,  16}, {5,  0}, {5,  16}, {6,  0}, {6,  16}, {7,  0}, {7,  16}, {40, 0},
        {8,  0}, {8,  16}, {9,  0}, {9,  16}, {10, 0}, {10, 16}, {11, 0}, {11, 16}, {40, 16},
        {12, 0}, {12, 16}, {13, 0}, {13, 16}, {14, 0}, {14, 16}, {15, 0}, {15, 16}, {41, 0},
        {16, 0}, {16, 16}, {17, 0}, {17, 16}, {18, 0}, {18, 16}, {19, 0}, {19, 16}, {41, 16},
        {20, 0}, {20, 16}, {21, 0}, {21, 16}, {22, 0}, {22, 16}, {23, 0}, {23, 16}, {42, 0},
        {24, 0}, {24, 16}, {25, 0}, {25, 16}, {26, 0}, {26, 16}, {27, 0}, {27, 16}, {42, 16},
        {28, 0}, {28, 16}, {29, 0}, {29, 16}, {30, 0}, {30, 16}, {31, 0}, {31, 16}, {43, 0},
        {32, 0}, {32, 16}, {33, 0}, {33, 16}, {34, 0}, {34, 16}, {35, 0}, {35, 16}, {43, 16},
        {36, 0}, {36, 16}, {37, 0}, {37, 16}, {38, 0}, {38, 16}, {39, 0}, {39, 16}};

    for(SudokuLoopIndex num=0; num<=Sudoku::SizeOfCandidates; ++num) {
        for(SudokuLoopIndex cellIndex=0; cellIndex<arraySizeof(testSet); ++cellIndex) {
            const auto test = testSet[cellIndex];
            SudokuSseElement expected = (num) ? (1 << (test.shift + num - 1)) : 0;

            SudokuOutStream sudokuOutStream;
            SudokuSseEnumeratorMap map(&sudokuOutStream);
            map.presetCell(cellIndex, num);
            CPPUNIT_ASSERT_EQUAL(expected, map.xmmRegSet_.regVal_[test.pos]);
        }

        SudokuOutStream sudokuOutStream;
        SudokuSseEnumeratorMap map(&sudokuOutStream);
        map.presetCell(Sudoku::SizeOfAllCells - 1, num);
        SudokuSseElement expected = (num) ? (1 << (num - 1)) : 0;
        CPPUNIT_ASSERT_EQUAL(expected, map.rightBottomElement_);
    }
}

struct EnumeratorTestCase {
    const char* arg;
    gRegister firstCell;
    SudokuCellCandidates rightBottomElement;
    SudokuSseElement expextedRegVal[10 * SudokuSse::RegisterWordCnt];
    const char* expctedPrintSolved;
};

constexpr EnumeratorTestCase EnumeratorTestCaseSet[] {
    {"", 0, 0, {0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
                0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
                0, 0, 0, 0,  0, 0, 0, 0},
     "0:0:0:0:0:0:0:0:0\n0:0:0:0:0:0:0:0:0\n0:0:0:0:0:0:0:0:0\n0:0:0:0:0:0:0:0:0\n0:0:0:0:0:0:0:0:0\n0:0:0:0:0:0:0:0:0\n0:0:0:0:0:0:0:0:0\n0:0:0:0:0:0:0:0:0\n0:0:0:0:0:0:0:0:0\n",
    },
    {"9", 1, 0, {0x100, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
                 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
                 0, 0, 0, 0,  0, 0, 0, 0},
     "9:0:0:0:0:0:0:0:0\n0:0:0:0:0:0:0:0:0\n0:0:0:0:0:0:0:0:0\n0:0:0:0:0:0:0:0:0\n0:0:0:0:0:0:0:0:0\n0:0:0:0:0:0:0:0:0\n0:0:0:0:0:0:0:0:0\n0:0:0:0:0:0:0:0:0\n0:0:0:0:0:0:0:0:0\n"
    },
    {"000000000000000000000000000000000000000000000000000000000000000000000000000000009",
     0, 0x100, {0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
                0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
                0, 0, 0, 0,  0, 0, 0, 0},
     "0:0:0:0:0:0:0:0:0\n0:0:0:0:0:0:0:0:0\n0:0:0:0:0:0:0:0:0\n0:0:0:0:0:0:0:0:0\n0:0:0:0:0:0:0:0:0\n0:0:0:0:0:0:0:0:0\n0:0:0:0:0:0:0:0:0\n0:0:0:0:0:0:0:0:0\n0:0:0:0:0:0:0:0:9\n",
    },
    // The solution of quiz 2 in the book
    //         |        |        |        |        |        |        |        |
    {"835126749416397258792548631643912875981754362527683194269835417178469523354271986",
     81, 0x20, {0x0040080, 0x0010010, 0x0200002, 0x0080040,
                0x0010008, 0x0040020, 0x0400100, 0x0100002,
                0x1000040, 0x0100002, 0x0800008, 0x0040020,
                0x0080020, 0x1000004, 0x0020001, 0x0400080,
                0x0800100, 0x0400001, 0x0080010, 0x0200004,
                0x0020010, 0x0200040, 0x0040080, 0x1000001,
                0x0200002, 0x0800100, 0x0100004, 0x0010008,
                0x0400001, 0x0080080, 0x1000020, 0x0020010,
                0x0100004, 0x0020008, 0x0010040, 0x0800100,
                0x0800100, 0x0100001, 0x0080002, 0x0040040},
     "8:3:5:1:2:6:7:4:9\n4:1:6:3:9:7:2:5:8\n7:9:2:5:4:8:6:3:1\n6:4:3:9:1:2:8:7:5\n9:8:1:7:5:4:3:6:2\n5:2:7:6:8:3:1:9:4\n2:6:9:8:3:5:4:1:7\n1:7:8:4:6:9:5:2:3\n3:5:4:2:7:1:9:8:6\n",
    },
};

void SudokuSseEnumeratorMapTest::test_Preset() {
    for(const auto& test : EnumeratorTestCaseSet) {
        SudokuOutStream sudokuOutStream;
        SudokuSseEnumeratorMap map(&sudokuOutStream);

        map.Preset(test.arg);
        CPPUNIT_ASSERT_EQUAL(test.firstCell, map.firstCell_);
        CPPUNIT_ASSERT_EQUAL(test.rightBottomElement, map.rightBottomElement_);
        CPPUNIT_ASSERT(sizeof(test.expextedRegVal) < sizeof(map.xmmRegSet_));

        constexpr size_t offset = SudokuSseEnumeratorMap::InitialRegisterNum * SudokuSse::RegisterWordCnt;
        for(size_t j=0; j <arraySizeof(test.expextedRegVal); ++j) {
            if (j < sizeof(map.xmmRegSet_)) {
                CPPUNIT_ASSERT_EQUAL(test.expextedRegVal[j], map.xmmRegSet_.regVal_[j + offset]);
            }
        }
    }
}

void SudokuSseEnumeratorMapTest::test_Print() {
    for(const auto& test : EnumeratorTestCaseSet) {
        SudokuOutStream sudokuOutStream;
        SudokuSseEnumeratorMap map(&sudokuOutStream);
        XmmRegisterSet xmmRegSet;
        ::memset(&xmmRegSet, 0, sizeof(xmmRegSet));

        static_assert(sizeof(test.expextedRegVal) < sizeof(map.xmmRegSet_),
                      "Unexpected test.expextedRegVal and map.xmmRegSet_ size");
        ::memmove(&(xmmRegSet.regXmmVal_[SudokuSseEnumeratorMap::InitialRegisterNum]),
                  test.expextedRegVal, sizeof(test.expextedRegVal));
        sudokuXmmRightBottomSolved = test.rightBottomElement;

        map.Print(true, xmmRegSet);
        std::string expected(test.expctedPrintSolved);
        std::string actual(sudokuOutStream.str());
        CPPUNIT_ASSERT(expected == actual);

        map.rightBottomElement_ = test.rightBottomElement;
        auto pos = expected.find_last_of(':');
        CPPUNIT_ASSERT(pos != std::string::npos);
        if (pos == std::string::npos) {
            continue;
        }

        do {
            SudokuOutStream sudokuOutStream;
            if (!test.rightBottomElement) {
                expected[pos + 1] = '0';
            }
            map.Print(false, xmmRegSet);
            CPPUNIT_ASSERT(expected == actual);
        } while(0);

        do {
            SudokuOutStream sudokuOutStream;
            map.xmmRegSet_ = xmmRegSet;
            map.Print();
            CPPUNIT_ASSERT(expected == actual);
        } while(0);
    }
}

void SudokuSseEnumeratorMapTest::test_PrintFromAsm() {
    constexpr SudokuPatternCount maxCount = 100;
    XmmRegisterSet xmmRegSet;

    for(SudokuPatternCount i = maxCount - 1; i < maxCount + 2; ++i) {
        ::memset(&xmmRegSet, 0, sizeof(xmmRegSet));
        SudokuOutStream sudokuOutStream;
        SudokuSseEnumeratorMap map(&sudokuOutStream);
        map.patternNumber_ = i;
        sudokuXmmPrintAllCandidate = maxCount;

        map.PrintFromAsm(xmmRegSet);
        if (i >= maxCount) {
            CPPUNIT_ASSERT(sudokuOutStream.str().size() == 0);
        } else {
            CPPUNIT_ASSERT(sudokuOutStream.str().size() > 0);
        }
    }

    for(const auto& test : EnumeratorTestCaseSet) {
        SudokuOutStream sudokuOutStream;
        SudokuSseEnumeratorMap map(&sudokuOutStream);
        map.patternNumber_ = 9875;
        sudokuXmmPrintAllCandidate = 10000;

        static_assert(sizeof(test.expextedRegVal) < sizeof(xmmRegSet), "Unexpected test.expextedRegVal and xmmRegSet");
        ::memmove(&(xmmRegSet.regXmmVal_[SudokuSseEnumeratorMap::InitialRegisterNum]),
                  test.expextedRegVal, sizeof(test.expextedRegVal));
        sudokuXmmRightBottomSolved = test.rightBottomElement;
        map.PrintFromAsm(xmmRegSet);

        std::string expected("[Solution 9876]\n");
        expected.append(test.expctedPrintSolved);
        std::string actual(sudokuOutStream.str());
        CPPUNIT_ASSERT(expected == actual);
    }
}

void SudokuSseEnumeratorMapTest::test_Enumerate() {
    constexpr SudokuPatternCount expectedSet[] {1, 1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,2,2,4, 4,4,4,4,8,12,48,96,288};
    std::string presetStr = "835126749416397258792548631643912875981754362527683194269835417178469523354271986";

    for(const auto& expected : expectedSet) {
        SudokuOutStream sudokuOutStream;
        SudokuSseEnumeratorMap map(&sudokuOutStream);
        map.Preset(presetStr);
        CPPUNIT_ASSERT_EQUAL(expected, map.Enumerate());
        presetStr.erase(presetStr.size() - 1);
    }
}

void SudokuSseEnumeratorMapTest::test_GetInstance() {
    CPPUNIT_ASSERT_EQUAL(pInstance_.get(), SudokuSseEnumeratorMap::GetInstance());
}

void SudokuSseEnumeratorMapTest::test_powerOfTwoPlusOne() {
    struct TestSet {
        SudokuSseElement arg;
        size_t expected;
    };

    constexpr TestSet testSet[] {
        {1, 1}, {2, 2}, {4, 3}, {8, 4},
        {0x10, 5}, {0x20, 6}, {0x40, 7}, {0x80, 8}, {0x100, 9}};

    for(const auto& test : testSet) {
        CPPUNIT_ASSERT_EQUAL(test.expected, pInstance_->powerOfTwoPlusOne(test.arg));
    }
}

// Call before running a test
void SudokuSseTest::setUp()
{
    return;
}

// Call after running a test
void SudokuSseTest::tearDown()
{
    return;
}

// Test cases
enum class Shiftpos {
    SHIFT_POSMIN,
    SHIFT_POS0 = SHIFT_POSMIN,
    SHIFT_POS1,
    SHIFT_POS2,
    SHIFT_POSCNT,
};

gRegister SudokuSseTest::rotateRowPart(gRegister arg)
{
    const gRegister argleftmost = (arg >> 18) & 0x1ff;
    return ((arg & 0x3ffff) << 9) | argleftmost;
}

void SudokuSseTest::rotateRow(uint64_t row[2])
{
    uint64_t newrow1 = row[0] >> 16;
    newrow1 >>= 16;;
    uint64_t newrow0 = row[0] << 16;
    newrow0 <<= 16;
    newrow0 |= row[1];

    row[0] = newrow0;
    row[1] = newrow1;
    return;
}

void SudokuSseTest::test_Compare()
{
    struct TestSet {
        gRegister right;
        gRegister left;
        gRegister expectedLE;
        gRegister expectedG;
        gRegister expectedGE;
    };

    constexpr TestSet testSet[] {
        {0, 0x8000000000000000ull, 1, 0, 0},
        {0, 0xffffffffffffffffull, 1, 0, 0},
        {0, 0, 1, 0, 1},
        {0, 1, 0, 1, 1},
        {0, 0x7fffffffffffffffull, 0, 1, 1},
        {0xffffffff, 0x8000000000000000ull, 1, 0, 0},
        {0xffffffff, 0xffffffffffffffffull, 1, 0, 0},
        {0xffffffff, 0, 1, 0, 0},
        {0xffffffff, 0xfffffffe, 1, 0, 0},
        {0xffffffff, 0xffffffff, 1, 0, 1},
        {0xffffffff, 0x100000000ull, 0, 1, 1},
        {0xffffffff, 0x7fffffffffffffffull, 0, 1, 1},
        {0xffffffffffffffffull, 0x8000000000000000ull, 1, 0, 0},
        {0xffffffffffffffffull, 0xfffffffffffffffeull, 1, 0, 0},
        {0xffffffffffffffffull, 0, 0, 1, 1},
        {0xffffffffffffffffull, 0x7fffffffffffffffull, 0, 1, 1},
        {0x8000000000000000ull, 0x8000000000000001ull, 0, 1, 1},
        {0x8000000000000000ull, 0xffffffffffffffffull, 0, 1, 1},
        {0x8000000000000000ull, 0, 0, 1, 1},
        {0x8000000000000000ull, 0x7fffffffffffffffull,  0, 1, 1}
    };

    for(const auto& test : testSet) {
        gRegister actual = 2;
        asm volatile (
            "call testCmovLessEqual\n\t"
            :"=a"(actual):"b"(test.left),"c"(test.right):"rdx");
        CPPUNIT_ASSERT_EQUAL(test.expectedLE, actual);

        actual = 2;
        asm volatile (
            "call testJumpIfGreater\n\t"
            :"=a"(actual):"b"(test.left),"c"(test.right):"rdx");
        CPPUNIT_ASSERT_EQUAL(test.expectedG, actual);

        actual = 2;
        asm volatile (
            "call testJumpIfGreaterEqual\n\t"
            :"=a"(actual):"b"(test.left),"c"(test.right):"rdx");
        CPPUNIT_ASSERT_EQUAL(test.expectedGE, actual);

        actual = 2;
        asm volatile (
            "call testJumpIfLessEqual\n\t"
            :"=a"(actual):"b"(test.left),"c"(test.right):"rdx");
        CPPUNIT_ASSERT_EQUAL(test.expectedLE, actual);
    }
}

void SudokuSseTest::callIsPowerOf2ToFlags(gRegister& arg, gRegister& result)
{
    asm volatile (
        "call testIsPowerOf2ToFlags\n\t"
        :"=b"(result):"a"(arg):"rcx");
    return;
}

void SudokuSseTest::callPowerOf2or0_64(gRegister& arg, gRegister& result)
{
    asm volatile (
        "call testPowerOf2or064\n\t"
        :"=b"(result):"a"(arg):"rcx");
    return;
}

void SudokuSseTest::callPowerOf2or0_32(gRegister& arg, gRegister& result)
{
    asm volatile (
        "call testPowerOf2or032\n\t"
        :"=b"(result):"a"(arg):"rcx");
    return;
}

void SudokuSseTest::callPowerOf2orAll1_64(gRegister& arg, gRegister& result)
{
    asm volatile (
        "call testPowerOf2orAll164\n\t"
        :"=b"(result):"a"(arg):"rcx");
    return;
}

void SudokuSseTest::callPowerOf2orAll1_32(gRegister& arg, gRegister& result)
{
    asm volatile (
        "call testPowerOf2orAll132\n\t"
        :"=b"(result):"a"(arg):"rcx");
    return;
}

void SudokuSseTest::test_PowerOf2()
{
    constexpr gRegister expectedAll0 = 0;
    constexpr gRegister expectedAll1_64 = ~0;
    constexpr gRegister expectedAll1_32 = 0xffffffff;
    gRegister arg = 0;
    gRegister result = 0;
    constexpr gRegister expected = 2;

    // 0
    callIsPowerOf2ToFlags(arg, result);
    CPPUNIT_ASSERT_EQUAL(expected, result);
    constexpr gRegister expected2or0 = 0;

    callPowerOf2or0_64(arg, result);
    CPPUNIT_ASSERT_EQUAL(expected2or0, result);
    callPowerOf2orAll1_64(arg, result);
    CPPUNIT_ASSERT_EQUAL(expectedAll1_64, result);

    callPowerOf2or0_32(arg, result);
    CPPUNIT_ASSERT_EQUAL(expected2or0, result);
    callPowerOf2orAll1_32(arg, result);
    CPPUNIT_ASSERT_EQUAL(expectedAll1_32, result);

    // power of 2
    arg = 1;
    for(size_t i=0; i < 64; ++i) {
        callIsPowerOf2ToFlags(arg, result);
        CPPUNIT_ASSERT_EQUAL(expected, result);
        callPowerOf2or0_64(arg, result);
        CPPUNIT_ASSERT_EQUAL(arg, result);
        callPowerOf2orAll1_64(arg, result);
        CPPUNIT_ASSERT_EQUAL(arg, result);

        if (i < 32) {
            callPowerOf2or0_32(arg, result);
            CPPUNIT_ASSERT_EQUAL(arg, result);
            callPowerOf2orAll1_32(arg, result);
            CPPUNIT_ASSERT_EQUAL(arg, result);

            gRegister arg64 = arg | 0x100000000ull;
            callPowerOf2or0_32(arg64, result);
            CPPUNIT_ASSERT_EQUAL(arg, result);
            callPowerOf2orAll1_32(arg64, result);
            CPPUNIT_ASSERT_EQUAL(arg, result);
        }
        arg <<= 1;
    }

    // others
    arg = 3;
    for(size_t i=0; i < 64; ++i) {
        callIsPowerOf2ToFlags(arg, result);
        CPPUNIT_ASSERT_EQUAL(expectedAll0, result);
        callPowerOf2or0_64(arg, result);
        CPPUNIT_ASSERT_EQUAL(expectedAll0, result);
        callPowerOf2orAll1_64(arg, result);
        CPPUNIT_ASSERT_EQUAL(expectedAll1_64, result);

        if (i < 32) {
            callPowerOf2or0_32(arg, result);
            CPPUNIT_ASSERT_EQUAL(expectedAll0, result);
            callPowerOf2orAll1_32(arg, result);
            CPPUNIT_ASSERT_EQUAL(expectedAll1_32, result);
        }

        arg <<= 1;
        arg |= 1;
    }

    return;
}

void SudokuSseTest::test_MergeThreeElements()
{
    struct TestSet {
        gRegister arg;
        gRegister expected;
    };

    constexpr TestSet testSet[] {
        {0, 0},
        {0x1, 0x1},
        {0x200, 0x1},
        {0x40000, 0x1},
        {0x700, 0x103},
        {0x4020000, 0x100},
        {0x3c0000f, 0xff},
        {0x7ffffff, 0x1ff}};

    for(const auto& test : testSet) {
        const gRegister arg = test.arg;
        gRegister result = 0;
        asm volatile (
            "call testMergeThreeElements64\n\t"
            :"=b"(result):"a"(arg):"rcx","rdx","r15");
        CPPUNIT_ASSERT_EQUAL(test.expected, result);

        result = 0;
        asm volatile (
            "call testMergeThreeElements32\n\t"
            :"=b"(result):"a"(arg):"rcx","rdx","r15");
        CPPUNIT_ASSERT_EQUAL(test.expected, result);
    }

    return;
}

void SudokuSseTest::test_OrThreeXmmRegs()
{
    struct TestSet {
        union {
            struct {
                uint64_t  arg[6];
                uint64_t  expected[2];
            };
            xmmRegister xexpected[4];
        };
    };

    constexpr TestSet testSet[] {
        {{{{0, 0, 0, 0, 0, 0}, {0, 0}}}},
        {{{{0x76543210, 1, 0xfecdba9800000000, 0, 0xf, 0x123456789abcdef0}, {0xfecdba987654321f, 0x123456789abcdef1}}}},
    };

    for(const auto& test : testSet) {
        xmmRegister xactual;
        asm volatile (
            "xorps   xmm1, xmm1\n\t"
            "movdqa  xmm2, xmmword ptr [%0]\n\t"
            "movdqa  xmm3, xmmword ptr [%0+16]\n\t"
            "movdqa  xmm4, xmmword ptr [%0+32]\n\t"
            "call testOrThreeXmmRegs\n\t"
            "movdqa  xmmword ptr [%1], xmm1\n\t"
            ::"r"(test.arg),"r"(&xactual));

        auto actual = memcmp(test.expected, &xactual, sizeof(xactual));
        constexpr decltype(actual) expected = 0;
        CPPUNIT_ASSERT_EQUAL(expected, actual);
    }

    return;
}

void SudokuSseTest::test_FilterUniqueCandidatesInRowPartSub()
{
    struct TestSet {
        gRegister bitmask;
        gRegister src;
        gRegister expected;
    };

    constexpr TestSet testSet[] {
        {0x1ff, 0, 0},
        {0x1ff, 0x300, 0x300},
        {0x1ff, 0xc08, 0x8},
        {0x1ff, 0x60000, 0x60000},
        {0x1ff, 0x70000, 0x40000},
        {0x1ff << 9, 0x40201, 0x40201},
        {0x1ff << 9, 0x140804, 0x804},
        {0x1ff << 9, 0x400080, 0x400080},
        {0x1ff << 9, 0x600100, 0x100}};

    for(const auto& test : testSet) {
        const gRegister bitmask = test.bitmask;
        const gRegister src = test.src;
        const gRegister preset = src & (~(bitmask << 9));
        gRegister result = 0;
        asm volatile (
            "call testFilterUniqueCandidatesInRowPartSub\n\t"
            :"=d"(result):"a"(bitmask),"b"(src),"c"(preset):"r12","r13","r14","r15");
        CPPUNIT_ASSERT_EQUAL(test.expected, result);
    }

    return;
}

void SudokuSseTest::test_FilterUniqueCandidatesInRowPart()
{
    struct TestSet {
        gRegister arg;
        gRegister expected;
    };

    constexpr TestSet testSet[] {
        {0, 0},
        {0x40201, 0x40201},
        {0x402010, 0x402010},
        {0x4020100, 0x4020100},
        {0x1101010, 0x1010},
        {0x1011011, 0x1000000}};

    for(const auto& test : testSet) {
        gRegister arg = test.arg;
        gRegister expected = test.expected;
        gRegister result = 0;
        asm volatile (
            "call testFilterUniqueCandidatesInRowPart0\n\t"
            :"=b"(result):"a"(arg):"r11","r12","r13","r14","r15");
        CPPUNIT_ASSERT_EQUAL(expected, result);
        arg <<= 32;
        expected <<= 32;
        asm volatile (
            "call testFilterUniqueCandidatesInRowPart1\n\t"
            :"=b"(result):"a"(arg):"r11","r12","r13","r14","r15");
        CPPUNIT_ASSERT_EQUAL(expected, result);
    }

    return;
}

void SudokuSseTest::test_CollectUniqueCandidatesInRowPart()
{
    struct TestSet {
        gRegister arg;
        gRegister sum;
        gRegister haszero;
    };

    constexpr TestSet testSet[] {
        {0, 0, 1},
        {0x200100, 0x108, 1},
        {0x200800, 0xc, 1},
        {0x002020, 0x30, 1},
        {0x100401, 0x7, 0},
        {0x401000f, 0x180, 0},
        {0x7f201ff, 0x100, 0},
        {0x004c010, 0x11,  0},
        {0x4000210, 0x111, 0},
    };

    for(const auto& test : testSet) {
        const gRegister arg = test.arg;
        gRegister sum = 0;
        gRegister haszero = 0;
        asm volatile (
            "call testCollectUniqueCandidatesInRowPart\n\t"
            :"=b"(sum),"=c"(haszero):"a"(arg):"r10","r11","r12","r13","r14","r15");
        CPPUNIT_ASSERT_EQUAL(test.sum, sum);
        CPPUNIT_ASSERT(((test.haszero == 0) && (haszero <= 0x1ff)) ||
                       ((test.haszero != 0) && (haszero > 0x7fffffff) && (haszero < 0x100000000ull)));
    }

    return;
}

void SudokuSseTest::test_CollectUniqueCandidatesInLine()
{
    struct TestSet {
        gRegister   arghigh;
        gRegister   arglow;
        gRegister   sum;
        gRegister   resulthigh;
        gRegister   resultlow;
        uint64_t    aborted;
    };

    enum class Func {
        MINFUNC,
        COLLECT = MINFUNC,
        FILTER,
        MAXFUNC,
    };

    constexpr TestSet testSet[] {
        {0, 0, 0, 0, 0, 1},
        {0x7ffffff, 0x7fffff07fffe01,         1, 0, 0x1,  0},
        {0x7ffffff, 0x7fffff07fc0401,       0x3, 0, 0x401,  0},
        {0x7ffffff, 0x7fffff00100401,       0x7, 0, 0x100401,  0},
        {0x7ffffff, 0x7fffe0800100401,   0xf, 0, 0x800100401, 0},
        {0x7ffffff, 0x7fc200800100401,  0x1f, 0, 0x200800100401, 0},
        {0x7ffffff, 0x80200800100401,   0x3f, 0, 0x80200800100401,  0},
        {0x7fffe40, 0x80200800100401,   0x7f, 0x40,      0x80200800100401, 0},
        {0x7fd0040, 0x80200800100401,   0xff, 0x10040,   0x80200800100401, 0},
        {0x4010040, 0x80200800100401,  0x1ff, 0x4010040, 0x80200800100401, 0},
        {0x783c1e0, 0x783c00800100401,   0xf, 0, 0x800100401, 0},
        {0x4010040, 0x80200f003c1e0f,  0x1f0, 0x4010040, 0x80200000000000, 0},
        {0x4010040, 0x802000003c1e0f,  0x1f0, 0x4010040, 0x80200000000000, 1},
    };

    for(const auto& test : testSet) {
        for(Func e=Func::MINFUNC; e<Func::MAXFUNC; e=static_cast<Func>(static_cast<int>(e)+1)) {
            xmmRegister arg;
            static_assert(sizeof(arg) == (sizeof(test.arglow) + sizeof(test.arghigh)),
                          "Unexpected xmmRegister size");
            *(reinterpret_cast<gRegister*>(&arg)) = test.arglow;
            *(reinterpret_cast<gRegister*>(&arg) + 1) = test.arghigh;

            gRegister sum = 0;
            gRegister elementCount = 0;
            xmmRegister result;
            uint64_t  actual;
            gRegister resulthigh;
            gRegister resultlow;
            constexpr gRegister expectedelementCount = 0;

            switch(e) {
            case Func::COLLECT:
                asm volatile (
                    "movdqa  xmm1, xmmword ptr [%0]\n\t"
                    "call testCollectUniqueCandidatesInLine\n\t"
                    :"=a"(actual),"=b"(elementCount),"=d"(sum):"r"(&arg):"rsi","rdi","r8","r9","r10","r11","r12","r13","r15");
                CPPUNIT_ASSERT_EQUAL(test.sum, sum);
                CPPUNIT_ASSERT_EQUAL(test.aborted, actual);
                CPPUNIT_ASSERT_EQUAL(expectedelementCount, elementCount);
                break;
            case Func::FILTER:
                asm volatile (
                    "movdqa  xmm1, xmmword ptr [%1]\n\t"
                    "call testFilterUniqueCandidatesInLine\n\t"
                    "movdqa  xmmword ptr [%0], xmm14\n\t"
                    ::"r"(&result),"r"(&arg):"r8","r9","r10","r11","r12","r13","r14","r15");
                static_assert(sizeof(result) == (sizeof(resulthigh) + sizeof(resultlow)),
                              "Unexpected xmmRegister size");
                resulthigh = *(reinterpret_cast<gRegister*>(&result) + 1);
                resultlow = *(reinterpret_cast<gRegister*>(&result));

                CPPUNIT_ASSERT_EQUAL(test.resulthigh, resulthigh);
                CPPUNIT_ASSERT_EQUAL(test.resultlow, resultlow);
                break;
            default:
                break;
            }
        }
    }

    return;
}

void SudokuSseTest::test_CollectUniqueCandidatesInThreeLine()
{
    struct TestSet {
        uint64_t  arg[6];
        uint64_t  expected[2];
    };

    constexpr TestSet testSet[] {
        {{0, 0, 0, 0, 0, 0}, {0, 0}},
        {{0x7ffffff07ffffff, 0x100401, 0x7ffffff00804008, 0x7ffffff, 0x402010007ffffff}, {0x402010000804008, 0x100401}},
        {{0x3f9fcfe03f9fcfe, 0x4000000, 0x3f9fcfe00000001, 0x3f9fcfe, 0x3f9fcfe03f9fcfe, 0x3f9fcfe}, {1, 0x4000000}}
    };

    for(const auto& test : testSet) {
        xmmRegister xmmreg[4];
        uint64_t    actual[2];

        assert(sizeof(xmmreg) >= sizeof(test.arg));
        memmove(xmmreg, test.arg, sizeof(test.arg));
        asm volatile (
            "movdqa  xmm1, xmmword ptr [%0]\n\t"
            "movdqa  xmm2, xmmword ptr [%0+16]\n\t"
            "movdqa  xmm3, xmmword ptr [%0+32]\n\t"
            "call    testCollectUniqueCandidatesInThreeLine\n\t"
            "movdqa  xmmword ptr [%0+48], xmm10\n\t"
            ::"r"(xmmreg):"r8","r9","r10","r11","r12","r13","r14","r15");

        static_assert(sizeof(actual) >= sizeof(xmmreg[3]), "Unexpected actual and xmmreg[] size");
        memmove(actual, &(xmmreg[3]), sizeof(actual));
        CPPUNIT_ASSERT_EQUAL(test.expected[0], actual[0]);
        CPPUNIT_ASSERT_EQUAL(test.expected[1], actual[1]);
    }

    return;
}

void SudokuSseTest::test_SelectRowParts()
{
    struct TestSet {
        gRegister   arghigh;
        gRegister   arglow;
        gRegister   resultrowpart2;
        gRegister   resultrowpart1;
        gRegister   resultrowpart0;
        gRegister   resulthigh;
        gRegister   resultlow;
        gRegister   iszero;
    };

    enum class Func {
        MINFUNC,
        SELECT0 = MINFUNC,
        SELECT1,
        SELECT2,
        COUNT,
        MAXFUNC,
    };

    constexpr TestSet testSet[] {
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0x7ffffff, 0x7ffffff07ffffff,   0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff07ffffff, 0},
        {0x4010040, 0x80200800100401, 0x4010040, 0x802008, 0x100401, 0x4010040, 0x80200800100401, 1},
        {0x4010040, 0x80200f003c1e0f, 0x4010040, 0x80200f, 0x3c1e0f, 0x4010040, 0x80200f003c1e0f, 0},
        {0x4010040, 0x802000003c1e0f, 0x4010040, 0x802000, 0x3c1e0f, 0x4010040, 0x802000003c1e0f, 0},
    };

    for(const auto& test : testSet) {
        for(Func e=Func::MINFUNC; e<Func::MAXFUNC; e=static_cast<Func>(static_cast<int>(e)+1)) {
            xmmRegister arg;
            static_assert(sizeof(arg) == (sizeof(test.arglow) + sizeof(test.arghigh)),
                          "Unexpected xmmRegister size");
            *(reinterpret_cast<gRegister*>(&arg)) = test.arglow;
            *(reinterpret_cast<gRegister*>(&arg) + 1) = test.arghigh;
            gRegister   resultrowpart;
            gRegister   resulthigh;
            gRegister   resultlow;
            gRegister   iszero;

            switch(e) {
            case Func::SELECT0:
                asm volatile (
                    "movdqa  xmm1, xmmword ptr [%0]\n\t"
                    "call testSelectRowParts0\n\t"
                    :"=a"(resultrowpart):"r"(&arg):"r15");
                CPPUNIT_ASSERT_EQUAL(test.resultrowpart0, resultrowpart);
                break;
            case Func::SELECT1:
                asm volatile (
                    "movdqa  xmm1, xmmword ptr [%0]\n\t"
                    "call testSelectRowParts1\n\t"
                    :"=a"(resultrowpart):"r"(&arg):"r15");
                CPPUNIT_ASSERT_EQUAL(test.resultrowpart1, resultrowpart);
                break;
            case Func::SELECT2:
                asm volatile (
                    "movdqa  xmm1, xmmword ptr [%0]\n\t"
                    "call testSelectRowParts2\n\t"
                    :"=a"(resultrowpart):"r"(&arg):"r15");
                CPPUNIT_ASSERT_EQUAL(test.resultrowpart2, resultrowpart);
                break;
            case Func::COUNT:
                asm volatile (
                    "movdqa  xmm1, xmmword ptr [%0]\n\t"
                    "call testSelectRowAndCount\n\t"
                    :"=a"(resulthigh),"=b"(resultlow),"=c"(iszero):"r"(&arg):"r14","r15");
                CPPUNIT_ASSERT_EQUAL(test.resulthigh, resulthigh);
                CPPUNIT_ASSERT_EQUAL(test.resultlow, resultlow);
                CPPUNIT_ASSERT_EQUAL(test.iszero, iszero);
                break;
            default:
                break;
            }
        }
    }

    return;
}

void SudokuSseTest::test_SelectElementInRowParts()
{
    struct TestSet {
        gRegister   arg;
        gRegister   result2;
        gRegister   result1;
        gRegister   result0;
    };

    enum class Func {
        MINFUNC,
        SELECT0 = MINFUNC,
        SELECT1,
        SELECT2,
        MAXFUNC,
    };

    constexpr TestSet testSet[] {
        {0, 0, 0, 0},
        {0x7ffffff, 0x1ff, 0x1ff, 0x1ff},
        {0x40201,   1, 1, 1},
        {0x4020100, 0x100, 0x100, 0x100},
        {0x7007007, 0x1c0, 0x38,  7},
    };

    for(const auto& test : testSet) {
        for(Func e=Func::MINFUNC; e<Func::MAXFUNC; e = static_cast<Func>(static_cast<int>(e)+1)) {
            gRegister arg = test.arg;
            gRegister result;

            switch(e) {
            case Func::SELECT0:
                asm volatile (
                    "call testSelectElementInRowParts0\n\t"
                    :"=b"(result):"a"(arg):"r15");
                CPPUNIT_ASSERT_EQUAL(test.result0, result);
                break;
            case Func::SELECT1:
                asm volatile (
                    "call testSelectElementInRowParts1\n\t"
                    :"=b"(result):"a"(arg):"r15");
                CPPUNIT_ASSERT_EQUAL(test.result1, result);
                break;
            case Func::SELECT2:
                asm volatile (
                    "call testSelectElementInRowParts2\n\t"
                    :"=b"(result):"a"(arg):"r15");
                CPPUNIT_ASSERT_EQUAL(test.result2, result);
                break;
            default:
                break;
            }
        }
    }

    return;
}

void SudokuSseTest::test_FillUniqueElement()
{
    struct TestSet {
        bool       alreadyunique;
        gRegister  prerowpart;
        gRegister  precolumn;
        gRegister  prebox;
        gRegister  prerow;
        gRegister  candidateset;
        gRegister  uniquecandidate;
        gRegister  expectedrowpart;
    };

    enum class Func {
        MINFUNC,
        UNIQUE0 = MINFUNC,
        UNIQUE1,
        UNIQUE2,
        MAXUNIQUE = UNIQUE2,
        ONE0,
        ONE1,
        ONE2,
        MAXFUNC,
    };

    constexpr TestSet testSet[] {
        {false, 0x7ffffff, 0, 0, 0, 0, 0, 0x7ffffff},
        {false, 0x7ffffff, 0x8001c0, 0x38, 0x6,  0x1fe, 1, 0x7fffe01},
        {false, 0x7fffff7, 0x800008, 0x10, 0x30, 0x38,  0, 0x7ffffc7},
        {true,  0x7fff010, 0x10,     0x10, 0x10, 0x10,  0, 0x7fff010}
    };

    for(const auto& test : testSet) {
        gRegister  prerowpart = test.prerowpart;
        gRegister  precolumn = test.precolumn;
        gRegister  prebox = test.prebox;
        gRegister  prerow = test.prerow;
        gRegister  uniquecandidate = test.uniquecandidate;
        gRegister  expectedrowpart = test.expectedrowpart;
        gRegister  expectedcolumn = 0;
        gRegister  expectedbox = 0;
        gRegister  expectedrow = 0;

        for(Func e=Func::MINFUNC; e<Func::MAXFUNC; e = static_cast<Func>(static_cast<int>(e)+1)) {
            if ((e <= Func::MAXUNIQUE) && test.alreadyunique) {
                continue;
            }

            gRegister candidateset = test.candidateset;
            gRegister actualrowpart = 0;
            gRegister column = 0;
            gRegister mergedbox = 0;
            gRegister mergedrow = 0;

            testFillUniqueElementPreRowPart = prerowpart;
            testFillUniqueElementPreColumn = precolumn;
            testFillUniqueElementPreBox = prebox;
            testFillUniqueElementPreRow = prerow;

            switch(e) {
            case Func::UNIQUE0:
                asm volatile (
                    "call testFillUniqueElement0\n\t"
                    :"=b"(actualrowpart),"=c"(column),"=d"(mergedbox),"=S"(mergedrow):"a"(candidateset):"r10","r11","r12","r13","r14","r15");
                break;
            case Func::UNIQUE1:
                asm volatile (
                    "call testFillUniqueElement1\n\t"
                    :"=b"(actualrowpart),"=c"(column),"=d"(mergedbox),"=S"(mergedrow):"a"(candidateset):"r10","r11","r12","r13","r14","r15");
                break;
            case Func::UNIQUE2:
                asm volatile (
                    "call testFillUniqueElement2\n\t"
                    :"=b"(actualrowpart),"=c"(column),"=d"(mergedbox),"=S"(mergedrow):"a"(candidateset):"r10","r11","r12","r13","r14","r15");
                break;
            case Func::ONE0:
                asm volatile (
                    "call testFillOneUniqueCandidates0\n\t"
                    :"=a"(actualrowpart),"=b"(column),"=c"(mergedbox),"=d"(mergedrow)::"r9","r10","r11","r12","r13","r14","r15");
                break;
            case Func::ONE1:
                asm volatile (
                    "call testFillOneUniqueCandidates1\n\t"
                    :"=a"(actualrowpart),"=b"(column),"=c"(mergedbox),"=d"(mergedrow)::"r9","r10","r11","r12","r13","r14","r15");
                break;
            case Func::ONE2:
                asm volatile (
                    "call testFillOneUniqueCandidates2\n\t"
                    :"=a"(actualrowpart),"=b"(column),"=c"(mergedbox),"=d"(mergedrow)::"r9","r10","r11","r12","r13","r14","r15");
                break;
            default:
                break;
            }

            expectedbox = test.prebox | (test.uniquecandidate);
            expectedrow = test.prerow | (test.uniquecandidate);
            expectedcolumn = precolumn | uniquecandidate;

            CPPUNIT_ASSERT_EQUAL(expectedrowpart, actualrowpart);
            CPPUNIT_ASSERT_EQUAL(expectedcolumn, column);
            CPPUNIT_ASSERT_EQUAL(expectedbox, mergedbox);
            CPPUNIT_ASSERT_EQUAL(expectedrow, mergedrow);

            prerowpart = rotateRowPart(prerowpart);
            precolumn = rotateRowPart(precolumn);
            expectedrowpart = rotateRowPart(expectedrowpart);
            uniquecandidate = rotateRowPart(uniquecandidate);
        }
    }

    return;
}

void SudokuSseTest::test_FillOneUniqueCandidates()
{
    return;
}

void SudokuSseTest::test_SaveLineSet()
{
    struct TestSet {
        Shiftpos   pos;
        gRegister  prerowhigh;
        gRegister  prerowlow;
        gRegister  preallhigh;
        gRegister  prealllow;
        gRegister  preboxhigh;
        gRegister  preboxlow;
        gRegister  arg;
        gRegister  expectedrowhigh;
        gRegister  expectedrowlow;
        gRegister  expectedallhigh;
        gRegister  expectedalllow;
        gRegister  expectedboxhigh;
        gRegister  expectedboxlow;
    };

    constexpr TestSet testSet[] {
        {Shiftpos::SHIFT_POS0, 0x7ffffff, 0x7ffffff07ffffff, 0x4000000, 0x200000000000000, 0x4000000, 0x200000000000000,
         0x1f03c0f, 0x7ffffff, 0x7ffffff01f03c0f, 0x4000000, 0x200000000000000, 0x4000000, 0x200000000000000},
        {Shiftpos::SHIFT_POS0, 0x7ffffff, 0x7ffffff07ffffff, 0x4000000,    0x200000000000000, 0x4000000, 0x200000000000000,
         0x40404, 0x7ffffff, 0x7ffffff00040404, 0x4000000, 0x200000000040404, 0x4000000, 0x200000000040404},
        {Shiftpos::SHIFT_POS1, 0x7ffffff, 0x7ffffff07ffffff, 3, 0xc, 3, 0xc,
         0x40100c0, 0x7ffffff, 0x40100c007ffffff, 3, 0xc, 3, 0xc},
        {Shiftpos::SHIFT_POS1, 0x7ffffff, 0x7ffffff07ffffff, 3, 0xc, 3, 0xc,
         0x4010080, 0x7ffffff, 0x401008007ffffff, 3, 0x40100800000000c, 3, 0x40100800000000c},
        {Shiftpos::SHIFT_POS2, 0x7ffffff, 0x7ffffff07ffffff, 8, 0x200000000000000, 8, 0x200000000000000,
         0x0c00608, 0xc00608,  0x7ffffff07ffffff, 8, 0x200000000000000, 8, 0x200000000000000},
        {Shiftpos::SHIFT_POS2, 0x7ffffff, 0x7ffffff07ffffff, 8, 0x200000000000000, 8, 0x200000000000000,
         0x80808,   0x80808,   0x7ffffff07ffffff, 0x80808, 0x200000000000000, 0x80808, 0x200000000000000},
    };
    for(const auto& test : testSet) {
        xmmRegister row;
        xmmRegister all;
        xmmRegister box;

        static_assert(sizeof(row) == (sizeof(test.prerowlow) + sizeof(test.prerowhigh)),
                      "Unexpected xmmRegister size");
        *(reinterpret_cast<gRegister*>(&row)) = test.prerowlow;
        *(reinterpret_cast<gRegister*>(&row) + 1) = test.prerowhigh;

        static_assert(sizeof(all) == (sizeof(test.prerowlow) + sizeof(test.prerowhigh)),
                      "Unexpected xmmRegister size");
        *(reinterpret_cast<gRegister*>(&all)) = test.prealllow;
        *(reinterpret_cast<gRegister*>(&all) + 1) = test.preallhigh;

        static_assert(sizeof(box) == (sizeof(test.prerowlow) + sizeof(test.prerowhigh)),
                      "Unexpected xmmRegister size");
        *(reinterpret_cast<gRegister*>(&box)) = test.preboxlow;
        *(reinterpret_cast<gRegister*>(&box) + 1) = test.preboxhigh;

        const gRegister expectedrowhigh = test.expectedrowhigh;
        const gRegister expectedrowlow = test.expectedrowlow;
        const gRegister expectedallhigh = test.expectedallhigh;
        const gRegister expectedalllow = test.expectedalllow;
        const gRegister expectedboxhigh = test.expectedboxhigh;
        const gRegister expectedboxlow = test.expectedboxlow;

        const gRegister arg = test.arg;
        switch(test.pos) {
        case Shiftpos::SHIFT_POS0:
            asm volatile (
                "movdqa  xmm1,  xmmword ptr [%0]\n\t"
                "movdqa  xmm0,  xmmword ptr [%1]\n\t"
                "movdqa  xmm10, xmmword ptr [%2]\n\t"
                "call testSaveLineSet0\n\t"
                "movdqa  xmmword ptr [%0], xmm1\n\t"
                "movdqa  xmmword ptr [%1], xmm0\n\t"
                "movdqa  xmmword ptr [%2], xmm10\n\t"
                ::"r"(&row),"r"(&all),"r"(&box),"a"(arg):"r11","r12","r13","r14","r15");
            break;
        case Shiftpos::SHIFT_POS1:
            asm volatile (
                "movdqa  xmm1,  xmmword ptr [%0]\n\t"
                "movdqa  xmm0,  xmmword ptr [%1]\n\t"
                "movdqa  xmm10, xmmword ptr [%2]\n\t"
                "call testSaveLineSet1\n\t"
                "movdqa  xmmword ptr [%0], xmm1\n\t"
                "movdqa  xmmword ptr [%1], xmm0\n\t"
                "movdqa  xmmword ptr [%2], xmm10\n\t"
                ::"r"(&row),"r"(&all),"r"(&box),"a"(arg):"r11","r12","r13","r14","r15");
            break;
        case Shiftpos::SHIFT_POS2:
            asm volatile (
                "movdqa  xmm1,  xmmword ptr [%0]\n\t"
                "movdqa  xmm0,  xmmword ptr [%1]\n\t"
                "movdqa  xmm10, xmmword ptr [%2]\n\t"
                "call testSaveLineSet2\n\t"
                "movdqa  xmmword ptr [%0], xmm1\n\t"
                "movdqa  xmmword ptr [%1], xmm0\n\t"
                "movdqa  xmmword ptr [%2], xmm10\n\t"
                ::"r"(&row),"r"(&all),"r"(&box),"a"(arg):"r11","r12","r13","r14","r15");
            break;
        default:
            break;
        }

        gRegister high = *(reinterpret_cast<gRegister*>(&row) + 1);
        gRegister low = *(reinterpret_cast<gRegister*>(&row));
        static_assert(sizeof(row) == (sizeof(high) + sizeof(low)), "Unexpected xmmRegister size");

        CPPUNIT_ASSERT_EQUAL(expectedrowhigh, high);
        CPPUNIT_ASSERT_EQUAL(expectedrowlow, low);

        static_assert(sizeof(all) == (sizeof(high) + sizeof(low)), "Unexpected xmmRegister size");
        high = *(reinterpret_cast<gRegister*>(&all) + 1);
        low = *(reinterpret_cast<gRegister*>(&all));
        CPPUNIT_ASSERT_EQUAL(expectedallhigh, high);
        CPPUNIT_ASSERT_EQUAL(expectedalllow, low);

        static_assert(sizeof(box) == (sizeof(high) + sizeof(low)), "Unexpected xmmRegister size");
        high = *(reinterpret_cast<gRegister*>(&box) + 1);
        low = *(reinterpret_cast<gRegister*>(&box));
        CPPUNIT_ASSERT_EQUAL(expectedboxhigh, high);
        CPPUNIT_ASSERT_EQUAL(expectedboxlow, low);
    }

    return;
}

void SudokuSseTest::test_MaskElementCandidates()
{
    struct TestSet {
        Shiftpos  pos;
        gRegister arg;
        gRegister sum;
    };

    constexpr TestSet testSet[] {
        {Shiftpos::SHIFT_POS0, 0x48007ff, 0x123},
        {Shiftpos::SHIFT_POS1, 0x352fe0c, 0xdc},
        {Shiftpos::SHIFT_POS2, 0x7fc1f81f, 0xff},
    };

    for(const auto& test : testSet) {
        const gRegister arg = test.arg;
        gRegister sum = 0;
        switch(test.pos) {
        case Shiftpos::SHIFT_POS0:
            asm volatile (
                "call testMergeTwoElements0\n\t"
                :"=b"(sum):"a"(arg):"r14","r15");
            break;
        case Shiftpos::SHIFT_POS1:
            asm volatile (
                "call testMergeTwoElements1\n\t"
                :"=b"(sum):"a"(arg):"r14","r15");
            break;
        case Shiftpos::SHIFT_POS2:
            asm volatile (
                "call testMergeTwoElements2\n\t"
                :"=b"(sum):"a"(arg):"r14","r15");
            break;
        default:
            break;
        }
        CPPUNIT_ASSERT_EQUAL(test.sum, sum);
    }

    return;
}

void SudokuSseTest::test_MergeTwoElements()
{
    struct TestSet {
        Shiftpos  pos;
        gRegister arg;
        gRegister candidates;
        gRegister result;
    };

    constexpr TestSet testSet[] {
        {Shiftpos::SHIFT_POS0, 0x7ffffff, 0x1fe, 0x7fffe01},
        {Shiftpos::SHIFT_POS0, 0x7ffffff, 1, 0x7ffffff},
        {Shiftpos::SHIFT_POS1, 0x7ffffff, 0x1ef, 0x7fc21ff},
        {Shiftpos::SHIFT_POS1, 0x7ffffff, 1, 0x7ffffff},
        {Shiftpos::SHIFT_POS2, 0x7ffffff, 0xff, 0x403ffff},
        {Shiftpos::SHIFT_POS2, 0x7ffffff, 1, 0x7ffffff},
    };

    for(const auto& test : testSet) {
        const gRegister arg = test.arg;
        const gRegister candidates = test.candidates;
        gRegister result = 0;
        switch(test.pos) {
        case Shiftpos::SHIFT_POS0:
            asm volatile (
                "call testMaskElementCandidates0\n\t"
                :"=c"(result):"a"(arg),"b"(candidates):"r12","r13","r14","r15");
            break;
        case Shiftpos::SHIFT_POS1:
            asm volatile (
                "call testMaskElementCandidates1\n\t"
                :"=c"(result):"a"(arg),"b"(candidates):"r12","r13","r14","r15");
            break;
        case Shiftpos::SHIFT_POS2:
            asm volatile (
                "call testMaskElementCandidates2\n\t"
                :"=c"(result):"a"(arg),"b"(candidates):"r12","r13","r14","r15");
            break;
        default:
            break;
        }
        CPPUNIT_ASSERT_EQUAL(test.result, result);
    }

    return;
}

void SudokuSseTest::test_FindElementCandidates()
{
    struct TestSet {
        gRegister  prerowpart;
        gRegister  prerowcandidates;
        gRegister  prebox;
        gRegister  precolumnrowpart;
        gRegister  expected;
        gRegister  expectedthree;
    };

    enum class Func {
        POSMIN,
        POS0 = POSMIN,
        POS1,
        POS2,
        POSMAX,
    };

    constexpr TestSet testSet[] {
        {0x7fffe01, 0x100, 0x80, 0x40,  0x7fffe01, 0x7fffe01},
        {0x40407,   0x1f8, 0,    0,     0x40404,   0x40404},
        {0x40101ff, 0,     0x3f, 0,     0x4010040, 0x4010040},
        {0x7ffffff, 0,     0,    0x1ef, 0x7fffe10, 0x7fffe10},
        {0x7ffffff, 1,     0x10, 0x100, 0x7ffffff, 0x7ffffff},
    };

    for(const auto& test : testSet) {
        gRegister prerowpart = test.prerowpart;
        const gRegister prerowcandidates = test.prerowcandidates;
        const gRegister prebox = test.prebox;
        gRegister precolumnrowpart = test.precolumnrowpart;
        gRegister expected = test.expected;
        gRegister actual = 0;

        asm volatile (
            "call testFindThreePartsCandidates\n\t"
            :"=S"(actual):"a"(test.prerowpart),"b"(test.prerowcandidates),"c"(test.prebox),"d"(test.precolumnrowpart):"r10","r11","r12","r13","r14","r15");
        CPPUNIT_ASSERT_EQUAL(test.expectedthree, actual);

        for(Func e=Func::POSMIN; e<Func::POSMAX; e=static_cast<Func>(static_cast<int>(e)+1)) {
            switch(e) {
            case Func::POS0:
                asm volatile (
                    "call testFindElementCandidates0\n\t"
                    :"=S"(actual):"a"(prerowpart),"b"(prerowcandidates),"c"(prebox),"d"(precolumnrowpart):"r10","r11","r12","r13","r14","r15");
                break;
            case Func::POS1:
                asm volatile (
                    "call testFindElementCandidates1\n\t"
                    :"=S"(actual):"a"(prerowpart),"b"(prerowcandidates),"c"(prebox),"d"(precolumnrowpart):"r10","r11","r12","r13","r14","r15");
                break;
            case Func::POS2:
                asm volatile (
                    "call testFindElementCandidates2\n\t"
                    :"=S"(actual):"a"(prerowpart),"b"(prerowcandidates),"c"(prebox),"d"(precolumnrowpart):"r10","r11","r12","r13","r14","r15");
                break;
            default:
                break;
            }

            CPPUNIT_ASSERT_EQUAL(expected, actual);
            prerowpart = rotateRowPart(prerowpart);
            precolumnrowpart = rotateRowPart(precolumnrowpart);
            expected = rotateRowPart(expected);
            break;
        }
    }

    return;
}

void SudokuSseTest::test_FindThreePartsCandidates()
{
    struct TestSet {
        gRegister  prerowpart;
        gRegister  prerowcandidates;
        gRegister  prebox;
        gRegister  precolumnrowpart;
        gRegister  expected;
    };

    constexpr TestSet testSet[] {
        {0xc0604,   0x1fc, 0, 0x7f80000, 0x40404},
        {0x7f00603, 0, 0x1fc, 0x3fc00, 0x7f00202},
    };

    for(const auto& test : testSet) {
        gRegister actual = 0;

        asm volatile (
            "call testFindThreePartsCandidates\n\t"
            :"=S"(actual):"a"(test.prerowpart),"b"(test.prerowcandidates),"c"(test.prebox),"d"(test.precolumnrowpart):"r10","r11","r12","r13","r14","r15");
        CPPUNIT_ASSERT_EQUAL(test.expected, actual);
    }

    return;
}

void SudokuSseTest::test_FillThreePartsUniqueCandidates()
{
    struct TestSet {
        union {
            struct {
                uint64_t   prerow0[2];
                uint64_t   prerow1[2];
                uint64_t   prerow2[2];
                uint64_t   prebox[2];
                uint64_t   precolumn[2];
                uint64_t   row[2];
                uint64_t   box[2];
                uint64_t   column[2];
                uint64_t   rowfillnine[2];
                uint64_t   boxfillnine[2];
                uint64_t   columnfillnine[2];
                uint64_t   rowfindnine[2];
                uint64_t   boxfindnine[2];
                uint64_t   columnfindnine[2];
                gRegister  argrow;
            };
            xmmRegister xarg[8];
        };
    };

    enum class Func {
        FUNCMIN,
        PART2 = FUNCMIN, // left, right, center
        PART0,
        PART1,
        ROW2,
        ROW0,
        ROW1,
        FILLNINE,
        FINDNINE,
        FUNCCNT,
    };

    constexpr TestSet testSet[] {
        // 1st row : *,  *, 3,   4,5,6, 7,8,9,
        // 2nd row : 4,  5, 6,   0,0,0, 0,0,0,
        // 3rd row : 7,  8, 9,   0,0,0, 0,0,0,
        // colunm  : 2-9,58,369, 4,5,6, 7,8,9, (except find nine)
        // colunm  : 47, 58,369, 4,5,6, 7,8,9, (find nine)
        // result  : 1,  2, ...
        {{{{0x20202001010100, 0x7fc0604}, {0x7ffffff07ffffff, 0x202020},  {0x7ffffff07ffffff, 0x1010100},
           {0x20202001010100, 0x1212124}, {0x20202001010100,  0x7f92124},
           {0x20202001010100, 0x40404},   {0x20202001010100,  0x1252524}, {0x20202001010100,  0x7fd2524},
           {0x20202001010100, 0x40404},   {0x20202001010100,  0x1252524}, {0x20202001010100,  0x7fd2524},
           {0x20202001010100, 0x7fc0604}, {0x7ffffff07ffffff, 0x1212120}, {0x7ffffff07ffffff, 0x1212120},
           0x1fc}}},
        // 1st row : *,  *, 3,   4,5,6, 0,0,0,
        // 2nd row : 4,  5, 6,   0,0,0, 0,0,0,
        // 3rd row : 7,  8, 9,   0,0,0, 0,0,0,
        // column  : 2-9,58,369, 4,5,6, 7,8,9, (except find nine)
        // column  : 47, 58,369, 4,5,6, 0,0,0, (find nine)
        // result  : 1,  2, ...
        {{{{0x20202007ffffff, 0x7fc0c04}, {0x7ffffff07ffffff, 0x202020},  {0x7ffffff07ffffff, 0x1010100},
           {0x20202000000000, 0x1212124}, {0x20202000000000,  0x7f92124},
           {0x20202007ffffff, 0x40404},   {0x20202000000000,  0x1252524}, {0x20202000000000,  0x7fd2524},
           {0x202020070381c0, 0x40404},   {0x20202000000000,  0x1252524}, {0x20202000000000,  0x7fd2524},
           {0x20202007ffffff, 0x40404},   {0x7ffffff07ffffff, 0x1212120}, {0x7ffffff07ffffff, 0x1212120},
           0x3c}}},
        // Top row is filled
        {{{{0x20202001010100, 0x40404}, {0x7ffffff07ffffff, 0x7ffffff}, {0x7ffffff07ffffff, 0x7ffffff},
           {0x20202001010100, 0x40404}, {0x20202001010100,  0x40404},
           {0x20202001010100, 0x40404}, {0x20202001010100,  0x40404},   {0x20202001010100,  0x40404},
           {0x20202001010100, 0x40404}, {0x20202001010100,  0x40404},   {0x20202001010100,  0x40404},
           {0x20202001010100, 0x40404}, {0x7ffffff07ffffff, 0x7ffffff}, {0x7ffffff07ffffff, 0x7ffffff},
           0x1ff}}}
    };

    for(const auto& testOriginal : testSet) {
        auto test = testOriginal;
        for(Func e=Func::FUNCMIN; e<Func::FUNCCNT; e=static_cast<Func>(static_cast<int>(e)+1)) {
            // memset() does not accept volatile*
            memset(const_cast<void*>(static_cast<volatile void*>(&testFillNineUniqueCandidatesRowX)),
                   0, sizeof(testFillNineUniqueCandidatesRowX));
            memset(const_cast<void*>(static_cast<volatile void*>(&testFillNineUniqueCandidatesBoxX)),
                   0, sizeof(testFillNineUniqueCandidatesBoxX));
            memset(const_cast<void*>(static_cast<volatile void*>(&testFillNineUniqueCandidatesColumnX)),
                   0, sizeof(testFillNineUniqueCandidatesColumnX));

            const uint64_t* expectedrow = test.row;
            const uint64_t* expectedbox = test.box;
            const uint64_t* expectedcolumn = test.column;

            switch(e) {
            case Func::PART0:
                asm volatile (
                    "movdqa  xmm1,  xmmword ptr [rdi]\n\t"
                    "movdqa  xmm2,  xmmword ptr [rdi+16]\n\t"
                    "movdqa  xmm3,  xmmword ptr [rdi+32]\n\t"
                    "movdqa  xmm10, xmmword ptr [rdi+48]\n\t"
                    "movdqa  xmm0,  xmmword ptr [rdi+64]\n\t"
                    "call testFillThreePartsUniqueCandidates0\n\t"
                    ::"D"(test.xarg),"a"(test.argrow):"rdx","rsi","r8","r9","r10","r11","r12","r13","r14","r15");
                break;
            case Func::PART1:
                asm volatile (
                    "movdqa  xmm1,  xmmword ptr [rdi]\n\t"
                    "movdqa  xmm2,  xmmword ptr [rdi+16]\n\t"
                    "movdqa  xmm3,  xmmword ptr [rdi+32]\n\t"
                    "movdqa  xmm10, xmmword ptr [rdi+48]\n\t"
                    "movdqa  xmm0,  xmmword ptr [rdi+64]\n\t"
                    "call testFillThreePartsUniqueCandidates1\n\t"
                    ::"D"(test.xarg),"a"(test.argrow):"rdx","rsi","r8","r9","r10","r11","r12","r13","r14","r15");
                break;
            case Func::PART2:
                asm volatile (
                    "movdqa  xmm1,  xmmword ptr [rdi]\n\t"
                    "movdqa  xmm2,  xmmword ptr [rdi+16]\n\t"
                    "movdqa  xmm3,  xmmword ptr [rdi+32]\n\t"
                    "movdqa  xmm10, xmmword ptr [rdi+48]\n\t"
                    "movdqa  xmm0,  xmmword ptr [rdi+64]\n\t"
                    "call testFillThreePartsUniqueCandidates2\n\t"
                    ::"D"(test.xarg),"a"(test.argrow):"rdx","rsi","r8","r9","r10","r11","r12","r13","r14","r15");
                break;
            case Func::ROW0:
                testFillNineUniqueCandidatesPreRow = test.argrow;
                asm volatile (
                    "movdqa  xmm1,  xmmword ptr [rdi]\n\t"
                    "movdqa  xmm2,  xmmword ptr [rdi+16]\n\t"
                    "movdqa  xmm3,  xmmword ptr [rdi+32]\n\t"
                    "movdqa  xmm10, xmmword ptr [rdi+48]\n\t"
                    "movdqa  xmm0,  xmmword ptr [rdi+64]\n\t"
                    "call testTestFillRowPartCandidates0\n\t"
                    ::"D"(test.xarg):"rax","rbx","rcx","rdx","rsi","r8","r9","r10","r11","r12","r13","r14","r15");
                break;
            case Func::ROW1:
                testFillNineUniqueCandidatesPreRow = test.argrow;
                asm volatile (
                    "movdqa  xmm1,  xmmword ptr [rdi]\n\t"
                    "movdqa  xmm2,  xmmword ptr [rdi+16]\n\t"
                    "movdqa  xmm3,  xmmword ptr [rdi+32]\n\t"
                    "movdqa  xmm10, xmmword ptr [rdi+48]\n\t"
                    "movdqa  xmm0,  xmmword ptr [rdi+64]\n\t"
                    "call testTestFillRowPartCandidates1\n\t"
                    ::"D"(test.xarg):"rax","rbx","rcx","rdx","rsi","r8","r9","r10","r11","r12","r13","r14","r15");
                break;
            case Func::ROW2:
                testFillNineUniqueCandidatesPreRow = test.argrow;
                asm volatile (
                    "movdqa  xmm1,  xmmword ptr [rdi]\n\t"
                    "movdqa  xmm2,  xmmword ptr [rdi+16]\n\t"
                    "movdqa  xmm3,  xmmword ptr [rdi+32]\n\t"
                    "movdqa  xmm10, xmmword ptr [rdi+48]\n\t"
                    "movdqa  xmm0,  xmmword ptr [rdi+64]\n\t"
                    "call testTestFillRowPartCandidates2\n\t"
                    ::"D"(test.xarg):"rax","rbx","rcx","rdx","rsi","r8","r9","r10","r11","r12","r13","r14","r15");
                break;
            case Func::FILLNINE:
                asm volatile (
                    "movdqa  xmm1,  xmmword ptr [rdi]\n\t"
                    "movdqa  xmm2,  xmmword ptr [rdi+16]\n\t"
                    "movdqa  xmm3,  xmmword ptr [rdi+32]\n\t"
                    "movdqa  xmm10, xmmword ptr [rdi+48]\n\t"
                    "movdqa  xmm0,  xmmword ptr [rdi+64]\n\t"
                    "call testFillNineUniqueCandidates\n\t"
                    ::"D"(test.xarg):"rax","rdx","rcx","rdx","rsi","r8","r9","r10","r11","r12","r13","r14","r15");
                expectedrow = test.rowfillnine;
                expectedbox = test.boxfillnine;
                expectedcolumn = test.columnfillnine;
                break;
            case Func::FINDNINE:
                asm volatile (
                    "movdqa  xmm1,  xmmword ptr [rdi]\n\t"
                    "movdqa  xmm2,  xmmword ptr [rdi+16]\n\t"
                    "movdqa  xmm3,  xmmword ptr [rdi+32]\n\t"
                    "movdqa  xmm10, xmmword ptr [rdi+48]\n\t"
                    "movdqa  xmm0,  xmmword ptr [rdi+64]\n\t"
                    "call testFindNineCandidates\n\t"
                    ::"D"(test.xarg):"rax","rdx","rcx","rdx","rsi","r8","r9","r10","r11","r12","r13","r14","r15");
                expectedrow = test.rowfindnine;
                expectedbox = test.boxfindnine;
                expectedcolumn = test.columnfindnine;
                break;
            default:
                break;
            }

            const xmmRegister actualrow = testFillNineUniqueCandidatesRowX;
            const xmmRegister actualbox = testFillNineUniqueCandidatesBoxX;
            const xmmRegister actualcolumn = testFillNineUniqueCandidatesColumnX;

            int cmpactual = memcmp(expectedrow, &actualrow, sizeof(actualrow));
            const decltype(cmpactual) cmpexpected = 0;
            CPPUNIT_ASSERT_EQUAL(cmpexpected, cmpactual);
            cmpactual = memcmp(expectedbox, &actualbox, sizeof(actualbox));
            CPPUNIT_ASSERT_EQUAL(cmpexpected, cmpactual);
            cmpactual = memcmp(expectedcolumn, &actualcolumn, sizeof(actualcolumn));
            CPPUNIT_ASSERT_EQUAL(cmpexpected, cmpactual);

            if (e >= Func::FILLNINE) {
                continue;
            }

            rotateRow(test.prerow0);
            rotateRow(test.prerow1);
            rotateRow(test.prerow2);
            rotateRow(test.prebox);
            rotateRow(test.precolumn);
            rotateRow(test.row);
            rotateRow(test.box);
            rotateRow(test.column);
            rotateRowPart(test.argrow);
        }
    }

    return;
}

void SudokuSseTest::test_FindRowPartCandidates()
{
    struct TestSet {
        union {
            struct {
                uint64_t   prebox[2];
                uint64_t   precolumn[2];
                uint64_t   box[2];
                uint64_t   column[2];
                gRegister  argrowparttarget;
                gRegister  argrowcandidates;
                gRegister  argbox;

                gRegister  rowpartleft;
                gRegister  candidatestarget;
                gRegister  candidatesleft;
                gRegister  candidatesother;

                gRegister  expectedrowparttarget;
                gRegister  expectedrowcandidates;
                gRegister  expectedbox;
            };
            xmmRegister xarg[4];
        };
    };

    constexpr TestSet testSet[] {
        {{{{0,0},{0,0},{0,0},{0,0}, 0x7ffffff, 0, 0,
           0, 0, 0, 0,  0x7ffffff, 0, 0}}},
        {{{{0,0},{0,0},{0,0},{0,0}, 0x1c0401, 0, 0,          // Decrease candidates by rows
           0x4010040, 0x3, 0x1c0, 0x38, 0x100401, 0x1f8, 0}}},
        {{{{0,0},{0x7eff7fe,0},{0,0},{0,0}, 0x7ffffff, 0, 0, // by columns
           0, 0, 0, 0,  0x100801, 0, 0}}},
        {{{{0x7000e18,0},{0,0},{0,0},{0,0}, 0x4000238, 0, 0, // by boxes
           0, 0, 0, 0,  0x4000220, 0, 0x1df}}},
    };

    for(const auto& testOriginal : testSet) {
        auto test = testOriginal;
        for(Shiftpos e=Shiftpos::SHIFT_POSMIN; e<Shiftpos::SHIFT_POSCNT; e=static_cast<Shiftpos>(static_cast<int>(e)+1)) {
            gRegister  argrowparttarget = test.argrowparttarget;
            gRegister  argrowcandidates = test.argrowcandidates;
            gRegister  argbox = test.argbox;
            testFindRowPartCandidatesPreRowPartTarget = argrowparttarget;
            testFindRowPartCandidatesPreRowCandidates = argrowcandidates;
            testFindRowPartCandidatesPreBox = argbox;
            testFindRowPartCandidatesRowPartTarget = 0;
            testFindRowPartCandidatesRowCandidates = 0;
            testFindRowPartCandidatesBox = 0;

            switch(e) {
            case Shiftpos::SHIFT_POS0:
                asm volatile (
                    "movdqa  xmm10, xmmword ptr [rdi]\n\t"
                    "movdqa  xmm0,  xmmword ptr [rdi+16]\n\t"
                    "call testFindRowPartCandidates0\n\t"
                    ::"D"(test.xarg),"a"(test.rowpartleft),"b"(test.candidatestarget),"c"(test.candidatesleft),"d"(test.candidatesother):
                     "rsi","r8","r9","r10","r11","r12","r13","r14","r15");
                break;
            case Shiftpos::SHIFT_POS1:
                asm volatile (
                    "movdqa  xmm10, xmmword ptr [rdi]\n\t"
                    "movdqa  xmm0,  xmmword ptr [rdi+16]\n\t"
                    "call testFindRowPartCandidates1\n\t"
                    ::"D"(test.xarg),"a"(test.rowpartleft),"b"(test.candidatestarget),"c"(test.candidatesleft),"d"(test.candidatesother):
                     "rsi","r8","r9","r10","r11","r12","r13","r14","r15");
                break;
            case Shiftpos::SHIFT_POS2:
                asm volatile (
                    "movdqa  xmm10, xmmword ptr [rdi]\n\t"
                    "movdqa  xmm0,  xmmword ptr [rdi+16]\n\t"
                    "call testFindRowPartCandidates2\n\t"
                    ::"D"(test.xarg),"a"(test.rowpartleft),"b"(test.candidatestarget),"c"(test.candidatesleft),"d"(test.candidatesother):
                     "rsi","r8","r9","r10","r11","r12","r13","r14","r15");
                break;
            default:
                break;
            }

            const gRegister actualrowparttarget = testFindRowPartCandidatesRowPartTarget;
            const gRegister actualrowcandidates = testFindRowPartCandidatesRowCandidates;
            const gRegister actualbox = testFindRowPartCandidatesBox;
            CPPUNIT_ASSERT_EQUAL(test.expectedrowparttarget, actualrowparttarget);
            CPPUNIT_ASSERT_EQUAL(test.expectedrowcandidates, actualrowcandidates);
            CPPUNIT_ASSERT_EQUAL(test.expectedbox, actualbox);
            rotateRow(test.prebox);
            rotateRow(test.precolumn);
            rotateRow(test.box);
            rotateRow(test.column);
        }
    }

    return;
}

void SudokuSseTest::test_MergeThreeDiagonalElements()
{
    constexpr size_t SizeOfCells = Sudoku::SizeOfCellsOnBoxEdge;
    struct TestSet {
        SudokuSseElement presetCells[SizeOfCells];
        gRegister expectedCells;
        gRegister expectedMerged;
    };

    constexpr TestSet testSet[] {
        {{0x4000000,0x10000,0x40}, 0x4010040, 0x1c0},
        {{0x800000,0x2000,8}, 0x802008, 0x38},
        {{0x100000,0x400,1}, 0x100401, 7},
        {{0x100000,0x400,0}, 0x100400, 6},
        {{0x100000,0x400,0x101}, 0x100400, 6},
        {{0x100000,0x20200,1}, 0x100001, 5},
        {{0x4040000,0x400,1}, 0x401, 3},
        {{0x40000,0x200,1}, 0x40201, 1}
    };

    for(const auto& test : testSet) {
        union {
            SudokuSseElement regSet[SizeOfCells * RegIndexStep];
            xmmRegister xmmSet[SizeOfCells];
        } xRegSet;

        for(Shiftpos e=Shiftpos::SHIFT_POSMIN; e<Shiftpos::SHIFT_POSCNT; e=static_cast<Shiftpos>(static_cast<int>(e)+1)) {
            ::memset(&xRegSet, 0, sizeof(xRegSet));
            size_t index = static_cast<decltype(index)>(e);
            for (const auto cell : test.presetCells) {
                assert(index < arraySizeof(xRegSet.regSet));
                xRegSet.regSet[index] = cell;
                index += RegIndexStep;
            }

            gRegister actualCells = 0;
            gRegister actualMerged = 0;

            switch(e) {
            case Shiftpos::SHIFT_POS0:
                asm volatile (
                    "movdqa  xmm4, xmmword ptr [rsi]\n\t"
                    "movdqa  xmm5, xmmword ptr [rsi+16]\n\t"
                    "movdqa  xmm6, xmmword ptr [rsi+32]\n\t"
                    "call testMergeThreeDiagonalElements0\n\t"
                    :"=a"(actualCells),"=b"(actualMerged):"S"(&xRegSet):"r8","r9","r10","r11","r15");
                break;
            case Shiftpos::SHIFT_POS1:
                asm volatile (
                    "movdqa  xmm4, xmmword ptr [rsi]\n\t"
                    "movdqa  xmm5, xmmword ptr [rsi+16]\n\t"
                    "movdqa  xmm6, xmmword ptr [rsi+32]\n\t"
                    "call testMergeThreeDiagonalElements1\n\t"
                    :"=a"(actualCells),"=b"(actualMerged):"S"(&xRegSet):"r8","r9","r10","r11","r15");
                break;
            case Shiftpos::SHIFT_POS2:
                asm volatile (
                    "movdqa  xmm4, xmmword ptr [rsi]\n\t"
                    "movdqa  xmm5, xmmword ptr [rsi+16]\n\t"
                    "movdqa  xmm6, xmmword ptr [rsi+32]\n\t"
                    "call testMergeThreeDiagonalElements2\n\t"
                    :"=a"(actualCells),"=b"(actualMerged):"S"(&xRegSet):"r8","r9","r10","r11","r15");
                break;
            default:
                break;
            }

            CPPUNIT_ASSERT_EQUAL(test.expectedCells, actualCells);
            CPPUNIT_ASSERT_EQUAL(test.expectedMerged, actualMerged);
        }
    }
}

void SudokuSseTest::test_MergeNineDiagonalElements()
{
    constexpr size_t SizeOfCells = Sudoku::SizeOfCellsPerGroup;
    struct TestSet {
        SudokuSseElement presetCells[SizeOfCells];
        gRegister expectedLeft;
        gRegister expectedCenter;
        gRegister expectedRight;
    };

    constexpr TestSet testSet[] {
        {{0x4000000,0x10000,0x40,0x800000,0x2000,8,0x100000,0x400,1},
                0x4010040000001c0ull, 0x80200800000038ull, 0x10040100000007ull},
        {{0x4040000,0x10000,0x40,0x800000,0x20200,8,0x100000,0x400,0x101},
                  0x10040000000c0ull, 0x80000800000028ull, 0x10040000000006ull},
        {{0x4000000,0x20200,0x40,0x800000,0x2000,0x101,0x4040000,0x400,1},
                0x400004000000140ull, 0x80200000000030ull,    0x40100000003ull},
        {{0x4000000,0x10000,0x101,0x4040000,0x2000,8,0x100000,0x20200,1},
                0x401000000000180ull,   0x200800000018ull, 0x10000100000005ull}
    };

    for(const auto& test : testSet) {
        union {
            SudokuSseElement regSet[SizeOfCells * RegIndexStep];
            xmmRegister xmmSet[SizeOfCells];
        } xRegSet;
        ::memset(&xRegSet, 0, sizeof(xRegSet));

        size_t cellIndex = 0;
        for(SudokuIndex outboxShift = 0; outboxShift < Sudoku::SizeOfBoxesOnEdge; ++outboxShift) {
            for(SudokuIndex inboxShift = 0; inboxShift < Sudoku::SizeOfCellsOnBoxEdge; ++inboxShift) {
                size_t index = cellIndex * RegIndexStep;
                index += (Sudoku::SizeOfCellsOnBoxEdge - outboxShift - 1);
                assert(index < arraySizeof(xRegSet.regSet));
                assert(cellIndex < arraySizeof(test.presetCells));
                xRegSet.regSet[index] = test.presetCells[cellIndex];
                ++cellIndex;
            }
        }

        gRegister actualLeft = 0;
        gRegister actualCenter = 0;
        gRegister actualRight = 0;

        asm volatile (
            "movdqa  xmm4, xmmword ptr [rsi]\n\t"
            "movdqa  xmm5, xmmword ptr [rsi+16]\n\t"
            "movdqa  xmm6, xmmword ptr [rsi+32]\n\t"
            "movdqa  xmm7, xmmword ptr [rsi+48]\n\t"
            "movdqa  xmm8, xmmword ptr [rsi+64]\n\t"
            "movdqa  xmm9, xmmword ptr [rsi+80]\n\t"
            "movdqa  xmm1, xmmword ptr [rsi+96]\n\t"
            "movdqa  xmm2, xmmword ptr [rsi+112]\n\t"
            "movdqa  xmm3, xmmword ptr [rsi+128]\n\t"
            "call testMergeNineDiagonalElements\n\t"
            :"=a"(actualLeft),"=b"(actualCenter),"=c"(actualRight):"S"(&xRegSet):"rdx","rdi","r8","r9","r10","r11","r12","r15");

        CPPUNIT_ASSERT_EQUAL(test.expectedLeft, actualLeft);
        CPPUNIT_ASSERT_EQUAL(test.expectedCenter, actualCenter);
        CPPUNIT_ASSERT_EQUAL(test.expectedRight, actualRight);
    }
}

void SudokuSseTest::test_FindUnusedOneThreeDiagonalElements()
{
    constexpr size_t SizeOfCells = Sudoku::SizeOfCellsOnBoxEdge;
    union CellSet {
        SudokuSseElement regSet[RegIndexStep * SizeOfCells];
        xmmRegister xmmSet[SizeOfCells];
    };

    for(SudokuIndex outboxShift = 0; outboxShift < Sudoku::SizeOfBoxesOnEdge; ++outboxShift) {
        gRegister srcCells = 0;
        gRegister srcMerged1 = 0;
        gRegister srcMerged2 = 0;
        SudokuSseElement expected = 0;
        SudokuSseElement expectedLeft = Sudoku::AllThreeCandidates;
        SudokuSseElement expectedCenter = Sudoku::AllThreeCandidates;
        SudokuSseElement expectedRight = Sudoku::AllThreeCandidates;
        SudokuSseElement actualLeft = 0;
        SudokuSseElement actualCenter = 0;
        SudokuSseElement actualRight = 0;

        const CellSet fulls = {{Sudoku::AllThreeCandidates, Sudoku::AllThreeCandidates, Sudoku::AllThreeCandidates, 0}};
        CellSet cells = fulls;
        static_assert(sizeof(fulls) == sizeof(cells), "Size mismatched");
        const auto regIndex = RegIndexStep * outboxShift;

        srcCells = 0;
        switch(outboxShift) {
        case 0:
            asm volatile (
                "movdqa  xmm2, xmmword ptr [rdi]\n\t"
                "call testFindUnusedOneDiagonalElements01\n\t"
                "movdqa  xmmword ptr [rdi], xmm2\n\t"
                ::"a"(srcCells),"b"(srcMerged1),"c"(srcMerged2),"D"(&cells):"r8","r9","r15");
            break;
        case 1:
            asm volatile (
                "movdqa  xmm2, xmmword ptr [rdi]\n\t"
                "call testFindUnusedOneDiagonalElements12\n\t"
                "movdqa  xmmword ptr [rdi], xmm2\n\t"
                ::"a"(srcCells),"b"(srcMerged1),"c"(srcMerged2),"D"(&cells):"r8","r9","r15");
            break;
        case 2:
            asm volatile (
                "movdqa  xmm2, xmmword ptr [rdi]\n\t"
                "call testFindUnusedOneDiagonalElements20\n\t"
                "movdqa  xmmword ptr [rdi], xmm2\n\t"
                ::"a"(srcCells),"b"(srcMerged1),"c"(srcMerged2),"D"(&cells):"r8","r9","r15");
            break;
        default:
            break;
        }

        assert(regIndex < arraySizeof(cells.regSet));
        auto actual = cells.regSet[regIndex];
        CPPUNIT_ASSERT_EQUAL(fulls.regSet[regIndex], actual);

        ::memmove(&cells, &fulls, sizeof(cells));
        switch(outboxShift) {
        case 0:
            asm volatile (
                "movdqa  xmm4, xmmword ptr [rdi]\n\t"
                "movdqa  xmm5, xmmword ptr [rdi]\n\t"
                "movdqa  xmm6, xmmword ptr [rdi]\n\t"
                "call testFindUnusedThreeDiagonalElements0\n\t"
                "movdqa  xmmword ptr [rdi], xmm4\n\t"
                "movdqa  xmmword ptr [rdi+16], xmm5\n\t"
                "movdqa  xmmword ptr [rdi+32], xmm6\n\t"
                ::"a"(srcCells),"b"(srcMerged1),"c"(srcMerged2),"D"(&cells):"r8","r9","r10","r11","r15");
            break;
        case 1:
            asm volatile (
                "movdqa  xmm4, xmmword ptr [rdi]\n\t"
                "movdqa  xmm5, xmmword ptr [rdi]\n\t"
                "movdqa  xmm6, xmmword ptr [rdi]\n\t"
                "call testFindUnusedThreeDiagonalElements1\n\t"
                "movdqa  xmmword ptr [rdi], xmm4\n\t"
                "movdqa  xmmword ptr [rdi+16], xmm5\n\t"
                "movdqa  xmmword ptr [rdi+32], xmm6\n\t"
                ::"a"(srcCells),"b"(srcMerged1),"c"(srcMerged2),"D"(&cells):"r8","r9","r10","r11","r15");
            break;
        case 2:
            asm volatile (
                "movdqa  xmm4, xmmword ptr [rdi]\n\t"
                "movdqa  xmm5, xmmword ptr [rdi]\n\t"
                "movdqa  xmm6, xmmword ptr [rdi]\n\t"
                "call testFindUnusedThreeDiagonalElements2\n\t"
                "movdqa  xmmword ptr [rdi], xmm4\n\t"
                "movdqa  xmmword ptr [rdi+16], xmm5\n\t"
                "movdqa  xmmword ptr [rdi+32], xmm6\n\t"
                ::"a"(srcCells),"b"(srcMerged1),"c"(srcMerged2),"D"(&cells):"r8","r9","r10","r11","r15");
            break;
        default:
            break;
        }

        assert(regIndex < arraySizeof(cells.regSet));
        actual = cells.regSet[regIndex];

        assert(outboxShift < arraySizeof(cells.regSet));
        actualLeft = cells.regSet[outboxShift];
        CPPUNIT_ASSERT_EQUAL(expectedLeft, actualLeft);

        auto index = RegIndexStep + outboxShift;
        assert(index < arraySizeof(cells.regSet));
        actualCenter = cells.regSet[index];
        CPPUNIT_ASSERT_EQUAL(expectedCenter, actualCenter);

        index = RegIndexStep * 2 + outboxShift;
        assert(index < arraySizeof(cells.regSet));
        actualRight = cells.regSet[index];
        CPPUNIT_ASSERT_EQUAL(expectedRight, actualRight);

        ::memmove(&cells, &fulls, sizeof(cells));
        srcMerged1 = 0x70;
        srcMerged2 = 0xf;
        switch(outboxShift) {
        case 0:
            srcCells = 0x100;
            asm volatile (
                "movdqa  xmm2, xmmword ptr [rdi]\n\t"
                "call testFindUnusedOneDiagonalElements01\n\t"
                "movdqa  xmmword ptr [rdi], xmm2\n\t"
                ::"a"(srcCells),"b"(srcMerged1),"c"(srcMerged2),"D"(&cells):"r8","r9","r15");
            expected = 0x7fd01ff;
            break;
        case 1:
            srcCells = 0x20000;
            asm volatile (
                "movdqa  xmm2, xmmword ptr [rdi]\n\t"
                "call testFindUnusedOneDiagonalElements12\n\t"
                "movdqa  xmmword ptr [rdi], xmm2\n\t"
                ::"a"(srcCells),"b"(srcMerged1),"c"(srcMerged2),"D"(&cells):"r8","r9","r15");
            expected = 0x203ffff;
            break;
        case 2:
            srcCells = 0x4000000;
            asm volatile (
                "movdqa  xmm2, xmmword ptr [rdi]\n\t"
                "call testFindUnusedOneDiagonalElements20\n\t"
                "movdqa  xmmword ptr [rdi], xmm2\n\t"
                ::"a"(srcCells),"b"(srcMerged1),"c"(srcMerged2),"D"(&cells):"r8","r9","r15");
            expected = 0x7fffe80;
            break;
        default:
            break;
        }

        assert(outboxShift < arraySizeof(cells.regSet));
        actual = cells.regSet[outboxShift];
        CPPUNIT_ASSERT_EQUAL(expected, actual);

        ::memmove(&cells, &fulls, sizeof(cells));
        // 100 000 000, 010 000 000, 001 000 000 = 100 0000 0001 0000 0000 0100 0000
        // 011 111 111, 101 111 111, 110 111 111 = 011 1111 1110 1111 1111 1011 1111
        // 100 000 111, 010 000 111, 001 000 111 = 100 0001 1111 1111 1111 1111 1111
        // 100 000 111, 010 000 111, 001 000 111 = 111 1111 1101 0000 1111 1111 1111
        // 100 000 000, 010 000 000, 001 000 000 = 111 1111 1111 1111 1110 0100 0111

        srcCells = 0x4010040; // {7,8,9}
        srcMerged1 = 0x8;     // {4}
        srcMerged2 = 0x30;    // {5,6}
        expectedLeft = 0x41fffff;    // {1,2,3,9}, all, all
        expectedCenter = 0x7fd0fff;  // all, {1,2,3,8}, all
        expectedRight = 0x7fffe47;   // all, all, {1,2,3,7}
        actualLeft = 0;
        actualCenter = 0;
        actualRight = 0;

        switch(outboxShift) {
        case 0:
            asm volatile (
                "movdqa  xmm4, xmmword ptr [rdi]\n\t"
                "movdqa  xmm5, xmmword ptr [rdi]\n\t"
                "movdqa  xmm6, xmmword ptr [rdi]\n\t"
                "call testFindUnusedThreeDiagonalElements0\n\t"
                "movdqa  xmmword ptr [rdi], xmm4\n\t"
                "movdqa  xmmword ptr [rdi+16], xmm5\n\t"
                "movdqa  xmmword ptr [rdi+32], xmm6\n\t"
                ::"a"(srcCells),"b"(srcMerged1),"c"(srcMerged2),"D"(&cells):"r8","r9","r10","r11","r15");
            break;
        case 1:
            asm volatile (
                "movdqa  xmm4, xmmword ptr [rdi]\n\t"
                "movdqa  xmm5, xmmword ptr [rdi]\n\t"
                "movdqa  xmm6, xmmword ptr [rdi]\n\t"
                "call testFindUnusedThreeDiagonalElements1\n\t"
                "movdqa  xmmword ptr [rdi], xmm4\n\t"
                "movdqa  xmmword ptr [rdi+16], xmm5\n\t"
                "movdqa  xmmword ptr [rdi+32], xmm6\n\t"
                ::"a"(srcCells),"b"(srcMerged1),"c"(srcMerged2),"D"(&cells):"r8","r9","r10","r11","r15");
            break;
        case 2:
            asm volatile (
                "movdqa  xmm4, xmmword ptr [rdi]\n\t"
                "movdqa  xmm5, xmmword ptr [rdi]\n\t"
                "movdqa  xmm6, xmmword ptr [rdi]\n\t"
                "call testFindUnusedThreeDiagonalElements2\n\t"
                "movdqa  xmmword ptr [rdi], xmm4\n\t"
                "movdqa  xmmword ptr [rdi+16], xmm5\n\t"
                "movdqa  xmmword ptr [rdi+32], xmm6\n\t"
                ::"a"(srcCells),"b"(srcMerged1),"c"(srcMerged2),"D"(&cells):"r8","r9","r10","r11","r15");
            break;
        default:
            break;
        }

        assert(outboxShift < arraySizeof(cells.regSet));
        actualLeft = cells.regSet[outboxShift];

        index = RegIndexStep + outboxShift;
        assert(index < arraySizeof(cells.regSet));
        actualCenter = cells.regSet[RegIndexStep + outboxShift];

        index = RegIndexStep * 2 + outboxShift;
        assert(index < arraySizeof(cells.regSet));
        actualRight = cells.regSet[RegIndexStep * 2 + outboxShift];

        CPPUNIT_ASSERT_EQUAL(expectedLeft, actualLeft);
        CPPUNIT_ASSERT_EQUAL(expectedCenter, actualCenter);
        CPPUNIT_ASSERT_EQUAL(expectedRight, actualRight);
    }
}

void SudokuSseTest::test_FindUnusedNineDiagonalElements()
{
    constexpr RowRegisterSet XmmRegSet = {
        0x7fffe20, 0x7ffffff, 0x7ffff,   0x0,
        0x7ffffff, 0x7ffffff, 0x7ffffff, 0x0,
        0x103ffff, 0x7ffffff, 0x7fffe02, 0x0,
        0x7ffffff, 0x7ffffff, 0x7ffffff, 0x0,
        0x7ffffff, 0x7ffffff, 0x7ffffff, 0x0,
        0x7ffffff, 0x7ffffff, 0x7ffffff, 0x0,
         0x13ffff, 0x7ffffff, 0x7fffe80, 0x0,
        0x7ffffff, 0x7ffffff, 0x7ffffff, 0x0,
        0x7fffe08, 0x7ffffff, 0x403ffff, 0x0
    };

    SudokuSseElement actualCenterCell = 0;
    SudokuSseElement expectedCenterCell = 0x7fc21ff;
    asm volatile (
        "movdqa  xmm1, xmmword ptr [rsi]\n\t"
        "movdqa  xmm2, xmmword ptr [rsi+16]\n\t"
        "movdqa  xmm3, xmmword ptr [rsi+32]\n\t"
        "movdqa  xmm4, xmmword ptr [rsi+48]\n\t"
        "movdqa  xmm5, xmmword ptr [rsi+64]\n\t"
        "movdqa  xmm6, xmmword ptr [rsi+80]\n\t"
        "movdqa  xmm7, xmmword ptr [rsi+96]\n\t"
        "movdqa  xmm8, xmmword ptr [rsi+112]\n\t"
        "movdqa  xmm9, xmmword ptr [rsi+128]\n\t"
        "call testFindUnusedAllDiagonalElements\n\t"
        :"=a"(actualCenterCell):"S"(&XmmRegSet):"rbx","rcx","rdx","r8","r9","r10","r11","r12","r13","r15");
    CPPUNIT_ASSERT_EQUAL(expectedCenterCell, actualCenterCell);
}

void SudokuSseTest::test_CountFilledElements()
{
    struct TestSet {
        gRegister prevpopcnt;
        gRegister loopcnt;
        gRegister preallhigh;
        gRegister prealllow;
        gRegister expectedpopcnt;
    };

    constexpr TestSet testSet[] {
        {0, 0, 0, 0, 0},
        {1, 2, 0xca, 0xedb7, 16},
        {80, 50, 0x7ffffff, 0x7ffffff07ffffff, 81}
    };

    for(const auto& test : testSet) {
        gRegister loopcnt = 0;
        gRegister actualpopcnt = 0;
        gRegister actualprevpopcnt = 0;
        xmmRegister looppopcnt;

        gRegister* pLooppopcntlow = reinterpret_cast<gRegister*>(&looppopcnt);
        gRegister* pLooppopcnthigh = pLooppopcntlow + 1;
        static_assert(sizeof(looppopcnt) == (sizeof(*pLooppopcntlow) + sizeof(*pLooppopcnthigh)),
                      "Unexpected xmmRegister size");
        *pLooppopcntlow = test.prevpopcnt;
        *pLooppopcnthigh = test.loopcnt;

        xmmRegister all;
        static_assert(sizeof(all) == (sizeof(test.prealllow) + sizeof(test.preallhigh)),
                      "Unexpected xmmRegister size");
        *(reinterpret_cast<gRegister*>(&all)) = test.prealllow;
        *(reinterpret_cast<gRegister*>(&all) + 1) = test.preallhigh;

        asm volatile (
            "movdqa  xmm15, xmmword ptr [rsi]\n\t"
            "movdqa  xmm0,  xmmword ptr [rdi]\n\t"
            "call testCountFilledElements\n\t"
            :"=a"(loopcnt),"=b"(actualpopcnt),"=c"(actualprevpopcnt):"S"(&looppopcnt),"D"(&all):"r14","r15");
        CPPUNIT_ASSERT_EQUAL(test.loopcnt, loopcnt);
        CPPUNIT_ASSERT_EQUAL(test.expectedpopcnt, actualpopcnt);
        CPPUNIT_ASSERT_EQUAL(test.prevpopcnt, actualprevpopcnt);

        asm volatile (
            "xorps xmm15, xmm15\n\t"
            "call testSaveLoopCnt\n\t"
            "movdqa  xmmword ptr [rdi], xmm15\n\t"
            ::"a"(loopcnt),"b"(actualpopcnt),"D"(&looppopcnt):"r15");
        CPPUNIT_ASSERT_EQUAL(*pLooppopcntlow, actualpopcnt);
        CPPUNIT_ASSERT_EQUAL(*pLooppopcnthigh, loopcnt);
    }

    return;
}

void SudokuSseTest::test_PopCountOrPowerOf2() {
    constexpr gRegister basePopCount = 128;
    gRegister cellValue = 0;

    for(gRegister candidates=0; candidates <= Sudoku::SizeOfCandidates; ++candidates) {
        for(gRegister i=0; i < Sudoku::SizeOfCellsOnBoxEdge; ++i) {
            gRegister popCount = 0;
            gRegister accumPopCount = 0;
            gRegister arg = cellValue;

            switch(i) {
            case 0:
                asm volatile (
                    "call testPopCountOrPowerOf20\n\t"
                    :"=a"(popCount),"=b"(accumPopCount):"c"(arg),"d"(basePopCount):"r8", "r15");
                break;
            case 1:
                arg = cellValue << Sudoku::SizeOfCandidates;
                asm volatile (
                    "call testPopCountOrPowerOf21\n\t"
                    :"=a"(popCount),"=b"(accumPopCount):"c"(arg),"d"(basePopCount):"r8", "r15");
                break;
            case 2:
            default:
                arg = cellValue << (Sudoku::SizeOfCandidates * 2);
                asm volatile (
                    "call testPopCountOrPowerOf22\n\t"
                    :"=a"(popCount),"=b"(accumPopCount):"c"(arg),"d"(basePopCount):"r8", "r15");
                break;
            }

            const gRegister expectedPopCount = (candidates <= Sudoku::SizeOfUniqueCandidate) ?
                (Sudoku::SizeOfCandidates + 1) : candidates;
            CPPUNIT_ASSERT_EQUAL(expectedPopCount, popCount);
            CPPUNIT_ASSERT_EQUAL(basePopCount + candidates, accumPopCount);
        }

        cellValue <<= 1;
        cellValue |= 1;
    }

    return;
}

void SudokuSseTest::test_CompareRegisterSet() {
    struct TestSet {
        gRegister leftHigh;
        gRegister leftLow;
        gRegister rightHigh;
        gRegister rightLow;
        gRegister zeroFlag;
        gRegister carryFlag;
    };

    constexpr TestSet testSet[] = {
        {0, 0, 0, 0, 1, 0},
        {0, 0, 0, 1, 0, 1},
        {0, 0, 1, 0, 0, 1},
        {0, 1, 0, 0, 0, 0},
        {0, 1, 0, 1, 1, 0},
        {0, 1, 1, 0, 0, 1},
        {1, 0, 0, 0, 0, 0},
        {1, 0, 0, 1, 0, 0},
        {1, 0, 1, 0, 1, 0},
        {1, 0, 1, 1, 0, 1},
        {0xaa, 0xfffe, 0xaa, 0xffff, 0, 1},
        {0xaa, 0xffff, 0xaa, 0xfffe, 0, 0},
        {0xab, 0xfffe, 0xaa, 0xffff, 0, 0}
    };

    for(const auto& test : testSet) {
        gRegister zeroFlag = 0;
        gRegister carryFlag = 0;
        asm volatile (
            "call testCompareRegisterSet64\n\t"
            :"=a"(zeroFlag),"=b"(carryFlag):"c"(test.leftHigh),"d"(test.leftLow),"S"(test.rightHigh),"D"(test.rightLow):"r8", "r9", "r10", "r15");
        CPPUNIT_ASSERT_EQUAL(test.zeroFlag, zeroFlag);
        CPPUNIT_ASSERT_EQUAL(test.carryFlag, carryFlag);

        zeroFlag = 0;
        carryFlag = 0;
        asm volatile (
            "call testCompareRegisterSet32\n\t"
            :"=a"(zeroFlag),"=b"(carryFlag):"c"(test.leftHigh),"d"(test.leftLow),"S"(test.rightHigh),"D"(test.rightLow):"r8", "r9", "r10", "r15");
        CPPUNIT_ASSERT_EQUAL(test.zeroFlag, zeroFlag);
        CPPUNIT_ASSERT_EQUAL(test.carryFlag, carryFlag);
    }

    return;
}

void SudokuSseTest::test_CountCellCandidates() {
    constexpr gRegister basePopCount = 128;
    gRegister cellValue = 0;

    for(gRegister candidates=0; candidates <= Sudoku::SizeOfCandidates; ++candidates) {
        for(gRegister baseMinCount=0; baseMinCount <= (candidates + 1); ++baseMinCount) {
            for(gRegister i=0; i < Sudoku::SizeOfCellsOnBoxEdge; ++i) {
                gRegister outBoxShift = 0;
                gRegister inBoxShift = 0;
                gRegister minCount = 0;
                gRegister popCount = 0;
                gRegister baseCount[2] = {baseMinCount, basePopCount};
                gRegister arg = cellValue << (i * Sudoku::SizeOfCandidates);

                switch(i) {
                case 0:
                    asm volatile (
                        "call testCountCellCandidates10\n\t"
                        :"=a"(outBoxShift),"=b"(inBoxShift),"=c"(minCount),"=d"(popCount):"S"(baseCount),"D"(arg):"r8", "r9", "r10", "r15");
                    break;
                case 1:
                    asm volatile (
                        "call testCountCellCandidates21\n\t"
                        :"=a"(outBoxShift),"=b"(inBoxShift),"=c"(minCount),"=d"(popCount):"S"(baseCount),"D"(arg):"r8", "r9", "r10", "r15");
                    break;
                case 2:
                default:
                    asm volatile (
                        "call testCountCellCandidates02\n\t"
                        :"=a"(outBoxShift),"=b"(inBoxShift),"=c"(minCount),"=d"(popCount):"S"(baseCount),"D"(arg):"r8", "r9", "r10", "r15");
                    break;
                }

                const auto invalid = (candidates <= Sudoku::SizeOfUniqueCandidate) || (candidates > baseMinCount);
                const gRegister expectedOutBoxShift = (invalid) ? InvalidShift : ((i + 1) % 3);
                const gRegister expectedInBoxShift = (invalid) ? InvalidShift : i;
                const gRegister expectedMinCount = (invalid) ? minCount : candidates;
                CPPUNIT_ASSERT_EQUAL(expectedOutBoxShift, outBoxShift);
                CPPUNIT_ASSERT_EQUAL(expectedInBoxShift, inBoxShift);
                CPPUNIT_ASSERT_EQUAL(expectedMinCount, minCount);
                CPPUNIT_ASSERT_EQUAL(basePopCount + candidates, popCount);
            }
        }

        cellValue <<= 1;
        cellValue |= 1;
    }

    return;
}

void SudokuSseTest::test_CountThreeCellCandidates() {
    constexpr gRegister basePopCount = 128;
    struct TestSet {
        gRegister arg;
        gRegister baseMinCount;
        gRegister inBoxShift;
        gRegister minCount;
        gRegister popCount;
    };

    constexpr TestSet testSet[] = {
        {CellSet654, 3, InvalidShift, 3, 15},
        {CellSet654, 4, 0, 4, 15},
        {CellSet654, 5, 0, 4, 15},
        {CellSet654, 6, 0, 4, 15},
        {CellSet654, 7, 0, 4, 15},
        {CellSet546, 4, 1, 4, 15},
        {CellSet456, 4, 2, 4, 15},
        {CellSet401, 4, 2, 4, 5},
    };

    for(const auto& test : testSet) {
        gRegister outBoxShift = 0;
        gRegister inBoxShift = 0;
        gRegister minCount = 0;
        gRegister popCount = 0;
        gRegister baseCount[2] = {test.baseMinCount, basePopCount};

        asm volatile (
            "call testCountThreeCellCandidates\n\t"
            :"=a"(outBoxShift),"=b"(inBoxShift),"=c"(minCount),"=d"(popCount):"S"(baseCount),"D"(test.arg):"r8", "r9", "r10", "r15");

        const gRegister expectedOutBoxShift = (test.inBoxShift == InvalidShift) ? InvalidShift : 2;
        CPPUNIT_ASSERT_EQUAL(expectedOutBoxShift, outBoxShift);
        CPPUNIT_ASSERT_EQUAL(test.inBoxShift, inBoxShift);
        CPPUNIT_ASSERT_EQUAL(test.minCount, minCount);
        CPPUNIT_ASSERT_EQUAL(basePopCount + test.popCount, popCount);
    }

    return;
}

void SudokuSseTest::test_CountRowCellCandidatesSub() {
    union RowData {
        SudokuSseElement regVal[4];
        xmmRegister regXmm;
    };
    static_assert(sizeof(RowData) == sizeof(xmmRegister), "Wrong size");

    struct TestSet {
        RowData   arg;
        gRegister baseMinCount;
        gRegister outBoxShift;
        gRegister inBoxShift;
        gRegister minCount;
        gRegister popCount;
    };

    constexpr TestSet testSet[] = {
        {{0, 0, 0, 0}, 3, InvalidShift, InvalidShift, 3, 0},
        {{CellSet654, CellSet654, CellSet654, 0}, 3, InvalidShift, InvalidShift, 3, 45},
        {{CellSet654, CellSet654, CellSet654, 0}, 4, 2, 0, 4, 45},
        {{CellSet654, CellSet654, CellSet654, 0}, 4, 2, 0, 4, 45},
        {{CellSet987, CellSet987, CellSet654, 0}, 4, 2, 0, 4, 63},
        {{CellSet987, CellSet456, CellSet987, 0}, 4, 1, 2, 4, 63},
        {{CellSet546, CellSet987, CellSet987, 0}, 4, 0, 1, 4, 63},
    };

    for(const auto& test : testSet) {
        gRegister outBoxShift = 0;
        gRegister inBoxShift = 0;
        gRegister minCount = 0;
        gRegister popCount = 0;
        testCountRowCellCandidatesMinCount = test.baseMinCount;
        testCountRowCellCandidatesRowPopCount = InvalidRowPopCount;
        testCountRowCellCandidatesRowX = test.arg.regXmm;

        asm volatile (
            "call testCountRowCellCandidatesSub\n\t"
            :"=a"(outBoxShift),"=b"(inBoxShift),"=c"(minCount),"=d"(popCount)::"r8", "r9", "r10", "r11", "r12", "r15");

        CPPUNIT_ASSERT_EQUAL(test.outBoxShift, outBoxShift);
        CPPUNIT_ASSERT_EQUAL(test.inBoxShift, inBoxShift);
        CPPUNIT_ASSERT_EQUAL(test.minCount, minCount);
        CPPUNIT_ASSERT_EQUAL(test.popCount, popCount);

        outBoxShift = 0;
        inBoxShift = 0;
        minCount = 0;
        popCount = 0;
        gRegister rowNumber = 0;
        const gRegister expectedPopCount = (test.outBoxShift == InvalidShift) ? InvalidRowPopCount : test.popCount;
        const gRegister expectedRowNumber = (test.outBoxShift == InvalidShift) ? InvalidRowNumber : CountCellCandidatesRowNumber;
        asm volatile (
            "call testCountRowCellCandidates\n\t"
            :"=a"(outBoxShift),"=b"(inBoxShift),"=c"(minCount),"=d"(popCount),"=S"(rowNumber)::"rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");

        CPPUNIT_ASSERT_EQUAL(test.outBoxShift, outBoxShift);
        CPPUNIT_ASSERT_EQUAL(test.inBoxShift, inBoxShift);
        CPPUNIT_ASSERT_EQUAL(test.minCount, minCount);
        CPPUNIT_ASSERT_EQUAL(expectedPopCount, popCount);
        CPPUNIT_ASSERT_EQUAL(expectedRowNumber, rowNumber);
    }

    return;
}

void SudokuSseTest::test_CountRowCellCandidates() {
    union RowData {
        SudokuSseElement regVal[4];
        xmmRegister regXmm;
    };
    static_assert(sizeof(RowData) == sizeof(xmmRegister), "Wrong size");

    struct TestSet {
        RowData   arg;
        gRegister baseMinCount;
        gRegister basePopCount;
        gRegister outBoxShift;
        gRegister inBoxShift;
        gRegister minCount;
        gRegister popCount;
        gRegister rowNumber;
    };

    constexpr TestSet testSet[] = {
        {{0, 0, 0, 0}, 3, InvalidRowPopCount, InvalidShift, InvalidShift, 3, InvalidRowPopCount, InvalidRowNumber},
        {{CellSet987, CellSet987, CellSet654, 0}, 4, 62, InvalidShift, InvalidShift, 4, 62, InvalidRowNumber},
        {{CellSet987, CellSet987, CellSet654, 0}, 4, 63, InvalidShift, InvalidShift, 4, 63, InvalidRowNumber},
        {{CellSet987, CellSet987, CellSet654, 0}, 4, 64, 2, 0, 4, 63, CountCellCandidatesRowNumber},
        {{CellSet987, CellSet987, CellSet654, 0}, 9, 62, 2, 0, 4, 63, CountCellCandidatesRowNumber}
    };

    for(const auto& test : testSet) {
        gRegister outBoxShift = 0;
        gRegister inBoxShift = 0;
        gRegister minCount = 0;
        gRegister popCount = 0;
        gRegister rowNumber = 0;
        testCountRowCellCandidatesMinCount = test.baseMinCount;
        testCountRowCellCandidatesRowPopCount = test.basePopCount;
        testCountRowCellCandidatesRowX = test.arg.regXmm;

        asm volatile (
            "call testCountRowCellCandidates\n\t"
            :"=a"(outBoxShift),"=b"(inBoxShift),"=c"(minCount),"=d"(popCount),"=S"(rowNumber)::"rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");

        CPPUNIT_ASSERT_EQUAL(test.outBoxShift, outBoxShift);
        CPPUNIT_ASSERT_EQUAL(test.inBoxShift, inBoxShift);
        CPPUNIT_ASSERT_EQUAL(test.minCount, minCount);
        CPPUNIT_ASSERT_EQUAL(test.popCount, popCount);
        CPPUNIT_ASSERT_EQUAL(test.rowNumber, rowNumber);
    }

    return;
}

void SudokuSseTest::test_SearchNextCandidate() {
    RowRegisterSet xmmRegSet;

    for(size_t i = 0; i < arraySizeof(xmmRegSet.regVal); i+=4) {
        xmmRegSet.regVal[i] = 0x40404;
        xmmRegSet.regVal[i + 1] = 0x0202020;
        xmmRegSet.regVal[i + 2] = 0x1010100;
        xmmRegSet.regVal[i + 3] = 0;
    }

    struct TestSet {
        gRegister found;
        gRegister outBoxShift;
        gRegister inBoxShift;
        gRegister rowNumber;
    };

    constexpr TestSet testSet[] = {
        {false, Sudoku::SizeOfBoxesOnEdge + 1, Sudoku::SizeOfCellsOnBoxEdge + 1, Sudoku::SizeOfGroupsPerMap + 1},
        {true, 1, 2, 3},
        {true, 2, 1, 8},
    };

    size_t index = 0;
    for(const auto& test : testSet) {
        ++index;
        gRegister found = 0;
        gRegister outBoxShift = 0;
        gRegister inBoxShift = 0;
        gRegister rowNumber = 0;

        switch(index) {
        case 2:
            xmmRegSet.regVal[13] = 0x3001003;
            xmmRegSet.regVal[17] = 0x7010100;
            break;
        case 3:
            xmmRegSet.regVal[13] = 0x7ffffff;
            xmmRegSet.regVal[17] = 0x7810100;
            xmmRegSet.regVal[34] = 0x1003100;
            break;
        default:
            break;
        }

        asm volatile (
            "movdqa  xmm1,  xmmword ptr [rax]\n\t"
            "movdqa  xmm2,  xmmword ptr [rax+16]\n\t"
            "movdqa  xmm3,  xmmword ptr [rax+32]\n\t"
            "movdqa  xmm4,  xmmword ptr [rax+48]\n\t"
            "movdqa  xmm5,  xmmword ptr [rax+64]\n\t"
            "movdqa  xmm6,  xmmword ptr [rax+80]\n\t"
            "movdqa  xmm7,  xmmword ptr [rax+96]\n\t"
            "movdqa  xmm8,  xmmword ptr [rax+112]\n\t"
            "movdqa  xmm9,  xmmword ptr [rax+128]\n\t"
            "push rax\n\t"
            "call testSearchNextCandidate\n\t"
            "pop  rax\n\t"
            :"=d"(outBoxShift),"=S"(inBoxShift),"=D"(rowNumber),"=c"(found):"a"(&xmmRegSet):"rbx", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");

        CPPUNIT_ASSERT_EQUAL(test.found, found);
        CPPUNIT_ASSERT_EQUAL(test.outBoxShift, outBoxShift);
        CPPUNIT_ASSERT_EQUAL(test.inBoxShift, inBoxShift);
        CPPUNIT_ASSERT_EQUAL(test.rowNumber, rowNumber);
    }

    return;
}

void SudokuSseTest::checkSearchNextCandidate(XmmRegisterSet& xmmRegSet,
                                             gRegister expectedFound, gRegister expectedOutBoxShift,
                                             gRegister expectedInBoxShift, gRegister expectedRowNumber) {
    gRegister actualFound = 0;
    gRegister actualOutBoxShift = 0;
    gRegister actualInBoxShift = 0;
    gRegister actualRowNumber = 0;

    asm volatile (
        "movdqa  xmm1,  xmmword ptr [rax]\n\t"
        "movdqa  xmm2,  xmmword ptr [rax+16]\n\t"
        "movdqa  xmm3,  xmmword ptr [rax+32]\n\t"
        "movdqa  xmm4,  xmmword ptr [rax+48]\n\t"
        "movdqa  xmm5,  xmmword ptr [rax+64]\n\t"
        "movdqa  xmm6,  xmmword ptr [rax+80]\n\t"
        "movdqa  xmm7,  xmmword ptr [rax+96]\n\t"
        "movdqa  xmm8,  xmmword ptr [rax+112]\n\t"
        "movdqa  xmm9,  xmmword ptr [rax+128]\n\t"
        "push rax\n\t"
        "call testSearchNextCandidate\n\t"
        "pop  rax\n\t"
        :"=c"(actualFound),"=d"(actualOutBoxShift),"=S"(actualInBoxShift),"=D"(actualRowNumber):"a"(&xmmRegSet):"rbx", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");

    CPPUNIT_ASSERT_EQUAL(expectedFound, actualFound);
    if (expectedFound) {
        CPPUNIT_ASSERT_EQUAL(expectedOutBoxShift, actualOutBoxShift);
        CPPUNIT_ASSERT_EQUAL(expectedInBoxShift, actualInBoxShift);
        CPPUNIT_ASSERT_EQUAL(expectedRowNumber, actualRowNumber);
    }

    return;
}

void SudokuSseTest::test_SearchNextCandidateParam() {
    XmmRegisterSet xmmRegSet;

    // All cells have no candidates.
    ::memset(&xmmRegSet, 0, sizeof(xmmRegSet));
    checkSearchNextCandidate(xmmRegSet, false, 0, 0, 0);

    // All cells have one candidate.
    SudokuTest::SetAllCellsFilled(0, xmmRegSet);
    checkSearchNextCandidate(xmmRegSet, false, 0, 0, 0);

    // All cells have all 1..9 candidates.
    SudokuTest::SetAllCellsFullCandidates(0, xmmRegSet);
    checkSearchNextCandidate(xmmRegSet, true, 2, 2, 0);

    // A cell in the right box and the bottom row have 8 candidates.
    size_t rowNumber = Sudoku::SizeOfGroupsPerMap - 1;
    size_t regIndex = rowNumber * SudokuSse::RegisterWordCnt + 2;
    xmmRegSet.regVal_[regIndex] = Sudoku::AllThreeCandidates ^ 1;
    checkSearchNextCandidate(xmmRegSet, true, 2, 0, rowNumber);

    // The 4th cell in the top row also has 8 candidates.
    rowNumber = 0;
    regIndex = 1;
    xmmRegSet.regVal_[regIndex] = Sudoku::AllThreeCandidates ^ (1 << (Sudoku::SizeOfCandidates * 2));
    checkSearchNextCandidate(xmmRegSet, true, 1, 2, rowNumber);

    // The 2nd cell in the top row also has 8 candidates.
    rowNumber = 0;
    regIndex = 2;
    xmmRegSet.regVal_[regIndex] = Sudoku::AllThreeCandidates ^ (1 << Sudoku::SizeOfCandidates);
    checkSearchNextCandidate(xmmRegSet, true, 2, 1, rowNumber);

    // The bottom-right box has two cells which have two candidates.
    // One of the cells is selected because the box has least candidates.
    rowNumber = 7;
    regIndex = rowNumber * SudokuSse::RegisterWordCnt;
    xmmRegSet.regVal_[regIndex] = Sudoku::AllThreeCandidates ^ (0x1d7 << Sudoku::SizeOfCandidates);
    checkSearchNextCandidate(xmmRegSet, true, 0, 1, rowNumber);

    // Cells which have no candidates are not selected.
    regIndex = 4 * SudokuSse::RegisterWordCnt + 1;
    xmmRegSet.regVal_[regIndex] = Sudoku::AllThreeCandidates ^ (Sudoku::AllCandidates << Sudoku::SizeOfCandidates);
    checkSearchNextCandidate(xmmRegSet, true, 0, 1, rowNumber);

    // Cells which have only one candidate are not selected.
    xmmRegSet.regVal_[regIndex] = Sudoku::AllThreeCandidates ^ (0x100 << Sudoku::SizeOfCandidates);
    checkSearchNextCandidate(xmmRegSet, true, 0, 1, rowNumber);

    return;
}

void SudokuSseTest::test_FoldRowParts() {
    struct TestSet {
        // Requires alignment for XMM registers
        union {
            gRegister arg[2];
            xmmRegister xmmReg;
        };
        gRegister left;
        gRegister center;
        gRegister right;
    };

    constexpr TestSet testSet[] {
        {{0, 0}, 0, 0, 0},
        {{0x100401, 0}, 0, 0, 7},
        {{0x80200800100401, 0}, 0, 0x38, 7},
        {{0x80200800100401, 0x4010040}, 0x1c0, 0x38, 7}
    };

    for(const auto& test : testSet) {
        gRegister actualLeft = 0;
        gRegister actualCenter = 0;
        gRegister actualRight = 0;

        asm volatile (
            "movdqa  xmm1, xmmword ptr [rsi]\n\t"
            "call testFoldRowParts\n\t"
            :"=a"(actualLeft),"=b"(actualCenter),"=c"(actualRight):"S"(&test.xmmReg):"r8","r9","r15");

        CPPUNIT_ASSERT_EQUAL(test.left, actualLeft);
        CPPUNIT_ASSERT_EQUAL(test.center, actualCenter);
        CPPUNIT_ASSERT_EQUAL(test.right, actualRight);
    }

    return;
}

void SudokuSseTest::test_CheckRow() {
    struct TestSet {
        // Requires alignment for XMM registers
        union {
            gRegister arg[2];
            xmmRegister xmmReg;
        };
        gRegister expected;
    };

    constexpr TestSet testSet[] {
        {{0, 0}, 1},
        {{0x40200800100401, 0x4010040}, 1},
        {{0x80200800100401, 0x4010040}, 0}
    };

    for(const auto& test : testSet) {
        gRegister actual = 2;
        asm volatile (
            "movdqa  xmm1, xmmword ptr [rsi]\n\t"
            "call testCheckRow\n\t"
            :"=a"(actual):"S"(&test.xmmReg):"r8","r9","r10","r11","r12","r13","r15");
        CPPUNIT_ASSERT_EQUAL(test.expected, actual);
    }

    return;
}

void SudokuSseTest::test_CheckRowSet() {
    const gRegister filledXmmReg[] = {0x80200800100401, 0x4010040};
    const gRegister voidXmmReg[] = {0x80200800100402, 0x4010040};

    constexpr size_t maxPlueOne = 10;
    for(size_t i=1; i<=maxPlueOne; ++i) {
        XmmRegisterSet xmmRegSet;
        ::memset(&xmmRegSet, 0, sizeof(xmmRegSet));
        for(size_t j=1; j<maxPlueOne; ++j) {
            ::memmove(&(xmmRegSet.regXmmVal_[j]), filledXmmReg, sizeof(filledXmmReg));
        }

        if (i < maxPlueOne) {
            ::memmove(&(xmmRegSet.regXmmVal_[i]), voidXmmReg, sizeof(voidXmmReg));
        }
        gRegister expected = (i == maxPlueOne) ? 0 : 1;

        gRegister actual = 0;
        asm volatile (
            "movdqa  xmm1, xmmword ptr [rsi+16]\n\t"
            "movdqa  xmm2, xmmword ptr [rsi+32]\n\t"
            "movdqa  xmm3, xmmword ptr [rsi+48]\n\t"
            "movdqa  xmm4, xmmword ptr [rsi+64]\n\t"
            "movdqa  xmm5, xmmword ptr [rsi+80]\n\t"
            "movdqa  xmm6, xmmword ptr [rsi+96]\n\t"
            "movdqa  xmm7, xmmword ptr [rsi+112]\n\t"
            "movdqa  xmm8, xmmword ptr [rsi+128]\n\t"
            "movdqa  xmm9, xmmword ptr [rsi+144]\n\t"
            "call testCheckRowSet\n\t"
            :"=a"(actual):"S"(&xmmRegSet):"r8","r9","r10","r11","r12","r13","r15");

        CPPUNIT_ASSERT_EQUAL(expected, actual);
    }

    return;
}

void SudokuSseTest::test_CheckColumn() {
    struct TestSet {
        // Requires alignment for XMM registers
        union {
            gRegister arg[2];
            xmmRegister xmmReg;
        };
        gRegister expected;
    };

    constexpr TestSet testSet[] {
        {{0, 0}, 1},
        {{0x3ffffff07ffffff, 0x7ffffff}, 1},
        {{0x7ffffff03ffffff, 0x7ffffff}, 1},
        {{0x7ffffff07ffffff, 0x3ffffff}, 1},
        {{0x7ffffff07ffffff, 0x7ffffff}, 0}
    };

    for(const auto& test : testSet) {
        gRegister actual = 2;
        asm volatile (
            "movdqa  xmm0, xmmword ptr [rsi]\n\t"
            "call testCheckColumn\n\t"
            :"=a"(actual):"S"(&test.xmmReg):"rbx","rcx","rdx","r8","r15");

        CPPUNIT_ASSERT_EQUAL(test.expected, actual);
    }

    return;
}

void SudokuSseTest::test_CheckBox() {
    struct TestSet {
        // Requires alignment for XMM registers
        union {
            gRegister arg[2];
            xmmRegister xmmReg;
        };
        gRegister expected;
    };

    constexpr TestSet testSet[] {
        {{0, 0}, 1},
        {{0x400700707007007, 0x7007007}, 1},
        {{0x700700704007007, 0x7007007}, 1},
        {{0x700700707007007, 0x4007007}, 1},
        {{0x700700707007007, 0x7007007}, 0}
    };

    for(const auto& test : testSet) {
        gRegister actual = 2;
        asm volatile (
            "movdqa  xmm10, xmmword ptr [rsi]\n\t"
            "call testCheckBox\n\t"
            :"=a"(actual):"S"(&test.xmmReg):"r8","r9","r10","r11","r12","r13","r15");

        CPPUNIT_ASSERT_EQUAL(test.expected, actual);
    }

    return;
}

void SudokuSseTest::test_CheckSetBox() {
    const gRegister filledXmmReg[] = {0x700700707007007, 0x7007007};
    const gRegister voidXmmReg[] = {0x400700707007007, 0x7007007};

    constexpr size_t maxPlueOne = 13;
    for(size_t i=10; i<=maxPlueOne; ++i) {
        XmmRegisterSet xmmRegSet;
        ::memset(&xmmRegSet, 0, sizeof(xmmRegSet));
        for(size_t j=10; j<maxPlueOne; ++j) {
            ::memmove(&(xmmRegSet.regXmmVal_[j]), filledXmmReg, sizeof(filledXmmReg));
        }

        if (i < maxPlueOne) {
            ::memmove(&(xmmRegSet.regXmmVal_[i]), voidXmmReg, sizeof(voidXmmReg));
        }
        gRegister expected = (i == maxPlueOne) ? 0 : 1;

        gRegister actual = 0;
        asm volatile (
            "movdqa  xmm10, xmmword ptr [rsi+160]\n\t"
            "movdqa  xmm11, xmmword ptr [rsi+176]\n\t"
            "movdqa  xmm12, xmmword ptr [rsi+192]\n\t"
            "call testCheckSetBox\n\t"
            :"=a"(actual):"S"(&xmmRegSet):"r8","r9","r10","r11","r12","r13","r15");

        CPPUNIT_ASSERT_EQUAL(expected, actual);
    }

    return;
}

void SudokuSseTest::test_CheckDiagonalThreeCells() {
    constexpr RowRegisterSet XmmRegSet = {
        0x100000, 0x800000, 0x4000000, 0x0,
        0x400, 0x2000, 0x10000, 0x0,
        0x1, 0x8, 0x40, 0x0};

    SudokuSseElement actual = 0;
    asm volatile (
        "movdqa  xmm4, xmmword ptr [rdi]\n\t"
        "movdqa  xmm5, xmmword ptr [rdi+16]\n\t"
        "movdqa  xmm6, xmmword ptr [rdi+32]\n\t"
        "call testCheckDiagonalThreeCells0\n\t"
        :"=a"(actual):"D"(&XmmRegSet):"r8","r15");
    SudokuSseElement expected = 0x7;
    CPPUNIT_ASSERT_EQUAL(expected, actual);

    actual = 0;
    asm volatile (
        "movdqa  xmm4, xmmword ptr [rdi]\n\t"
        "movdqa  xmm5, xmmword ptr [rdi+16]\n\t"
        "movdqa  xmm6, xmmword ptr [rdi+32]\n\t"
        "call testCheckDiagonalThreeCells1\n\t"
        :"=a"(actual):"D"(&XmmRegSet):"r8","r15");
    expected = 0x38;
    CPPUNIT_ASSERT_EQUAL(expected, actual);

    actual = 0;
    asm volatile (
        "movdqa  xmm4, xmmword ptr [rdi]\n\t"
        "movdqa  xmm5, xmmword ptr [rdi+16]\n\t"
        "movdqa  xmm6, xmmword ptr [rdi+32]\n\t"
        "call testCheckDiagonalThreeCells2\n\t"
        :"=a"(actual):"D"(&XmmRegSet):"r8","r15");
    expected = 0x1c0;
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void SudokuSseTest::test_CheckDiagonalNineCells() {
    RowRegisterSet XmmRegSet = {
        0x7fffe01, 0x7ffffff, 0x403ffff, 0x0,
        0x7fc05ff, 0x7ffffff, 0x7fd01ff, 0x0,
         0x13ffff, 0x7ffffff, 0x7fffe40, 0x0,
        0x7ffffff,  0x23fe20, 0x7ffffff, 0x0,
        0x7ffffff, 0x7ffffff, 0x7ffffff, 0x0,
        0x7ffffff,  0x23fe20, 0x7ffffff, 0x0,
         0x13ffff, 0x7ffffff, 0x7fffe40, 0x0,
        0x7fc05ff, 0x7ffffff, 0x7fd01ff, 0x0,
        0x7fffe01, 0x7ffffff, 0x403ffff, 0x0
    };

    SudokuSseElement expected = 0;
    const SudokuSseElement centerSet [] = {0x7fc21ff, 0};
    for(auto center : centerSet) {
        constexpr size_t centerIndex = 17;
        assert(centerIndex < arraySizeof(XmmRegSet.regVal));
        XmmRegSet.regVal[centerIndex] = center;
        SudokuSseElement actual = 0;

        asm volatile (
            "movdqa  xmm1,  xmmword ptr [rsi]\n\t"
            "movdqa  xmm2,  xmmword ptr [rsi+16]\n\t"
            "movdqa  xmm3,  xmmword ptr [rsi+32]\n\t"
            "movdqa  xmm4,  xmmword ptr [rsi+48]\n\t"
            "movdqa  xmm5,  xmmword ptr [rsi+64]\n\t"
            "movdqa  xmm6,  xmmword ptr [rsi+80]\n\t"
            "movdqa  xmm7,  xmmword ptr [rsi+96]\n\t"
            "movdqa  xmm8,  xmmword ptr [rsi+112]\n\t"
            "movdqa  xmm9,  xmmword ptr [rsi+128]\n\t"
            "push rsi\n\t"
            "call testCheckDiagonalNineCells\n\t"
            "pop  rsi\n\t"
            :"=a"(actual):"S"(&XmmRegSet):"r8","r9","r10","r11","r15");
        CPPUNIT_ASSERT_EQUAL(expected, actual);

        if (DiagonalSudokuMode) {
            asm volatile (
                "movdqa  xmm1,  xmmword ptr [rsi]\n\t"
                "movdqa  xmm2,  xmmword ptr [rsi+16]\n\t"
                "movdqa  xmm3,  xmmword ptr [rsi+32]\n\t"
                "movdqa  xmm4,  xmmword ptr [rsi+48]\n\t"
                "movdqa  xmm5,  xmmword ptr [rsi+64]\n\t"
                "movdqa  xmm6,  xmmword ptr [rsi+80]\n\t"
                "movdqa  xmm7,  xmmword ptr [rsi+96]\n\t"
                "movdqa  xmm8,  xmmword ptr [rsi+112]\n\t"
                "movdqa  xmm9,  xmmword ptr [rsi+128]\n\t"
                "push rsi\n\t"
                "call testCheckConsistency\n\t"
                "pop  rsi\n\t"
                :"=a"(actual):"S"(&XmmRegSet):"rdx","rcx","rdx","r8","r9","r10");
            CPPUNIT_ASSERT_EQUAL(expected, actual);
        }

        expected = 1;
    }
}

void SudokuSseTest::test_CheckConsistency() {
    if (DiagonalSudokuMode) {
        // Check it in test_CheckDiagonalNineCells()
        return;
    }

    // SudokuTestPattern::NoBacktrackString
    constexpr RowRegisterSet originalXmmRegSet = {
        0x0084040, 0x0101100, 0x2000210, 0x0,
        0x2020004, 0x0802001, 0x1000408, 0x0,
        0x0200210, 0x2008002, 0x0120020, 0x0,
        0x0800900, 0x0400480, 0x0041040, 0x0,
        0x1001001, 0x4000820, 0x0410002, 0x0,
        0x0410002, 0x0200240, 0x4004004, 0x0,
        0x4002020, 0x1010008, 0x0080801, 0x0,
        0x0100408, 0x0060010, 0x0808080, 0x0,
        0x0048080, 0x0084004, 0x0202100, 0x0
    };

    for(int i=0; i<4; ++i) {
        RowRegisterSet xmmRegSet = originalXmmRegSet;
        gRegister result = 2;
        gRegister expected = (i == 0) ? 0 : 1;
        switch (i) {
        case 1:
            xmmRegSet.regVal[0] = 0x84010;
            break;
        case 2:
            xmmRegSet.regVal[34] = 0x2020800;
            break;
        case 3:
            xmmRegSet.regVal[17] = 0x4000020;
            break;
        case 0:
        default:
            break;
        }

        asm volatile (
            "movdqa  xmm1,  xmmword ptr [rsi]\n\t"
            "movdqa  xmm2,  xmmword ptr [rsi+16]\n\t"
            "movdqa  xmm3,  xmmword ptr [rsi+32]\n\t"
            "movdqa  xmm4,  xmmword ptr [rsi+48]\n\t"
            "movdqa  xmm5,  xmmword ptr [rsi+64]\n\t"
            "movdqa  xmm6,  xmmword ptr [rsi+80]\n\t"
            "movdqa  xmm7,  xmmword ptr [rsi+96]\n\t"
            "movdqa  xmm8,  xmmword ptr [rsi+112]\n\t"
            "movdqa  xmm9,  xmmword ptr [rsi+128]\n\t"
            "push rsi\n\t"
            "call testCheckConsistency\n\t"
            "pop  rsi\n\t"
            :"=a"(result):"S"(&xmmRegSet):"rdx", "rcx", "rdx", "r8", "r9", "r10");

        CPPUNIT_ASSERT_EQUAL(expected, result);
    }

    return;
}

void SudokuSseTest::test_FastCollectCandidatesAtRow() {
    struct TestSet {
        gRegister arghigh;
        gRegister arglow;
        gRegister righttop;
        gRegister rightbottom;
        gRegister expectedsum;
        gRegister expectedtarget;
    };

    enum class Target {
        POS_MIN,
        POS_0_0 = POS_MIN,
        POS_1_0,
        POS_2_0,
        POS_3_0,
        POS_4_0,
        POS_5_0,
        POS_6_0,
        POS_7_0,
        POS_8_0,
        POS_8_8,
        POS_CNT
    };

    constexpr TestSet testSet00[] {
        {0, 0, 0, 0, 0, 0},
        {0, 0x100, 0, 0, 0, 0x100},
        {0, 0, 0x100, 0, 0x100, 0},
        {0, 0x1000200040008ull, 0x100, 0x1ff, 0x107, 0x8},
        {0x80004000200010, 0x8000400020001ull, 0x100, 0x1ff, 0x1fe, 0x1},
    };

    constexpr TestSet testSet10[] {{0x80004000200010, 0x8000400020001, 0x100, 0x1ff, 0x1fd, 0x2}};
    constexpr TestSet testSet20[] {{0x80004000200010, 0x8000400020001, 0x100, 0x1ff, 0x1fb, 0x4}};
    constexpr TestSet testSet30[] {{0x80004000200010, 0x8000400020001, 0x100, 0x1ff, 0x1f7, 0x8}};
    constexpr TestSet testSet40[] {{0x80004000200010, 0x8000400020001, 0x100, 0x1ff, 0x1ef, 0x10}};
    constexpr TestSet testSet50[] {{0x80004000200010, 0x8000400020001, 0x100, 0x1ff, 0x1df, 0x20}};
    constexpr TestSet testSet60[] {{0x80004000200010, 0x8000400020001, 0x100, 0x1ff, 0x1bf, 0x40}};
    constexpr TestSet testSet70[] {{0x80004000200010, 0x8000400020001, 0x100, 0x1ff, 0x17f, 0x80}};

    constexpr TestSet testSet80[] {
        {0, 0, 0, 0, 0, 0},
        {0, 0, 0x100, 0, 0, 0x100},
        {0, 0x1000200040008ull, 0x100, 0x1ff, 0xf, 0x100},
        {0x100004000200010, 0x8000400020001, 0x80, 0x1ff, 0x17f, 0x80},
    };

    constexpr TestSet testSet88[] {
        {0, 0, 0, 0, 0, 0},
        {0, 0, 0x100, 0x100, 0, 0x100},
        {0, 0x1000200040008ull, 0x100, 0x100, 0xf, 0x100},
        {0x100004000200010, 0x8000400020001, 0x80, 0x80, 0x17f, 0x80},
    };

    for(Target target=Target::POS_MIN; target<Target::POS_CNT; target=static_cast<Target>(static_cast<int>(target)+1)) {
        size_t numberOfCases = 0;
        const TestSet* testSet = nullptr;
        switch(target) {
        case Target::POS_0_0:
            numberOfCases = arraySizeof(testSet00);
            testSet = testSet00;
            break;
        case Target::POS_1_0:
            numberOfCases = arraySizeof(testSet10);
            testSet = testSet10;
            break;
        case Target::POS_2_0:
            numberOfCases = arraySizeof(testSet20);
            testSet = testSet20;
            break;
        case Target::POS_3_0:
            numberOfCases = arraySizeof(testSet30);
            testSet = testSet30;
            break;
        case Target::POS_4_0:
            numberOfCases = arraySizeof(testSet40);
            testSet = testSet40;
            break;
        case Target::POS_5_0:
            numberOfCases = arraySizeof(testSet50);
            testSet = testSet50;
            break;
        case Target::POS_6_0:
            numberOfCases = arraySizeof(testSet60);
            testSet = testSet60;
            break;
        case Target::POS_7_0:
            numberOfCases = arraySizeof(testSet70);
            testSet = testSet70;
            break;
        case Target::POS_8_0:
            numberOfCases = arraySizeof(testSet80);
            testSet = testSet80;
            break;
        case Target::POS_8_8:
            numberOfCases = arraySizeof(testSet88);
            testSet = testSet88;
            break;
        default:
            break;
        }

        for(size_t i=0; i <numberOfCases; ++i) {
            TestSet test = testSet[i];
            gRegister actualsum;
            gRegister actualtarget;

            xmmRegister arg;
            static_assert(sizeof(arg) == (sizeof(test.arglow) + sizeof(test.arghigh)),
                          "Unexpected xmmRegister size");
            *(reinterpret_cast<gRegister*>(&arg)) = test.arglow;
            *(reinterpret_cast<gRegister*>(&arg) + 1) = test.arghigh;

            switch(target) {
            case Target::POS_0_0:
                asm volatile (
                    "movdqa  xmm1, xmmword ptr [rdi]\n\t"
                    "xorps   xmm10, xmm10\n\t"
                    "pinsrw  xmm10, rdx, 0\n\t"
                    "call testFastCollectCandidatesAtRow00\n\t"
                    :"=a"(actualsum),"=b"(actualtarget):"c"(test.rightbottom),"d"(test.righttop),"D"(&arg):"r8","r9","r10","r11","r12","r13","r14","r15");
                break;
            case Target::POS_1_0:
                asm volatile (
                    "movdqa  xmm1, xmmword ptr [rdi]\n\t"
                    "xorps   xmm10, xmm10\n\t"
                    "pinsrw  xmm10, rdx, 0\n\t"
                    "call testFastCollectCandidatesAtRow10\n\t"
                    :"=a"(actualsum),"=b"(actualtarget):"c"(test.rightbottom),"d"(test.righttop),"D"(&arg):"r8","r9","r10","r11","r12","r13","r14","r15");
                break;
            case Target::POS_2_0:
                asm volatile (
                    "movdqa  xmm1, xmmword ptr [rdi]\n\t"
                    "xorps   xmm10, xmm10\n\t"
                    "pinsrw  xmm10, rdx, 0\n\t"
                    "call testFastCollectCandidatesAtRow20\n\t"
                    :"=a"(actualsum),"=b"(actualtarget):"c"(test.rightbottom),"d"(test.righttop),"D"(&arg):"r8","r9","r10","r11","r12","r13","r14","r15");
                break;
            case Target::POS_3_0:
                asm volatile (
                    "movdqa  xmm1, xmmword ptr [rdi]\n\t"
                    "xorps   xmm10, xmm10\n\t"
                    "pinsrw  xmm10, rdx, 0\n\t"
                    "call testFastCollectCandidatesAtRow30\n\t"
                    :"=a"(actualsum),"=b"(actualtarget):"c"(test.rightbottom),"d"(test.righttop),"D"(&arg):"r8","r9","r10","r11","r12","r13","r14","r15");
                break;
            case Target::POS_4_0:
                asm volatile (
                    "movdqa  xmm1, xmmword ptr [rdi]\n\t"
                    "xorps   xmm10, xmm10\n\t"
                    "pinsrw  xmm10, rdx, 0\n\t"
                    "call testFastCollectCandidatesAtRow40\n\t"
                    :"=a"(actualsum),"=b"(actualtarget):"c"(test.rightbottom),"d"(test.righttop),"D"(&arg):"r8","r9","r10","r11","r12","r13","r14","r15");
                break;
            case Target::POS_5_0:
                asm volatile (
                    "movdqa  xmm1, xmmword ptr [rdi]\n\t"
                    "xorps   xmm10, xmm10\n\t"
                    "pinsrw  xmm10, rdx, 0\n\t"
                    "call testFastCollectCandidatesAtRow50\n\t"
                    :"=a"(actualsum),"=b"(actualtarget):"c"(test.rightbottom),"d"(test.righttop),"D"(&arg):"r8","r9","r10","r11","r12","r13","r14","r15");
                break;
            case Target::POS_6_0:
                asm volatile (
                    "movdqa  xmm1, xmmword ptr [rdi]\n\t"
                    "xorps   xmm10, xmm10\n\t"
                    "pinsrw  xmm10, rdx, 0\n\t"
                    "call testFastCollectCandidatesAtRow60\n\t"
                    :"=a"(actualsum),"=b"(actualtarget):"c"(test.rightbottom),"d"(test.righttop),"D"(&arg):"r8","r9","r10","r11","r12","r13","r14","r15");
                break;
            case Target::POS_7_0:
                asm volatile (
                    "movdqa  xmm1, xmmword ptr [rdi]\n\t"
                    "xorps   xmm10, xmm10\n\t"
                    "pinsrw  xmm10, rdx, 0\n\t"
                    "call testFastCollectCandidatesAtRow70\n\t"
                    :"=a"(actualsum),"=b"(actualtarget):"c"(test.rightbottom),"d"(test.righttop),"D"(&arg):"r8","r9","r10","r11","r12","r13","r14","r15");
                break;
            case Target::POS_8_0:
                asm volatile (
                    "movdqa  xmm1, xmmword ptr [rdi]\n\t"
                    "xorps   xmm10, xmm10\n\t"
                    "pinsrw  xmm10, rdx, 0\n\t"
                    "call testFastCollectCandidatesAtRow80\n\t"
                    :"=a"(actualsum),"=b"(actualtarget):"c"(test.rightbottom),"d"(test.righttop),"D"(&arg):"r8","r9","r10","r11","r12","r13","r14","r15");
                break;
            case Target::POS_8_8:
                asm volatile (
                    "movdqa  xmm9, xmmword ptr [rdi]\n\t"
                    "xorps   xmm10, xmm10\n\t"
                    "call testFastCollectCandidatesAtRow88\n\t"
                    :"=a"(actualsum),"=b"(actualtarget):"c"(test.rightbottom),"d"(test.righttop),"D"(&arg):"r8","r9","r10","r11","r12","r13","r14","r15");
            default:
                break;
            }

            CPPUNIT_ASSERT_EQUAL(test.expectedsum, actualsum);
            CPPUNIT_ASSERT_EQUAL(test.expectedtarget, actualtarget);
        }
    }
}

void SudokuSseTest::test_FastCollectCandidatesAtBox() {
    constexpr gRegister expectedSet[] {0x110, 0x11, 0x11};

    for(size_t i=0; i <arraySizeof(expectedSet); ++i) {
        gRegister actualsum = 0;
        xmmRegister arg;
        static_assert(sizeof(arg) >= (sizeof(gRegister) * 2), "Unexpected xmmRegister size");
        *(reinterpret_cast<gRegister*>(&arg)) = 0;
        *(reinterpret_cast<gRegister*>(&arg) + 1) = 0x10010000000000ull;
        switch(i) {
        case 0:
            asm volatile (
                "movdqa  xmm1, xmmword ptr [rdi]\n\t"
                "call testFastCollectCandidatesAtBox670\n\t"
                :"=a"(actualsum):"D"(&arg):"r8","r9","r10","r11","r12","r13","r14","r15");
            break;
        case 1:
            asm volatile (
                "movdqa  xmm1, xmmword ptr [rdi]\n\t"
                "call testFastCollectCandidatesAtBox780\n\t"
                :"=a"(actualsum):"D"(&arg):"r8","r9","r10","r11","r12","r13","r14","r15");
            break;
        case 2:
            asm volatile (
                "movdqa  xmm9, xmmword ptr [rdi]\n\t"
                "call testFastCollectCandidatesAtBox788\n\t"
                :"=a"(actualsum):"D"(&arg):"r8","r9","r10","r11","r12","r13","r14","r15");
            break;
        default:
            break;
        }

        CPPUNIT_ASSERT_EQUAL(expectedSet[i], actualsum);
    }
}

void SudokuSseTest::test_FastCollectCandidatesAtColumn() {
    constexpr gRegister expectedSet[] {0x1, 0x80, 0x1fd, 0xff};

    for(size_t i=0; i <arraySizeof(expectedSet); ++i) {
        gRegister actualsum = 0;
        switch(i) {
        case 0:
            asm volatile (
                "call testFastCollectCandidatesAtColumn01\n\t"
                :"=a"(actualsum)::"r8","r9","r10","r11","r12","r13","r14","r15");
            break;
        case 1:
            asm volatile (
                "call testFastCollectCandidatesAtColumn71\n\t"
                :"=a"(actualsum)::"r8","r9","r10","r11","r12","r13","r14","r15");
            break;
        case 2:
            asm volatile (
                "call testFastCollectCandidatesAtColumn81\n\t"
                :"=a"(actualsum)::"r8","r9","r10","r11","r12","r13","r14","r15");
            break;
        case 3:
            asm volatile (
                "call testFastCollectCandidatesAtColumn88\n\t"
                :"=a"(actualsum)::"r8","r9","r10","r11","r12","r13","r14","r15");
            break;
        default:
            break;
        }

        CPPUNIT_ASSERT_EQUAL(expectedSet[i], actualsum);
    }
}

void SudokuSseTest::test_FastSetUniqueCandidatesAtCellSub() {
    struct TestSet {
        gRegister rowHigh;
        gRegister rowLow;
        gRegister columnHigh;
        gRegister columnLow;
        gRegister rightBottom;
    };

    constexpr TestSet expectedSet[] {{0, 1, 0, 0, 0}, {0, 0, 0, 1, 0}, {0, 1, 0, 0, 0}, {0, 0, 0, 0, 1}};

    for(size_t i=0; i <arraySizeof(expectedSet); ++i) {
        const auto test = expectedSet[i];
        gRegister rowHigh;
        gRegister rowLow;
        gRegister columnHigh;
        gRegister columnLow;
        gRegister rightBottom;

        switch(i) {
        case 0:
            asm volatile (
                "call testFastSetUniqueCandidatesAtCellSub00\n\t"
                :"=a"(rowHigh),"=b"(rowLow),"=c"(columnHigh),"=d"(columnLow),"=S"(rightBottom)::"r8","r9","r10","r11","r12","r13","r14","r15");
            break;
        case 1:
            asm volatile (
                "call testFastSetUniqueCandidatesAtCellSub80\n\t"
                :"=a"(rowHigh),"=b"(rowLow),"=c"(columnHigh),"=d"(columnLow),"=S"(rightBottom)::"r8","r9","r10","r11","r12","r13","r14","r15");
            break;
        case 2:
            asm volatile (
                "call testFastSetUniqueCandidatesAtCellSub08\n\t"
                :"=a"(rowHigh),"=b"(rowLow),"=c"(columnHigh),"=d"(columnLow),"=S"(rightBottom)::"r8","r9","r10","r11","r12","r13","r14","r15");
            break;
        case 3:
            asm volatile (
                "call testFastSetUniqueCandidatesAtCellSub88\n\t"
                :"=a"(rowHigh),"=b"(rowLow),"=c"(columnHigh),"=d"(columnLow),"=S"(rightBottom)::"r8","r9","r10","r11","r12","r13","r14","r15");
            break;
        default:
            break;
        }

        CPPUNIT_ASSERT_EQUAL(test.rowHigh, rowHigh);
        CPPUNIT_ASSERT_EQUAL(test.rowLow, rowLow);
        CPPUNIT_ASSERT_EQUAL(test.columnHigh, columnHigh);
        CPPUNIT_ASSERT_EQUAL(test.columnLow, columnLow);
        CPPUNIT_ASSERT_EQUAL(test.rightBottom, rightBottom);
    }
}

/*
Local Variables:
mode: c++
coding: utf-8-unix
tab-width: nil
c-file-style: "stroustrup"
End:
*/
