// Testing class SudokuCell
// Copyright (C) 2012-2017 Zettsu Tatsuya
//
// I use CppUnit code on the website.
// http://www.atmarkit.co.jp/fdotnet/cpptest/cpptest02/cpptest02_02.html

#include <cppunit/extensions/HelperMacros.h>
#include <cassert>
#include <memory>
#include "sudoku.h"
#include "sudokutest.h"

template <class TestedT, class CandidatesT>
class SudokuCellCommonTest {
public:
    SudokuCellCommonTest(std::shared_ptr<TestedT>& pInst);
    virtual ~SudokuCellCommonTest();
    void test_Preset(void);
    void test_Print(void);

private:
    std::shared_ptr<TestedT> pInstance_;
};

class SudokuCellTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(SudokuCellTest);
    CPPUNIT_TEST(test_Preset);
    CPPUNIT_TEST(test_SetIndex);
    CPPUNIT_TEST(test_Print);
    CPPUNIT_TEST(test_GetIndex);
    CPPUNIT_TEST(test_IsFilled);
    CPPUNIT_TEST(test_HasMultipleCandidates);
    CPPUNIT_TEST(test_IsConsistent);
    CPPUNIT_TEST(test_HasCandidate);
    CPPUNIT_TEST(test_HasNoCandidates);
    CPPUNIT_TEST(test_SetCandidates);
    CPPUNIT_TEST(test_GetCandidates);
    CPPUNIT_TEST(test_GetUniqueCandidate);
    CPPUNIT_TEST(test_CountCandidates);
    CPPUNIT_TEST(test_CountCandidatesIfMultiple);
    CPPUNIT_TEST(test_MaskCandidatesUnlessMultiple);
    CPPUNIT_TEST(test_MaskCandidatesUnlessMultiple);
    CPPUNIT_TEST(test_IsEmptyCandidates);
    CPPUNIT_TEST(test_IsUniqueCandidate);
    CPPUNIT_TEST(test_GetEmptyCandidates);
    CPPUNIT_TEST(test_FlipCandidates);
    CPPUNIT_TEST(test_MergeCandidates);
    CPPUNIT_TEST(test_GetInitialCandidate);
    CPPUNIT_TEST(test_GetNextCandidate);
    CPPUNIT_TEST(test_updateState);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;
protected:
    void test_Preset();
    void test_SetIndex();
    void test_Print();
    void test_GetIndex();
    void test_IsFilled();
    void test_HasMultipleCandidates();
    void test_IsConsistent();
    void test_HasCandidate();
    void test_HasNoCandidates();
    void test_SetCandidates();
    void test_GetCandidates();
    void test_GetUniqueCandidate();
    void test_CountCandidates();
    void test_CountCandidatesIfMultiple();
    void test_MaskCandidatesUnlessMultiple();
    void test_IsEmptyCandidates();
    void test_IsUniqueCandidate();
    void test_GetEmptyCandidates();
    void test_FlipCandidates();
    void test_MergeCandidates();
    void test_GetInitialCandidate();
    void test_GetNextCandidate();
    void test_updateState();
private:
    void verifyTestVector(void);
    void checkConstructor(void);
    static bool isUniqueCandidates(SudokuCellCandidates candidates);
    static bool hasMultipleCandidates(SudokuCellCandidates candidates);
    static SudokuIndex countCandidates(SudokuCellCandidates candidates);

    std::shared_ptr<SudokuCell> pInstance_;   // tested object
    std::unique_ptr<SudokuCellCommonTest<SudokuCell, SudokuCellCandidates>> pCommonTester_;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SudokuCellTest);

class SudokuSseCellTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(SudokuSseCellTest);
    CPPUNIT_TEST(test_Preset);
    CPPUNIT_TEST(test_Print);
    CPPUNIT_TEST(test_SetCandidates);
    CPPUNIT_TEST(test_GetCandidates);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;
protected:
    void test_Preset();
    void test_Print();
    void test_SetCandidates();
    void test_GetCandidates();
private:
    void checkConstructor(void);

    std::shared_ptr<SudokuSseCell> pInstance_;   // tested object
    std::unique_ptr<SudokuCellCommonTest<SudokuSseCell, SudokuSseElement>> pCommonTester_;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SudokuSseCellTest);

// Confirms that constants for testing are correct
void SudokuCellTest::verifyTestVector(void) {
    assert(isUniqueCandidates(SudokuTestCandidates::Empty) == false);
    assert(hasMultipleCandidates(SudokuTestCandidates::Empty) == false);
    assert(isUniqueCandidates(SudokuTestCandidates::OneOnly) == true);
    assert(isUniqueCandidates(SudokuTestCandidates::TwoOnly) == true);
    assert(isUniqueCandidates(SudokuTestCandidates::CenterOnly) == true);
    assert(isUniqueCandidates(SudokuTestCandidates::NineOnly) == true);
    assert(hasMultipleCandidates(SudokuTestCandidates::CenterLine) == true);
    assert(hasMultipleCandidates(SudokuTestCandidates::FourAndSix) == true);
    assert(hasMultipleCandidates(SudokuTestCandidates::DoubleLine) == true);
    assert(hasMultipleCandidates(SudokuTestCandidates::TwoToNine) == true);
    assert(hasMultipleCandidates(SudokuTestCandidates::ExceptTwo) == true);
    assert(hasMultipleCandidates(SudokuTestCandidates::ExceptCenter) == true);
    assert(hasMultipleCandidates(SudokuTestCandidates::Odds) == true);
    assert(hasMultipleCandidates(SudokuTestCandidates::Evens) == true);
    assert(hasMultipleCandidates(SudokuTestCandidates::All) == true);
    assert((SudokuTestCandidates::OneOnly + SudokuTestCandidates::TwoToNine)
           == SudokuTestCandidates::All);
    assert((SudokuTestCandidates::TwoOnly + SudokuTestCandidates::ExceptTwo)
           == SudokuTestCandidates::All);
    assert((SudokuTestCandidates::CenterOnly + SudokuTestCandidates::ExceptCenter)
           == SudokuTestCandidates::All);
    assert((SudokuTestCandidates::CenterOnly + SudokuTestCandidates::FourAndSix)
           == SudokuTestCandidates::CenterLine);
    assert((SudokuTestCandidates::CenterLine + SudokuTestCandidates::DoubleLine)
           == SudokuTestCandidates::All);
    assert((SudokuTestCandidates::Odds + SudokuTestCandidates::Evens)
           == SudokuTestCandidates::All);
    assert(arraySizeof(SudokuTestCandidates::CandidateSetOne)
           == arraySizeof(SudokuTestCandidates::CandidateSetTwo));
    assert(arraySizeof(SudokuTestCandidates::CandidateSetOne)
           == arraySizeof(SudokuTestCandidates::CandidateSetCenter));
    return;
}

// Checks whether an instance of SudokuCell is constructed well
void SudokuCellTest::checkConstructor(void) {
    CPPUNIT_ASSERT(pInstance_->indexNumber_ == 0);
    CPPUNIT_ASSERT(pInstance_->candidates_ == SudokuCell::SudokuAllCandidates);
    return;
}

// Returns true if only 1 bit is set (equal to 1), or false otherwise.
bool SudokuCellTest::isUniqueCandidates(SudokuCellCandidates candidates) {
    auto result = false;
    auto restCandidates = candidates;

    for(size_t i=0; i < bitsOf(restCandidates); ++i) {
        if ((restCandidates & 1) == 1) {
            if (result != false) {
                // 2 bits are equal to 1
                return false;
            } else {
                // a bit that equals to 1 is found
                result = true;
            }
        }
        restCandidates >>= 1;
    }
    return result;
}

// Returns true if two or more bits are set (equal to 1), or false otherwise.
bool SudokuCellTest::hasMultipleCandidates(SudokuCellCandidates candidates) {
    auto found = false;
    auto restCandidates = candidates;

    for(size_t i=0; i < bitsOf(restCandidates); ++i) {
        if ((restCandidates & 1) == 1) {
            if (found != false) {
                // 2 bits are equal to 1
                return true;
            } else {
                // a bit that equals to 1 is found
                found = true;
            }
        }
        restCandidates >>= 1;
    }
    return false;
}

// It is better to use a population count instruction or combination
// of bit manipulation to calculate faster. I write this in a simple way
// to avoid test testing itself.
SudokuIndex SudokuCellTest::countCandidates(SudokuCellCandidates candidates) {
    SudokuIndex result = 0;
    auto restCandidates = candidates;

    for(size_t i=0; i < bitsOf(restCandidates); ++i) {
        if ((restCandidates & 1) == 1) {
            ++result;
        }
        restCandidates >>= 1;
    }
    return result;
}

// Call before run a test
void SudokuCellTest::setUp() {
    pInstance_ = decltype(pInstance_)(new SudokuCell());
    pCommonTester_ = decltype(pCommonTester_)(new SudokuCellCommonTest<SudokuCell, SudokuCellCandidates>(pInstance_));
    verifyTestVector();
    checkConstructor();
    return;
}

// Call after run a test
void SudokuCellTest::tearDown() {
    assert(pCommonTester_);
    assert(pInstance_);
    pCommonTester_.reset();

    assert(pInstance_);
    pInstance_.reset();
    return;
}

// Test cases
template <class TestedT, class CandidatesT>
SudokuCellCommonTest<TestedT, CandidatesT>::SudokuCellCommonTest(std::shared_ptr<TestedT>& pInst)
    : pInstance_(pInst) {
    return;
}

template <class TestedT, class CandidatesT>
SudokuCellCommonTest<TestedT, CandidatesT>::~SudokuCellCommonTest() {
    return;
}

template <class TestedT, class CandidatesT>
void SudokuCellCommonTest<TestedT, CandidatesT>::test_Preset() {
    constexpr CandidatesT dirty = 0xe6;          // candidates when this cell in not set
    constexpr char invalidCharSet[] {"0Ab. "};   // invalid characters for preset

    // If non-digit char and 0 are given, candidates of cells are not changed.
    pInstance_->candidates_ = dirty;
    for(const auto& test : invalidCharSet) {
        pInstance_->Preset(test);
        CPPUNIT_ASSERT_EQUAL(dirty, pInstance_->candidates_);
    }

    // 'nul' is invalid
    pInstance_->Preset('\0');
    CPPUNIT_ASSERT_EQUAL(dirty, pInstance_->candidates_);

    // 1..9 are valid
    for(char ofs = 1; ofs <= 9; ++ofs) {
        pInstance_->Preset('0'+ofs);
        auto expected = SudokuTestCommon::ConvertToCandidate(ofs);
        CPPUNIT_ASSERT_EQUAL(expected, pInstance_->candidates_);
    }

    return;
}

template <class TestedT, class CandidatesT>
void SudokuCellCommonTest<TestedT, CandidatesT>::test_Print() {
    for(CandidatesT candidates=0; candidates < Sudoku::SizeOfLookUpCell; ++candidates) {
        SudokuOutStream sudokuOutStream;
        pInstance_->_candidates = candidates;
        pInstance_->Print(&sudokuOutStream);
        auto expected = SudokuCell::CellLookUp_[candidates].NumberOfCandidates;
        // Expect the length is equal to number of candidates
        CPPUNIT_ASSERT_EQUAL(expected, sudokuOutStream.str().length());
    }

    // Do not crash when the argument is null
    pInstance_->_candidates = SudokuTestCandidates::CenterLine;
    pInstance_->Print(nullptr);
    return;
}

void SudokuCellTest::test_Preset() {
    pCommonTester_->test_Preset();
    return;
}

void SudokuCellTest::test_SetIndex() {
    for(SudokuIndex i=0;i<Sudoku::SizeOfAllCells;++i) {
        pInstance_->SetIndex(i);
        CPPUNIT_ASSERT_EQUAL(i, pInstance_->indexNumber_);
    }

    return;
}

void SudokuCellTest::test_Print() {
    pCommonTester_->test_Preset();
    return;
}

void SudokuCellTest::test_GetIndex() {
    // First initialize with one-of-range value
    pInstance_->indexNumber_ = Sudoku::SizeOfAllCells + 1;

    for(SudokuIndex i=0;i<Sudoku::SizeOfAllCells;++i) {
        pInstance_->indexNumber_ = i;
        CPPUNIT_ASSERT_EQUAL(i, pInstance_->GetIndex());
    }

    return;
}

void SudokuCellTest::test_IsFilled() {
    for(SudokuCellCandidates candidates=0; candidates < Sudoku::SizeOfLookUpCell; ++candidates) {
        pInstance_->candidates_ = candidates;
        CPPUNIT_ASSERT_EQUAL(isUniqueCandidates(candidates), pInstance_->IsFilled());
    }

    return;
}

void SudokuCellTest::test_HasMultipleCandidates() {
    for(SudokuCellCandidates candidates=0; candidates < Sudoku::SizeOfLookUpCell; ++candidates) {
        pInstance_->candidates_ = candidates;
        CPPUNIT_ASSERT_EQUAL(hasMultipleCandidates(candidates), pInstance_->HasMultipleCandidates());
    }

    return;
}

void SudokuCellTest::test_IsConsistent() {
    // not filled
    struct TestSetNotFilled{
        SudokuCellCandidates candidates;
        bool consistent;
    };

    constexpr TestSetNotFilled testSetNotFilled[] {
        {SudokuTestCandidates::TwoToNine, true},
        {SudokuTestCandidates::Empty, false}};

    for(const auto& test : testSetNotFilled) {
        auto expacted = test.consistent;
        pInstance_->candidates_ = test.candidates;
        for(const auto& arg : SudokuTestCandidates::CandidateSetOne) {
            CPPUNIT_ASSERT_EQUAL(expacted, pInstance_->IsConsistent(arg));
        }
    }

    // filled
    struct TestSetFilled{
        SudokuCellCandidates candidates;
        const SudokuCellCandidates *candidatesSet;
        bool consistent[SudokuTestCandidates::candidateSetSize];
    };

    constexpr TestSetFilled testSetFilled[] {
        {SudokuTestCandidates::OneOnly, SudokuTestCandidates::CandidateSetOne,
         {false, true, false, true, false}},
        {SudokuTestCandidates::TwoOnly, SudokuTestCandidates::CandidateSetTwo,
         {false, true, true, false, false}}};

    static_assert(arraySizeof(testSetFilled[0].consistent) == SudokuTestCandidates::candidateSetSize,
        "Unexpected testSetFilled[0].consistent size");

    for(const auto& test : testSetFilled) {
        pInstance_->candidates_ = test.candidates;
        for(size_t j=0; j < arraySizeof(test.consistent); ++j) {
            auto expacted = test.consistent[j];
            CPPUNIT_ASSERT_EQUAL(expacted, pInstance_->IsConsistent(test.candidatesSet[j]));
        }
    }

    return;
}

void SudokuCellTest::test_HasCandidate() {
    for(SudokuCellCandidates candidates=0; candidates < Sudoku::SizeOfLookUpCell; ++candidates) {
        pInstance_->candidates_ = candidates;
        for(SudokuIndex i=0;i<Sudoku::SizeOfCandidates;++i) {
            const SudokuCellCandidates candidate = 1 << i;
            const bool expected = (candidates & candidate);
            CPPUNIT_ASSERT_EQUAL(expected, pInstance_->HasCandidate(candidate));
        }
    }
}

void SudokuCellTest::test_HasNoCandidates() {
    for(SudokuCellCandidates candidates=0; candidates < Sudoku::SizeOfLookUpCell; ++candidates) {
        pInstance_->candidates_ = candidates;
        auto expected = (candidates == SudokuTestCandidates::Empty);
        CPPUNIT_ASSERT_EQUAL(expected, pInstance_->HasNoCandidates());
    }

    return;
}

void SudokuCellTest::test_SetCandidates() {
    struct TestSet{
        SudokuCellCandidates pre;
        SudokuCellCandidates mask;
        SudokuCellCandidates post;
    };

    constexpr TestSet testSet[] {
        // These decrease candidates
        {SudokuTestCandidates::All, SudokuTestCandidates::OneOnly, SudokuTestCandidates::OneOnly},
        {SudokuTestCandidates::All, SudokuTestCandidates::ExceptTwo, SudokuTestCandidates::ExceptTwo},
        {SudokuTestCandidates::ExceptCenter, SudokuTestCandidates::CenterLine, SudokuTestCandidates::FourAndSix},
        // These do not change candidates
        {SudokuTestCandidates::OneOnly, SudokuTestCandidates::OneOnly, SudokuTestCandidates::OneOnly},
        {SudokuTestCandidates::ExceptTwo, SudokuTestCandidates::ExceptTwo, SudokuTestCandidates::ExceptTwo},
        // This clear all candidates
        {SudokuTestCandidates::Odds, SudokuTestCandidates::Evens, SudokuTestCandidates::Empty}
    };

    for(const auto& test : testSet) {
        pInstance_->candidates_ = test.pre;
        auto expacted = test.post;
        pInstance_->SetCandidates(test.mask);
        CPPUNIT_ASSERT_EQUAL(expacted, pInstance_->candidates_);
    }

    return;
}

void SudokuCellTest::test_GetCandidates() {
    for(SudokuCellCandidates candidates=0; candidates < Sudoku::SizeOfLookUpCell; ++candidates) {
        pInstance_->candidates_ = candidates;
        CPPUNIT_ASSERT_EQUAL(candidates, pInstance_->GetCandidates());
    }

    return;
}

void SudokuCellTest::test_GetUniqueCandidate() {
    for(SudokuCellCandidates candidates=0; candidates < Sudoku::SizeOfLookUpCell; ++candidates) {
        pInstance_->candidates_ = candidates;
        SudokuCellCandidates expected = isUniqueCandidates(candidates) ?
            candidates : SudokuTestCandidates::Empty;
        CPPUNIT_ASSERT_EQUAL(expected, pInstance_->GetUniqueCandidate());
    }

    return;
}

void SudokuCellTest::test_CountCandidates() {
    for(SudokuCellCandidates candidates=0; candidates < Sudoku::SizeOfLookUpCell; ++candidates) {
        pInstance_->candidates_ = candidates;
        auto expected = countCandidates(candidates);
        CPPUNIT_ASSERT_EQUAL(expected, pInstance_->CountCandidates());
    }

    return;
}

void SudokuCellTest::test_CountCandidatesIfMultiple() {
    for(SudokuCellCandidates candidates=0; candidates < Sudoku::SizeOfLookUpCell; ++candidates) {
        pInstance_->candidates_ = candidates;
        auto count = countCandidates(candidates);
        auto expected = count;

        switch(count) {
        case 0:
        case 1:
            expected = Sudoku::OutOfRangeCandidates;
            break;
        default:
            break;
        }
        assert(expected != SudokuTestCandidates::Empty);
        CPPUNIT_ASSERT_EQUAL(expected, pInstance_->CountCandidatesIfMultiple());
    }

    return;
}

void SudokuCellTest::test_MaskCandidatesUnlessMultiple() {
    // 2 to 9
    for(SudokuIndex i=Sudoku::SizeOfUniqueCandidate+1;i<Sudoku::SizeOfCandidates;++i) {
        CPPUNIT_ASSERT_EQUAL(i, pInstance_->MaskCandidatesUnlessMultiple(i));
    }

    // otherwise
    CPPUNIT_ASSERT_EQUAL(static_cast<SudokuIndex>(0),
                         pInstance_->MaskCandidatesUnlessMultiple(Sudoku::OutOfRangeCandidates));
    return;
}

void SudokuCellTest::test_IsEmptyCandidates() {
    for(SudokuCellCandidates candidates=0; candidates < Sudoku::SizeOfLookUpCell; ++candidates) {
        bool expected = (candidates == SudokuTestCandidates::Empty);
        CPPUNIT_ASSERT_EQUAL(expected, pInstance_->IsEmptyCandidates(candidates));
    }

    return;
}

void SudokuCellTest::test_IsUniqueCandidate() {
    for(SudokuCellCandidates candidates=0; candidates < Sudoku::SizeOfLookUpCell; ++candidates) {
        CPPUNIT_ASSERT_EQUAL(isUniqueCandidates(candidates), pInstance_->IsUniqueCandidate(candidates));
    }

    return;
}

void SudokuCellTest::test_GetEmptyCandidates() {
    CPPUNIT_ASSERT_EQUAL(SudokuTestCandidates::Empty, pInstance_->GetEmptyCandidates());
    return;
}

void SudokuCellTest::test_FlipCandidates() {
    struct TestSet {
        SudokuCellCandidates arg;
        SudokuCellCandidates result;
    };

    constexpr TestSet testSet[] {
        {SudokuTestCandidates::All, SudokuTestCandidates::Empty},
        {SudokuTestCandidates::Empty, SudokuTestCandidates::All},
        {SudokuTestCandidates::OneOnly, SudokuTestCandidates::TwoToNine},
        {SudokuTestCandidates::ExceptTwo, SudokuTestCandidates::TwoOnly},
        {SudokuTestCandidates::CenterOnly, SudokuTestCandidates::ExceptCenter},
        {SudokuTestCandidates::Odds, SudokuTestCandidates::Evens}
    };

    for(const auto& test : testSet) {
        CPPUNIT_ASSERT_EQUAL(test.result, pInstance_->FlipCandidates(test.arg));
    }

    return;
}

void SudokuCellTest::test_MergeCandidates() {
    struct TestSet {
        SudokuCellCandidates arg1;
        SudokuCellCandidates arg2;
        SudokuCellCandidates result;
    };

    constexpr TestSet testSet[] {
        // These add candidates
        {SudokuTestCandidates::OneOnly, SudokuTestCandidates::TwoToNine, SudokuTestCandidates::All},
        {SudokuTestCandidates::ExceptTwo, SudokuTestCandidates::TwoOnly, SudokuTestCandidates::All},
        {SudokuTestCandidates::Odds, SudokuTestCandidates::Evens, SudokuTestCandidates::All},
        {SudokuTestCandidates::Evens, SudokuTestCandidates::Odds, SudokuTestCandidates::All},
        {SudokuTestCandidates::CenterOnly, SudokuTestCandidates::FourAndSix, SudokuTestCandidates::CenterLine},
        // These do not change candidates
        {SudokuTestCandidates::OneOnly, SudokuTestCandidates::OneOnly, SudokuTestCandidates::OneOnly},
        {SudokuTestCandidates::ExceptTwo, SudokuTestCandidates::ExceptTwo, SudokuTestCandidates::ExceptTwo},
        {SudokuTestCandidates::All, SudokuTestCandidates::All, SudokuTestCandidates::All},
        {SudokuTestCandidates::CenterLine, SudokuTestCandidates::FourAndSix, SudokuTestCandidates::CenterLine},
        {SudokuTestCandidates::FourAndSix, SudokuTestCandidates::CenterLine, SudokuTestCandidates::CenterLine},
        {SudokuTestCandidates::OneOnly, SudokuTestCandidates::Empty, SudokuTestCandidates::OneOnly},
        {SudokuTestCandidates::ExceptTwo, SudokuTestCandidates::Empty, SudokuTestCandidates::ExceptTwo},
        {SudokuTestCandidates::All, SudokuTestCandidates::Empty, SudokuTestCandidates::All},
        {SudokuTestCandidates::Empty, SudokuTestCandidates::Empty, SudokuTestCandidates::Empty}
    };

    for(const auto& test : testSet) {
        CPPUNIT_ASSERT_EQUAL(test.result, pInstance_->MergeCandidates(test.arg1, test.arg2));
    }

    return;
}

void SudokuCellTest::test_GetInitialCandidate() {
    CPPUNIT_ASSERT_EQUAL(SudokuTestCandidates::OneOnly, pInstance_->GetInitialCandidate());
    return;
}

void SudokuCellTest::test_GetNextCandidate() {
    SudokuCellCandidates candidates = SudokuTestCandidates::OneOnly;

    // Call too many times and confirm it gets 'Empty'
    for(SudokuIndex i=0;i<=(Sudoku::SizeOfCandidates+3);++i) {
        SudokuCellCandidates expected = ((i+1) >= Sudoku::SizeOfCandidates) ?
            SudokuTestCandidates::Empty : (candidates << 1);
        CPPUNIT_ASSERT_EQUAL(expected, pInstance_->GetNextCandidate(candidates));
        candidates <<= 1;
    }

    return;
}

void SudokuCellTest::test_updateState() {
    // Confirm this returns
    pInstance_->updateState();
    return;
}

void SudokuSseCellTest::checkConstructor(void) {
    CPPUNIT_ASSERT(pInstance_->candidates_ == SudokuSseCell::AllCandidates);
    return;
}

// Call before run a test
void SudokuSseCellTest::setUp() {
    pInstance_ = decltype(pInstance_)(new SudokuSseCell());
    pCommonTester_ = decltype(pCommonTester_)(new SudokuCellCommonTest<SudokuSseCell, SudokuSseElement>(pInstance_));
    checkConstructor();
    return;
}

// Call after run a test
void SudokuSseCellTest::tearDown() {
    assert(pCommonTester_);
    assert(pInstance_);
    pCommonTester_.reset();

    assert(pInstance_);
    pInstance_.reset();
    return;
}

void SudokuSseCellTest::test_Preset() {
    pCommonTester_->test_Preset();
    return;
}

void SudokuSseCellTest::test_Print() {
    pCommonTester_->test_Preset();
    return;
}

void SudokuSseCellTest::test_SetCandidates() {
    struct TestSet {
        SudokuSseElement pre;
        SudokuSseElement post;
    };

    constexpr TestSet testSet[] {{0xffffffff, 0}, {0, 1}, {1, 0x4000000}, {0x80000000, 0x7ffffff}};
    SudokuSseElement post;

    for(const auto& test : testSet) {
        pInstance_->candidates_ = test.pre;
        post = test.post;
        pInstance_->SetCandidates(post);
        CPPUNIT_ASSERT_EQUAL(post, pInstance_->candidates_);
    }
    return;
}

void SudokuSseCellTest::test_GetCandidates() {
    constexpr SudokuSseElement testSet[] {0, 1, 0x4000000, 0x7ffffff};
    SudokuSseElement expected;
    SudokuSseElement actual;

    for(const auto& test : testSet) {
        expected = test;
        pInstance_->candidates_ = expected;
        actual = pInstance_->GetCandidates();
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
