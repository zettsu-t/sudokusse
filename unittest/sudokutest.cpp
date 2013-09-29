// 共通関数
// Copyright (C) 2012-2013 Zettsu Tatsuya

#include <cppunit/extensions/HelperMacros.h>
#include <cassert>
#include "sudoku.h"
#include "sudokutest.h"

class SudokuMacroTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(SudokuMacroTest);
    CPPUNIT_TEST(test_arraySizeof);
    CPPUNIT_TEST_SUITE_END();
protected:
    void test_arraySizeof();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SudokuMacroTest);

void SudokuMacroTest::test_arraySizeof() {
    const size_t expected = 7;
    int array[expected];
    CPPUNIT_ASSERT_EQUAL(expected, arraySizeofSafe(array));
    CPPUNIT_ASSERT_EQUAL(expected, arraySizeof(array));
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
