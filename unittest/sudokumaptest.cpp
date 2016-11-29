// Sudokumapクラスをテストする
// Copyright (C) 2012-2015 Zettsu Tatsuya
//
// クラス定義は下記から流用
// http://www.atmarkit.co.jp/fdotnet/cpptest/cpptest02/cpptest02_02.html

#include <cppunit/extensions/HelperMacros.h>
#include <cassert>
#include <cstring>
#include <memory>
#include "sudoku.h"
#include "sudokutest.h"

class SudokuMapTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(SudokuMapTest);
    CPPUNIT_TEST(test_Preset);
    CPPUNIT_TEST(test_Print);
    CPPUNIT_TEST(test_IsFilled);
    CPPUNIT_TEST(test_FillCrossing);
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
    std::unique_ptr<SudokuMap> pInstance_;   // インスタンス
};

CPPUNIT_TEST_SUITE_REGISTRATION(SudokuMapTest);

class SudokuSseMapTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(SudokuSseMapTest);
    CPPUNIT_TEST(test_Preset);
    CPPUNIT_TEST(test_Print);
    CPPUNIT_TEST(test_FillCrossing);
    CPPUNIT_TEST(test_SearchNextNew);
    CPPUNIT_TEST(test_SearchNextOld);
    CPPUNIT_TEST(test_SetUniqueCell);
    CPPUNIT_TEST(test_getRegisterIndex);
    CPPUNIT_TEST(test_countCandidates);
    CPPUNIT_TEST(test_countCandidatesIfMultiple);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

protected:
    void test_Preset();
    void test_Print();
    void test_FillCrossing();
    void test_SearchNextNew();
    void test_SearchNextOld();
    void test_SetUniqueCell();
    void test_getRegisterIndex();
    void test_countCandidates();
    void test_countCandidatesIfMultiple();

private:
    void checkConstructor(void);
    void setAllCellsFullCandidates(void);
    void checkCandidateCell(const SudokuSseCandidateCell& expected, const SudokuSseCandidateCell& actual);
    std::unique_ptr<SudokuSseMap> pInstance_;   // インスタンス
};

CPPUNIT_TEST_SUITE_REGISTRATION(SudokuSseMapTest);

// コンストラクタを調べる
void SudokuMapTest::checkConstructor(void) {
    checkCellIndexes();
    return;
}

// インデックスが正しいかどうか検査する
void SudokuMapTest::checkCellIndexes(void) {
    decltype(SudokuCell::indexNumber_) i = 0;

    for(const auto& cell : pInstance_->cells_) {
        CPPUNIT_ASSERT(cell.indexNumber_ == i);
        ++i;
    }
    return;
}

// 設定する数字を候補に変換する
SudokuCellCandidates SudokuMapTest::indexToCandidate(SudokuIndex index) {
    if ((index < 1) || (index > 9)) {
        return SudokuCell::SudokuAllCandidates;
    }

    return SudokuTestCommon::ConvertToCandidate(index);
}

// 全マス埋める
void SudokuMapTest::setAllCellsFilled(void) {
    for(SudokuIndex i=0;i<Sudoku::SizeOfAllCells;++i) {
        pInstance_->cells_[i].candidates_ = SudokuTestCandidates::OneOnly;
    }
    return;
}

// 全マス複数候補にする
void SudokuMapTest::setAllCellsMultiCandidates(void) {
    for(SudokuIndex i=0;i<Sudoku::SizeOfAllCells;++i) {
        pInstance_->cells_[i].candidates_ = SudokuTestCandidates::TwoToNine;
    }

    return;
}

// 全マス全候補にする
void SudokuMapTest::setAllCellsFullCandidates(void) {
    for(SudokuIndex i=0;i<Sudoku::SizeOfAllCells;++i) {
        pInstance_->cells_[i].candidates_ = SudokuTestCandidates::All;
    }

    return;
}

// 候補を全3*3マスに設定する
void SudokuMapTest::setConsistentCells(SudokuCellCandidates candidates) {
    // 3*3マスを埋める
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

// 最上行を右端以外設定める
void SudokuMapTest::setTopLineExceptRightest(void) {
    for(SudokuIndex i=0;i<Sudoku::SizeOfCellsPerGroup - 1;++i) {
        pInstance_->cells_[i].candidates_ = indexToCandidate(i+1);
    }

    return;
}

// 各テスト・ケースの実行直前に呼ばれる
void SudokuMapTest::setUp() {
    pInstance_ = decltype(pInstance_)(new SudokuMap());
    checkConstructor();
    return;
}

// 各テスト・ケースの実行直後に呼ばれる
void SudokuMapTest::tearDown() {
    return;
}

// これ以降はテスト・ケースの実装内容
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

    // 短すぎる場合
    pInstance_->Preset(ShortStr, 0);
    return;
}

void SudokuMapTest::test_Print() {
    SudokuCellCandidates candidates = 0;
    SudokuIndex index = 0;
    size_t expected = 0;  // 予想文字列長

    for(SudokuIndex y=0; y < Sudoku::SizeOfCellsPerGroup; ++y) {
        for(SudokuIndex x=0; x < Sudoku::SizeOfCellsPerGroup; ++x) {
            pInstance_->cells_[index++].candidates_ = candidates;
            expected += y+1; // 区切り文字分
        }
        candidates = (candidates << 1) | SudokuTestCandidates::UniqueBase;
        ++expected; // 改行分
    }

    // 最後の要素を8要素から全部にする
    pInstance_->cells_[Sudoku::SizeOfAllCells - 1].candidates_ = SudokuTestCandidates::All;
    ++expected;
    // 最後の改行
    ++expected;

    SudokuOutStream sudokuOutStream;
    pInstance_->Print(&sudokuOutStream);
    CPPUNIT_ASSERT_EQUAL(expected, sudokuOutStream.str().length());

    // 引数がnull
    pInstance_->cells_[0].candidates_ = SudokuTestCandidates::CenterLine;
    pInstance_->Print(0);
    return;
}

void SudokuMapTest::test_IsFilled() {
    // 全マス埋まっている
    setAllCellsFilled();
    CPPUNIT_ASSERT_EQUAL(true, pInstance_->IsFilled());

    // 一つずつ解除
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
    // ?=1, *=[4,6], !=2, ^=9が埋まる
    // @を4にすると矛盾が生じる
    // これらを置換して全マスを検査する
    // 3*3マス内の縦横の列を入れ替える
    // 3*3マスの縦横を入れ替える
    // つまり上表のx,yを、試験対象のx,yに入れ替える

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
    // マスを入れ替えて設定する
    for(const auto& test : testSet) {
        const auto target = convertCellPosition(test.cellIndex, boxX, boxY, inboxOfsX, inboxOfsY);
        pInstance_->cells_[target].candidates_ = indexToCandidate(test.candidatesIndex);
    }

    CPPUNIT_ASSERT_EQUAL(false, pInstance_->SudokuMap::FillCrossing());

    // 検査する
    for(const auto& result : resultSet) {
        const auto target = convertCellPosition(result.cellIndex, boxX, boxY, inboxOfsX, inboxOfsY);
        CPPUNIT_ASSERT_EQUAL(result.candidates, pInstance_->cells_[target].candidates_);
    }

    // 矛盾させる
    for(const auto& conflict : conflictSet) {
        const auto target = convertCellPosition(conflict.cellIndex, boxX, boxY, inboxOfsX, inboxOfsY);
        pInstance_->cells_[target].candidates_ = indexToCandidate(conflict.candidatesIndex);
    }

    CPPUNIT_ASSERT_EQUAL(true, pInstance_->SudokuMap::FillCrossing());
    return;
}

void SudokuMapTest::test_FillCrossing() {
    // 3*3マス
    for(SudokuIndex boxX = 0; boxX < Sudoku::SizeOfBoxesOnEdge; ++boxX) {
        for(SudokuIndex boxY = 0; boxY < Sudoku::SizeOfBoxesOnEdge; ++boxY) {
            // 3*3マスの中
            for(SudokuIndex ofsX = 0; ofsX < Sudoku::SizeOfCellsOnBoxEdge; ++ofsX) {
                for(SudokuIndex ofsY = 0; ofsY < Sudoku::SizeOfCellsOnBoxEdge; ++ofsY) {
                    testFillCrossingBox(boxX, boxY, ofsX, ofsY);
                }
            }
        }
    }

    return;
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
        // 9,8,7,6,5,4,3,2,1の順
        auto candidate = indexToCandidate(9 - index);
        pInstance_->cells_[i].candidates_ = testSet[index].candidates;
        CPPUNIT_ASSERT_EQUAL(expected, pInstance_->SetUniqueCell(i, candidate));
        // 1..9の順
        candidate = indexToCandidate(index + 1);
        pInstance_->cells_[i].candidates_ = testSet[index].candidates;
        CPPUNIT_ASSERT_EQUAL(expected, pInstance_->SetUniqueCell(i, candidate));
    }

    return;
}

void SudokuMapTest::test_CountFilledCells() {
    // 全マス未定
    setAllCellsMultiCandidates();
    CPPUNIT_ASSERT_EQUAL(static_cast<SudokuIndex>(0), pInstance_->CountFilledCells());

    // 一つずつ解除
    SudokuIndex expected = 0;
    for(SudokuIndex i=0;i<Sudoku::SizeOfAllCells;++i) {
        ++expected;
        pInstance_->cells_[i].candidates_ = SudokuTestCandidates::OneOnly;
        CPPUNIT_ASSERT_EQUAL(expected, pInstance_->CountFilledCells());
    }

    // 全マス埋まっている
    setAllCellsFilled();
    CPPUNIT_ASSERT_EQUAL(Sudoku::SizeOfAllCells, pInstance_->CountFilledCells());

    // 一つずつ未定にする
    SudokuIndex i = Sudoku::SizeOfAllCells;
    do {
        --i;
        pInstance_->cells_[i].candidates_ = SudokuTestCandidates::TwoToNine;
        CPPUNIT_ASSERT_EQUAL(i, pInstance_->CountFilledCells());
    } while(i != 0);

    return;
}

void SudokuMapTest::test_SelectBacktrackedCellIndex() {
    // まず矛盾のない場合
    // 全マス全候補
    setAllCellsFullCandidates();
    auto expected = SudokuTestPosition::Head;
    CPPUNIT_ASSERT_EQUAL(expected, pInstance_->SelectBacktrackedCellIndex());

    // 最後に8候補
    expected = SudokuTestPosition::Last;
    pInstance_->cells_[expected].candidates_ = SudokuTestCandidates::ExceptCenter;
    CPPUNIT_ASSERT_EQUAL(expected, pInstance_->SelectBacktrackedCellIndex());

    // 二番目にも8候補
    expected = SudokuTestPosition::HeadNext;
    pInstance_->cells_[expected].candidates_ = SudokuTestCandidates::TwoToNine;
    CPPUNIT_ASSERT_EQUAL(expected, pInstance_->SelectBacktrackedCellIndex());

    // 右下に2候補のマスが二つある。3*3マスの総候補が少ないので選ばれる。
    expected = SudokuTestPosition::Backtracked;
    pInstance_->cells_[expected].candidates_ = SudokuTestCandidates::FourAndSix;
    CPPUNIT_ASSERT_EQUAL(expected, pInstance_->SelectBacktrackedCellIndex());

    // 真ん中に候補のないマスがあるがこれは関係ない
    pInstance_->cells_[SudokuTestPosition::Center].candidates_ = SudokuTestCandidates::Empty;
    CPPUNIT_ASSERT_EQUAL(expected, pInstance_->SelectBacktrackedCellIndex());

    // 真ん中に確定したマスがあるがこれは関係ない
    pInstance_->cells_[SudokuTestPosition::Center].candidates_ = SudokuTestCandidates::CenterOnly;
    CPPUNIT_ASSERT_EQUAL(expected, pInstance_->SelectBacktrackedCellIndex());

    return;
}

// 要見直し
void SudokuMapTest::test_IsConsistent() {
    // まず矛盾のない場合
    // 全マス全候補
    setAllCellsFullCandidates();
    CPPUNIT_ASSERT_EQUAL(true, pInstance_->IsConsistent());

    // 1をすべての3*3マスに設定する
    setConsistentCells(SudokuTestCandidates::OneOnly);
    CPPUNIT_ASSERT_EQUAL(true, pInstance_->IsConsistent());

    // 空白にする
    pInstance_->cells_[SudokuTestPosition::Last].candidates_ = SudokuTestCandidates::Empty;
    CPPUNIT_ASSERT_EQUAL(false, pInstance_->IsConsistent());

    // 矛盾がある
    setConsistentCells(SudokuTestCandidates::OneOnly);
    pInstance_->cells_[SudokuTestPosition::Conflict].candidates_ = SudokuTestCandidates::OneOnly;
    CPPUNIT_ASSERT_EQUAL(false, pInstance_->IsConsistent());

    return;
}

// 式を共通化できない?
void SudokuMapTest::test_findUnusedCandidate() {
    // 横一列
    setAllCellsFullCandidates();
    setTopLineExceptRightest();
    auto target = Sudoku::SizeOfCellsPerGroup - 1;
    CPPUNIT_ASSERT_EQUAL(false, pInstance_->findUnusedCandidate(pInstance_->cells_[target]));
    auto expected = indexToCandidate(target+1);
    CPPUNIT_ASSERT_EQUAL(expected, pInstance_->cells_[target].candidates_);

    // 矛盾
    setAllCellsFullCandidates();
    setTopLineExceptRightest();
    pInstance_->cells_[Sudoku::SizeOfAllCells - 1].candidates_ = indexToCandidate(Sudoku::SizeOfCellsPerGroup);
    CPPUNIT_ASSERT_EQUAL(true, pInstance_->findUnusedCandidate(pInstance_->cells_[target]));
    expected = SudokuTestCandidates::Empty;
    CPPUNIT_ASSERT_EQUAL(expected, pInstance_->cells_[target].candidates_);

    // 縦一列
    setAllCellsFullCandidates();
    for(SudokuIndex i=0;i<Sudoku::SizeOfCellsPerGroup - 1;++i) {
        pInstance_->cells_[Sudoku::SizeOfCellsPerGroup - 1 + Sudoku::SizeOfCellsPerGroup * i].
            candidates_ = indexToCandidate(i+1);
    }
    target = Sudoku::SizeOfAllCells - 1;
    CPPUNIT_ASSERT_EQUAL(false, pInstance_->findUnusedCandidate(pInstance_->cells_[target]));
    expected = indexToCandidate(Sudoku::SizeOfCellsPerGroup);
    CPPUNIT_ASSERT_EQUAL(expected, pInstance_->cells_[target].candidates_);

    // 3*3
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

    // 合成
    struct TestSet{
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
    // 横一列
    setAllCellsFullCandidates();
    auto target = Sudoku::SizeOfCellsPerGroup - 1;
    for(SudokuIndex i=0;i<target;++i) {
        pInstance_->cells_[i].candidates_ = SudokuTestCandidates::TwoToNine;
    }
    CPPUNIT_ASSERT_EQUAL(false, pInstance_->findUniqueCandidate(pInstance_->cells_[target]));
    auto expected = SudokuTestCandidates::OneOnly;
    CPPUNIT_ASSERT_EQUAL(expected, pInstance_->cells_[target].candidates_);

    // これは候補を絞らない
    setAllCellsFullCandidates();
    target = Sudoku::SizeOfCellsPerGroup - 1;
    pInstance_->cells_[SudokuTestPosition::Head].candidates_ = SudokuTestCandidates::OneOnly;
    pInstance_->cells_[SudokuTestPosition::HeadNext].candidates_ = SudokuTestCandidates::DoubleLine;
    CPPUNIT_ASSERT_EQUAL(false, pInstance_->findUniqueCandidate(pInstance_->cells_[target]));
    expected = SudokuTestCandidates::All;
    CPPUNIT_ASSERT_EQUAL(expected, pInstance_->cells_[target].candidates_);

    // 縦一列
    setAllCellsFullCandidates();
    for(SudokuIndex i=0;i<Sudoku::SizeOfCellsPerGroup - 1;++i) {
        pInstance_->cells_[Sudoku::SizeOfCellsPerGroup - 1 + Sudoku::SizeOfCellsPerGroup * i].
            candidates_ = SudokuTestCandidates::ExceptTwo;
    }
    target = Sudoku::SizeOfAllCells - 1;
    CPPUNIT_ASSERT_EQUAL(false, pInstance_->findUniqueCandidate(pInstance_->cells_[target]));
    expected = SudokuTestCandidates::TwoOnly;
    CPPUNIT_ASSERT_EQUAL(expected, pInstance_->cells_[target].candidates_);

    // 3*3
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

    // 矛盾
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

// 各テスト・ケースの実行直前に呼ばれる
void SudokuSseMapTest::setUp() {
    pInstance_ = decltype(pInstance_)(new SudokuSseMap());
    checkConstructor();
    return;
}

// 各テスト・ケースの実行直後に呼ばれる
void SudokuSseMapTest::tearDown() {
    return;
}

// コンストラクタを調べる
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

void SudokuSseMapTest::setAllCellsFullCandidates(void) {
    for(size_t i = SudokuSseMap::InitialRegisterNum;
        i < (SudokuSseMap::InitialRegisterNum + Sudoku::SizeOfGroupsPerMap); ++i) {
        for(size_t j = 0; j < (SudokuSse::RegisterWordCnt - 1); ++j) {
            pInstance_->xmmRegSet_.regVal_[i * SudokuSse::RegisterWordCnt + j] = Sudoku::AllThreeCandidates;
        }
        pInstance_->xmmRegSet_.regVal_[(i + 1) * SudokuSse::RegisterWordCnt - 1] = 0;
    }

    return;
}

void SudokuSseMapTest::checkCandidateCell(const SudokuSseCandidateCell& expected, const SudokuSseCandidateCell& actual) {
    CPPUNIT_ASSERT_EQUAL(expected.regIndex, actual.regIndex);
    CPPUNIT_ASSERT_EQUAL(expected.shift, actual.shift);
    CPPUNIT_ASSERT_EQUAL(expected.mask, actual.mask);
    CPPUNIT_ASSERT_EQUAL(expected.count, actual.count);
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
    size_t expected = 0;  // 予想文字列長

    for(SudokuIndex y=0; y < Sudoku::SizeOfCellsPerGroup; ++y) {
        for(SudokuIndex x=0; x < Sudoku::SizeOfBoxesOnEdge; ++x) {
            // 3マスまとめて設定する
            pInstance_->xmmRegSet_.regVal_[regIndex++] = candidates;
            expected += 3 * (y + 1);
        }
        ++regIndex;
        candidates = (candidates << 1) | candidates;  // 候補を1マス1個ずつ増やす
        expected += Sudoku::SizeOfCellsPerGroup;    // 区切り文字分
        ++expected; // 改行分
    }

    // 最後の改行
    ++expected;

    SudokuOutStream sudokuOutStream;
    pInstance_->Print(&sudokuOutStream);
    CPPUNIT_ASSERT_EQUAL(expected, sudokuOutStream.str().length());

    // 引数がnull
    pInstance_->Print(0);
    return;
}

void SudokuSseMapTest::test_FillCrossing() {}

void SudokuSseMapTest::test_SearchNextNew() {
    const SudokuSseCandidateCell emptyCell = {SudokuSseMap::InitialRegisterNum * SudokuSse::RegisterWordCnt, 0, 0, 0};

    // 全マス全候補はありえない
    setAllCellsFullCandidates();
    SudokuSseCandidateCell cell = emptyCell;
    SudokuSseCandidateCell expected = {SudokuSseMap::InitialRegisterNum * SudokuSse::RegisterWordCnt,
                                       0, 0, Sudoku::SizeOfCandidates};
    CPPUNIT_ASSERT_EQUAL(false, pInstance_->SearchNext(cell));

    // 最後に8候補
    cell = emptyCell;
    size_t regIndex = (SudokuSseMap::InitialRegisterNum + Sudoku::SizeOfGroupsPerMap) * SudokuSse::RegisterWordCnt - 2;
    expected = {regIndex, 0, Sudoku::AllCandidates, Sudoku::SizeOfCandidates - 1};
    pInstance_->xmmRegSet_.regVal_[regIndex] = Sudoku::AllThreeCandidates ^ 1;
    CPPUNIT_ASSERT_EQUAL(true, pInstance_->SearchNext(cell));
    checkCandidateCell(expected, cell);

    // 二番目にも8候補
    cell = emptyCell;
    regIndex = SudokuSseMap::InitialRegisterNum * SudokuSse::RegisterWordCnt + 2;
    expected = {regIndex, Sudoku::SizeOfCandidates,
                Sudoku::AllCandidates << Sudoku::SizeOfCandidates,
                Sudoku::SizeOfCandidates - 1};
    pInstance_->xmmRegSet_.regVal_[regIndex] = Sudoku::AllThreeCandidates ^ (1 << Sudoku::SizeOfCandidates);
    CPPUNIT_ASSERT_EQUAL(true, pInstance_->SearchNext(cell));
    checkCandidateCell(expected, cell);

    // 右下に2候補のマスが二つある。3*3マスの総候補が少ないので選ばれる。
    cell = emptyCell;
    regIndex = (SudokuSseMap::InitialRegisterNum + 7) * SudokuSse::RegisterWordCnt;
    expected = {regIndex, Sudoku::SizeOfCandidates,
                Sudoku::AllCandidates << Sudoku::SizeOfCandidates, 2};
    pInstance_->xmmRegSet_.regVal_[regIndex] = Sudoku::AllThreeCandidates ^ (0x1d7 << Sudoku::SizeOfCandidates);
    CPPUNIT_ASSERT_EQUAL(true, pInstance_->SearchNext(cell));
    checkCandidateCell(expected, cell);

    // 真ん中に候補のないマスがあるがこれは関係ない
    cell = emptyCell;
    regIndex = (SudokuSseMap::InitialRegisterNum + 4) * SudokuSse::RegisterWordCnt + 1;
    pInstance_->xmmRegSet_.regVal_[regIndex] = Sudoku::AllThreeCandidates ^ (Sudoku::AllCandidates << Sudoku::SizeOfCandidates);
    CPPUNIT_ASSERT_EQUAL(true, pInstance_->SearchNext(cell));
    checkCandidateCell(expected, cell);

    // 真ん中に確定したマスがあるがこれは関係ない
    cell = emptyCell;
    pInstance_->xmmRegSet_.regVal_[regIndex] = Sudoku::AllThreeCandidates ^ (0x100 << Sudoku::SizeOfCandidates);
    CPPUNIT_ASSERT_EQUAL(true, pInstance_->SearchNext(cell));
    checkCandidateCell(expected, cell);

    return;
}

void SudokuSseMapTest::test_SearchNextOld() {
    constexpr SudokuSseElement expectedRegVal[] {
        0, 0, 0, 0,
        0x7fffe03, 0x0c3ffff, 0x7ffffff, 0, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0,
        0x7ffffff, 0x7ffffff, 0x7ffffff, 0, 0x0100401, 0x0802008, 0x4010040, 0, 0x7ffffff, 0x7ffffff, 0x7fc1dff, 0,
        0x7ffffff, 0x7ffffff, 0x7ffffff, 0, 0x7ffffff, 0x7ffffff, 0x783ffff, 0, 0x7ffffff, 0x7ffffff, 0x7ff01ff, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    static_assert(sizeof(expectedRegVal) == sizeof(pInstance_->xmmRegSet_), "Unexpected pInstance_->xmmRegSet_ size");

    struct TestSet{
        SudokuSseSearchStateMember postSearchState;
    };
    constexpr TestSet testSet[] {
        {{1, 2, 0, 0, 0}}, {{2, 2, 0, 0, 0}}, {{0x10, 2, 0, 2, 1}},
        {{0x20, 2, 0, 2, 1}}, {{0x80, 2, 8, 1, 2}}, {{0x100, 2, 8, 1, 2}},
        {{2, 3, 5, 1, 2}}, {{4, 3, 5, 1, 2}}, {{8, 3, 5, 1, 2}},
        {{0x20, 4, 7, 2, 2}}, {{0x40, 4, 7, 2, 2}}, {{0x80, 4, 7, 2, 2}},
        {{0x100, 4, 7, 2, 2}}, {{1, 9, 0, 1, 0}}
    };

    SudokuSseSearchState searchState;
    auto expected = true;
    auto actual = false;

    for(const auto& test : testSet) {
        // マスクした状態SearchNextからで戻ってくるので元に戻す
        memmove(&pInstance_->xmmRegSet_, expectedRegVal, sizeof(expectedRegVal));

        actual = pInstance_->SearchNext(searchState);
        CPPUNIT_ASSERT_EQUAL(test.postSearchState.uniqueCandidate_, searchState.member_.uniqueCandidate_);
        CPPUNIT_ASSERT_EQUAL(test.postSearchState.candidateCnt_, searchState.member_.candidateCnt_);
        CPPUNIT_ASSERT_EQUAL(test.postSearchState.candidateRow_, searchState.member_.candidateRow_);
        CPPUNIT_ASSERT_EQUAL(test.postSearchState.candidateInBoxShift_, searchState.member_.candidateInBoxShift_);
        CPPUNIT_ASSERT_EQUAL(test.postSearchState.candidateOutBoxShift_, searchState.member_.candidateOutBoxShift_);
        CPPUNIT_ASSERT_EQUAL(expected, actual);
        searchState.member_ = test.postSearchState;
    }

    // 配列の要素数 + 1の場合
    // マスクした状態SearchNextからで戻ってくるので元に戻す
    memmove(&pInstance_->xmmRegSet_, expectedRegVal, sizeof(expectedRegVal));
    // これ以上は候補が9個のマスだけ
    actual = pInstance_->SearchNext(searchState);
    const uint64_t expectedCandidateCnt = Sudoku::SizeOfCandidates;
    CPPUNIT_ASSERT_EQUAL(expectedCandidateCnt, searchState.member_.candidateCnt_);
    CPPUNIT_ASSERT_EQUAL(expected, actual);

    // これ以上無い
    constexpr SudokuSseSearchStateMember lastSearchStateMember {0x100, 9, 8, 2, 2};
    SudokuSseSearchState lastSearchState;
    lastSearchState.member_ = lastSearchStateMember;
    expected = false;
    actual = pInstance_->SearchNext(lastSearchState);
    CPPUNIT_ASSERT_EQUAL(expected, actual);

    return;
}

void SudokuSseMapTest::test_SetUniqueCell() {
    constexpr size_t regIndex = 0;
    SudokuCellCandidates candidate = 1;
    SudokuSseCandidateCell cell = {regIndex, 0, Sudoku::AllCandidates, 0};

    for(SudokuIndex i=0; i < Sudoku::SizeOfCandidates; ++i) {
        pInstance_->xmmRegSet_.regVal_[regIndex] = candidate | (1 << i);
        CPPUNIT_ASSERT_EQUAL(true, pInstance_->SetUniqueCell(cell, candidate));
        CPPUNIT_ASSERT_EQUAL(candidate, pInstance_->xmmRegSet_.regVal_[regIndex]);
    }

    candidate = 0x100;
    for(SudokuIndex i=0; i < (Sudoku::SizeOfCandidates - 1); ++i) {
        SudokuSseElement value = 3 | (1 << i);
        pInstance_->xmmRegSet_.regVal_[regIndex] = value;
        CPPUNIT_ASSERT_EQUAL(false, pInstance_->SetUniqueCell(cell, candidate));
        CPPUNIT_ASSERT_EQUAL(value, pInstance_->xmmRegSet_.regVal_[regIndex]);
    }

    candidate = 0x80;
    cell = {regIndex, Sudoku::SizeOfCandidates, Sudoku::AllCandidates << Sudoku::SizeOfCandidates, 0};
    pInstance_->xmmRegSet_.regVal_[regIndex] = candidate * 3;
    CPPUNIT_ASSERT_EQUAL(false, pInstance_->SetUniqueCell(cell, candidate));
    pInstance_->xmmRegSet_.regVal_[regIndex] <<= Sudoku::SizeOfCandidates;
    CPPUNIT_ASSERT_EQUAL(true, pInstance_->SetUniqueCell(cell, candidate));
    CPPUNIT_ASSERT_EQUAL(candidate << Sudoku::SizeOfCandidates, pInstance_->xmmRegSet_.regVal_[regIndex]);
}

void SudokuSseMapTest::test_getRegisterIndex() {
    SudokuSseCandidateCell expected = {SudokuSseMap::InitialRegisterNum * SudokuSse::RegisterWordCnt + 2,
                                       Sudoku::SizeOfCandidates * 2,
                                       Sudoku::AllCandidates << (Sudoku::SizeOfCandidates * 2), 0};
    for(SudokuIndex row = 0; row < Sudoku::SizeOfGroupsPerMap; ++row) {
        for(SudokuIndex column = 0; column < Sudoku::SizeOfCellsPerGroup; ++column) {
            SudokuSseCandidateCell actual = {0, 0, 0, 0};
            pInstance_->getRegisterIndex(row, column, actual);
            checkCandidateCell(expected, actual);
            if ((column + 1) % Sudoku::SizeOfBoxesOnEdge == 0) {
                expected.regIndex -= 1;
            }
            expected.shift = (expected.shift == 0) ?
                (Sudoku::SizeOfCandidates * 2) : (expected.shift - Sudoku::SizeOfCandidates);
            expected.mask = Sudoku::AllCandidates << expected.shift;
        }
        expected.regIndex += SudokuSse::RegisterWordCnt + 3;
    }
}

void SudokuSseMapTest::test_countCandidates() {
    CPPUNIT_ASSERT_EQUAL(static_cast<SudokuIndex>(0), pInstance_->countCandidates(0));

    SudokuSseElement value = 1;
    for(SudokuIndex i=0; i < sizeof(SudokuSseElement) * 8; ++i) {
        value |= (1 << i);
        auto actual = pInstance_->countCandidates(value);
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(actual)>(i + 1), actual);
    }
}

void SudokuSseMapTest::test_countCandidatesIfMultiple() {
    const SudokuIndex expectedSet[] = {Sudoku::OutOfRangeCandidates, Sudoku::OutOfRangeCandidates,
                                       2, 3, 4, 5, 6, 7, 8, 9};
    SudokuSseElement value = 0;
    size_t i = 0;
    for(auto expected : expectedSet) {
        CPPUNIT_ASSERT_EQUAL(expected, pInstance_->countCandidatesIfMultiple(value));
        value |= 1 << i;
        ++i;
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
