// SSE 4.2 ASM部テスト
// Copyright (C) 2012-2013 Zettsu Tatsuya
//
// クラス定義は下記から流用
// http://www.atmarkit.co.jp/fdotnet/cpptest/cpptest02/cpptest02_03.html

#include <cppunit/extensions/HelperMacros.h>
#include <cassert>
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
    void setUp();
    void tearDown();
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
    SudokuSseEnumeratorMap* pInstance_;
    SudokuOutStream* pSudokuOutStream_;
};

CPPUNIT_TEST_SUITE_REGISTRATION( SudokuSseEnumeratorMapTest );

class SudokuSseTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(SudokuSseTest);
    CPPUNIT_TEST(test_MaskLower32bit);
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
    CPPUNIT_TEST(test_FindRowPartCandidates);
    CPPUNIT_TEST(test_FillThreePartsUniqueCandidates);
    CPPUNIT_TEST(test_CountRowPartElements);
    CPPUNIT_TEST(test_CountFilledElements);
    CPPUNIT_TEST(test_ChooseNextCadidate);
    CPPUNIT_TEST(test_SearchRowPartElements);
    CPPUNIT_TEST(test_SearchRowElements);
    CPPUNIT_TEST(test_FastCollectCandidatesAtRow);
    CPPUNIT_TEST(test_FastCollectCandidatesAtBox);
    CPPUNIT_TEST(test_FastCollectCandidatesAtColumn);
    CPPUNIT_TEST(test_FastSetUniqueCandidatesAtCellSub);
    CPPUNIT_TEST_SUITE_END();

protected:

public:
    void setUp();
    void tearDown();

protected:
    void test_MaskLower32bit();
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
    void test_FindRowPartCandidates();
    void test_FillThreePartsUniqueCandidates();
    void test_CountRowPartElements();
    void test_CountFilledElements();
    void test_ChooseNextCadidate();
    void test_SearchRowPartElements();
    void test_SearchRowElements();
    void test_FastCollectCandidatesAtRow();
    void test_FastCollectCandidatesAtBox();
    void test_FastCollectCandidatesAtColumn();
    void test_FastSetUniqueCandidatesAtCellSub();
private:
    void callIsPowerOf2ToFlags(gRegister& arg, gRegister& result);
    void callPowerOf2or0(gRegister& arg, gRegister& result);
    void callPowerOf2orAll1(gRegister& arg, gRegister& result);
    gRegister rotateRowPart(gRegister arg);
    void rotateRow(uint64_t row[2]);
};

CPPUNIT_TEST_SUITE_REGISTRATION( SudokuSseTest );

void SudokuSseEnumeratorMapTest::setUp() {
    pSudokuOutStream_ = new SudokuOutStream();
    pInstance_ = new SudokuSseEnumeratorMap(pSudokuOutStream_);
}

void SudokuSseEnumeratorMapTest::tearDown() {
    delete pInstance_;
    pInstance_ = 0;
    delete pSudokuOutStream_;
    pSudokuOutStream_ = 0;
}

void SudokuSseEnumeratorMapTest::test_Constructor() {
    do {
        CPPUNIT_ASSERT_EQUAL(static_cast<SudokuCellCandidates>(0), pInstance_->rightBottomElement_);
        CPPUNIT_ASSERT_EQUAL(static_cast<gRegister>(0), pInstance_->firstCell_);
        CPPUNIT_ASSERT_EQUAL(static_cast<SudokuPatternCount>(0), pInstance_->patternNumber_);
        CPPUNIT_ASSERT_EQUAL(pInstance_, pInstance_->pInstance_);
        for(size_t i=0; i <arraySizeof(pInstance_->xmmRegSet_.regVal_); ++i) {
            CPPUNIT_ASSERT_EQUAL(static_cast<SudokuSseElement>(0), pInstance_->xmmRegSet_.regVal_[i]);
        }

        uint64_t actual = sudokuXmmPrintAllCandidate;
        uint64_t expected = 0ull;
        CPPUNIT_ASSERT_EQUAL(expected, actual);
    } while(0);

    CPPUNIT_ASSERT_EQUAL(pInstance_, SudokuSseEnumeratorMap::pInstance_);
}

void SudokuSseEnumeratorMapTest::test_SetToPrint() {
    const SudokuPatternCount testSet[] = {0, 1, 0xffffffff, 0x1000000000ull, 0xffffffffffffffffull};

    for(size_t i=0; i <arraySizeof(testSet); ++i) {
        SudokuPatternCount expected = testSet[i];
        pInstance_->SetToPrint(expected);
        SudokuPatternCount actual = sudokuXmmPrintAllCandidate;
        CPPUNIT_ASSERT_EQUAL(expected, actual);
    }
}

void SudokuSseEnumeratorMapTest::test_presetCell() {
    struct testcase {
        size_t pos;
        size_t shift;
    };

    const testcase testSet[] = {
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
            const testcase test = testSet[cellIndex];
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

const EnumeratorTestCase EnumeratorTestCaseSet[] = {
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
    /* 問題2の解答
       |        |        |        |        |        |        |        |        | */
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
    for(size_t i=0; i <arraySizeof(EnumeratorTestCaseSet); ++i) {
        SudokuOutStream sudokuOutStream;
        SudokuSseEnumeratorMap map(&sudokuOutStream);

        EnumeratorTestCase test = EnumeratorTestCaseSet[i];
        map.Preset(test.arg);
        CPPUNIT_ASSERT_EQUAL(test.firstCell, map.firstCell_);
        CPPUNIT_ASSERT_EQUAL(test.rightBottomElement, map.rightBottomElement_);
        CPPUNIT_ASSERT(sizeof(test.expextedRegVal) < sizeof(map.xmmRegSet_));

        const size_t offset = SudokuSseEnumeratorMap::InitialRegisterNum * SudokuSse::RegisterWordCnt;
        for(size_t j=0; j <arraySizeof(test.expextedRegVal); ++j) {
            if (j < sizeof(map.xmmRegSet_)) {
                CPPUNIT_ASSERT_EQUAL(test.expextedRegVal[j], map.xmmRegSet_.regVal_[j + offset]);
            }
        }
    }
}

void SudokuSseEnumeratorMapTest::test_Print() {
    for(size_t i=0; i <arraySizeof(EnumeratorTestCaseSet); ++i) {
        SudokuOutStream sudokuOutStream;
        SudokuSseEnumeratorMap map(&sudokuOutStream);
        EnumeratorTestCase test = EnumeratorTestCaseSet[i];

        XmmRegisterSet xmmRegSet;
        ::memset(&xmmRegSet, 0, sizeof(xmmRegSet));

        CPPUNIT_ASSERT(sizeof(test.expextedRegVal) < sizeof(map.xmmRegSet_));
        if (sizeof(test.expextedRegVal) >= sizeof(map.xmmRegSet_)) {
            break;
        }

        ::memmove(&(xmmRegSet.regXmmVal_[SudokuSseEnumeratorMap::InitialRegisterNum]),
                  test.expextedRegVal, sizeof(test.expextedRegVal));
        sudokuXmmRightBottomSolved = test.rightBottomElement;

        map.Print(true, xmmRegSet);
        std::string expected(test.expctedPrintSolved);
        std::string actual(sudokuOutStream.str());
        CPPUNIT_ASSERT(expected == actual);

        map.rightBottomElement_ = test.rightBottomElement;
        std::string::size_type pos = expected.find_last_of(':');
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
    const SudokuPatternCount maxCount = 100;
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

    for(size_t i=0; i <arraySizeof(EnumeratorTestCaseSet); ++i) {
        SudokuOutStream sudokuOutStream;
        SudokuSseEnumeratorMap map(&sudokuOutStream);
        map.patternNumber_ = 9875;
        sudokuXmmPrintAllCandidate = 10000;

        EnumeratorTestCase test = EnumeratorTestCaseSet[i];

        CPPUNIT_ASSERT(sizeof(test.expextedRegVal) < sizeof(xmmRegSet));
        if (sizeof(test.expextedRegVal) >= sizeof(xmmRegSet)) {
            break;
        }
        ::memmove(&(xmmRegSet.regXmmVal_[SudokuSseEnumeratorMap::InitialRegisterNum]),
                  test.expextedRegVal, sizeof(test.expextedRegVal));
        sudokuXmmRightBottomSolved = test.rightBottomElement;
        map.PrintFromAsm(xmmRegSet);

        std::string expected("[Pattern 9876]\n");
        expected.append(test.expctedPrintSolved);
        std::string actual(sudokuOutStream.str());
        CPPUNIT_ASSERT(expected == actual);
    }
}

void SudokuSseEnumeratorMapTest::test_Enumerate() {
    const SudokuPatternCount expectedSet[] = {1, 1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,2,2,4, 4,4,4,4,8,12,48,96,288};
    std::string presetStr = "835126749416397258792548631643912875981754362527683194269835417178469523354271986";

    for(size_t i=0; i <arraySizeof(expectedSet); ++i) {
        SudokuOutStream sudokuOutStream;
        SudokuSseEnumeratorMap map(&sudokuOutStream);
        map.Preset(presetStr);
        SudokuPatternCount expected = expectedSet[i];
        CPPUNIT_ASSERT_EQUAL(expected, map.Enumerate());
        presetStr.erase(presetStr.size() - 1);
    }
}

void SudokuSseEnumeratorMapTest::test_GetInstance() {
    CPPUNIT_ASSERT_EQUAL(pInstance_, SudokuSseEnumeratorMap::GetInstance());
}

void SudokuSseEnumeratorMapTest::test_powerOfTwoPlusOne() {
    struct testcase {
        SudokuSseElement arg;
        size_t expected;
    };

    const testcase testSet[] = {
        {1, 1}, {2, 2}, {4, 3}, {8, 4},
        {0x10, 5}, {0x20, 6}, {0x40, 7}, {0x80, 8}, {0x100, 9}};

    for(size_t i=0; i <arraySizeof(testSet); ++i) {
        CPPUNIT_ASSERT_EQUAL(testSet[i].expected, pInstance_->powerOfTwoPlusOne(testSet[i].arg));
    }
}

// 各テスト・ケースの実行直前に呼ばれる
void SudokuSseTest::setUp()
{
    return;
}

// 各テスト・ケースの実行直後に呼ばれる
void SudokuSseTest::tearDown()
{
    return;
}

// これ以降はテスト・ケースの実装内容
enum shiftpos {
    POSMIN,
    POS0 = POSMIN,
    POS1,
    POS2,
    POSCNT,
};

gRegister SudokuSseTest::rotateRowPart(gRegister arg)
{
    gRegister argleftmost = (arg >> 18) & 0x1ff;
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

void SudokuSseTest::test_MaskLower32bit()
{
    struct testcase {
        gRegister arg;
        gRegister result;
        gRegister resulthigh;
        gRegister resultlow;
    };

    enum func {
        MINFUNC,
        MASK = MINFUNC,
        SPLIT,
        MAXFUNC,
    };

    const testcase testSet[] = {
        {0, 0},
        {0x87654321, 0x87654321, 0, 0x87654321},
        {0x1234567800000000, 0, 0x12345678, 0},
        {0xA1B2C3D4E5F67890, 0xE5F67890, 0xA1B2C3D4, 0xE5F67890}
    };

    for(size_t i=0; i <arraySizeof(testSet); ++i) {
        for(int e=static_cast<int>(MINFUNC); e<MAXFUNC; ++e) {
            const testcase test = testSet[i];
            gRegister arg = test.arg;
            gRegister result = 0;
            gRegister resulthigh;
            gRegister resultlow;
            switch(e) {
            case MASK:
                asm volatile (
                    "call testMaskLower32bit\n\t"
                    :"=b"(result):"a"(arg));
                CPPUNIT_ASSERT_EQUAL(test.result, result);
                break;
            case SPLIT:
                asm volatile (
                    "call testSplitRowLowParts\n\t"
                    :"=b"(resulthigh),"=c"(resultlow):"a"(arg));
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

void SudokuSseTest::callIsPowerOf2ToFlags(gRegister& arg, gRegister& result)
{
    asm volatile (
        "call testIsPowerOf2ToFlags\n\t"
        :"=b"(result):"a"(arg));
    return;
}

void SudokuSseTest::callPowerOf2or0(gRegister& arg, gRegister& result)
{
    asm volatile (
        "call testPowerOf2or0\n\t"
        :"=b"(result):"a"(arg):"rcx");
    return;
}

void SudokuSseTest::callPowerOf2orAll1(gRegister& arg, gRegister& result)
{
    asm volatile (
        "call testPowerOf2orAll1\n\t"
        :"=b"(result):"a"(arg):"rcx");
    return;
}

void SudokuSseTest::test_PowerOf2()
{
    const gRegister expectedAll0 = 0;
    const gRegister expectedAll1 = ~0;
    gRegister arg = 0;
    gRegister result = 0;
    gRegister expected = 1;

    // 0
    callIsPowerOf2ToFlags(arg, result);
    CPPUNIT_ASSERT_EQUAL(expected, result);
    expected = 0;
    callPowerOf2or0(arg, result);
    CPPUNIT_ASSERT_EQUAL(expected, result);
    callPowerOf2orAll1(arg, result);
    CPPUNIT_ASSERT_EQUAL(expectedAll1, result);

    // 2のべき乗
    expected = 1;
    arg = 1;
    for(size_t i=0; i < 64; ++i) {
        callIsPowerOf2ToFlags(arg, result);
        CPPUNIT_ASSERT_EQUAL(expected, result);
        callPowerOf2or0(arg, result);
        CPPUNIT_ASSERT_EQUAL(arg, result);
        callPowerOf2orAll1(arg, result);
        CPPUNIT_ASSERT_EQUAL(arg, result);
        arg <<= 1;
    }

    // それ以外
    arg = 3;
    for(size_t i=0; i < 64; ++i) {
        callIsPowerOf2ToFlags(arg, result);
        CPPUNIT_ASSERT_EQUAL(expectedAll0, result);
        callPowerOf2or0(arg, result);
        CPPUNIT_ASSERT_EQUAL(expectedAll0, result);
        callPowerOf2orAll1(arg, result);
        CPPUNIT_ASSERT_EQUAL(expectedAll1, result);
        arg <<= 1;
        arg |= 1;
    }

    return;
}

void SudokuSseTest::test_MergeThreeElements()
{
    struct testcase {
        gRegister arg;
        gRegister expected;
    };

    const testcase testSet[] = {
        {0, 0},
        {0x1, 0x1},
        {0x200, 0x1},
        {0x40000, 0x1},
        {0x700, 0x103},
        {0x4020000, 0x100},
        {0x3c0000f, 0xff},
        {0x7ffffff, 0x1ff}};
    for(size_t i=0; i <arraySizeof(testSet); ++i) {
        const testcase test = testSet[i];
        gRegister arg = test.arg;
        gRegister result = 0;
        asm volatile (
            "call testMergeThreeElements\n\t"
            :"=b"(result):"a"(arg):"rcx","r15");
        CPPUNIT_ASSERT_EQUAL(test.expected, result);
        asm volatile (
            "call testMergeThreeElements2\n\t"
            :"=b"(result):"a"(arg):"rcx","rdx","r15");
        CPPUNIT_ASSERT_EQUAL(test.expected, result);
    }

    return;
}

void SudokuSseTest::test_OrThreeXmmRegs()
{
    struct testcase {
        union {
            struct {
                uint64_t  arg[6];
                uint64_t  expected[2];
            };
            xmmRegister xexpected[4];
        };
    };

    const testcase testSet[] = {
        {{{{0, 0, 0, 0, 0, 0}, {0, 0}}}},
        {{{{0x76543210, 1, 0xfecdba9800000000, 0, 0xf, 0x123456789abcdef0}, {0xfecdba987654321f, 0x123456789abcdef1}}}},
    };

    for(size_t i=0; i <arraySizeof(testSet); ++i) {
        const testcase test = testSet[i];
        xmmRegister xactual;
        asm volatile (
            "xorps   xmm1, xmm1\n\t"
            "movdqa  xmm2, xmmword ptr [%0]\n\t"
            "movdqa  xmm3, xmmword ptr [%0+16]\n\t"
            "movdqa  xmm4, xmmword ptr [%0+32]\n\t"
            "call testOrThreeXmmRegs\n\t"
            "movdqa  xmmword ptr [%1], xmm1\n\t"
            ::"r"(test.arg),"r"(&xactual));

        int actual = memcmp(test.expected, &xactual, sizeof(xactual));
        int expected = 0;
        CPPUNIT_ASSERT_EQUAL(expected, actual);
    }

    return;
}

void SudokuSseTest::test_FilterUniqueCandidatesInRowPartSub()
{
    struct testcase {
        gRegister bitmask;
        gRegister src;
        gRegister expected;
    };

    const testcase testSet[] = {
        {0x1ff, 0, 0},
        {0x1ff, 0x300, 0x300},
        {0x1ff, 0xc08, 0x8},
        {0x1ff, 0x60000, 0x60000},
        {0x1ff, 0x70000, 0x40000},
        {0x1ff << 9, 0x40201, 0x40201},
        {0x1ff << 9, 0x140804, 0x804},
        {0x1ff << 9, 0x400080, 0x400080},
        {0x1ff << 9, 0x600100, 0x100}};
    for(size_t i=0; i <arraySizeof(testSet); ++i) {
        const testcase test = testSet[i];
        gRegister bitmask = test.bitmask;
        gRegister src = test.src;
        gRegister preset = src & (~(bitmask << 9));
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
    struct testcase {
        gRegister arg;
        gRegister expected;
    };

    const testcase testSet[] = {
        {0, 0},
        {0x40201, 0x40201},
        {0x402010, 0x402010},
        {0x4020100, 0x4020100},
        {0x1101010, 0x1010},
        {0x1011011, 0x1000000}};
    for(size_t i=0; i <arraySizeof(testSet); ++i) {
        const testcase test = testSet[i];
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
    struct testcase {
        gRegister arg;
        gRegister sum;
        gRegister haszero;
    };

    const testcase testSet[] = {
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
    for(size_t i=0; i <arraySizeof(testSet); ++i) {
        const testcase test = testSet[i];
        gRegister arg = test.arg;
        gRegister sum = 0;
        gRegister haszero = 0;
        asm volatile (
            "call testCollectUniqueCandidatesInRowPart\n\t"
            :"=b"(sum),"=c"(haszero):"a"(arg):"r10","r11","r12","r13","r14","r15");
        CPPUNIT_ASSERT_EQUAL(test.sum, sum);
        CPPUNIT_ASSERT(((test.haszero == 0) && (haszero <= 0x1ff)) ||
                       ((test.haszero != 0) && (haszero > 0x7fffffffffffffff)));
    }

    return;
}

void SudokuSseTest::test_CollectUniqueCandidatesInLine()
{
    struct testcase {
        gRegister   arghigh;
        gRegister   arglow;
        gRegister   sum;
        gRegister   resulthigh;
        gRegister   resultlow;
        uint64_t    aborted;
    };

    enum func {
        MINFUNC,
        COLLECT = MINFUNC,
        FILTER,
        MAXFUNC,
    };

    const testcase testSet[] = {
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
    for(size_t i=0; i <arraySizeof(testSet); ++i) {
        for(int e=static_cast<int>(MINFUNC); e<MAXFUNC; ++e) {
            const testcase test = testSet[i];
            xmmRegister arg;
            *(reinterpret_cast<gRegister*>(&arg)) = test.arglow;
            *(reinterpret_cast<gRegister*>(&arg) + 1) = test.arghigh;
            gRegister sum = 0;
            xmmRegister result;
            uint64_t actual;
            gRegister resulthigh;
            gRegister resultlow;

            switch(e) {
            case COLLECT:
                asm volatile (
                    "movdqa  xmm1, xmmword ptr [%0]\n\t"
                    "call testCollectUniqueCandidatesInLine\n\t"
                    :"=a"(sum):"r"(&arg):"rbx","rcx","r8","r9","r10","r11","r12","r13","r14","r15");
                actual = sudokuXmmAborted;
                CPPUNIT_ASSERT_EQUAL(test.sum, sum);
                CPPUNIT_ASSERT_EQUAL(test.aborted, actual);
                break;
            case FILTER:
                asm volatile (
                    "movdqa  xmm1, xmmword ptr [%1]\n\t"
                    "call testFilterUniqueCandidatesInLine\n\t"
                    "movdqa  xmmword ptr [%0], xmm14\n\t"
                    ::"r"(&result),"r"(&arg):"r8","r9","r10","r11","r12","r13","r14","r15");
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
    struct testcase {
        uint64_t  arg[6];
        uint64_t  expected[2];
    };

    const testcase testSet[] = {
        {{0, 0, 0, 0, 0, 0}, {0, 0}},
        {{0x7ffffff07ffffff, 0x100401, 0x7ffffff00804008, 0x7ffffff, 0x402010007ffffff}, {0x402010000804008, 0x100401}},
        {{0x3f9fcfe03f9fcfe, 0x4000000, 0x3f9fcfe00000001, 0x3f9fcfe, 0x3f9fcfe03f9fcfe, 0x3f9fcfe}, {1, 0x4000000}}
    };
    for(size_t i=0; i <arraySizeof(testSet); ++i) {
        const testcase test = testSet[i];
        xmmRegister xmmreg[4];
        uint64_t    actual[2];

        assert(sizeof(xmmreg) >= sizeof(test.arg));
        memmove(xmmreg, test.arg, sizeof(test.arg));
        asm volatile (
            "movdqa  xmm1, xmmword ptr [%0]\n\t"
            "movdqa  xmm2, xmmword ptr [%0+16]\n\t"
            "movdqa  xmm3, xmmword ptr [%0+32]\n\t"
            "call    testFCollectUniqueCandidatesInThreeLine\n\t"
            "movdqa  xmmword ptr [%0+48], xmm10\n\t"
            ::"r"(xmmreg):"r8","r9","r10","r11","r12","r13","r14","r15");

        assert(sizeof(actual) >= sizeof(xmmreg[3]));
        memmove(actual, &(xmmreg[3]), sizeof(actual));
        CPPUNIT_ASSERT_EQUAL(test.expected[0], actual[0]);
        CPPUNIT_ASSERT_EQUAL(test.expected[1], actual[1]);
    }

    return;
}

void SudokuSseTest::test_SelectRowParts()
{
    struct testcase {
        gRegister   arghigh;
        gRegister   arglow;
        gRegister   resultrowpart2;
        gRegister   resultrowpart1;
        gRegister   resultrowpart0;
        gRegister   resulthigh;
        gRegister   resultlow;
        gRegister   iszero;
    };

    enum func {
        MINFUNC,
        SELECT0 = MINFUNC,
        SELECT1,
        SELECT2,
        COUNT,
        MAXFUNC,
    };

    const testcase testSet[] = {
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0x7ffffff, 0x7ffffff07ffffff,   0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff07ffffff, 0},
        {0x4010040, 0x80200800100401, 0x4010040, 0x802008, 0x100401, 0x4010040, 0x80200800100401, 1},
        {0x4010040, 0x80200f003c1e0f, 0x4010040, 0x80200f, 0x3c1e0f, 0x4010040, 0x80200f003c1e0f, 0},
        {0x4010040, 0x802000003c1e0f, 0x4010040, 0x802000, 0x3c1e0f, 0x4010040, 0x802000003c1e0f, 0},
    };
    for(size_t i=0; i <arraySizeof(testSet); ++i) {
        for(int e=static_cast<int>(MINFUNC); e<MAXFUNC; ++e) {
            const testcase test = testSet[i];
            xmmRegister arg;
            *(reinterpret_cast<gRegister*>(&arg)) = test.arglow;
            *(reinterpret_cast<gRegister*>(&arg) + 1) = test.arghigh;
            gRegister   resultrowpart;
            gRegister   resulthigh;
            gRegister   resultlow;
            gRegister   iszero;

            switch(e) {
            case SELECT0:
                asm volatile (
                    "movdqa  xmm1, xmmword ptr [%0]\n\t"
                    "call testSelectRowParts0\n\t"
                    :"=a"(resultrowpart):"r"(&arg):"r15");
                CPPUNIT_ASSERT_EQUAL(test.resultrowpart0, resultrowpart);
                break;
            case SELECT1:
                asm volatile (
                    "movdqa  xmm1, xmmword ptr [%0]\n\t"
                    "call testSelectRowParts1\n\t"
                    :"=a"(resultrowpart):"r"(&arg):"r15");
                CPPUNIT_ASSERT_EQUAL(test.resultrowpart1, resultrowpart);
                break;
            case SELECT2:
                asm volatile (
                    "movdqa  xmm1, xmmword ptr [%0]\n\t"
                    "call testSelectRowParts2\n\t"
                    :"=a"(resultrowpart):"r"(&arg):"r15");
                CPPUNIT_ASSERT_EQUAL(test.resultrowpart2, resultrowpart);
                break;
            case COUNT:
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
    struct testcase {
        gRegister   arg;
        gRegister   result2;
        gRegister   result1;
        gRegister   result0;
    };

    enum func {
        MINFUNC,
        SELECT0 = MINFUNC,
        SELECT1,
        SELECT2,
        MAXFUNC,
    };

    const testcase testSet[] = {
        {0, 0, 0, 0},
        {0x7ffffff, 0x1ff, 0x1ff, 0x1ff},
        {0x40201,   1, 1, 1},
        {0x4020100, 0x100, 0x100, 0x100},
        {0x7007007, 0x1c0, 0x38,  7},
    };
    for(size_t i=0; i <arraySizeof(testSet); ++i) {
        for(int e=static_cast<int>(MINFUNC); e<MAXFUNC; ++e) {
            const testcase test = testSet[i];
            gRegister   arg = test.arg;
            gRegister   result;

            switch(e) {
            case SELECT0:
                asm volatile (
                    "call testSelectElementInRowParts0\n\t"
                    :"=b"(result):"a"(arg):"r15");
                CPPUNIT_ASSERT_EQUAL(test.result0, result);
                break;
            case SELECT1:
                asm volatile (
                    "call testSelectElementInRowParts1\n\t"
                    :"=b"(result):"a"(arg):"r15");
                CPPUNIT_ASSERT_EQUAL(test.result1, result);
                break;
            case SELECT2:
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
    struct testcase {
        bool       alreadyunique;
        gRegister  prerowpart;
        gRegister  precolumn;
        gRegister  prebox;
        gRegister  prerow;
        gRegister  candidateset;
        gRegister  uniquecandidate;
        gRegister  expectedrowpart;
    };

    enum func {
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

    const testcase testSet[] = {
        {false, 0x7ffffff, 0, 0, 0, 0, 0, 0x7ffffff},
        {false, 0x7ffffff, 0x8001c0, 0x38, 0x6,  0x1fe, 1, 0x7fffe01},
        {false, 0x7fffff7, 0x800008, 0x10, 0x30, 0x38,  0, 0x7ffffc7},
        {true,  0x7fff010, 0x10,     0x10, 0x10, 0x10,  0, 0x7fff010}
    };
    for(size_t i=0; i <arraySizeof(testSet); ++i) {
        const testcase test = testSet[i];
        gRegister  prerowpart = test.prerowpart;
        gRegister  precolumn = test.precolumn;
        gRegister  prebox = test.prebox;
        gRegister  prerow = test.prerow;
        gRegister  uniquecandidate = test.uniquecandidate;
        gRegister  expectedrowpart = test.expectedrowpart;
        gRegister  expectedcolumn = 0;
        gRegister  expectedbox = 0;
        gRegister  expectedrow = 0;
        for(int e=static_cast<int>(MINFUNC); e<MAXFUNC; ++e) {
            if ((e <= MAXUNIQUE) && test.alreadyunique) {
                continue;
            }

            gRegister   candidateset = test.candidateset;
            gRegister   actualrowpart = 0;
            gRegister   column = 0;
            gRegister   mergedbox = 0;
            gRegister   mergedrow = 0;

            testFillUniqueElementPreRowPart = prerowpart;
            testFillUniqueElementPreColumn = precolumn;
            testFillUniqueElementPreBox = prebox;
            testFillUniqueElementPreRow = prerow;

            switch(e) {
            case UNIQUE0:
                asm volatile (
                    "call testFillUniqueElement0\n\t"
                    :"=b"(actualrowpart),"=c"(column),"=d"(mergedbox),"=S"(mergedrow):"a"(candidateset):"r10","r11","r12","r13","r14","r15");
                break;
            case UNIQUE1:
                asm volatile (
                    "call testFillUniqueElement1\n\t"
                    :"=b"(actualrowpart),"=c"(column),"=d"(mergedbox),"=S"(mergedrow):"a"(candidateset):"r10","r11","r12","r13","r14","r15");
                break;
            case UNIQUE2:
                asm volatile (
                    "call testFillUniqueElement2\n\t"
                    :"=b"(actualrowpart),"=c"(column),"=d"(mergedbox),"=S"(mergedrow):"a"(candidateset):"r10","r11","r12","r13","r14","r15");
                break;
            case ONE0:
                asm volatile (
                    "call testFillOneUniqueCandidates0\n\t"
                    :"=a"(actualrowpart),"=b"(column),"=c"(mergedbox),"=d"(mergedrow)::"r9","r10","r11","r12","r13","r14","r15");
                break;
            case ONE1:
                asm volatile (
                    "call testFillOneUniqueCandidates1\n\t"
                    :"=a"(actualrowpart),"=b"(column),"=c"(mergedbox),"=d"(mergedrow)::"r9","r10","r11","r12","r13","r14","r15");
                break;
            case ONE2:
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
    struct testcase {
        shiftpos   pos;
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

    const testcase testSet[] = {
        {POS0,      0x7ffffff, 0x7ffffff07ffffff, 0x4000000, 0x200000000000000, 0x4000000, 0x200000000000000,
         0x1f03c0f, 0x7ffffff, 0x7ffffff01f03c0f, 0x4000000, 0x200000000000000, 0x4000000, 0x200000000000000},
        {POS0,   0x7ffffff, 0x7ffffff07ffffff, 0x4000000,    0x200000000000000, 0x4000000, 0x200000000000000,
         0x40404, 0x7ffffff, 0x7ffffff00040404, 0x4000000, 0x200000000040404, 0x4000000, 0x200000000040404},
        {POS1,      0x7ffffff, 0x7ffffff07ffffff, 3, 0xc, 3, 0xc,
         0x40100c0, 0x7ffffff, 0x40100c007ffffff, 3, 0xc, 3, 0xc},
        {POS1,      0x7ffffff, 0x7ffffff07ffffff, 3, 0xc, 3, 0xc,
         0x4010080, 0x7ffffff, 0x401008007ffffff, 3, 0x40100800000000c, 3, 0x40100800000000c},
        {POS2,      0x7ffffff, 0x7ffffff07ffffff, 8, 0x200000000000000, 8, 0x200000000000000,
         0x0c00608, 0xc00608,  0x7ffffff07ffffff, 8, 0x200000000000000, 8, 0x200000000000000},
        {POS2,      0x7ffffff, 0x7ffffff07ffffff, 8, 0x200000000000000, 8, 0x200000000000000,
         0x80808,   0x80808,   0x7ffffff07ffffff, 0x80808, 0x200000000000000, 0x80808, 0x200000000000000},
    };
    for(size_t i=0; i <arraySizeof(testSet); ++i) {
        const testcase test = testSet[i];
        xmmRegister row;
        xmmRegister all;
        xmmRegister box;
        *(reinterpret_cast<gRegister*>(&row)) = test.prerowlow;
        *(reinterpret_cast<gRegister*>(&row) + 1) = test.prerowhigh;
        *(reinterpret_cast<gRegister*>(&all)) = test.prealllow;
        *(reinterpret_cast<gRegister*>(&all) + 1) = test.preallhigh;
        *(reinterpret_cast<gRegister*>(&box)) = test.preboxlow;
        *(reinterpret_cast<gRegister*>(&box) + 1) = test.preboxhigh;

        gRegister expectedrowhigh = test.expectedrowhigh;
        gRegister expectedrowlow = test.expectedrowlow;
        gRegister expectedallhigh = test.expectedallhigh;
        gRegister expectedalllow = test.expectedalllow;
        gRegister expectedboxhigh = test.expectedboxhigh;
        gRegister expectedboxlow = test.expectedboxlow;

        gRegister arg = test.arg;
        switch(test.pos) {
        case POS0:
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
        case POS1:
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
        case POS2:
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
        CPPUNIT_ASSERT_EQUAL(expectedrowhigh, high);
        CPPUNIT_ASSERT_EQUAL(expectedrowlow, low);

        high = *(reinterpret_cast<gRegister*>(&all) + 1);
        low = *(reinterpret_cast<gRegister*>(&all));
        CPPUNIT_ASSERT_EQUAL(expectedallhigh, high);
        CPPUNIT_ASSERT_EQUAL(expectedalllow, low);

        high = *(reinterpret_cast<gRegister*>(&box) + 1);
        low = *(reinterpret_cast<gRegister*>(&box));
        CPPUNIT_ASSERT_EQUAL(expectedboxhigh, high);
        CPPUNIT_ASSERT_EQUAL(expectedboxlow, low);
    }

    return;
}

void SudokuSseTest::test_MaskElementCandidates()
{
    struct testcase {
        shiftpos  pos;
        gRegister arg;
        gRegister sum;
    };

    const testcase testSet[] = {
        {POS0, 0x48007ff, 0x123},
        {POS1, 0x352fe0c, 0xdc},
        {POS2, 0x7fc1f81f, 0xff},
    };
    for(size_t i=0; i <arraySizeof(testSet); ++i) {
        const testcase test = testSet[i];
        gRegister arg = test.arg;
        gRegister sum = 0;
        switch(test.pos) {
        case POS0:
            asm volatile (
                "call testMergeTwoElements0\n\t"
                :"=b"(sum):"a"(arg):"r14","r15");
            break;
        case POS1:
            asm volatile (
                "call testMergeTwoElements1\n\t"
                :"=b"(sum):"a"(arg):"r14","r15");
            break;
        case POS2:
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
    struct testcase {
        shiftpos  pos;
        gRegister arg;
        gRegister candidates;
        gRegister result;
    };

    const testcase testSet[] = {
        {POS0, 0x7ffffff, 0x1fe, 0x7fffe01},
        {POS0, 0x7ffffff, 1, 0x7ffffff},
        {POS1, 0x7ffffff, 0x1ef, 0x7fc21ff},
        {POS1, 0x7ffffff, 1, 0x7ffffff},
        {POS2, 0x7ffffff, 0xff, 0x403ffff},
        {POS2, 0x7ffffff, 1, 0x7ffffff},
    };
    for(size_t i=0; i <arraySizeof(testSet); ++i) {
        const testcase test = testSet[i];
        gRegister arg = test.arg;
        gRegister candidates = test.candidates;
        gRegister result = 0;
        switch(test.pos) {
        case POS0:
            asm volatile (
                "call testMaskElementCandidates0\n\t"
                :"=c"(result):"a"(arg),"b"(candidates):"r12","r13","r14","r15");
            break;
        case POS1:
            asm volatile (
                "call testMaskElementCandidates1\n\t"
                :"=c"(result):"a"(arg),"b"(candidates):"r12","r13","r14","r15");
            break;
        case POS2:
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
    struct testcase {
        gRegister  prerowpart;
        gRegister  prerowcandidates;
        gRegister  prebox;
        gRegister  precolumnrowpart;
        gRegister  expected;
        gRegister  expectedthree;
    };

    enum func {
        POSMIN,
        POS0 = POSMIN,
        POS1,
        POS2,
        POSMAX,
    };

    const testcase testSet[] = {
        {0x7fffe01, 0x100, 0x80, 0x40,  0x7fffe01, 0x7fffe01},
        {0x40407,   0x1f8, 0,    0,     0x40404,   0x40404},
        {0x40101ff, 0,     0x3f, 0,     0x4010040, 0x4010040},
        {0x7ffffff, 0,     0,    0x1ef, 0x7fffe10, 0x7fffe10},
        {0x7ffffff, 1,     0x10, 0x100, 0x7ffffff, 0x7ffffff},
    };

    for(size_t i=0; i <arraySizeof(testSet); ++i) {
        const testcase test = testSet[i];
        gRegister  prerowpart = test.prerowpart;
        gRegister  prerowcandidates = test.prerowcandidates;
        gRegister  prebox = test.prebox;
        gRegister  precolumnrowpart = test.precolumnrowpart;
        gRegister  expected = test.expected;
        gRegister  actual = 0;

        asm volatile (
            "call testFindThreePartsCandidates\n\t"
            :"=S"(actual):"a"(test.prerowpart),"b"(test.prerowcandidates),"c"(test.prebox),"d"(test.precolumnrowpart):"r10","r11","r12","r13","r14","r15");
        CPPUNIT_ASSERT_EQUAL(test.expectedthree, actual);

        for(int e=static_cast<int>(POSMIN); e<POSCNT; ++e) {
            switch(e) {
            case POS0:
                asm volatile (
                    "call testFindElementCandidates0\n\t"
                    :"=S"(actual):"a"(prerowpart),"b"(prerowcandidates),"c"(prebox),"d"(precolumnrowpart):"r10","r11","r12","r13","r14","r15");
                break;
            case POS1:
                asm volatile (
                    "call testFindElementCandidates1\n\t"
                    :"=S"(actual):"a"(prerowpart),"b"(prerowcandidates),"c"(prebox),"d"(precolumnrowpart):"r10","r11","r12","r13","r14","r15");
                break;
            case POS2:
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
    struct testcase {
        gRegister  prerowpart;
        gRegister  prerowcandidates;
        gRegister  prebox;
        gRegister  precolumnrowpart;
        gRegister  expected;
    };


    const testcase testSet[] = {
        {0xc0604,   0x1fc, 0, 0x7f80000, 0x40404},
        {0x7f00603, 0, 0x1fc, 0x3fc00, 0x7f00202},
    };

    for(size_t i=0; i <arraySizeof(testSet); ++i) {
        const testcase test = testSet[i];
        gRegister  actual = 0;

        asm volatile (
            "call testFindThreePartsCandidates\n\t"
            :"=S"(actual):"a"(test.prerowpart),"b"(test.prerowcandidates),"c"(test.prebox),"d"(test.precolumnrowpart):"r10","r11","r12","r13","r14","r15");
        CPPUNIT_ASSERT_EQUAL(test.expected, actual);
    }

    return;
}

void SudokuSseTest::test_FillThreePartsUniqueCandidates()
{
    struct testcase {
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

    enum func {
        FUNCMIN,
        PART2 = FUNCMIN, // 左、右、中の順
        PART0,
        PART1,
        ROW2,
        ROW0,
        ROW1,
        FILLNINE,
        FINDNINE,
        FUNCCNT,
    };

    const testcase testSet[] = {
        /* 1行目 : *,  *, 3,   4,5,6, 7,8,9,
           2行目 : 4,  5, 6,   0,0,0, 0,0,0,
           3行目 : 7,  8, 9,   0,0,0, 0,0,0,
           列    : 2-9,58,369, 4,5,6, 7,8,9, (find nine以外)
           列    : 47, 58,369, 4,5,6, 7,8,9, (find nine)
           結果  : 1,  2, ...  */
        {{{{0x20202001010100, 0x7fc0604}, {0x7ffffff07ffffff, 0x202020},  {0x7ffffff07ffffff, 0x1010100},
           {0x20202001010100, 0x1212124}, {0x20202001010100,  0x7f92124},
           {0x20202001010100, 0x40404},   {0x20202001010100,  0x1252524}, {0x20202001010100,  0x7fd2524},
           {0x20202001010100, 0x40404},   {0x20202001010100,  0x1252524}, {0x20202001010100,  0x7fd2524},
           {0x20202001010100, 0x7fc0604}, {0x7ffffff07ffffff, 0x1212120}, {0x7ffffff07ffffff, 0x1212120},
           0x1fc}}},
        /* 1行目 : *,  *, 3,   4,5,6, 0,0,0,
           2行目 : 4,  5, 6,   0,0,0, 0,0,0,
           3行目 : 7,  8, 9,   0,0,0, 0,0,0,
           列    : 2-9,58,369, 4,5,6, 7,8,9, (find nine以外)
           列    : 47, 58,369, 4,5,6, 0,0,0, (find nine)
            結果  : 1,  2, ...  */
        {{{{0x20202007ffffff, 0x7fc0c04}, {0x7ffffff07ffffff, 0x202020},  {0x7ffffff07ffffff, 0x1010100},
           {0x20202000000000, 0x1212124}, {0x20202000000000,  0x7f92124},
           {0x20202007ffffff, 0x40404},   {0x20202000000000,  0x1252524}, {0x20202000000000,  0x7fd2524},
           {0x202020070381c0, 0x40404},   {0x20202000000000,  0x1252524}, {0x20202000000000,  0x7fd2524},
           {0x20202007ffffff, 0x40404},   {0x7ffffff07ffffff, 0x1212120}, {0x7ffffff07ffffff, 0x1212120},
           0x3c}}},
        /* 1行目がすべて埋まっている */
        {{{{0x20202001010100, 0x40404}, {0x7ffffff07ffffff, 0x7ffffff}, {0x7ffffff07ffffff, 0x7ffffff},
           {0x20202001010100, 0x40404}, {0x20202001010100,  0x40404},
           {0x20202001010100, 0x40404}, {0x20202001010100,  0x40404},   {0x20202001010100,  0x40404},
           {0x20202001010100, 0x40404}, {0x20202001010100,  0x40404},   {0x20202001010100,  0x40404},
           {0x20202001010100, 0x40404}, {0x7ffffff07ffffff, 0x7ffffff}, {0x7ffffff07ffffff, 0x7ffffff},
           0x1ff}}}
    };

    for(size_t i=0; i <arraySizeof(testSet); ++i) {
        testcase test = testSet[i];
        for(int e=static_cast<int>(FUNCMIN); e<FUNCCNT; ++e) {
            // memsetはvolatile*を受け付けない
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
            case PART0:
                asm volatile (
                    "movdqa  xmm1,  xmmword ptr [rdi]\n\t"
                    "movdqa  xmm2,  xmmword ptr [rdi+16]\n\t"
                    "movdqa  xmm3,  xmmword ptr [rdi+32]\n\t"
                    "movdqa  xmm10, xmmword ptr [rdi+48]\n\t"
                    "movdqa  xmm0,  xmmword ptr [rdi+64]\n\t"
                    "call testFillThreePartsUniqueCandidates0\n\t"
                    ::"D"(test.xarg),"a"(test.argrow):"rdx","rsi","r8","r9","r10","r11","r12","r13","r14","r15");
                break;
            case PART1:
                asm volatile (
                    "movdqa  xmm1,  xmmword ptr [rdi]\n\t"
                    "movdqa  xmm2,  xmmword ptr [rdi+16]\n\t"
                    "movdqa  xmm3,  xmmword ptr [rdi+32]\n\t"
                    "movdqa  xmm10, xmmword ptr [rdi+48]\n\t"
                    "movdqa  xmm0,  xmmword ptr [rdi+64]\n\t"
                    "call testFillThreePartsUniqueCandidates1\n\t"
                    ::"D"(test.xarg),"a"(test.argrow):"rdx","rsi","r8","r9","r10","r11","r12","r13","r14","r15");
                break;
            case PART2:
                asm volatile (
                    "movdqa  xmm1,  xmmword ptr [rdi]\n\t"
                    "movdqa  xmm2,  xmmword ptr [rdi+16]\n\t"
                    "movdqa  xmm3,  xmmword ptr [rdi+32]\n\t"
                    "movdqa  xmm10, xmmword ptr [rdi+48]\n\t"
                    "movdqa  xmm0,  xmmword ptr [rdi+64]\n\t"
                    "call testFillThreePartsUniqueCandidates2\n\t"
                    ::"D"(test.xarg),"a"(test.argrow):"rdx","rsi","r8","r9","r10","r11","r12","r13","r14","r15");
                break;
            case ROW0:
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
            case ROW1:
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
            case ROW2:
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
            case FILLNINE:
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
            case FINDNINE:
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

            xmmRegister actualrow = testFillNineUniqueCandidatesRowX;
            xmmRegister actualbox = testFillNineUniqueCandidatesBoxX;
            xmmRegister actualcolumn = testFillNineUniqueCandidatesColumnX;

            int cmpactual = memcmp(expectedrow, &actualrow, sizeof(actualrow));
            int cmpexpected = 0;
            CPPUNIT_ASSERT_EQUAL(cmpexpected, cmpactual);
            cmpactual = memcmp(expectedbox, &actualbox, sizeof(actualbox));
            CPPUNIT_ASSERT_EQUAL(cmpexpected, cmpactual);
            cmpactual = memcmp(expectedcolumn, &actualcolumn, sizeof(actualcolumn));
            CPPUNIT_ASSERT_EQUAL(cmpexpected, cmpactual);

            if (e >= FILLNINE) {
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
    struct testcase {
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

    const testcase testSet[] = {
        {{{{0,0},{0,0},{0,0},{0,0}, 0x7ffffff, 0, 0,
           0, 0, 0, 0,  0x7ffffff, 0, 0}}},
        {{{{0,0},{0,0},{0,0},{0,0}, 0x1c0401, 0, 0,          // 行で絞る
           0x4010040, 0x3, 0x1c0, 0x38, 0x100401, 0x1f8, 0}}},
        {{{{0,0},{0x7eff7fe,0},{0,0},{0,0}, 0x7ffffff, 0, 0, // 列
           0, 0, 0, 0,  0x100801, 0, 0}}},
        {{{{0x7000e18,0},{0,0},{0,0},{0,0}, 0x4000238, 0, 0, // 箱
           0, 0, 0, 0,  0x4000220, 0, 0x1df}}},
    };

    for(size_t i=0; i <arraySizeof(testSet); ++i) {
        testcase test = testSet[i];
        for(int e=static_cast<int>(POSMIN); e<POSCNT; ++e) {
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
            case POS0:
                asm volatile (
                    "movdqa  xmm10, xmmword ptr [rdi]\n\t"
                    "movdqa  xmm0,  xmmword ptr [rdi+16]\n\t"
                    "call testFindRowPartCandidates0\n\t"
                    ::"D"(test.xarg),"a"(test.rowpartleft),"b"(test.candidatestarget),"c"(test.candidatesleft),"d"(test.candidatesother):
                     "rsi","r8","r9","r10","r11","r12","r13","r14","r15");
                break;
            case POS1:
                asm volatile (
                    "movdqa  xmm10, xmmword ptr [rdi]\n\t"
                    "movdqa  xmm0,  xmmword ptr [rdi+16]\n\t"
                    "call testFindRowPartCandidates1\n\t"
                    ::"D"(test.xarg),"a"(test.rowpartleft),"b"(test.candidatestarget),"c"(test.candidatesleft),"d"(test.candidatesother):
                     "rsi","r8","r9","r10","r11","r12","r13","r14","r15");
                break;
                break;
            case POS2:
                asm volatile (
                    "movdqa  xmm10, xmmword ptr [rdi]\n\t"
                    "movdqa  xmm0,  xmmword ptr [rdi+16]\n\t"
                    "call testFindRowPartCandidates2\n\t"
                    ::"D"(test.xarg),"a"(test.rowpartleft),"b"(test.candidatestarget),"c"(test.candidatesleft),"d"(test.candidatesother):
                     "rsi","r8","r9","r10","r11","r12","r13","r14","r15");
                break;
                break;
            default:
                break;
            }

            gRegister  actualrowparttarget = testFindRowPartCandidatesRowPartTarget;
            gRegister  actualrowcandidates = testFindRowPartCandidatesRowCandidates;
            gRegister  actualbox = testFindRowPartCandidatesBox;
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

void SudokuSseTest::test_CountRowPartElements()
{
    struct testcase {
        gRegister arg;
        gRegister count;
        gRegister element;
        gRegister isequal;
    };

    const testcase testSet[] = {
        {0x20081ff, 2, 0x1ff, 0},
        {0x2008101, 2, 0x101, 1},
        {0x2008101, 3, 0x101, 0},
        {0x2008111, 3, 0x111, 1},
        {0x20081ff, 9, 0x1ff, 1},
    };
    for(size_t i=0; i <arraySizeof(testSet); ++i) {
        const testcase test = testSet[i];
        gRegister arg = test.arg;
        gRegister count = test.count;
        for(int e=static_cast<int>(POSMIN); e<POSCNT; ++e) {
            gRegister element = 0;
            gRegister isequal = 0;
            switch(e) {
            case POS0:
                asm volatile (
                    "call testCountRowPartElements0\n\t"
                    :"=c"(element),"=d"(isequal):"a"(arg),"b"(count):"r14","r15");
                break;
            case POS1:
                asm volatile (
                    "call testCountRowPartElements1\n\t"
                    :"=c"(element),"=d"(isequal):"a"(arg),"b"(count):"r14","r15");
                break;
            case POS2:
                asm volatile (
                    "call testCountRowPartElements2\n\t"
                    :"=c"(element),"=d"(isequal):"a"(arg),"b"(count):"r14","r15");
                break;
            default:
                break;
            }
            CPPUNIT_ASSERT_EQUAL(test.element, element);
            CPPUNIT_ASSERT_EQUAL(test.isequal, isequal);
            arg = rotateRowPart(arg);
        }
    }

    return;
}

void SudokuSseTest::test_CountFilledElements()
{
    struct testcase {
        gRegister prevpopcnt;
        gRegister loopcnt;
        gRegister preallhigh;
        gRegister prealllow;
        gRegister expectedpopcnt;
    };

    const testcase testSet[] = {
        {0, 0, 0, 0, 0},
        {1, 2, 0xca, 0xedb7, 16},
        {80, 50, 0x7ffffff, 0x7ffffff07ffffff, 81}
    };

    for(size_t i=0; i <arraySizeof(testSet); ++i) {
        const testcase test = testSet[i];
        gRegister loopcnt = 0;
        gRegister actualpopcnt = 0;
        gRegister actualprevpopcnt = 0;
        xmmRegister looppopcnt;
        gRegister* pLooppopcntlow = reinterpret_cast<gRegister*>(&looppopcnt);
        gRegister* pLooppopcnthigh = pLooppopcntlow + 1;
        *pLooppopcntlow = test.prevpopcnt;
        *pLooppopcnthigh = test.loopcnt;
        xmmRegister all;
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

void SudokuSseTest::test_ChooseNextCadidate()
{
    struct testcase {
        gRegister argrowpart;
        gRegister arguniquecandidate;
        gRegister expectedrowpart;
        gRegister expecteduniquecandidate;
        gRegister chosen;
    };

    const testcase testSet[] = {
        {0x7fffe03, 0, 0x7fffe01, 1, 1},
        {0x7fffe03, 1, 0x7fffe02, 2, 1},
        {0x7fffe03, 2, 0x7fffe03, 0, 0},
        {0x7ffff42, 0,     0x7fffe02, 2, 1},
        {0x7ffff42, 2,     0x7fffe40, 0x40, 1},
        {0x7ffff42, 0x40,  0x7ffff00, 0x100, 1},
        {0x7ffff42, 0x100, 0x7ffff42, 0, 0},
    };
    for(size_t i=0; i <arraySizeof(testSet); ++i) {
        const testcase test = testSet[i];
        gRegister argrowpart = test.argrowpart;
        gRegister arguniquecandidate = test.arguniquecandidate;
        gRegister expectedrowpart = test.expectedrowpart;
        for(int e=static_cast<int>(POSMIN); e<POSCNT; ++e) {
            gRegister searched = 0;
            gRegister actualrowpart = 0;
            gRegister actualuniquecandidate = 0;
            gRegister chosen = 0;
            switch(e) {
            case POS0:
                asm volatile (
                    "call testChooseNextCadidate0\n\t"
                    :"=c"(searched),"=d"(actualrowpart),"=S"(actualuniquecandidate),"=D"(chosen):"a"(argrowpart),"b"(arguniquecandidate):"r11","r12","r13","r14","r15");
                break;
            case POS1:
                asm volatile (
                    "call testChooseNextCadidate1\n\t"
                    :"=c"(searched),"=d"(actualrowpart),"=S"(actualuniquecandidate),"=D"(chosen):"a"(argrowpart),"b"(arguniquecandidate):"r11","r12","r13","r14","r15");
                break;
            case POS2:
                asm volatile (
                    "call testChooseNextCadidate2\n\t"
                    :"=c"(searched),"=d"(actualrowpart),"=S"(actualuniquecandidate),"=D"(chosen):"a"(argrowpart),"b"(arguniquecandidate):"r11","r12","r13","r14","r15");
                break;
            default:
                break;
            }
            CPPUNIT_ASSERT_EQUAL(actualuniquecandidate, searched);
            CPPUNIT_ASSERT_EQUAL(test.expecteduniquecandidate, actualuniquecandidate);
            CPPUNIT_ASSERT_EQUAL(expectedrowpart, actualrowpart);
            CPPUNIT_ASSERT_EQUAL(test.chosen, chosen);
            argrowpart = rotateRowPart(argrowpart);
            expectedrowpart = rotateRowPart(expectedrowpart);
        }
    }

    return;
}

void SudokuSseTest::test_SearchRowPartElements()
{
    struct testcase {
        gRegister candidatecnt;
        gRegister prerowpart;
        gRegister preuniquecandidate;
        gRegister preinboxshift;
        gRegister preoutboxshift;
        gRegister searched;
        gRegister expectedrowpart;
        gRegister expecteduniquecandidate;
        gRegister expectedinboxshift;
        gRegister expectedoutboxshift;
    };

    const testcase testSet[] = {
        // 91-91-91
        {2, 0x4060301,     0, 0, 0,  1,     0x4060201, 1,     0, 0},
        {2, 0x4060301,     1, 0, 0,  0x100, 0x4060300, 0x100, 0, 0},
        {2, 0x4060301, 0x100, 0, 0,  1,     0x4040301, 1,     1, 0},
        {2, 0x4060301, 0x100, 0, 0,  1,     0x4040301, 1,     1, 0},
        {2, 0x4060301,     1, 1, 0,  0x100, 0x4060101, 0x100, 1, 0},
        {2, 0x4060301, 0x100, 1, 0,  1,     0x0060301, 1,     2, 0},
        {2, 0x4060301,     1, 2, 0,  0x100, 0x4020301, 0x100, 2, 0},
        {2, 0x4060301, 0x100, 2, 0,  0,     0x4060301, 0,     0, 1},
        // 82-*-*
        {2, 0x20bffff,     0, 0, 1,  2,     0x00bffff, 2,     2, 1},
        {2, 0x20bffff,     2, 2, 1,  0x80,  0x203ffff, 0x80,  2, 1},
        {2, 0x20bffff,  0x80, 2, 1,  0,     0x20bffff, 0,     0, 2},
        // 678-*-234
        {3, 0x380020e,     0, 0, 2,  2,     0x3800202, 2,     0, 2},
        {3, 0x380020e,     2, 0, 2,  4,     0x3800204, 4,     0, 2},
        {3, 0x380020e,     4, 0, 2,  8,     0x3800208, 8,     0, 2},
        {3, 0x380020e,     8, 0, 2, 0x20,   0x080020e, 0x20,  2, 2},
        {3, 0x380020e,  0x20, 2, 2, 0x40,   0x100020e, 0x40,  2, 2},
        {3, 0x380020e,  0x40, 2, 2, 0x80,   0x200020e, 0x80,  2, 2},
        {3, 0x380020e,  0x80, 2, 2, 0,      0x380020e, 0,     0, 3},
        // *-*-*
        {3, 0x0f0f0f0,     0, 0, 2,  0,     0x0f0f0f0, 0,     0, 3},
    };
    for(size_t i=0; i <arraySizeof(testSet); ++i) {
        const testcase test = testSet[i];
        gRegister candidatecnt = test.candidatecnt;
        gRegister searched = 0;
        gRegister actualrowpart = 0;
        gRegister actualuniquecandidate = 0;
        gRegister actualinboxshift = 0;
        gRegister actualoutboxshift = 0;
        testSearchRowPartElementsPreRowPart = test.prerowpart;
        testSearchRowPartElementsPreUniqueCandidate = test.preuniquecandidate;
        testSearchRowPartElementsPreInBoxShift = test.preinboxshift;
        testSearchRowPartElementsPreOutBoxShift = test.preoutboxshift;

        asm volatile (
            "call testSearchRowPartElements\n\t"
            :"=b"(searched),"=c"(actualrowpart),"=d"(actualuniquecandidate),"=S"(actualinboxshift),"=D"(actualoutboxshift):"a"(candidatecnt):"r11","r12","r13","r14","r15");
        CPPUNIT_ASSERT_EQUAL(test.searched, searched);
        CPPUNIT_ASSERT_EQUAL(test.expectedrowpart, actualrowpart);
        CPPUNIT_ASSERT_EQUAL(test.expecteduniquecandidate, actualuniquecandidate);
        CPPUNIT_ASSERT_EQUAL(test.expectedinboxshift, actualinboxshift);
        CPPUNIT_ASSERT_EQUAL(test.expectedoutboxshift, actualoutboxshift);
    }

    return;
}

void SudokuSseTest::test_SearchRowElements()
{
    struct testcase {
        gRegister candidatecnt;
        gRegister arghigh;
        gRegister arglow;
        gRegister preuniquecandidate;
        gRegister precandidaterow;
        gRegister preinboxshift;
        gRegister preoutboxshift;
        gRegister expecteduniquecandidate;
        gRegister expectedcandidaterow;
        gRegister expectedinboxshift;
        gRegister expectedoutboxshift;
    };

    const testcase testSet[] = {
        // 1(45)2-(45)36-789
        {2, 0x43002, 0x60082001010100, 0,    0, 0, 0,  8,    0,  2, 1},
        {2, 0x43002, 0x60082001010100, 8,    1, 2, 1,  0x10, 1,  2, 1},
        {2, 0x43002, 0x60082001010100, 0x10, 3, 2, 1,  8,    3,  1, 2},
        {2, 0x43002, 0x60082001010100, 8,    5, 1, 2,  0x10, 5,  1, 2},
        {2, 0x43002, 0x60082001010100, 0x10, 9, 1, 2,  0,    10, 0, 0},
        // ***-***-***(8642)
        {4, 0x7fffff, 0x7ffffff07fffeaa, 0,    1, 0, 0,  2,    1,  0, 0},
        {4, 0x7fffff, 0x7ffffff07fffeaa, 2,    1, 0, 0,  8,    1,  0, 0},
        {4, 0x7fffff, 0x7ffffff07fffeaa, 8,    1, 0, 0,  0x20, 1,  0, 0},
        {4, 0x7fffff, 0x7ffffff07fffeaa, 0x20, 1, 0, 0,  0x80, 1,  0, 0},
        {4, 0x7fffff, 0x7ffffff07fffeaa, 0x80, 8, 0, 0,  0,    9,  0, 0},
    };
    for(size_t i=0; i <arraySizeof(testSet); ++i) {
        const testcase test = testSet[i];
        gRegister candidatecnt = test.candidatecnt;
        gRegister actualuniquecandidate = 0;
        gRegister actualcandidaterow = 0;
        gRegister actualinboxshift = 0;
        gRegister actualoutboxshift = 0;
        testSearchRowElementsPreUniqueCandidate = test.preuniquecandidate;
        testSearchRowElementsPreCandidateRow = test.precandidaterow;
        testSearchRowElementsPreInBoxShift = test.preinboxshift;
        testSearchRowElementsPreOutBoxShift = test.preoutboxshift;
        xmmRegister arg;
        *(reinterpret_cast<gRegister*>(&arg)) = test.arglow;
        *(reinterpret_cast<gRegister*>(&arg) + 1) = test.arghigh;

        asm volatile (
            "movdqa  xmm1, xmmword ptr [rdi]\n\t"
            "call testSearchRowElements\n\t"
            :"=b"(actualuniquecandidate),"=c"(actualcandidaterow),"=d"(actualinboxshift),"=S"(actualoutboxshift):"a"(candidatecnt),"D"(&arg):"r8","r9","r10","r11","r12","r13","r14","r15");
        CPPUNIT_ASSERT_EQUAL(test.expecteduniquecandidate, actualuniquecandidate);
        CPPUNIT_ASSERT_EQUAL(test.expectedcandidaterow, actualcandidaterow);
        CPPUNIT_ASSERT_EQUAL(test.expectedinboxshift, actualinboxshift);
        CPPUNIT_ASSERT_EQUAL(test.expectedoutboxshift, actualoutboxshift);
    }

    return;
}

void SudokuSseTest::test_FastCollectCandidatesAtRow() {
    struct testcase {
        gRegister arghigh;
        gRegister arglow;
        gRegister righttop;
        gRegister rightbottom;
        gRegister expectedsum;
        gRegister expectedtarget;
    };

    enum Target {
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

    const testcase testSet00[] = {{0, 0, 0, 0, 0, 0},
                                  {0, 0x100, 0, 0, 0, 0x100},
                                  {0, 0, 0x100, 0, 0x100, 0},
                                  {0, 0x1000200040008ull, 0x100, 0x1ff, 0x107, 0x8},
                                  {0x80004000200010, 0x8000400020001ull, 0x100, 0x1ff, 0x1fe, 0x1},
    };

    const testcase testSet10[] = {{0x80004000200010, 0x8000400020001, 0x100, 0x1ff, 0x1fd, 0x2}};
    const testcase testSet20[] = {{0x80004000200010, 0x8000400020001, 0x100, 0x1ff, 0x1fb, 0x4}};
    const testcase testSet30[] = {{0x80004000200010, 0x8000400020001, 0x100, 0x1ff, 0x1f7, 0x8}};
    const testcase testSet40[] = {{0x80004000200010, 0x8000400020001, 0x100, 0x1ff, 0x1ef, 0x10}};
    const testcase testSet50[] = {{0x80004000200010, 0x8000400020001, 0x100, 0x1ff, 0x1df, 0x20}};
    const testcase testSet60[] = {{0x80004000200010, 0x8000400020001, 0x100, 0x1ff, 0x1bf, 0x40}};
    const testcase testSet70[] = {{0x80004000200010, 0x8000400020001, 0x100, 0x1ff, 0x17f, 0x80}};

    const testcase testSet80[] = {{0, 0, 0, 0, 0, 0},
                                  {0, 0, 0x100, 0, 0, 0x100},
                                  {0, 0x1000200040008ull, 0x100, 0x1ff, 0xf, 0x100},
                                  {0x100004000200010, 0x8000400020001, 0x80, 0x1ff, 0x17f, 0x80},
    };

    const testcase testSet88[] = {{0, 0, 0, 0, 0, 0},
                                  {0, 0, 0x100, 0x100, 0, 0x100},
                                  {0, 0x1000200040008ull, 0x100, 0x100, 0xf, 0x100},
                                  {0x100004000200010, 0x8000400020001, 0x80, 0x80, 0x17f, 0x80},
    };

    for(int i=0; i<static_cast<int>(POS_CNT); ++i) {
        Target target = static_cast<Target>(i);
        size_t numberOfCases = 0;
        const testcase* testSet = 0;
        switch(target) {
        case POS_0_0:
            numberOfCases = arraySizeof(testSet00);
            testSet = testSet00;
            break;
        case POS_1_0:
            numberOfCases = arraySizeof(testSet10);
            testSet = testSet10;
            break;
        case POS_2_0:
            numberOfCases = arraySizeof(testSet20);
            testSet = testSet20;
            break;
        case POS_3_0:
            numberOfCases = arraySizeof(testSet30);
            testSet = testSet30;
            break;
        case POS_4_0:
            numberOfCases = arraySizeof(testSet40);
            testSet = testSet40;
            break;
        case POS_5_0:
            numberOfCases = arraySizeof(testSet50);
            testSet = testSet50;
            break;
        case POS_6_0:
            numberOfCases = arraySizeof(testSet60);
            testSet = testSet60;
            break;
        case POS_7_0:
            numberOfCases = arraySizeof(testSet70);
            testSet = testSet70;
            break;
        case POS_8_0:
            numberOfCases = arraySizeof(testSet80);
            testSet = testSet80;
            break;
        case POS_8_8:
            numberOfCases = arraySizeof(testSet88);
            testSet = testSet88;
            break;
        default:
            break;
        }

        for(size_t i=0; i <numberOfCases; ++i) {
            testcase test = testSet[i];
            gRegister actualsum;
            gRegister actualtarget;
            xmmRegister arg;
            *(reinterpret_cast<gRegister*>(&arg)) = test.arglow;
            *(reinterpret_cast<gRegister*>(&arg) + 1) = test.arghigh;

            switch(target) {
            case POS_0_0:
                asm volatile (
                    "movdqa  xmm1, xmmword ptr [rdi]\n\t"
                    "xorps   xmm10, xmm10\n\t"
                    "pinsrw  xmm10, rdx, 0\n\t"
                    "call testFastCollectCandidatesAtRow00\n\t"
                    :"=a"(actualsum),"=b"(actualtarget):"c"(test.rightbottom),"d"(test.righttop),"D"(&arg):"r8","r9","r10","r11","r12","r13","r14","r15");
                break;
            case POS_1_0:
                asm volatile (
                    "movdqa  xmm1, xmmword ptr [rdi]\n\t"
                    "xorps   xmm10, xmm10\n\t"
                    "pinsrw  xmm10, rdx, 0\n\t"
                    "call testFastCollectCandidatesAtRow10\n\t"
                    :"=a"(actualsum),"=b"(actualtarget):"c"(test.rightbottom),"d"(test.righttop),"D"(&arg):"r8","r9","r10","r11","r12","r13","r14","r15");
                break;
            case POS_2_0:
                asm volatile (
                    "movdqa  xmm1, xmmword ptr [rdi]\n\t"
                    "xorps   xmm10, xmm10\n\t"
                    "pinsrw  xmm10, rdx, 0\n\t"
                    "call testFastCollectCandidatesAtRow20\n\t"
                    :"=a"(actualsum),"=b"(actualtarget):"c"(test.rightbottom),"d"(test.righttop),"D"(&arg):"r8","r9","r10","r11","r12","r13","r14","r15");
                break;
            case POS_3_0:
                asm volatile (
                    "movdqa  xmm1, xmmword ptr [rdi]\n\t"
                    "xorps   xmm10, xmm10\n\t"
                    "pinsrw  xmm10, rdx, 0\n\t"
                    "call testFastCollectCandidatesAtRow30\n\t"
                    :"=a"(actualsum),"=b"(actualtarget):"c"(test.rightbottom),"d"(test.righttop),"D"(&arg):"r8","r9","r10","r11","r12","r13","r14","r15");
                break;
            case POS_4_0:
                asm volatile (
                    "movdqa  xmm1, xmmword ptr [rdi]\n\t"
                    "xorps   xmm10, xmm10\n\t"
                    "pinsrw  xmm10, rdx, 0\n\t"
                    "call testFastCollectCandidatesAtRow40\n\t"
                    :"=a"(actualsum),"=b"(actualtarget):"c"(test.rightbottom),"d"(test.righttop),"D"(&arg):"r8","r9","r10","r11","r12","r13","r14","r15");
                break;
            case POS_5_0:
                asm volatile (
                    "movdqa  xmm1, xmmword ptr [rdi]\n\t"
                    "xorps   xmm10, xmm10\n\t"
                    "pinsrw  xmm10, rdx, 0\n\t"
                    "call testFastCollectCandidatesAtRow50\n\t"
                    :"=a"(actualsum),"=b"(actualtarget):"c"(test.rightbottom),"d"(test.righttop),"D"(&arg):"r8","r9","r10","r11","r12","r13","r14","r15");
                break;
            case POS_6_0:
                asm volatile (
                    "movdqa  xmm1, xmmword ptr [rdi]\n\t"
                    "xorps   xmm10, xmm10\n\t"
                    "pinsrw  xmm10, rdx, 0\n\t"
                    "call testFastCollectCandidatesAtRow60\n\t"
                    :"=a"(actualsum),"=b"(actualtarget):"c"(test.rightbottom),"d"(test.righttop),"D"(&arg):"r8","r9","r10","r11","r12","r13","r14","r15");
                break;
            case POS_7_0:
                asm volatile (
                    "movdqa  xmm1, xmmword ptr [rdi]\n\t"
                    "xorps   xmm10, xmm10\n\t"
                    "pinsrw  xmm10, rdx, 0\n\t"
                    "call testFastCollectCandidatesAtRow70\n\t"
                    :"=a"(actualsum),"=b"(actualtarget):"c"(test.rightbottom),"d"(test.righttop),"D"(&arg):"r8","r9","r10","r11","r12","r13","r14","r15");
                break;
            case POS_8_0:
                asm volatile (
                    "movdqa  xmm1, xmmword ptr [rdi]\n\t"
                    "xorps   xmm10, xmm10\n\t"
                    "pinsrw  xmm10, rdx, 0\n\t"
                    "call testFastCollectCandidatesAtRow80\n\t"
                    :"=a"(actualsum),"=b"(actualtarget):"c"(test.rightbottom),"d"(test.righttop),"D"(&arg):"r8","r9","r10","r11","r12","r13","r14","r15");
                break;
            case POS_8_8:
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
    const gRegister expectedSet[] = {0x110, 0x11, 0x11};

    for(size_t i=0; i <arraySizeof(expectedSet); ++i) {
        gRegister actualsum = 0;
        xmmRegister arg;
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
    const gRegister expectedSet[] = {0x1, 0x80, 0x1fd, 0xff};

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
    struct testcase {
        gRegister rowHigh;
        gRegister rowLow;
        gRegister columnHigh;
        gRegister columnLow;
        gRegister rightBottom;
    };

    const testcase expectedSet[] = {{0, 1, 0, 0, 0}, {0, 0, 0, 1, 0}, {0, 1, 0, 0, 0}, {0, 0, 0, 0, 1}};

    for(size_t i=0; i <arraySizeof(expectedSet); ++i) {
        testcase test = expectedSet[i];
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
