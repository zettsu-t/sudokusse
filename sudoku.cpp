// Sudoku solver with SSE 4.2
// Copyright (C) 2012-2015 Zettsu Tatsuya
//
// 数独を解く
// 第一引数を何かつけると、その回数だけ実行し、経過時間以外は表示しない
// 実行回数の前に-を付けると、解く過程を示しながらその回数だけ実行する(-5なら5回)
// 実行回数を0にすると、取り得る解を数える。第二引数があればその数だけ解を列挙する。
//
// sudoku [実行回数 [列挙回数]] < 初期マップ

#include <iostream>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <ctime>
#include <limits>
#include <typeinfo>
#include <windows.h>
#include "sudoku.h"

// 定数を定義する
// const SudokuIndex SudokuMap::Group_[Sudoku::SizeOfGroupsPerCell][Sudoku::SizeOfGroupsPerMap][Sudoku::SizeOfCellsPerGroup];
// const SudokuIndex SudokuMap::ReverseGroup_[Sudoku::SizeOfAllCells][Sudoku::SizeOfGroupsPerCell];
// const SudokuCellLookUp SudokuCell::CellLookUp_[Sudoku::SizeOfLookUpCell];
#include "sudokuConstAll.h"

namespace {
    constexpr bool FastMode = FAST_MODE;  // 読みにくいが高速にする
}

extern "C" {
    void PrintPattern(void) {
        auto pInstance = SudokuSseEnumeratorMap::GetInstance();
        if (pInstance) {
            pInstance->PrintFromAsm(sudokuXmmToPrint);
        }
        return;
    }
}

namespace Sudoku {
    // あらかじめ与えられた数字が妥当であれば設定する
    template <typename SudokuNumberType>
    bool ConvertCharToSudokuCandidate(SudokuNumberType minNum, SudokuNumberType maxNum, char c, int& num) {
        char s[2] {'\0', '\0'};

        if (!::isdigit(c)) {
            return false;
        }

        s[0] = c;
        num = (::atoi(s));
        if ((num < minNum) || (num > maxNum)) {
            return false;
        }

        return true;
    }

    // マスの全候補を表示する
    template <typename SudokuElementType>
    void PrintSudokuElement(SudokuElementType candidates, SudokuElementType uniqueCandidates,
                            SudokuElementType emptyCandidates, std::ostream* pSudokuOutStream) {
        for(SudokuLoopIndex i=1;i<=Sudoku::SizeOfCandidates;++i) {
            if ((uniqueCandidates & candidates) != emptyCandidates) {
                if (pSudokuOutStream != nullptr) {
                    (*pSudokuOutStream) << i;
                }
            }
            uniqueCandidates <<= 1;
        }

        return;
    }
}

SudokuBaseSolver::SudokuBaseSolver(std::ostream* pSudokuOutStream) {
    count_ = 0;
    pSudokuOutStream_ = pSudokuOutStream;
    return;
}

void SudokuBaseSolver::printType(const std::string& typeStr, std::ostream* pSudokuOutStream) {
    if (pSudokuOutStream) {
        (*pSudokuOutStream) << "[" << typeStr << "]\n";
    }
    return;
}

// コンストラクタ
SudokuCell::SudokuCell(void) : indexNumber_(0), candidates_(SudokuAllCandidates) {
    return;
}

// デストラクタ
#ifndef NO_DESTRUCTOR
SudokuCell::~SudokuCell() {
    // 何もしないなら定義も不要
    return;
}
#endif

// あらかじめ与えられた数字があれば設定する
void SudokuCell::Preset(char c) {
    int num = 0;
    if (Sudoku::ConvertCharToSudokuCandidate(SudokuMinCandidatesNumber, SudokuMaxCandidatesNumber, c, num)) {
        // 0を指定されたらシフトしない
        candidates_ = (num == 0) ? candidates_ : (SudokuUniqueCandidates << (num - 1));
    }
    return;
}

// 通し番号を設定する
void SudokuCell::SetIndex(SudokuIndex indexNumber) {
    indexNumber_ = indexNumber;
    return;
}

// マスの候補を表示する
void SudokuCell::Print(std::ostream* pSudokuOutStream) const {
    Sudoku::PrintSudokuElement(candidates_, SudokuUniqueCandidates, SudokuEmptyCandidates, pSudokuOutStream);
    return;
}

// 通し番号を返す
INLINE SudokuIndex SudokuCell::GetIndex(void) const {
    return indexNumber_;
}

// マスが埋まっているかどうか
INLINE bool SudokuCell::IsFilled(void) const {
    return IsUniqueCandidate(candidates_);
}

// マスの候補が複数かどうか
INLINE bool SudokuCell::HasMultipleCandidates(void) const {
    return CellLookUp_[candidates_].IsMultiple;
}

// 候補が矛盾していないかどうか返す
INLINE bool SudokuCell::IsConsistent(SudokuCellCandidates candidates) const {
    if (HasMultipleCandidates() != false) {
        return true;
    }
    // 上の後に評価する方が実行速度が上がる
    if (candidates_ == SudokuEmptyCandidates) {
        return false;
    }
    return IsEmptyCandidates(candidates_ & candidates);
}

// 候補がないかどうか返す
INLINE bool SudokuCell::HasNoCandidates(void) const {
    return IsEmptyCandidates(candidates_);
}

// 候補を絞り込む
INLINE void SudokuCell::SetCandidates(SudokuCellCandidates candidates) {
    candidates_ &= candidates;
    // mutableな状態を計算しておく
    updateState();
    return;
}

// マスの候補を取得する
INLINE SudokuCellCandidates SudokuCell::GetCandidates(void) const {
    return candidates_;
}

// 候補が唯一なら取得する
INLINE SudokuCellCandidates SudokuCell::GetUniqueCandidate(void) const {
    /* インライン展開しないとなぜかリンクが通らない */
    return (IsFilled()) ? candidates_ : Sudoku::EmptyCandidates;
}

// 候補の数を数える
INLINE SudokuIndex SudokuCell::CountCandidates(void) const {
    return CellLookUp_[candidates_ & SudokuAllCandidates].NumberOfCandidates;
}

// 候補の数を数える(候補が全くないか1つなら範囲外の値を返す)
INLINE SudokuIndex SudokuCell::CountCandidatesIfMultiple(void) const {
    auto count = CellLookUp_[candidates_ & SudokuAllCandidates].NumberOfCandidates;
    return (count > Sudoku::SizeOfUniqueCandidate) ? count : Sudoku::OutOfRangeCandidates;
}

// 範囲外を含む候補の数をマスクする(候補が全くないか1つなら0を返す、候補が2つ以上ならその数を返す)
INLINE SudokuIndex SudokuCell::MaskCandidatesUnlessMultiple(SudokuIndex numberOfCandidates) {
    return (numberOfCandidates & Sudoku::OutOfRangeMask);
}

// 候補が空かどうか返す
INLINE bool SudokuCell::IsEmptyCandidates(SudokuCellCandidates candidates) {
    return (candidates == SudokuEmptyCandidates);
}

// マスの唯一の候補かどうか
INLINE bool SudokuCell::IsUniqueCandidate(SudokuCellCandidates candidates) {
    // 本当はSudokuAllCandidatesでマスクして配列を引く返すのが安全だが、速度重視
    return CellLookUp_[candidates].IsUnique;
}

// 空の候補を返す
INLINE SudokuCellCandidates SudokuCell::GetEmptyCandidates(void) {
    return SudokuEmptyCandidates;
}

// 候補にないものを返す
INLINE SudokuCellCandidates SudokuCell::FlipCandidates(SudokuCellCandidates candidates) {
    // 本当はSudokuAllCandidatesでマスクして返すのが安全だが、速度重視
    return (candidates ^ SudokuAllCandidates);
}

// 候補を統合する
INLINE SudokuCellCandidates SudokuCell::MergeCandidates(SudokuCellCandidates candidatesA, SudokuCellCandidates candidatesB) {
    return (candidatesA | candidatesB);
}

// 繰り返し探索の最初の候補を返す
INLINE SudokuCellCandidates SudokuCell::GetInitialCandidate() {
    return SudokuUniqueCandidates;
}

// 繰り返し探索の次の候補を返す(なければ空の候補を返す)
INLINE SudokuCellCandidates SudokuCell::GetNextCandidate(SudokuCellCandidates candidate) {
    return ((candidate << 1) & SudokuAllCandidates);
}

// 候補の状態を更新する
INLINE void SudokuCell::updateState(void) {
    /* 今のところすべきことはない */
    return;
}

// コンストラクタ
SudokuMap::SudokuMap(void) {
    for(SudokuIndex i=0;i<Sudoku::SizeOfAllCells;++i) {
        // 割り当ては、テーブルと同じにする
        cells_[i].SetIndex(i);
    }

    backtrackedGroup_ = 0;
    return;
}

// デストラクタ
#ifndef NO_DESTRUCTOR
SudokuMap::~SudokuMap() {
    // 何もしないなら定義も不要
    return;
}
#endif

// あらかじめ与えられた数字があれば設定する
void SudokuMap::Preset(const std::string& presetStr, SudokuIndex seed) {
    for(SudokuLoopIndex i=0;i<Sudoku::SizeOfAllCells;++i) {
        if (i < presetStr.length()) {
            cells_[i].Preset(presetStr.at(i));
        } else {
            // 短すぎる場合は空白で補う
            cells_[i].Preset(' ');
        }
    }

    // どの値がうまくいくかは状況次第
    backtrackedGroup_ = seed % Sudoku::SizeOfGroupsPerCell;
    assert(backtrackedGroup_ < Sudoku::SizeOfGroupsPerCell);
    return;
}

// マスの候補を表示する
void SudokuMap::Print(std::ostream* pSudokuOutStream) const {
    SudokuIndex cellIndex = 0;

    if (pSudokuOutStream == nullptr) {
        return;
    }

    for(SudokuLoopIndex i=0;i<Sudoku::SizeOfGroupsPerMap;++i) {
        for(SudokuLoopIndex j=0;j<Sudoku::SizeOfCellsPerGroup;++j) {
            cells_[cellIndex++].Print(pSudokuOutStream);
            (*pSudokuOutStream) << ":";
        }
        (*pSudokuOutStream) << "\n";
    }
    (*pSudokuOutStream) << "\n";

    return;
}

// 高速化
// breakがあるので、do-while(0)では囲めない
#define unrolledSudokuMapIsFilled(index) \
    { if (cells_[index].IsFilled() == false) { break; } }

// マスが埋まっているかどうか
INLINE bool SudokuMap::IsFilled(void) const {
    if (FastMode == false) {
        for(SudokuLoopIndex i=0;i<Sudoku::SizeOfAllCells;++i) {
            if(cells_[i].IsFilled() == false) {
                return false;
            }
        }
        return true;
    } else {
        for(;;) {
            // 0,0
            unrolledSudokuMapIsFilled(0);
            unrolledSudokuMapIsFilled(1);
            unrolledSudokuMapIsFilled(2);
            unrolledSudokuMapIsFilled(3);
            unrolledSudokuMapIsFilled(4);
            unrolledSudokuMapIsFilled(5);
            unrolledSudokuMapIsFilled(6);
            unrolledSudokuMapIsFilled(7);
            unrolledSudokuMapIsFilled(8);
            // 1,1
            unrolledSudokuMapIsFilled(36);
            unrolledSudokuMapIsFilled(37);
            unrolledSudokuMapIsFilled(38);
            unrolledSudokuMapIsFilled(39);
            unrolledSudokuMapIsFilled(40);
            unrolledSudokuMapIsFilled(41);
            unrolledSudokuMapIsFilled(42);
            unrolledSudokuMapIsFilled(43);
            unrolledSudokuMapIsFilled(44);
            // 2,2
            unrolledSudokuMapIsFilled(72);
            unrolledSudokuMapIsFilled(73);
            unrolledSudokuMapIsFilled(74);
            unrolledSudokuMapIsFilled(75);
            unrolledSudokuMapIsFilled(76);
            unrolledSudokuMapIsFilled(77);
            unrolledSudokuMapIsFilled(78);
            unrolledSudokuMapIsFilled(79);
            unrolledSudokuMapIsFilled(80);
            // 1,0
            unrolledSudokuMapIsFilled(9);
            unrolledSudokuMapIsFilled(10);
            unrolledSudokuMapIsFilled(11);
            unrolledSudokuMapIsFilled(12);
            unrolledSudokuMapIsFilled(13);
            unrolledSudokuMapIsFilled(14);
            unrolledSudokuMapIsFilled(15);
            unrolledSudokuMapIsFilled(16);
            unrolledSudokuMapIsFilled(17);
            // 2,0
            unrolledSudokuMapIsFilled(18);
            unrolledSudokuMapIsFilled(19);
            unrolledSudokuMapIsFilled(20);
            unrolledSudokuMapIsFilled(21);
            unrolledSudokuMapIsFilled(22);
            unrolledSudokuMapIsFilled(23);
            unrolledSudokuMapIsFilled(24);
            unrolledSudokuMapIsFilled(25);
            unrolledSudokuMapIsFilled(26);
            // 0,1
            unrolledSudokuMapIsFilled(27);
            unrolledSudokuMapIsFilled(28);
            unrolledSudokuMapIsFilled(29);
            unrolledSudokuMapIsFilled(30);
            unrolledSudokuMapIsFilled(31);
            unrolledSudokuMapIsFilled(32);
            unrolledSudokuMapIsFilled(33);
            unrolledSudokuMapIsFilled(34);
            unrolledSudokuMapIsFilled(35);
            // 2,1
            unrolledSudokuMapIsFilled(45);
            unrolledSudokuMapIsFilled(46);
            unrolledSudokuMapIsFilled(47);
            unrolledSudokuMapIsFilled(48);
            unrolledSudokuMapIsFilled(49);
            unrolledSudokuMapIsFilled(50);
            unrolledSudokuMapIsFilled(51);
            unrolledSudokuMapIsFilled(52);
            unrolledSudokuMapIsFilled(53);
            // 0,2
            unrolledSudokuMapIsFilled(54);
            unrolledSudokuMapIsFilled(55);
            unrolledSudokuMapIsFilled(56);
            unrolledSudokuMapIsFilled(57);
            unrolledSudokuMapIsFilled(58);
            unrolledSudokuMapIsFilled(59);
            unrolledSudokuMapIsFilled(60);
            unrolledSudokuMapIsFilled(61);
            unrolledSudokuMapIsFilled(62);
            // 1,2
            unrolledSudokuMapIsFilled(63);
            unrolledSudokuMapIsFilled(64);
            unrolledSudokuMapIsFilled(65);
            unrolledSudokuMapIsFilled(66);
            unrolledSudokuMapIsFilled(67);
            unrolledSudokuMapIsFilled(68);
            unrolledSudokuMapIsFilled(69);
            unrolledSudokuMapIsFilled(70);
            unrolledSudokuMapIsFilled(71);
            return true;
        }
    }

    return false;
}

// マスを埋めて矛盾があれば打ち切る
// 予想外のマクロ展開を防ぐために、do-while(0)で囲むを、単に{}としている
#define unrolledFindUnusedCandidate(index) \
    { \
        if ((cells_[index].IsFilled() == false) && (findUnusedCandidate(cells_[index]))) { \
            return true; \
        } \
    } \

// マスを埋めて矛盾があれば打ち切る
// 予想外のマクロ展開を防ぐために、do-while(0)で囲むを、単に{}としている
#define unrolledFindUniqueCandidate(index) \
    { \
        if ((cells_[index].IsFilled() == false) && (findUniqueCandidate(cells_[index]))) { \
            return true; \
        } \
    } \

// マスを埋める(INLINE化すると却って遅くなる)
bool SudokuMap::FillCrossing(void) {
    // 矛盾が発生したらtrueを返す

     // アルゴリズムをまとめて適用したほうが早い
    if (FastMode == false) {
        for(SudokuLoopIndex i=0;i<Sudoku::SizeOfAllCells;++i) {
            if (cells_[i].IsFilled()) {
                continue;
            }
            if (findUnusedCandidate(cells_[i])) {
                return true;
            }
        }

        for(SudokuLoopIndex i=0;i<Sudoku::SizeOfAllCells;++i) {
            if (cells_[i].IsFilled()) {
                continue;
            }
            if (findUniqueCandidate(cells_[i])) {
                return true;
            }
        }
    } else {
        for(;;) {
            // 0,0
            unrolledFindUnusedCandidate(0);
            unrolledFindUnusedCandidate(1);
            unrolledFindUnusedCandidate(2);
            unrolledFindUnusedCandidate(3);
            unrolledFindUnusedCandidate(4);
            unrolledFindUnusedCandidate(5);
            unrolledFindUnusedCandidate(6);
            unrolledFindUnusedCandidate(7);
            unrolledFindUnusedCandidate(8);
            // 1,1
            unrolledFindUnusedCandidate(36);
            unrolledFindUnusedCandidate(37);
            unrolledFindUnusedCandidate(38);
            unrolledFindUnusedCandidate(39);
            unrolledFindUnusedCandidate(40);
            unrolledFindUnusedCandidate(41);
            unrolledFindUnusedCandidate(42);
            unrolledFindUnusedCandidate(43);
            unrolledFindUnusedCandidate(44);
            // 2,2
            unrolledFindUnusedCandidate(72);
            unrolledFindUnusedCandidate(73);
            unrolledFindUnusedCandidate(74);
            unrolledFindUnusedCandidate(75);
            unrolledFindUnusedCandidate(76);
            unrolledFindUnusedCandidate(77);
            unrolledFindUnusedCandidate(78);
            unrolledFindUnusedCandidate(79);
            unrolledFindUnusedCandidate(80);

            // 1,0
            unrolledFindUnusedCandidate(9);
            unrolledFindUnusedCandidate(10);
            unrolledFindUnusedCandidate(11);
            unrolledFindUnusedCandidate(12);
            unrolledFindUnusedCandidate(13);
            unrolledFindUnusedCandidate(14);
            unrolledFindUnusedCandidate(15);
            unrolledFindUnusedCandidate(16);
            unrolledFindUnusedCandidate(17);
            // 2,0
            unrolledFindUnusedCandidate(18);
            unrolledFindUnusedCandidate(19);
            unrolledFindUnusedCandidate(20);
            unrolledFindUnusedCandidate(21);
            unrolledFindUnusedCandidate(22);
            unrolledFindUnusedCandidate(23);
            unrolledFindUnusedCandidate(24);
            unrolledFindUnusedCandidate(25);
            unrolledFindUnusedCandidate(26);
            // 0,1
            unrolledFindUnusedCandidate(27);
            unrolledFindUnusedCandidate(28);
            unrolledFindUnusedCandidate(29);
            unrolledFindUnusedCandidate(30);
            unrolledFindUnusedCandidate(31);
            unrolledFindUnusedCandidate(32);
            unrolledFindUnusedCandidate(33);
            unrolledFindUnusedCandidate(34);
            unrolledFindUnusedCandidate(35);

            // 2,1
            unrolledFindUnusedCandidate(45);
            unrolledFindUnusedCandidate(46);
            unrolledFindUnusedCandidate(47);
            unrolledFindUnusedCandidate(48);
            unrolledFindUnusedCandidate(49);
            unrolledFindUnusedCandidate(50);
            unrolledFindUnusedCandidate(51);
            unrolledFindUnusedCandidate(52);
            unrolledFindUnusedCandidate(53);
            // 0,2
            unrolledFindUnusedCandidate(54);
            unrolledFindUnusedCandidate(55);
            unrolledFindUnusedCandidate(56);
            unrolledFindUnusedCandidate(57);
            unrolledFindUnusedCandidate(58);
            unrolledFindUnusedCandidate(59);
            unrolledFindUnusedCandidate(60);
            unrolledFindUnusedCandidate(61);
            unrolledFindUnusedCandidate(62);
            // 1,2
            unrolledFindUnusedCandidate(63);
            unrolledFindUnusedCandidate(64);
            unrolledFindUnusedCandidate(65);
            unrolledFindUnusedCandidate(66);
            unrolledFindUnusedCandidate(67);
            unrolledFindUnusedCandidate(68);
            unrolledFindUnusedCandidate(69);
            unrolledFindUnusedCandidate(70);
            unrolledFindUnusedCandidate(71);

            // 0,0
            unrolledFindUniqueCandidate(0);
            unrolledFindUniqueCandidate(1);
            unrolledFindUniqueCandidate(2);
            unrolledFindUniqueCandidate(3);
            unrolledFindUniqueCandidate(4);
            unrolledFindUniqueCandidate(5);
            unrolledFindUniqueCandidate(6);
            unrolledFindUniqueCandidate(7);
            unrolledFindUniqueCandidate(8);
            // 1,0
            unrolledFindUniqueCandidate(9);
            unrolledFindUniqueCandidate(10);
            unrolledFindUniqueCandidate(11);
            unrolledFindUniqueCandidate(12);
            unrolledFindUniqueCandidate(13);
            unrolledFindUniqueCandidate(14);
            unrolledFindUniqueCandidate(15);
            unrolledFindUniqueCandidate(16);
            unrolledFindUniqueCandidate(17);
            // 2,0
            unrolledFindUniqueCandidate(18);
            unrolledFindUniqueCandidate(19);
            unrolledFindUniqueCandidate(20);
            unrolledFindUniqueCandidate(21);
            unrolledFindUniqueCandidate(22);
            unrolledFindUniqueCandidate(23);
            unrolledFindUniqueCandidate(24);
            unrolledFindUniqueCandidate(25);
            unrolledFindUniqueCandidate(26);

            // 0,1
            unrolledFindUniqueCandidate(27);
            unrolledFindUniqueCandidate(28);
            unrolledFindUniqueCandidate(29);
            unrolledFindUniqueCandidate(30);
            unrolledFindUniqueCandidate(31);
            unrolledFindUniqueCandidate(32);
            unrolledFindUniqueCandidate(33);
            unrolledFindUniqueCandidate(34);
            unrolledFindUniqueCandidate(35);
            // 1,1
            unrolledFindUniqueCandidate(36);
            unrolledFindUniqueCandidate(37);
            unrolledFindUniqueCandidate(38);
            unrolledFindUniqueCandidate(39);
            unrolledFindUniqueCandidate(40);
            unrolledFindUniqueCandidate(41);
            unrolledFindUniqueCandidate(42);
            unrolledFindUniqueCandidate(43);
            unrolledFindUniqueCandidate(44);
            // 2,1
            unrolledFindUniqueCandidate(45);
            unrolledFindUniqueCandidate(46);
            unrolledFindUniqueCandidate(47);
            unrolledFindUniqueCandidate(48);
            unrolledFindUniqueCandidate(49);
            unrolledFindUniqueCandidate(50);
            unrolledFindUniqueCandidate(51);
            unrolledFindUniqueCandidate(52);
            unrolledFindUniqueCandidate(53);

            // 0,2
            unrolledFindUniqueCandidate(54);
            unrolledFindUniqueCandidate(55);
            unrolledFindUniqueCandidate(56);
            unrolledFindUniqueCandidate(57);
            unrolledFindUniqueCandidate(58);
            unrolledFindUniqueCandidate(59);
            unrolledFindUniqueCandidate(60);
            unrolledFindUniqueCandidate(61);
            unrolledFindUniqueCandidate(62);
            // 1,2
            unrolledFindUniqueCandidate(63);
            unrolledFindUniqueCandidate(64);
            unrolledFindUniqueCandidate(65);
            unrolledFindUniqueCandidate(66);
            unrolledFindUniqueCandidate(67);
            unrolledFindUniqueCandidate(68);
            unrolledFindUniqueCandidate(69);
            unrolledFindUniqueCandidate(70);
            unrolledFindUniqueCandidate(71);
            // 2,2
            unrolledFindUniqueCandidate(72);
            unrolledFindUniqueCandidate(73);
            unrolledFindUniqueCandidate(74);
            unrolledFindUniqueCandidate(75);
            unrolledFindUniqueCandidate(76);
            unrolledFindUniqueCandidate(77);
            unrolledFindUniqueCandidate(78);
            unrolledFindUniqueCandidate(79);
            unrolledFindUniqueCandidate(80);
            break;
        }
    }

    return false;
}

// マスの候補を強制的に一つに絞る
INLINE bool SudokuMap::SetUniqueCell(SudokuIndex cellIndex, SudokuCellCandidates candidate) {
    auto& cell = cells_[cellIndex];
    cell.SetCandidates(candidate);
    return cell.IsFilled();
}

// 高速化
template <SudokuIndex index>
INLINE SudokuIndex SudokuMap::unrolledCountFilledCells(SudokuIndex accumCount) const {
    return cells_[index].IsFilled() + unrolledCountFilledCells<index-1>(accumCount);
}

template <>
INLINE SudokuIndex SudokuMap::unrolledCountFilledCells<0>(SudokuIndex accumCount) const {
    return cells_[0].IsFilled();
}

// 埋まっているマスを数える
INLINE SudokuIndex SudokuMap::CountFilledCells(void) const {
    SudokuIndex count = 0;

    if (FastMode == false) {
        for(SudokuLoopIndex i=0;i<Sudoku::SizeOfAllCells;++i) {
            if (cells_[i].IsFilled()) {
                ++count;
            }
        }
    } else {
        count = unrolledCountFilledCells<80>(count);
    }

    return count;
}

// 高速化
INLINE SudokuIndex SudokuMap::unrolledSelectBacktrakedCellIndexInnerCommon(SudokuIndex outerIndex, SudokuIndex innerIndex,
                                                                           SudokuIndex& leastCountOfGroup,
                                                                           SudokuIndex& candidateCellIndex) const {
    const auto cellIndex = Group_[backtrackedGroup_][outerIndex][innerIndex];
    // 条件分岐を削除するために、範囲外の値を取得できる
    const auto countOrOutOfRange = cells_[cellIndex].CountCandidatesIfMultiple();
    assert(countOrOutOfRange > Sudoku::SizeOfUniqueCandidate);
    const auto count = SudokuCell::MaskCandidatesUnlessMultiple(countOrOutOfRange);
    assert(count <= Sudoku::SizeOfCandidates);

    // 候補がなければcountはマスの数より大きい
    if (leastCountOfGroup > countOrOutOfRange) {
        leastCountOfGroup = count;
        candidateCellIndex = cellIndex;
    }
    return count;
}

template <SudokuIndex innerIndex>
INLINE SudokuIndex SudokuMap::unrolledSelectBacktrakedCellIndexInner(SudokuIndex outerIndex, SudokuIndex& leastCountOfGroup,
                                                                     SudokuIndex& candidateCellIndex) const {
    return unrolledSelectBacktrakedCellIndexInnerCommon(outerIndex, innerIndex, leastCountOfGroup, candidateCellIndex)
        + unrolledSelectBacktrakedCellIndexInner<innerIndex-1>(outerIndex, leastCountOfGroup, candidateCellIndex);
}

template <>
INLINE SudokuIndex SudokuMap::unrolledSelectBacktrakedCellIndexInner<0>(SudokuIndex outerIndex, SudokuIndex& leastCountOfGroup,
                                                                        SudokuIndex& candidateCellIndex) const {
    return unrolledSelectBacktrakedCellIndexInnerCommon(outerIndex, 0, leastCountOfGroup, candidateCellIndex);
}

// バックトラッキング候補のマスを選ぶ
INLINE SudokuIndex SudokuMap::SelectBacktrakedCellIndex(void) const {
    SudokuIndex leastGroupCount = Sudoku::SizeOfCandidates * Sudoku::SizeOfCellsPerGroup;
    SudokuIndex leastCount = Sudoku::SizeOfCandidates;
    SudokuIndex resultCellIndex = 0;

    // 外ループをアンローリングすると却って遅くなる
    for(SudokuLoopIndex i=0;i<Sudoku::SizeOfGroupsPerMap;++i) {
        SudokuIndex groupCount = 0;
        SudokuIndex leastCountOfGroup = Sudoku::SizeOfCandidates;
        SudokuIndex candidateCellIndex = 0;
        if (FastMode == false) {
            for(SudokuLoopIndex j=0;j<Sudoku::SizeOfCellsPerGroup;++j) {
                // 埋まっていないマスで最小の候補を選ぶ
                const SudokuIndex cellIndex = Group_[backtrackedGroup_][i][j];
                const SudokuIndex count = cells_[cellIndex].CountCandidates();
                groupCount += count;
                if ((leastCountOfGroup > count) && (count > 1)) {
                    candidateCellIndex = cellIndex;
                    leastCountOfGroup = count;
                }
            }
        } else {
            groupCount = unrolledSelectBacktrakedCellIndexInner<Sudoku::SizeOfCellsPerGroup-1>
                (i, leastCountOfGroup, candidateCellIndex);
        }
        // 候補の最小のマス、候補の最小のグループの順で選ぶ
        if ((leastCount > leastCountOfGroup) ||
            ((leastCount == leastCountOfGroup) && (leastGroupCount > groupCount))) {
            leastCount = leastCountOfGroup;
            resultCellIndex = candidateCellIndex;
        }
        if (leastGroupCount > groupCount) {
            leastGroupCount = groupCount;
        }
    }

    return resultCellIndex;
}

// 候補が矛盾していないかどうか返す(解きながら調べるのでこの関数は呼ばれない)
bool SudokuMap::IsConsistent(void) const {
    for(SudokuLoopIndex i=0;i<Sudoku::SizeOfGroupsPerCell;++i) {
        for(SudokuLoopIndex j=0;j<Sudoku::SizeOfGroupsPerMap;++j) {
            auto allCandidates = SudokuCell::GetEmptyCandidates();
            for(SudokuLoopIndex k=0;k<Sudoku::SizeOfCellsPerGroup;++k) {
                const SudokuIndex cellIndex = Group_[i][j][k];
                const auto& cell = cells_[cellIndex];
                if (cell.IsConsistent(allCandidates) == false) {
                    return false;
                }
                // 埋まっているマスと重なっているかどうか
                const auto candidate = cell.GetUniqueCandidate();
                allCandidates = cell.MergeCandidates(allCandidates, candidate);
            }
        }
    }

    return true;
}

// 高速化
INLINE SudokuCellCandidates SudokuMap::unrolledFindUnusedCandidateInnerCommon(SudokuIndex targetCellIndex, SudokuIndex outerIndex,
                                                                              SudokuIndex groupIndex, SudokuIndex innerIndex,
                                                                              SudokuCellCandidates candidates) const {
    auto newCandidates = candidates;
    const auto cellIndex = Group_[outerIndex][groupIndex][innerIndex];
    if (cellIndex != targetCellIndex) {
        const auto cellCandidates = cells_[cellIndex].GetUniqueCandidate();
        newCandidates = SudokuCell::MergeCandidates(candidates, cellCandidates);
    }
    return newCandidates;
}

template <SudokuIndex innerIndex>
INLINE SudokuCellCandidates SudokuMap::unrolledFindUnusedCandidateInner(SudokuIndex targetCellIndex, SudokuIndex outerIndex,
                                                                        SudokuIndex groupIndex, SudokuCellCandidates candidates) const {
    const auto newCandidates =
        unrolledFindUnusedCandidateInnerCommon(targetCellIndex, outerIndex, groupIndex, innerIndex, candidates);
    return unrolledFindUnusedCandidateInner<innerIndex-1>(targetCellIndex, outerIndex, groupIndex, newCandidates);
}

template <>
INLINE SudokuCellCandidates SudokuMap::unrolledFindUnusedCandidateInner<0>(SudokuIndex targetCellIndex, SudokuIndex outerIndex,
                                                                           SudokuIndex groupIndex, SudokuCellCandidates candidates) const {
    return unrolledFindUnusedCandidateInnerCommon(targetCellIndex, outerIndex, groupIndex, 0, candidates);
}

INLINE SudokuCellCandidates SudokuMap::unrolledFindUnusedCandidateOuterCommon(SudokuIndex targetCellIndex, SudokuIndex outerIndex,
                                                                              SudokuCellCandidates candidates) const {
    const auto groupIndex = ReverseGroup_[targetCellIndex][outerIndex];
    return unrolledFindUnusedCandidateInner<Sudoku::SizeOfCellsPerGroup-1>
        (targetCellIndex, outerIndex, groupIndex, candidates);
}

template <SudokuIndex outerIndex>
INLINE SudokuCellCandidates SudokuMap::unrolledFindUnusedCandidateOuter(SudokuIndex targetCellIndex,
                                                                        SudokuCellCandidates candidates) const {
    const auto newCandidates = unrolledFindUnusedCandidateOuterCommon(targetCellIndex, outerIndex, candidates);
    return unrolledFindUnusedCandidateOuter<outerIndex-1>(targetCellIndex, newCandidates);
}

template <>
INLINE SudokuCellCandidates SudokuMap::unrolledFindUnusedCandidateOuter<0>(SudokuIndex targetCellIndex,
                                                                           SudokuCellCandidates candidates) const {
    return unrolledFindUnusedCandidateOuterCommon(targetCellIndex, 0, candidates);
}

// 十字方向に検索して、使われていない数字が一つに絞れたら設定する
bool SudokuMap::findUnusedCandidate(SudokuCell& targetCell) const {
    // 矛盾が発生したらtrueを返す
    auto candidates = SudokuCell::GetEmptyCandidates();
    const auto targetCellIndex = targetCell.GetIndex();

    if (FastMode == false) {
        for(SudokuLoopIndex i=0;i<Sudoku::SizeOfGroupsPerCell;++i) {
            const auto groupIndex = ReverseGroup_[targetCellIndex][i];
            for(SudokuLoopIndex j=0;j<Sudoku::SizeOfCellsPerGroup;++j) {
                const auto cellIndex = Group_[i][groupIndex][j];
                if (cellIndex != targetCellIndex) {
                    const auto cellCandidates = cells_[cellIndex].GetUniqueCandidate();
                    candidates = SudokuCell::MergeCandidates(candidates, cellCandidates);
                }
            }
        }
    } else {
        candidates = unrolledFindUnusedCandidateOuter<Sudoku::SizeOfGroupsPerCell-1>(targetCellIndex, candidates);
    }

    candidates = SudokuCell::FlipCandidates(candidates);
    targetCell.SetCandidates(candidates);
    return cells_[targetCellIndex].HasNoCandidates();
}

INLINE SudokuCellCandidates SudokuMap::unrolledFindUniqueCandidateInnerCommon(SudokuIndex targetCellIndex, SudokuIndex outerIndex,
                                                                              SudokuIndex groupIndex, SudokuIndex innerIndex,
                                                                              SudokuCellCandidates candidates) const {
    auto newCandidates = candidates;
    const auto cellIndex = Group_[outerIndex][groupIndex][innerIndex];
    if (cellIndex != targetCellIndex) {
        newCandidates = SudokuCell::MergeCandidates(candidates, cells_[cellIndex].GetCandidates());
    }
    return newCandidates;
}

template <SudokuIndex innerIndex>
INLINE SudokuCellCandidates SudokuMap::unrolledFindUniqueCandidateInner(SudokuIndex targetCellIndex, SudokuIndex outerIndex,
                                                                        SudokuIndex groupIndex, SudokuCellCandidates candidates) const {
    const auto newCandidates =
        unrolledFindUniqueCandidateInnerCommon(targetCellIndex, outerIndex, groupIndex, innerIndex, candidates);
    return unrolledFindUniqueCandidateInner<innerIndex-1>(targetCellIndex, outerIndex, groupIndex, newCandidates);
}

template <>
INLINE SudokuCellCandidates SudokuMap::unrolledFindUniqueCandidateInner<0>(SudokuIndex targetCellIndex, SudokuIndex outerIndex,
                                                                           SudokuIndex groupIndex, SudokuCellCandidates candidates) const {
    return unrolledFindUniqueCandidateInnerCommon(targetCellIndex, outerIndex, groupIndex, 0, candidates);
}

// 予想外のマクロ展開を防ぐために、do-while(0)で囲むのと同じ
#define findUniqueCandidateOuterLoop(index) { \
    { \
        auto candidates = SudokuCell::GetEmptyCandidates(); \
        const auto groupIndex = ReverseGroup_[targetCellIndex][index]; \
        candidates = unrolledFindUniqueCandidateInner<Sudoku::SizeOfCellsPerGroup-1> \
            (targetCellIndex, index, groupIndex, candidates); \
        candidates = SudokuCell::FlipCandidates(candidates); \
        if (SudokuCell::IsUniqueCandidate(candidates) != false) { \
            targetCell.SetCandidates(candidates); \
            return cells_[targetCellIndex].HasNoCandidates(); \
        } \
        if (SudokuCell::IsEmptyCandidates(candidates) == false) { \
            return true; \
        } \
    } \
}

// このマスにしか候補になりえない数字があれば設定する
bool SudokuMap::findUniqueCandidate(SudokuCell& targetCell) const {
    // 矛盾が発生したらtrueを返す
    const auto targetCellIndex = targetCell.GetIndex();

    if (FastMode == false) {
        for(SudokuLoopIndex i=0;i<Sudoku::SizeOfGroupsPerCell;++i) {
            auto candidates = SudokuCell::GetEmptyCandidates();
            const auto groupIndex = ReverseGroup_[targetCellIndex][i];
            if (FastMode == false) {
                for(SudokuLoopIndex j=0;j<Sudoku::SizeOfCellsPerGroup;++j) {
                    const auto cellIndex = Group_[i][groupIndex][j];
                    if (cellIndex != targetCellIndex) {
                        candidates = SudokuCell::MergeCandidates(candidates, cells_[cellIndex].GetCandidates());
                    }
                }
            }
            candidates = SudokuCell::FlipCandidates(candidates);
            if (SudokuCell::IsUniqueCandidate(candidates) != false) {
                targetCell.SetCandidates(candidates);
                return cells_[targetCellIndex].HasNoCandidates();
            }
            if (SudokuCell::IsEmptyCandidates(candidates) != false) {
                continue;
            }
            return true;
        }
    } else {
        findUniqueCandidateOuterLoop(0);
        findUniqueCandidateOuterLoop(1);
        findUniqueCandidateOuterLoop(2); // SizeOfGroupPerCell - 1
    }

    return false;
}

SudokuSseEnumeratorMap* SudokuSseEnumeratorMap::pInstance_ = nullptr;

SudokuSseEnumeratorMap::SudokuSseEnumeratorMap(std::ostream* pSudokuOutStream)
    : rightBottomElement_(0), firstCell_(0), patternNumber_(0), pSudokuOutStream_(pSudokuOutStream) {
    ::memset(&xmmRegSet_, 0, sizeof(xmmRegSet_));
    pInstance_ = this;
    sudokuXmmPrintAllCandidate = 0;
    return;
}

SudokuSseEnumeratorMap::~SudokuSseEnumeratorMap() {
    pInstance_ = nullptr;
    return;
}

void SudokuSseEnumeratorMap::SetToPrint(SudokuPatternCount printAllCadidate) {
    sudokuXmmPrintAllCandidate = printAllCadidate;
    return;
}

void SudokuSseEnumeratorMap::Preset(const std::string& presetStr) {
    ::memset(&xmmRegSet_, 0, sizeof(xmmRegSet_));
    auto foundEmpty = false;
    for(SudokuLoopIndex i=0;i<Sudoku::SizeOfAllCells;++i) {
        if (i >= presetStr.length()) {
            break;
        }
        int num = 0;
        if (!Sudoku::ConvertCharToSudokuCandidate(Sudoku::MinCandidatesNumber,
                                                  Sudoku::MaxCandidatesNumber, presetStr[i], num)) {
            foundEmpty = true;
            continue;
        }
        presetCell(i, num);
        if (!foundEmpty) {
            firstCell_ = i + 1;
        }
    }
    return;
}

void SudokuSseEnumeratorMap::Print(void) const {
    SudokuSseEnumeratorMap::Print(false, xmmRegSet_);
    return;
}

void SudokuSseEnumeratorMap::Print(bool solved, const XmmRegisterSet& xmmRegSet) const {
    for(size_t row=0; row<Sudoku::SizeOfGroupsPerMap; ++row) {
        size_t pos = (InitialRegisterNum + row) * (sizeof(xmmRegSet.regXmmVal_[0]) / sizeof(xmmRegSet.regVal_[0]));
        for(size_t column=0; column<(Sudoku::SizeOfGroupsPerMap - 1); column+=2) {
            const SudokuSseElement regValue = xmmRegSet.regVal_[pos];
            const SudokuSseElement regLowValue = regValue & ((1 << CellBitWidth) - 1);
            const SudokuSseElement regHighValue = (regValue >> CellBitWidth) & ((1 << CellBitWidth) - 1);
            (*pSudokuOutStream_) << powerOfTwoPlusOne(regLowValue) << ":" << powerOfTwoPlusOne(regHighValue) << ":";
            ++pos;
        }

        SudokuSseElement regValue = 0;
        // 以下の代入ではbit数が減るが、値を取りうる範囲から問題ない
        static_assert(CellBitWidth <= std::numeric_limits<decltype(regValue)>::digits, "regValue too small");

        if ((row + 1) == Sudoku::SizeOfGroupsPerMap) {
            const uint64_t value = (solved) ? sudokuXmmRightBottomSolved : rightBottomElement_;
            regValue = value & ((1 << CellBitWidth) - 1);
        } else {
            const size_t regIndex = RightColumnRegisterNum * (sizeof(xmmRegSet.regXmmVal_[0]) / sizeof(xmmRegSet.regVal_[0]));
            regValue = xmmRegSet.regVal_[regIndex + row / 2];
            regValue = (row % 2) ? (regValue >> CellBitWidth) : regValue;
            regValue &= (1 << CellBitWidth) - 1;
        }

        (*pSudokuOutStream_) << powerOfTwoPlusOne(regValue) << "\n";
    }

    return;
}

void SudokuSseEnumeratorMap::PrintFromAsm(const XmmRegisterSet& xmmRegSet) {
    if (patternNumber_ >= sudokuXmmPrintAllCandidate) {
        return;
    }

    ++patternNumber_;
    /* レジスタはasm側で保存する */
    pSudokuOutStream_->flush();
    (*pSudokuOutStream_) << "[Pattern " << patternNumber_ << "]\n";
    Print(true, xmmRegSet);
    return;
}

SudokuPatternCount SudokuSseEnumeratorMap::Enumerate(void) {
    patternNumber_ = 0;
    Print();

    sudokuXmmRightBottomElement = rightBottomElement_;

    static_assert((alignof(xmmRegSet_) % alignof(xmmRegister)) == 0, "Unexpected xmmRegSet_ alignment");
    Sudoku::LoadXmmRegistersFromMem(reinterpret_cast<const xmmRegister *>(xmmRegSet_.regVal_));

    asm volatile (
        "call sudokuXmmCountFromCell\n\t"
        ::"a"(firstCell_):"rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
    Sudoku::SaveXmmRegistersToMem(reinterpret_cast<xmmRegister *>(xmmRegSet_.regVal_));

    rightBottomElement_ = static_cast<decltype(rightBottomElement_)>(sudokuXmmRightBottomElement);
    return sudokuXmmAllPatternCnt;
}

SudokuSseEnumeratorMap* SudokuSseEnumeratorMap::GetInstance(void) {
    return pInstance_;
}

size_t SudokuSseEnumeratorMap::powerOfTwoPlusOne(SudokuSseElement regValue) const {
    decltype(regValue) mask = 1;
    for(size_t i=1; i<=Sudoku::SizeOfCandidates; ++i) {
        if (regValue & mask) {
            return i;
        }
        mask <<= 1;
    }
    return 0;
}

void SudokuSseEnumeratorMap::presetCell(SudokuLoopIndex index, int num) {
    SudokuSseElement candidate = static_cast<decltype(candidate)>(num);

    if ((candidate == 0) || (index >= Sudoku::SizeOfAllCells)) {
        return;
    }

    if ((index + 1) == Sudoku::SizeOfAllCells) {
        rightBottomElement_ = 1 << (candidate - 1);
        return;
    }

    size_t pos = 0;
    size_t shift = 0;
    if ((index % Sudoku::SizeOfCellsPerGroup) + 1 == Sudoku::SizeOfCellsPerGroup) {
        pos = index / Sudoku::SizeOfCellsPerGroup;
        shift = pos % 2;
        pos /= 2;
        pos += (RightColumnRegisterNum * (sizeof(xmmRegSet_.regXmmVal_[0]) / sizeof(xmmRegSet_.regVal_[0])));
    } else {
        pos += index % Sudoku::SizeOfCellsPerGroup;
        shift = pos % 2;
        pos /= 2;
        pos += (InitialRegisterNum + index / Sudoku::SizeOfCellsPerGroup)
            * (sizeof(xmmRegSet_.regXmmVal_[0]) / sizeof(xmmRegSet_.regVal_[0]));
    }

    xmmRegSet_.regVal_[pos] |= (1 << (shift * CellBitWidth + candidate - 1));
    return;
}

// コンストラクタ
SudokuSolver::SudokuSolver(const std::string& presetStr, SudokuIndex seed, std::ostream* pSudokuOutStream)
    : SudokuBaseSolver(pSudokuOutStream) {
    // 実はseedは0固定の方が早い
    map_.Preset(presetStr, 0);
    return;
}

SudokuSolver::SudokuSolver(const std::string& presetStr, SudokuIndex seed, std::ostream* pSudokuOutStream, SudokuPatternCount printAllCadidate)
    : SudokuBaseSolver(pSudokuOutStream) {
    // printAllCadidate はSSE版との互換
    map_.Preset(presetStr, 0);
    return;
}

// デストラクタ
SudokuSolver::~SudokuSolver() {
    // 何もしないなら定義も不要
    return;
}

// 解いて結果を出力する
bool SudokuSolver::Exec(bool silent, bool verbose) {
    auto result = solve(map_, true, verbose);
    if (silent == false) {
        map_.Print(pSudokuOutStream_);
    }
    return result;
}

// 種類を表示する
void SudokuSolver::PrintType(void) {
    SudokuBaseSolver::printType("C++", pSudokuOutStream_);
    return;
}

// 解く
// topLevelはSSE版との互換
bool SudokuSolver::solve(SudokuMap& map, bool topLevel, bool verbose) {
    auto oldCount = map.CountFilledCells();

    for(;;) {
        if (fillCells(map, topLevel, verbose) == false) {
            return false;
        }
        if (verbose) {
            map.Print(pSudokuOutStream_);
        }
        if (map.IsFilled()) {
            // 解けた
            return true;
        }
        const auto newCount = map.CountFilledCells();
        if (oldCount == newCount) {
            break;
        }
        oldCount = newCount;
    }

    // 減らないので候補を決め打ちしてバックトラッキングする
    const auto cellIndex = map.SelectBacktrakedCellIndex();
    // 1から順に試す
    auto candidate = SudokuCell::GetInitialCandidate();
    for(;;) {
        // 単に値をコピーして新たな状態を作る
        auto newMap = map;

        const auto result = newMap.SetUniqueCell(cellIndex, candidate);
        // 候補を矛盾なく設定できたら解く
        if (result) {
            if (solve(newMap, false, verbose)) {
                // 単に値をコピーして状態を書き戻す
                map = newMap;
                // 解けた
                return true;
            }
        }

        // 次の候補がまだあれば試す
        candidate = SudokuCell::GetNextCandidate(candidate);
        if (SudokuCell::IsEmptyCandidates(candidate) != false) {
            break;
        }
    }

    return false;
}

// topLevelはSSE版との互換
bool SudokuSolver::fillCells(SudokuMap& map, bool topLevel, bool verbose) {
    ++count_;
    if (verbose) {
        if (pSudokuOutStream_ != nullptr) {
            (*pSudokuOutStream_) << "Step " << count_ << "\n";
        }
    }
    return (map.FillCrossing() == false);
}

/* --------------- SSE 4.2版 --------------- */

// バックトラッキング候補の検索を開始する
SudokuSseSearchState::SudokuSseSearchState() {
    member_.uniqueCandidate_ = 0;
    member_.candidateCnt_ = 0;
    member_.candidateRow_ = 0;
    member_.candidateInBoxShift_ = 0;
    member_.candidateOutBoxShift_ = 0;
    return;
}

// デストラクタ
#ifndef NO_DESTRUCTOR
SudokuSseSearchState::~SudokuSseSearchState() {
    // 何もしないなら定義も不要
    return;
}
#endif

void SudokuSseSearchState::Print(std::ostream* pSudokuOutStream) const {
    if (pSudokuOutStream == nullptr) {
        return;
    }

    (*pSudokuOutStream) << "fill unique candidate " << member_.uniqueCandidate_ << ", cnt ";
    (*pSudokuOutStream) << member_.candidateCnt_ << ", row "  << member_.candidateRow_ << ", in " ;
    (*pSudokuOutStream) << member_.candidateInBoxShift_ << ", out "  << member_.candidateOutBoxShift_ << "\n";
    return;
}

// コンストラクタ
SudokuSseCell::SudokuSseCell(void) {
    candidates_ = AllCandidates;
    return;
}

// デストラクタ
#ifndef NO_DESTRUCTOR
SudokuSseCell::~SudokuSseCell() {
    // 何もしないなら定義も不要
    return;
}
#endif

void SudokuSseCell::Preset(char c) {
    int num = 0;
    if (Sudoku::ConvertCharToSudokuCandidate(SudokuMinCandidatesNumber, SudokuMaxCandidatesNumber, c, num)) {
        candidates_ = SudokuUniqueCandidates << (num - 1);
    }
    return;
}

void SudokuSseCell::SetCandidates(SudokuSseElement candidates) {
    candidates_ = candidates;
    return;
}

SudokuSseElement SudokuSseCell::GetCandidates(void) {
    return candidates_;
}

void SudokuSseCell::Print(std::ostream* pSudokuOutStream) const {
    Sudoku::PrintSudokuElement(candidates_, SudokuUniqueCandidates, SudokuEmptyCandidates, pSudokuOutStream);
    return;
}

// コンストラクタ
SudokuSseMap::SudokuSseMap(void) {
    memset(&xmmRegSet_, 0, sizeof(xmmRegSet_));
    size_t index = InitialRegisterNum * SudokuSse::RegisterWordCnt;

    // 未確定のマスはすべての候補を設定する
    for(size_t i=0; i<Sudoku::SizeOfGroupsPerMap; ++i) {
        for(size_t j=0; j<(SudokuSse::RegisterWordCnt-1); ++j) {
            xmmRegSet_.regVal_[index++] = (SudokuSseCell::AllCandidates << (Sudoku::SizeOfCandidates * 2)) |
                (SudokuSseCell::AllCandidates << Sudoku::SizeOfCandidates) | SudokuSseCell::AllCandidates;
        }
        xmmRegSet_.regVal_[index++] = 0;
    }

    assert(index <= (sizeof(xmmRegSet_.regVal_)/sizeof(xmmRegSet_.regVal_[0])));
    return;
}

// デストラクタ
#ifndef NO_DESTRUCTOR
SudokuSseMap::~SudokuSseMap() {
    // 何もしないなら定義も不要
    return;
}
#endif

// あらかじめ与えられた数字があれば設定する
void SudokuSseMap::Preset(const std::string& presetStr) {
    size_t strIndex = 0;
    size_t index = InitialRegisterNum * SudokuSse::RegisterWordCnt + 2;

    for(size_t i=0; i<Sudoku::SizeOfGroupsPerMap; ++i) {
        for(size_t j=0; j<Sudoku::SizeOfBoxesOnEdge; ++j) {
            SudokuSseElement regVal = 0;
            for(size_t k=0; k<Sudoku::SizeOfCellsOnBoxEdge; ++k) {
                SudokuSseCell cell;
                SudokuSseElement oneElementVal = 0;
                if (strIndex < presetStr.length()) {
                    cell.Preset(presetStr.at(strIndex++));
                    oneElementVal = cell.GetCandidates();
                }
                oneElementVal = (oneElementVal) ? oneElementVal : SudokuSseCell::AllCandidates;
                regVal = (regVal << Sudoku::SizeOfCandidates) | oneElementVal;
            }
            xmmRegSet_.regVal_[index--] = regVal;
        }
        index += 7;
    }

    assert(index <= (sizeof(xmmRegSet_.regVal_)/sizeof(xmmRegSet_.regVal_[0])));
    return;
}

// 解く
void SudokuSseMap::FillCrossing(bool loadXmm) {
    Sudoku::LoadXmmRegistersFromMem(xmmRegSet_.regXmmVal_);

    // 使うレジスタはすべて記述する(Cygwinではレジスタが無いとエラーが出る)
    asm volatile (
        "call solveSudokuAsm\n\t"
        :::"rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");

    Sudoku::SaveXmmRegistersToMem(xmmRegSet_.regXmmVal_);
    return;
}

// マスの候補を表示する
void SudokuSseMap::Print(std::ostream* pSudokuOutStream) const {
    size_t index = InitialRegisterNum * SudokuSse::RegisterWordCnt + 2;

    if (pSudokuOutStream == nullptr) {
        return;
    }

    for(size_t i=0; i<Sudoku::SizeOfGroupsPerMap; ++i) {
        for(size_t j=0; j<Sudoku::SizeOfBoxesOnEdge; ++j) {
            const SudokuSseElement regVal = xmmRegSet_.regVal_[index--];
            SudokuSseElement valSet[3];
            valSet[0] = regVal >> (Sudoku::SizeOfCandidates * 2);
            valSet[1] = (regVal >> Sudoku::SizeOfCandidates) & SudokuSseCell::AllCandidates;
            valSet[2] = regVal & SudokuSseCell::AllCandidates;
            for(size_t k=0; k<Sudoku::SizeOfCellsOnBoxEdge; ++k) {
                SudokuSseCell cell;
                cell.SetCandidates(valSet[k]);
                cell.Print(pSudokuOutStream);
                (*pSudokuOutStream) << ":";
            }
        }
        index += 7;
        (*pSudokuOutStream) << "\n";
    }
    (*pSudokuOutStream) << "\n";

    assert(index <= (sizeof(xmmRegSet_.regVal_)/sizeof(xmmRegSet_.regVal_[0])));
    return;
}

// 次のバックトラッキング候補を見つける
bool SudokuSseMap::SearchNext(SudokuSseSearchState& searchState) {
    sudokuXmmUniqueCandidate = searchState.member_.uniqueCandidate_;
    sudokuXmmCandidateCnt = searchState.member_.candidateCnt_;
    sudokuXmmCandidateRow = searchState.member_.candidateRow_;
    sudokuXmmCandidateInBoxShift = searchState.member_.candidateInBoxShift_;
    sudokuXmmCandidateOutBoxShift = searchState.member_.candidateOutBoxShift_;

    // cout<<などを呼び出すとXMMレジスタの状態が変わるので必ずロードする
    Sudoku::LoadXmmRegistersFromMem(xmmRegSet_.regXmmVal_);

    // 使うレジスタはすべて記述する(Cygwinではレジスタが無いとエラーが出る)
    asm volatile (
        "call searchNextCandidate\n\t"
        :::"rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");

    Sudoku::SaveXmmRegistersToMem(xmmRegSet_.regXmmVal_);

    searchState.member_.uniqueCandidate_ = sudokuXmmUniqueCandidate;
    searchState.member_.candidateCnt_ = sudokuXmmCandidateCnt;
    searchState.member_.candidateRow_ = sudokuXmmCandidateRow;
    searchState.member_.candidateInBoxShift_ = sudokuXmmCandidateInBoxShift;
    searchState.member_.candidateOutBoxShift_ = sudokuXmmCandidateOutBoxShift;

    return (searchState.member_.candidateCnt_ <= Sudoku::SizeOfCandidates);
}

// コンストラクタ
SudokuSseSolver::SudokuSseSolver(const std::string& presetStr, std::ostream* pSudokuOutStream,
                                 SudokuPatternCount printAllCadidate)
    : SudokuBaseSolver(pSudokuOutStream), enumeratorMap_(pSudokuOutStream), printAllCadidate_(printAllCadidate) {
    initialize(presetStr, pSudokuOutStream);
    return;
}

// seedはSSE版との互換
SudokuSseSolver::SudokuSseSolver(const std::string& presetStr, SudokuIndex seed,
                                 std::ostream* pSudokuOutStream, SudokuPatternCount printAllCadidate)
    : SudokuBaseSolver(pSudokuOutStream), enumeratorMap_(pSudokuOutStream), printAllCadidate_(printAllCadidate) {
    initialize(presetStr, pSudokuOutStream);
    return;
}

void SudokuSseSolver::initialize(const std::string& presetStr, std::ostream* pSudokuOutStream) {
    map_.Preset(presetStr);
    enumeratorMap_.Preset(presetStr);
    return;
}

// デストラクタ
SudokuSseSolver::~SudokuSseSolver() {
    // 何もしないなら定義も不要
    return;
}

bool SudokuSseSolver::Exec(bool silent, bool verbose) {
    const auto result = solve(map_, true, verbose);
    if (silent == false) {
        map_.Print(pSudokuOutStream_);
    }
    return result;
}

// 解を数える
void SudokuSseSolver::Enumerate(void) {
    if (printAllCadidate_) {
        enumeratorMap_.SetToPrint(printAllCadidate_);
    }
    const auto result = enumeratorMap_.Enumerate();
    (*pSudokuOutStream_) << "Number of pattern : " << result << "\n";
    return;
}

// 種類を表示する
void SudokuSseSolver::PrintType(void) {
    SudokuBaseSolver::printType("SSE 4.2", pSudokuOutStream_);
    return;
}

// 解く
bool SudokuSseSolver::solve(SudokuSseMap& map, bool topLevel, bool verbose) {
    for(;;) {
        // 解けるところまで解く
        fillCells(map, topLevel, verbose);
        // バックトラッキング中に矛盾を発見したのでバックトラッキングをやり直す
        if (sudokuXmmAborted) {
            return false;
        }

        // 全マス埋めた
        if (sudokuXmmElementCnt == Sudoku::SizeOfAllCells) {
            return true;
        }

        SudokuSseSearchState searchState;
        for(;;) {
            SudokuSseMap newMap = map;
            // 候補を一つ見つけて決め打ちする
            if (!newMap.SearchNext(searchState)) {
                // これ以上候補がない
                break;
            }
            if (verbose) {
                map.Print(pSudokuOutStream_);
                searchState.Print(pSudokuOutStream_);
            }

            // バックトラッキング開始
            const auto result = solve(newMap, false, verbose);
            if (result) {
                // 単に値をコピーして状態を書き戻す
                map = newMap;
                // 解けた
                return true;
            }
        }
        break;
    }

    return false;
}

bool SudokuSseSolver::fillCells(SudokuSseMap& map, bool topLevel, bool verbose) {
    ++count_;
    if (verbose) {
        if (pSudokuOutStream_ != nullptr) {
            (*pSudokuOutStream_) << "Step " << count_ << "\n";
        }
    }
    map.FillCrossing(topLevel);
    return true;
}

SudokuLoader::SudokuLoader(int argc, const char * const argv[], std::istream* pSudokuInStream, std::ostream* pSudokuOutStream)
    : isBenchmark_(false), verbose_(true), measureCount_(1), printAllCadidate_(0), pSudokuOutStream_(nullptr) {
    std::string sudokuStr;

    if ((pSudokuInStream == nullptr) || (pSudokuOutStream == nullptr)) {
        return;
    }
    pSudokuOutStream_ = pSudokuOutStream;

    // ベンチマーク回数または饒舌モードを設定する
    if (argc > 1) {
        const auto measureCount = getMeasureCount(argv[1]);
        if (measureCount > 0) {
            measureCount_ = measureCount;
            isBenchmark_ = true;
            verbose_ = false;
        } else {
            if (measureCount < 0) {
                measureCount_ = -measureCount;
            } else {
                // 0回を設定されたら解の数を数える
                measureCount_ = 0;
            }
        }

        if (argc > 2) {
            int patternCount = 0;
            std::istringstream candidateStream(argv[2]);
            candidateStream >> patternCount;
            printAllCadidate_ = (patternCount >= 0) ? patternCount : 0;
        }
    }

    auto readNineLines = false;
    for(SudokuLoopIndex lineCount = 0; lineCount<Sudoku::SizeOfGroupsPerMap; ++lineCount) {
        std::string lineStr;
        getline(*pSudokuInStream, lineStr);
        /* 一行に全マスが書いてあったらそれを使う */
        if (!readNineLines && (lineStr.size() >= Sudoku::SizeOfAllCells)) {
            sudokuStr = lineStr.substr(0,Sudoku::SizeOfAllCells);
            break;
        }
        sudokuStr += lineStr.substr(0,Sudoku::SizeOfCellsPerGroup);
        readNineLines = true;
    }
    sudokuStr_ = sudokuStr;

    return;
}

// デストラクタ
#ifndef NO_DESTRUCTOR
SudokuLoader::~SudokuLoader() {
    // 何もしないなら定義も不要
    return;
}
#endif

// Windows時刻を取得する
void GetTimeOfSys(FILETIME *pTime) {
    GetSystemTimeAsFileTime(pTime);
    return;
}

// CPUクロックカウンタを取得する
void GetTimeOfClock(FILETIME *pTime) {
    asm volatile (
        "RDTSC\n\t"
        "mov [%0], eax\n\t"
        "mov [%0+4], edx\n\t"
        ::"r"(pTime):"eax", "edx"
        );
    return;
}

int SudokuLoader::Exec(void) {
    if (measureCount_) {
        measureTimeToSolve(SudokuSolverType::SOLVER_GENERAL);
    }
    measureTimeToSolve(SudokuSolverType::SOLVER_SSE_4_2);
    return 0;
}

bool SudokuLoader::CanLaunch(int argc, const char * const argv[]) {
    if (!sudokuXmmAssumeCellsPacked) {
        return true;
    }
    if ((argc <= 1) || getMeasureCount(argv[1])) {
        return false;
    }
    return true;
}

int SudokuLoader::getMeasureCount(const char *arg) {
    if (!arg) {
        return 0;
    }
    std::istringstream countStream(arg);

    decltype(getMeasureCount(nullptr)) measureCount = 0;
    countStream >> measureCount;
    return measureCount;
}

// 解く時間を測る
void SudokuLoader::measureTimeToSolve(SudokuSolverType solverType) {
    FILETIME startTimeSys, startTimeClock, stopTimeSys, stopTimeClock;
    auto showAverage = true;

    for(int trial=0;trial<2;++trial) {
        // 初回はキャッシュがヒットしないので遅いはずなので、キャッシュがこなれるまで繰り返す
        for(int i=0;i<measureCount_;++i) {
            solveSudoku(solverType, i, true);
        }

        // 二回目以降は早いはず
        SudokuTime leastTimeClock = 0;
        GetTimeOfSys(&startTimeSys);
        GetTimeOfClock(&startTimeClock);

        if (!measureCount_) {
            showAverage = false;
            const auto oneTimeClock = enumerateSudoku();
            if (oneTimeClock > 0) {
                leastTimeClock = ((leastTimeClock == 0) || (oneTimeClock < leastTimeClock)) ? oneTimeClock : leastTimeClock;
            }
        } else {
            for(int i=0;i<measureCount_;++i) {
                const auto oneTimeClock = solveSudoku(solverType, i, false);
                if (oneTimeClock > 0) {
                    leastTimeClock = ((leastTimeClock == 0) || (oneTimeClock < leastTimeClock)) ? oneTimeClock : leastTimeClock;
                }
            }
        }
        GetTimeOfClock(&stopTimeClock);
        GetTimeOfSys(&stopTimeSys);

        // SudokuTimeにキャストしている
        printTime(startTimeSys, stopTimeSys, startTimeClock, stopTimeClock, measureCount_, leastTimeClock, showAverage);
    }
}

// 解く
SudokuTime SudokuLoader::solveSudoku(SudokuSolverType solverType, int count, bool warmup) {
    const SudokuIndex solverseed = static_cast<decltype(solverseed)>(count);
    SudokuSolver cppSolver(sudokuStr_, solverseed, pSudokuOutStream_);
    SudokuSseSolver sseSolver(sudokuStr_, pSudokuOutStream_, printAllCadidate_);
    SudokuBaseSolver* pSolver = nullptr;

    switch(solverType) {
    case SudokuSolverType::SOLVER_GENERAL:
        pSolver = &cppSolver;
        break;
    case SudokuSolverType::SOLVER_SSE_4_2:
        pSolver = &sseSolver;
        break;
    default:
        return 0;
    }

    if ((!count) && warmup) {
        if (pSudokuOutStream_ != nullptr) {
            pSolver->PrintType();
            (*pSudokuOutStream_) << "Solving(warm up and measure)\n";
        }
    }

    FILETIME startTimeClock, stopTimeClock;
    GetTimeOfClock(&startTimeClock);
    pSolver->Exec(isBenchmark_, verbose_);
    GetTimeOfClock(&stopTimeClock);

    return convertTimeToNum(stopTimeClock) - convertTimeToNum(startTimeClock);
}

// 数える
SudokuTime SudokuLoader::enumerateSudoku(void) {
    SudokuSseSolver sseSolver(sudokuStr_, pSudokuOutStream_, printAllCadidate_);

    FILETIME startTimeClock, stopTimeClock;
    GetTimeOfClock(&startTimeClock);
    sseSolver.Enumerate();
    GetTimeOfClock(&stopTimeClock);

    return convertTimeToNum(stopTimeClock) - convertTimeToNum(startTimeClock);
}

// 処理時間を表示する
void SudokuLoader::printTime(const FILETIME& start100nsTime, const FILETIME& stop100nsTime, const FILETIME& startClock,
                             const FILETIME& stopClock, SudokuTime count, SudokuTime leastClock, bool showAverage) {
    // 解の数だけ求める場合
    const SudokuTime actualCount = (count) ? count : 1;
    const SudokuTime usecTime = (convertTimeToNum(stop100nsTime) - convertTimeToNum(start100nsTime)) / SudokuTimeUnitInUsec;
    const SudokuTime clockElapsed = convertTimeToNum(stopClock) - convertTimeToNum(startClock);
    const double usecOnceTime = static_cast<decltype(usecOnceTime)>(usecTime) /
        static_cast<decltype(usecOnceTime)>(actualCount);
    const SudokuTime clockOnce = clockElapsed / actualCount;
    const double leastUsecOnceTime = static_cast<decltype(leastUsecOnceTime)>(leastClock  * usecTime) /
        static_cast<decltype(leastUsecOnceTime)>(clockElapsed);

    SudokuTime secTime = usecTime / SudokuTimeUsecPerSec;
    const SudokuTime minTime = secTime / SudokuTimeSecPerMinute;
    secTime = secTime % SudokuTimeSecPerMinute;

    if (pSudokuOutStream_ != nullptr) {
        (*pSudokuOutStream_) << std::dec;
        (*pSudokuOutStream_) << "Total : ";
        if (minTime > 0) {
            (*pSudokuOutStream_) << minTime << "min ";
        }
        if ((minTime > 0) || (secTime > 0)) {
            (*pSudokuOutStream_) << secTime << "sec, ";
        }
        (*pSudokuOutStream_) << std::dec << usecTime << "usec, ";
        (*pSudokuOutStream_) << std::dec << clockElapsed << "clock\n";
        if (showAverage) {
            (*pSudokuOutStream_) << "average : " << std::fixed << std::setprecision(3) << usecOnceTime << "usec, ";
            (*pSudokuOutStream_) << std::dec << clockOnce << "clock\n";
            (*pSudokuOutStream_) << "Once least : " << std::fixed << std::setprecision(3) << leastUsecOnceTime << "usec, ";
            (*pSudokuOutStream_) << std::dec << leastClock << "clock\n\n";
        }
    }
    return;
}

// 100nsec単位の時間(Windows API)またはクロックを整数にする
SudokuTime SudokuLoader::convertTimeToNum(const FILETIME& filetime) {
    SudokuTime timeIn100nsec = filetime.dwHighDateTime;

    // 一度に32bitまとめてシフトしない(x86のシフト命令はシフト幅が5bitしかない)
    for(size_t i=0;i<sizeof(filetime.dwLowDateTime); ++i) {
        timeIn100nsec <<= 8;
    }
    timeIn100nsec += filetime.dwLowDateTime;

    return timeIn100nsec;
}

/*
Local Variables:
mode: c++
coding: utf-8-unix
tab-width: nil
c-file-style: "stroustrup"
End:
*/
