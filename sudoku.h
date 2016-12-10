// Sudoku solver with SSE 4.2 / AVX
// Copyright (C) 2012-2015 Zettsu Tatsuya

#ifndef SUDOKU_H_INCLUDED
#define SUDOKU_H_INCLUDED

#include <stdint.h>
#include <string>
#include <vector>
#include <nmmintrin.h>

// 仮想関数を一切禁止すると速くなる。許可するなら下段のマクロを無効にする
#define NO_DESTRUCTOR_AND_VTABLE (1)

// 読みにくいが高速にする
#define FAST_MODE (true)

// 単体テスト時はインライン化しない
#if !defined(UNITTEST)
  #define INLINE inline
#else
  #define INLINE
#endif

#ifdef NO_DESTRUCTOR_AND_VTABLE
  #define ALLOW_VIRTUAL
  #define NO_DESTRUCTOR (1)
#else
  #define ALLOW_VIRTUAL virtual
#endif

// WindowsとLinuxで異なる実装を行うが、I/Fは共通にする
#include "sudoku_os_dependent.h"

/* 配列のサイズを求める
 * arrayにポインタを使わせないために、テンプレートにする
 * C++98では、関数内で定義した構造体を配列にしたときは、このテンプレートはコンパイルエラーになる
 * C++11では、コンパイルエラーにならない
 */
template<typename T, size_t n>
constexpr size_t arraySizeof(const T (&)[n]) {
    return n;
}

// コマンドライン引数
namespace SudokuOption {
    const char * const CommandLineArgSseSolver[] = {"1", "sse", "avx"};
    const char * const CommandLineNoChecking[] = {"1", "off"};
    const char * const CommandLinePrint[] = {"2", "print"};
    // コマンドラインで指定した値が用意したものに一致したら値を設定する
    template <typename T, size_t n>
    void setMode(int argc, const char * const argv[], int argIndex, const char * const (&pOptionSet)[n], T& target, T value) {
        if (argc <= argIndex) {
            return;
        }

        std::string param = argv[argIndex];
        for(auto& pOpt : pOptionSet) {
            std::string opt = pOpt;
            if (param == opt) {
                target = value;
            }
        }

        return;
    }
}

// 型宣言(32Kbyte L1 Data Cacheに収まること)
using SudokuIndex = unsigned short;          // マスとマスの集合の番号(shortの方が速い)
using SudokuLoopIndex = unsigned int;        // マスとマスの集合の番号のループインデックス(intの方が速い)
using SudokuCellCandidates = unsigned int;   // マスの候補の集合
using SudokuNumber = int;                    // マスの初期設定の候補となる数字
using SudokuSseElement = uint32_t;           // SSE4.2命令で解く場合のN byteデータアクセス単位(3マス分)
using gRegister = uint64_t;                  // 汎用レジスタ(必ず符号なし)
using xmmRegister = __m128;                  // XMMレジスタ
using SudokuPatternCount = uint64_t;         // 解の数

static_assert(sizeof(SudokuSseElement) == 4, "Unexpected SudokuSseElement size");
static_assert(sizeof(xmmRegister) == 16, "Unexpected xmmRegister size");
static_assert((alignof(xmmRegister) % 16) == 0, "Unexpected xmmRegister alignment");

// SSE4.2設定
namespace SudokuSse {
    constexpr size_t RegisterCnt = 16;    // 数独の結果を入れるXMMレジスタ数
    constexpr size_t RegisterWordCnt = 4; // XMMレジスタのword数
}

// 全XMMレジスタ(128bit * 16本)
union XmmRegisterSet {
    SudokuSseElement regVal_[SudokuSse::RegisterCnt * SudokuSse::RegisterWordCnt];
    xmmRegister      regXmmVal_[SudokuSse::RegisterCnt];
};

// 数独の定数
namespace Sudoku {
    constexpr SudokuIndex SizeOfCellsPerGroup = 9;    // 列、行、箱に含むマスの数
    constexpr SudokuIndex SizeOfGroupsPerMap = 9;     // 列、行、箱の数
    constexpr SudokuIndex SizeOfAllCells = 81;        // すべてのマスの数
    constexpr SudokuIndex SizeOfCandidates = 9;       // マスの候補数
    constexpr SudokuIndex SizeOfUniqueCandidate = 1;  // 唯一のマスの候補数
    constexpr SudokuIndex OutOfRangeCandidates = 0x10;    // 候補が全くないか1つのときの範囲外の候補数 = 2^n
    constexpr SudokuIndex OutOfRangeMask = OutOfRangeCandidates - 1;   // 候補が全くないか1つのときの範囲外の候補数のマスク
    constexpr SudokuIndex SizeOfGroupsPerCell = 3;    // マスが属する列、行、箱
    constexpr SudokuIndex SizeOfLookUpCell = 512;     // マスの属性の早見表の要素数
    constexpr SudokuIndex SizeOfBoxesOnEdge = 3;      // 3*3の箱が一辺に3個ある
    constexpr SudokuIndex SizeOfCellsOnBoxEdge = 3;   // 3*3の箱の一辺に3マスある
    // bit数の少ない型で定義して使う方で拡張する
    constexpr unsigned short EmptyCandidates = 0;     // 空集合
    constexpr unsigned short UniqueCandidates = 1;    // 唯一の候補
    constexpr unsigned short AllCandidates = 0x1ff;   // 全集合(bit8..0が1)
    constexpr SudokuSseElement AllThreeCandidates = 0x7ffffff;   // 3マスの全集合(bit8..0が1)
    constexpr short MinCandidatesNumber = 1;          // マスの初期設定の候補となる数字の最小値
    constexpr short MaxCandidatesNumber = 9;          // マスの初期設定の候補となる数字の最大値
}

// 共通関数
namespace Sudoku {
    void LoadXmmRegistersFromMem(const xmmRegister *pData);  // メモリからXMMレジスタにロードする
    void SaveXmmRegistersToMem(xmmRegister *pData);          // XMMレジスタの内容をメモリにセーブする

    // あらかじめ与えられた数字が妥当であれば設定する
    template <typename SudokuNumberType>
    bool ConvertCharToSudokuCandidate(SudokuNumberType minNum, SudokuNumberType maxNum, char c, int& num);

    // マスの全候補を表示する
    template <typename SudokuElementType>
    void PrintSudokuElement(SudokuElementType candidates, SudokuElementType uniqueCandidates,
                            SudokuElementType emptyCandidates, std::ostream* pSudokuOutStream);
}

// 解き方
enum class SudokuSolverType {
    SOLVER_GENERAL,  // C++テンプレートプログラミング
    SOLVER_SSE_4_2,  // SSE4.2 assembly
};

// 解いた結果を検査するかどうか
enum class SudokuSolverCheck {
    CHECK,         // 結果を検査する
    DO_NOT_CHECK,  // 結果を検査しない
};

// 複数の問題を解くときに、結果を表示するかどうか
enum class SudokuSolverPrint {
    DO_NOT_PRINT,  // 結果を表示しない
    PRINT,         // 結果を表示する
};

// 解法(共通)
class SudokuBaseSolver {
public:
    virtual bool Exec(bool silent, bool verbose) = 0;
    virtual void PrintType(void) = 0;
protected:
    SudokuBaseSolver(std::ostream* pSudokuOutStream);
    virtual void printType(const std::string& presetStr, std::ostream* pSudokuOutStream);
    int            count_;             // 手順を試した回数
    std::ostream*  pSudokuOutStream_;  // 結果の出力先
};

class SudokuCellLookUp {
public:
    bool        IsUnique;           // 候補は一つしかない
    bool        IsMultiple;         // 複数の候補がある
    SudokuIndex NumberOfCandidates; // 候補の数
};

// マス
class SudokuCell {
    // Unit test
    friend class SudokuCellTest;
    friend class SudokuMapTest;
    friend class SudokuSolverTest;
    template <class TestedT, class CandidatesT> friend class SudokuCellCommonTest;
    template <class TestedT, class CandidatesT> friend class SudokuSolverCommonTest;

public:
    SudokuCell(void);
#ifndef NO_DESTRUCTOR
    ALLOW_VIRTUAL ~SudokuCell();
#endif
    // 初期化と出力
    void Preset(char c);
    void SetIndex(SudokuIndex indexNumber);
    void Print(std::ostream* pSudokuOutStream) const;
    INLINE SudokuIndex GetIndex(void) const;
    // 数独操作
    INLINE bool IsFilled(void) const;
    INLINE bool HasMultipleCandidates(void) const;
    INLINE bool IsConsistent(SudokuCellCandidates candidates) const;
    INLINE bool HasCandidate(SudokuCellCandidates candidate) const;
    INLINE bool HasNoCandidates(void) const;
    INLINE void SetCandidates(SudokuCellCandidates candidates);
    INLINE SudokuCellCandidates GetCandidates(void) const;
    INLINE SudokuCellCandidates GetUniqueCandidate(void) const;
    INLINE SudokuIndex CountCandidates(void) const;
    INLINE SudokuIndex CountCandidatesIfMultiple(void) const;
    // クラス関数
    INLINE static SudokuIndex MaskCandidatesUnlessMultiple(SudokuIndex numberOfCandidates);
    INLINE static bool IsEmptyCandidates(SudokuCellCandidates candidates);
    INLINE static bool IsUniqueCandidate(SudokuCellCandidates candidates);
    INLINE static SudokuCellCandidates GetEmptyCandidates(void);
    INLINE static SudokuCellCandidates FlipCandidates(SudokuCellCandidates candidates);
    INLINE static SudokuCellCandidates MergeCandidates(SudokuCellCandidates candidatesA, SudokuCellCandidates candidatesB);
    INLINE static SudokuCellCandidates GetInitialCandidate();
    INLINE static SudokuCellCandidates GetNextCandidate(SudokuCellCandidates candidate);
private:
    INLINE void updateState(void);
    // メンバ(値ごとコピーできる)
    SudokuIndex          indexNumber_;  // すべてのマスの通し番号
    SudokuCellCandidates candidates_;   // マスの候補(1..9が候補ならbit1..9が1)
    // エントリ属性
    static const SudokuCellLookUp CellLookUp_[Sudoku::SizeOfLookUpCell];

    // 空集合
    static constexpr SudokuCellCandidates SudokuEmptyCandidates = Sudoku::EmptyCandidates;
    // 唯一の候補
    static constexpr SudokuCellCandidates SudokuUniqueCandidates = Sudoku::UniqueCandidates;
    // 全集合(bit8..0が1)
    static constexpr SudokuCellCandidates SudokuAllCandidates = Sudoku::AllCandidates;
    // マスの初期設定の候補となる数字の最小値
    static constexpr SudokuNumber SudokuMinCandidatesNumber = Sudoku::MinCandidatesNumber;
    // マスの初期設定の候補となる数字の最大値
    static constexpr SudokuNumber SudokuMaxCandidatesNumber = Sudoku::MaxCandidatesNumber;
};

// 全マス(C++テンプレートプログラミング)
class SudokuMap {
    // Unit test
    friend class SudokuMapTest;
    friend class SudokuSolverTest;
    template <class TestedT, class CandidatesT> friend class SudokuSolverCommonTest;

public:
    SudokuMap(void);
#ifndef NO_DESTRUCTOR
    ALLOW_VIRTUAL ~SudokuMap();
#endif
    // 初期化と出力
    void Preset(const std::string& presetStr, SudokuIndex seed);
    void Print(std::ostream* pSudokuOutStream) const;
    // 数独操作
    INLINE bool IsFilled(void) const;
    bool FillCrossing(void);
    INLINE bool CanSetUniqueCell(SudokuIndex cellIndex, SudokuCellCandidates candidate) const;
    INLINE void SetUniqueCell(SudokuIndex cellIndex, SudokuCellCandidates candidate);
    INLINE SudokuIndex CountFilledCells(void) const;
    INLINE SudokuIndex SelectBacktrackedCellIndex(void) const;
    bool IsConsistent(void) const;
private:
    bool findUnusedCandidate(SudokuCell& targetCell) const;
    bool findUniqueCandidate(SudokuCell& targetCell) const;
    // すべてのマス
    SudokuCell cells_[Sudoku::SizeOfAllCells];
    // バックトラック対象の選択に横、縦、3*3箱のどれを選ぶか
    SudokuIndex backtrackedGroup_;

    // すべての列と行と箱に属するマス
    static const SudokuIndex Group_[Sudoku::SizeOfGroupsPerCell][Sudoku::SizeOfGroupsPerMap][Sudoku::SizeOfCellsPerGroup];

    // すべてマスの属する、列と行と箱
    static const SudokuIndex ReverseGroup_[Sudoku::SizeOfAllCells][Sudoku::SizeOfGroupsPerCell];

    // 3*3マスのグループ集合番号
    static constexpr SudokuIndex SudokuBoxGroupId = 2;
    // 高速化
    template <SudokuIndex index> INLINE SudokuIndex unrolledCountFilledCells
        (SudokuIndex accumCount) const;
    template <SudokuIndex innerIndex> INLINE SudokuCellCandidates unrolledFindUnusedCandidateInner
        (SudokuIndex targetCellIndex, SudokuIndex outerIndex, SudokuIndex groupIndex, SudokuCellCandidates candidates) const;
    template <SudokuIndex outerIndex> INLINE SudokuCellCandidates unrolledFindUnusedCandidateOuter
        (SudokuIndex targetCellIndex, SudokuCellCandidates candidates) const;
    template <SudokuIndex innerIndex> INLINE SudokuCellCandidates unrolledFindUniqueCandidateInner
        (SudokuIndex targetCellIndex, SudokuIndex outerIndex, SudokuIndex groupIndex, SudokuCellCandidates candidates) const;
    template <SudokuIndex innerIndex> INLINE SudokuIndex unrolledSelectBacktrackedCellIndexInner
        (SudokuIndex outerIndex, SudokuIndex& leastCountOfGroup, SudokuIndex& candidateCellIndex) const;
    INLINE SudokuCellCandidates unrolledFindUnusedCandidateInnerCommon
        (SudokuIndex targetCellIndex, SudokuIndex outerIndex, SudokuIndex groupIndex, SudokuIndex innerIndex,
         SudokuCellCandidates candidates) const;
    INLINE SudokuCellCandidates unrolledFindUnusedCandidateOuterCommon
        (SudokuIndex targetCellIndex, SudokuIndex outerIndex, SudokuCellCandidates candidates) const;
    INLINE SudokuCellCandidates unrolledFindUniqueCandidateInnerCommon
        (SudokuIndex targetCellIndex, SudokuIndex outerIndex, SudokuIndex groupIndex, SudokuIndex innerIndex,
         SudokuCellCandidates candidates) const;
    INLINE SudokuIndex unrolledSelectBacktrackedCellIndexInnerCommon
        (SudokuIndex outerIndex, SudokuIndex innerIndex,
         SudokuIndex& leastCountOfGroup, SudokuIndex& candidateCellIndex) const;
};

// 解法(C++テンプレートプログラミング)
class SudokuSolver : public SudokuBaseSolver {
    // Unit test
    friend class SudokuSolverTest;
    template <class TestedT, class CandidatesT> friend class SudokuSolverCommonTest;
public:
    SudokuSolver(const std::string& presetStr, SudokuIndex seed, std::ostream* pSudokuOutStream);
    SudokuSolver(const std::string& presetStr, SudokuIndex seed, std::ostream* pSudokuOutStream, SudokuPatternCount printAllCadidate);
    virtual ~SudokuSolver();
    virtual bool Exec(bool silent, bool verbose) override;
    virtual void PrintType(void) override;
private:
    bool solve(SudokuMap& map, bool topLevel, bool verbose);
    bool fillCells(SudokuMap& map, bool topLevel, bool verbose);
    // メンバ
    SudokuMap map_;    // 数独(バックトラッキングは配列であらかじめ確保するよりスタックに確保した方が速い)
};

// マス(SSE4.2)
class SudokuSseCell {
    // Unit test
    friend class SudokuSseCellTest;
    template <class TestedT, class CandidatesT> friend class SudokuCellCommonTest;
    template <class TestedT, class CandidatesT> friend class SudokuSolverCommonTest;

public:
    SudokuSseCell(void);
#ifndef NO_DESTRUCTOR
    ALLOW_VIRTUAL ~SudokuSseCell();
#endif
    void Preset(char c);
    void Print(std::ostream* pSudokuOutStream) const;
    void SetCandidates(SudokuSseElement candidates);
    SudokuSseElement GetCandidates(void);
    static constexpr SudokuSseElement AllCandidates = Sudoku::AllCandidates;  // 全候補のときは全bitを立てる
private:
    static constexpr SudokuSseElement SudokuEmptyCandidates = Sudoku::EmptyCandidates;
    static constexpr SudokuSseElement SudokuUniqueCandidates = Sudoku::UniqueCandidates;
    static constexpr SudokuNumber SudokuMinCandidatesNumber = Sudoku::MinCandidatesNumber;
    static constexpr SudokuNumber SudokuMaxCandidatesNumber = Sudoku::MaxCandidatesNumber;
    SudokuSseElement candidates_;   // マスの候補(1..9が候補ならbit0..8が1)
};

// ASMで定義する
extern "C" {
    // パラメータ(説明はASMを参照)
    extern volatile uint64_t sudokuXmmAborted;
    extern volatile uint64_t sudokuXmmNextCellFound;
    extern volatile uint64_t sudokuXmmNextOutBoxShift;
    extern volatile uint64_t sudokuXmmNextInBoxShift;
    extern volatile uint64_t sudokuXmmNextRowNumber;
    extern volatile uint64_t sudokuXmmElementCnt;
    extern volatile uint64_t sudokuXmmPrintAllCandidate;
    extern volatile uint64_t sudokuXmmRightBottomElement;
    extern volatile uint64_t sudokuXmmRightBottomSolved;
    extern volatile uint64_t sudokuXmmAllPatternCnt;
    extern volatile uint64_t sudokuXmmPrintFunc;
    extern volatile uint64_t sudokuXmmAssumeCellsPacked;
    extern volatile uint64_t sudokuXmmUseAvx;
    extern volatile uint64_t sudokuXmmDebug;
    extern XmmRegisterSet sudokuXmmToPrint;
}

// バックトラッキング候補
struct SudokuSseCandidateCell {
    size_t           regIndex;  // マスが入っている汎用レジスタの番号
    SudokuSseElement shift;     // マスが入っている汎用レジスタのビット位置
    SudokuSseElement mask;      // マスが入っている汎用レジスタのビットマスク
};

// 全マス(SSE4.2)
class SudokuSseMap {
    // Unit test
    friend class SudokuSseMapTest;
    template <class TestedT, class CandidatesT> friend class SudokuSolverCommonTest;
private:
    static constexpr size_t InitialRegisterNum = 1;  // 最初の行を格納するXMMレジスタ番号
    XmmRegisterSet xmmRegSet_;
public:
    SudokuSseMap(void);
#ifndef NO_DESTRUCTOR
    ALLOW_VIRTUAL ~SudokuSseMap();
#endif
    // 初期化と出力
    void Preset(const std::string& presetStr);
    void Print(std::ostream* pSudokuOutStream) const;
    void FillCrossing(bool loadXmm);
    INLINE bool GetNextCell(SudokuSseCandidateCell& cell);
    INLINE bool CanSetUniqueCell(const SudokuSseCandidateCell& cell, SudokuCellCandidates candidate) const;
    INLINE void SetUniqueCell(const SudokuSseCandidateCell& cell, SudokuCellCandidates candidate);
};

// 全パターンを数えるためのマス(SSE4.2)
class SudokuSseEnumeratorMap {
    // Unit test
    friend class SudokuSseEnumeratorMapTest;
private:
    static constexpr size_t InitialRegisterNum = 1;      // 最初の行を格納するXMMレジスタ番号
    static constexpr size_t RightColumnRegisterNum = 10; // 最右列を格納するXMMレジスタ番号
    static constexpr size_t CellBitWidth = 16;           // マスのbit幅
    static constexpr size_t BitsPerByte = 8;             // Byteのbit数
    SudokuCellCandidates rightBottomElement_;
    gRegister firstCell_;
    SudokuPatternCount patternNumber_;
    static SudokuSseEnumeratorMap* pInstance_;
    XmmRegisterSet xmmRegSet_;
public:
    SudokuSseEnumeratorMap(std::ostream* pSudokuOutStream);
    virtual ~SudokuSseEnumeratorMap();
    // 初期化と出力
    void SetToPrint(SudokuPatternCount printAllCadidate);
    void Preset(const std::string& presetStr);
    void Print(void) const;
    void Print(bool solved, const XmmRegisterSet& xmmRegSet) const;
    void PrintFromAsm(const XmmRegisterSet& xmmRegSet);
    SudokuPatternCount Enumerate(void);
    static SudokuSseEnumeratorMap* GetInstance(void);
private:
    void presetCell(SudokuLoopIndex index, int num);
    size_t powerOfTwoPlusOne(SudokuSseElement regValue) const;
    std::ostream* pSudokuOutStream_;
};

// 解法(SSE4.2)
class SudokuSseSolver : public SudokuBaseSolver {
    // Unit test
    friend class SudokuSseSolverTest;
    template <class TestedT, class CandidatesT> friend class SudokuSolverCommonTest;

public:
    SudokuSseSolver(const std::string& presetStr, std::ostream* pSudokuOutStream, SudokuPatternCount printAllCadidate);
    SudokuSseSolver(const std::string& presetStr, SudokuIndex seed, std::ostream* pSudokuOutStream, SudokuPatternCount printAllCadidate);
    virtual ~SudokuSseSolver();
    virtual bool Exec(bool silent, bool verbose) override;
    virtual void Enumerate(void);
    virtual void PrintType(void) override;
private:
    void initialize(const std::string& presetStr, std::ostream* pSudokuOutStream);
    bool solve(SudokuSseMap& map, bool topLevel, bool verbose);
    bool fillCells(SudokuSseMap& map, bool topLevel, bool verbose);
    // メンバ
    SudokuSseMap map_;    // 数独
    SudokuSseEnumeratorMap enumeratorMap_;
    SudokuPatternCount printAllCadidate_;
};

// 解が正しいかどうか判定する
class SudokuCheckerTest;
class SudokuChecker {
    // ユニットテスト
    friend class SudokuCheckerTest;
public:
    SudokuChecker(const std::string& puzzle, const std::string& solution, SudokuSolverPrint printSolution, std::ostream* pSudokuOutStream);
    virtual ~SudokuChecker() = default;
    bool Valid() const;  // 解が正しければtrue
private:
    using Group = std::vector<SudokuNumber>;
    using Grid = std::vector<Group>;
    bool parse(const std::string& puzzle, const std::string& solution, SudokuSolverPrint printSolution, std::ostream* pSudokuOutStream);
    bool parseRow(SudokuIndex row, const std::string& rowLine, Grid& grid, std::string& solutionLine);
    bool compare(const std::string& puzzle, const std::string& solution, std::ostream* pSudokuOutStream);
    bool check(const Grid& grid, std::ostream* pSudokuOutStream);
    bool checkRowSet(const Grid& grid, std::ostream* pSudokuOutStream);
    bool checkColumnSet(const Grid& grid, std::ostream* pSudokuOutStream);
    bool checkBoxSet(const Grid& grid, std::ostream* pSudokuOutStream);
    bool checkUnique(const Group& line);
    bool valid_;
};

// 読み込みと実行時間測定
class SudokuLoader {
    // Unit test
    friend class SudokuLoaderTest;

public:
    SudokuLoader(int argc, const char * const argv[], std::istream* pSudokuInStream, std::ostream* pSudokuOutStream);

#ifndef NO_DESTRUCTOR
    ALLOW_VIRTUAL ~SudokuLoader();
#endif
    int Exec(void);
    static bool CanLaunch(int argc, const char * const argv[]);
private:
    static int getMeasureCount(const char *arg);
    void setSingleMode(int argc, const char * const argv[], std::istream* pSudokuInStream);
    bool setMultiMode(int argc, const char * const argv[]);
    int execSingle(void);
    int execMulti(void);
    int execMulti(std::istream* pSudokuInStream);
    void measureTimeToSolve(SudokuSolverType solverType);
    SudokuTime solveSudoku(SudokuSolverType solverType, int count, bool warmup);
    SudokuTime enumerateSudoku(void);
    // メンバ
    std::string sudokuStr_; // 初期マップの文字列
    std::string multiLineFilename_;  // 各行に数独パズルを書いたファイル名
    SudokuSolverType  solverType_;   // 各行に数独パズルを書いたファイルを解く方法
    SudokuSolverCheck check_;        // 解いた結果を検査するかどうか
    SudokuSolverPrint print_;        // 複数の問題を解くときに、結果を表示するかどうか
    bool   isBenchmark_;    // ベンチマークかどうか
    bool   verbose_;        // 解く過程を示すかどうか
    int    measureCount_;   // 測定回数
    SudokuPatternCount printAllCadidate_;
    std::ostream* pSudokuOutStream_;  // 結果の出力先
    static const int ExitStatusPassed;  // 正常終了
    static const int ExitStatusFailed;  // 異常終了
};

extern "C" {
    void PrintPattern(void);
}

#endif // SUDOKU_H_INCLUDED

/*
Local Variables:
mode: c++
coding: utf-8-unix
tab-width: nil
c-file-style: "stroustrup"
End:
*/
