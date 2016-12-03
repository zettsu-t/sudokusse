// 共通関数
// Copyright (C) 2012-2015 Zettsu Tatsuya

#include <cppunit/extensions/HelperMacros.h>
#include <cassert>
#include "sudoku.h"
#include "sudokutest.h"

class SudokuMacroTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(SudokuMacroTest);
    CPPUNIT_TEST(test_arraySizeof);
    CPPUNIT_TEST(test_useAvx);
    CPPUNIT_TEST(test_setMode);
    CPPUNIT_TEST_SUITE_END();
protected:
    void test_arraySizeof();
    void test_useAvx();
    void test_setMode();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SudokuMacroTest);

void SudokuMacroTest::test_arraySizeof() {
    constexpr size_t expected = 7;
    static int array[expected];
    CPPUNIT_ASSERT_EQUAL(expected, arraySizeof(array));

    struct LocalStruct {
        int a;
        int b;
    };
    static LocalStruct structArray[expected];
    CPPUNIT_ASSERT_EQUAL(expected, arraySizeof(structArray));
}

void SudokuMacroTest::test_useAvx() {
    constexpr uint64_t expected = SUDOKU_USE_AVX;
    const uint64_t actual = sudokuXmmUseAvx;
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void SudokuMacroTest::test_setMode() {
    const char * const OptionSet[] = {"on", "enable"};
    const char * const Argv[] = {"command", "on", "off", "enable", nullptr};
    constexpr int invalid = -1;
    constexpr int valid = 2;

    struct Test{
        int argc;
        int argIndex;
        int expected;
    };

    const Test testSet[] = {
        {0, 0, invalid},
        {1, 0, invalid},
        {1, 1, invalid},
        {2, 1, valid},
        {3, 3, invalid},
        {3, 2, invalid},
        {4, 3, valid},
    };

    for(auto& test : testSet) {
        int actual = invalid;
        SudokuOption::setMode(test.argc, Argv, test.argIndex, OptionSet, actual, valid);
        CPPUNIT_ASSERT_EQUAL(test.expected, actual);
    }
}

SudokuCellCandidates SudokuTestCommon::ConvertToCandidate(char index) {
    return convertToCandidate(static_cast<SudokuIndex>(index));
}

SudokuCellCandidates SudokuTestCommon::ConvertToCandidate(SudokuIndex index) {
    return convertToCandidate(index);
}

SudokuCellCandidates SudokuTestCommon::convertToCandidate(SudokuIndex index) {
    assert((index >= 1) && (index <= 9));
    return (index != 1) ? (SudokuTestCandidates::UniqueBase << (index - 1)) : SudokuTestCandidates::UniqueBase;
}

/*
Local Variables:
mode: c++
coding: utf-8-unix
tab-width: nil
c-file-style: "stroustrup"
End:
*/
