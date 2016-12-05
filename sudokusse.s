# Sudoku solver with SSE 4.2 / AVX
# Copyright (C) 2012-2015 Zettsu Tatsuya

.intel_syntax noprefix
.file   "sudokuxmm.s"
        # ビルド設定
        # 1にするとAVX、0にするとSSEを使う
        # Makefileで指定するのでここでは指定しない
        # .set      EnableAvx, 1

        # 動作設定
        # 1にすると、すでに埋まっているマスは左上に固まっていると仮定する
        # Makefileで指定するのでここでは指定しない
        # .set    CellsPacked, 0

        # 10にすると、最下行+残り1マス以後は候補が高々1つしかないと仮定する
        .set    LastCellsToFilled, 10
        # 1にすると、念のために行っている冗長な計算をなくす
        .set    TrimRedundancy, 1
        # 1にすると64レジスタをできるだけ使用する(0のときは32bitレジスタをできるだけ使用する)
        .set    UseReg64Most, 1

        # 本体コード
        .global sudokuXmmAborted
        .global sudokuXmmElementCnt
        .global sudokuXmmPrintAllCandidate
        .global sudokuXmmRightBottomElement
        .global sudokuXmmRightBottomSolved
        .global sudokuXmmAllPatternCnt
        .global sudokuXmmPrintFunc
        .global sudokuXmmReturnAddr
        .global sudokuXmmAssumeCellsPacked
        .global sudokuXmmUseAvx
        .global sudokuXmmDebug
        .global sudokuXmmToPrint

        # テスト
        .global testSearchRowPartElementsPreRowPart
        .global testSearchRowPartElementsPreUniqueCandidate
        .global testSearchRowElementsPreCandidateRow
        .global testSearchRowPartElementsPreInBoxShift
        .global testSearchRowPartElementsPreOutBoxShift
        .global testSearchRowElementsPreUniqueCandidate
        .global testSearchRowElementsPreInBoxShift
        .global testSearchRowElementsPreOutBoxShift
        .global testFillUniqueElementPreRowPart
        .global testFillUniqueElementPreColumn
        .global testFillUniqueElementPreBox
        .global testFillUniqueElementPreRow
        .global testFindRowPartCandidatesPreRowPartTarget
        .global testFindRowPartCandidatesPreRowCandidates
        .global testFindRowPartCandidatesPreBox
        .global testFindRowPartCandidatesRowPartTarget
        .global testFindRowPartCandidatesRowCandidates
        .global testFindRowPartCandidatesBox
        .global testFillNineUniqueCandidatesPreRow
        .global testCountRowCellCandidatesMinCount
        .global testCountRowCellCandidatesRowPopCount
        .global testFillNineUniqueCandidatesRowX
        .global testFillNineUniqueCandidatesBoxX
        .global testFillNineUniqueCandidatesColumnX
        .global testCountRowCellCandidatesRowX

.data
# 本体コード
.set  sudokuMaxLoopcnt,         81         # ループの上限回数
sudokuXmmAborted:               .quad 0    # マスの矛盾を発見した(これ以上進められない)とき非0
sudokuXmmElementCnt:            .quad 0    # 埋まった(候補が一意になった)マスの数(0..81)
sudokuXmmPrintAllCandidate:     .quad 0    # 非0なら候補をすべて表示する
sudokuXmmRightBottomElement:    .quad 0    # 右下の初期値
sudokuXmmRightBottomSolved:     .quad 0    # 右下の解
sudokuXmmAllPatternCnt:         .quad 0    # すべての解のパターン数
sudokuXmmPrintFunc:             .quad 0    # 表示関数のアドレス
sudokuXmmStackPointer:          .quad 0    # 表示関数を呼び出す前のrsp
sudokuXmmReturnAddr:            .quad 0    # return先アドレス
sudokuXmmAssumeCellsPacked:     .quad CellsPacked    # マスは左上に固まっていると仮定する
sudokuXmmUseAvx:                .quad EnableAvx      # SSE命令の代わりにAVX命令を使う
sudokuXmmDebug:                 .quad 0    # デバッグ用

# 81の繰り返し中のマスの候補
.set  arrayElementByteSize, 8
.align 16
sudokuXmmCandidateArray:  .quad 0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0
                          .quad 0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0
                          .quad 0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0 ,0

# マスを処理するラベル
.set  funcPtrByteSize, 8        # ジャンプ先のサイズ(byte単位)
.set  funcPtrByteSizeLog2, 3    # ジャンプ先のサイズ(左シフト回数)

.align 16
sudokuXmmCountFuncTable:
    .quad countAt00, countAt01, countAt02, countAt03, countAt04, countAt05, countAt06, countAt07, countAt08, countAt09
    .quad countAt10, countAt11, countAt12, countAt13, countAt14, countAt15, countAt16, countAt17, countAt18, countAt19
    .quad countAt20, countAt21, countAt22, countAt23, countAt24, countAt25, countAt26, countAt27, countAt28, countAt29
    .quad countAt30, countAt31, countAt32, countAt33, countAt34, countAt35, countAt36, countAt37, countAt38, countAt39
    .quad countAt40, countAt41, countAt42, countAt43, countAt44, countAt45, countAt46, countAt47, countAt48, countAt49
    .quad countAt50, countAt51, countAt52, countAt53, countAt54, countAt55, countAt56, countAt57, countAt58, countAt59
    .quad countAt60, countAt61, countAt62, countAt63, countAt64, countAt65, countAt66, countAt67, countAt68, countAt69
    .quad countAt70, countAt71, countAt72, countAt73, countAt74, countAt75, countAt76, countAt77, countAt78, countAt79
    .quad countAt80, countAt81

.align 16
sudokuXmmToPrint:
        .quad 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0

# テスト
.align 16
testSearchRowPartElementsPreRowPart:          .quad 0
testSearchRowPartElementsPreUniqueCandidate:  .quad 0
testSearchRowPartElementsPreInBoxShift:       .quad 0
testSearchRowPartElementsPreOutBoxShift:      .quad 0
testSearchRowElementsPreUniqueCandidate:      .quad 0
testSearchRowElementsPreCandidateRow:         .quad 0
testSearchRowElementsPreInBoxShift:           .quad 0
testSearchRowElementsPreOutBoxShift:          .quad 0
testFillUniqueElementPreRowPart:              .quad 0
testFillUniqueElementPreColumn:               .quad 0
testFillUniqueElementPreBox:                  .quad 0
testFillUniqueElementPreRow:                  .quad 0
testFindRowPartCandidatesPreRowPartTarget:    .quad 0
testFindRowPartCandidatesPreRowCandidates:    .quad 0
testFindRowPartCandidatesPreBox:              .quad 0
testFindRowPartCandidatesRowPartTarget:       .quad 0
testFindRowPartCandidatesRowCandidates:       .quad 0
testFindRowPartCandidatesBox:                 .quad 0
testFillNineUniqueCandidatesPreRow:           .quad 0
testCountRowCellCandidatesMinCount:           .quad 0
testCountRowCellCandidatesRowPopCount:        .quad 0

# XMMレジスタ = 64bit * 2の配列
.align 16
testFillNineUniqueCandidatesRowX:             .quad 0, 0
testFillNineUniqueCandidatesBoxX:             .quad 0, 0
testFillNineUniqueCandidatesColumnX:          .quad 0, 0
testCountRowCellCandidatesRowX:               .quad 0, 0

.text
# 注意! 各テストは必ず前にInitMaskRegister を付けること

# 定数
.set    maxElementNumber,  81  # 最大要素数
.set    boxRowByteSize,     4  # 3x3箱の一行を収めるためのbyte数
.set    bitPerByte,         8  # byte当たりbit数
.set    candidatesNum,      9  # 行、列、箱の要素数
.set    candidatesTooMany, 10  # 行、列、箱の要素数が多すぎる
.set    numberOfRows,       9  # 行の数
.set    numberOfColumns,    9  # 列の数
.set    candidatesInRowPart,      3    # バックトラッキング候補を探すときの3マス
.set    rowPartNum,               3    # バックトラッキング候補を探すときの行にある3マスの数
.set    inBoxIndexInvalid, (candidatesInRowPart + 1)
.set    outBoxIndexInvalid, (rowPartNum + 1)
.set    minCandidatesNumToSearch, 2    # バックトラッキング候補を探すときの最低要素数
.set    uniqueCandidatesToSearch, 1    # バックトラッキング候補の最初
.set    elementBitMask, 0x1ff          # 3x3箱の一要素のビットマスク
.set    rowPartBitMask, 0x7ffffff      # 3x3箱の一行のビットマスク
.set    rowPartBitMask0, ((elementBitMask << (candidatesNum * 2)) | (elementBitMask << candidatesNum))
.set    rowPartBitMask1, ((elementBitMask << (candidatesNum * 2)) | elementBitMask)
.set    rowPartBitMask2, ((elementBitMask << candidatesNum) | elementBitMask)

# 固定でレジスタを割り当てる
.set    xRegRowAll,  xmm0
.set    xRegRow1,    xmm1
.set    xRegRow2,    xmm2
.set    xRegRow3,    xmm3
.set    xRegRow4,    xmm4
.set    xRegRow5,    xmm5
.set    xRegRow6,    xmm6
.set    xRegRow7,    xmm7
.set    xRegRow8,    xmm8
.set    xRegRow9,    xmm9
.set    xRegRow1to3, xmm10
.set    xRegRow4to6, xmm11
.set    xRegRow7to9, xmm12
.set    xRegWork1,   xmm13
.set    xRegWork2,   xmm14
.set    xLoopPopCnt, xmm15
.set    gRegBitMask, r15
.set    xRegColumnAll, xmm13

.macro InitMaskRegister
        mov     gRegBitMask, elementBitMask
.endm

# SSEとAVXでオペランド数が変わらない命令
.macro MacroMovq op1, op2
.if (EnableAvx != 0)
    vmovq \op1, \op2
.else
    movq  \op1, \op2
.endif
.endm

.macro MacroMovdqa op1, op2
.if (EnableAvx != 0)
    vmovdqa \op1, \op2
.else
    movdqa  \op1, \op2
.endif
.endm

.macro MacroPextrq op1, op2, op3
.if (EnableAvx != 0)
    vpextrq \op1, \op2, \op3
.else
    pextrq  \op1, \op2, \op3
.endif
.endm

.macro MacroPextrd op1, op2, op3
.if (EnableAvx != 0)
    vpextrd \op1, \op2, \op3
.else
    pextrd  \op1, \op2, \op3
.endif
.endm

.macro MacroPextrw op1, op2, op3
.if (EnableAvx != 0)
    vpextrw \op1, \op2, \op3
.else
    pextrw  \op1, \op2, \op3
.endif
.endm

# SSEとAVXでオペランド数が変わる命令
.macro MacroPinsrq regDstX, regSrcX, op3
.if (EnableAvx != 0)
    vpinsrq \regDstX, \regDstX, \regSrcX, \op3
.else
    pinsrq  \regDstX, \regSrcX, \op3
.endif
.endm

.macro MacroPinsrw regDstX, regSrcX, op3
.if (EnableAvx != 0)
    vpinsrw \regDstX, \regDstX, \regSrcX, \op3
.else
    pinsrw  \regDstX, \regSrcX, \op3
.endif
.endm

.macro MacroAndnps regX, op2
.if (EnableAvx != 0)
    vandnps \regX, \regX, \op2
.else
    andnps  \regX, \op2
.endif
.endm

.macro MacroAndps regX, op2
.if (EnableAvx != 0)
    vandps \regX, \regX, \op2
.else
    andps  \regX, \op2
.endif
.endm

.macro MacroOrps regX, op2
.if (EnableAvx != 0)
    vorps \regX, \regX, \op2
.else
    orps  \regX, \op2
.endif
.endm

.macro MacroOrps3op regDstX, regSrcX1, regSrcX2
.if (EnableAvx != 0)
    vorps \regDstX, \regSrcX1, \regSrcX2
.else
    MacroMovdqa \regDstX, \regSrcX1
    MacroOrps   \regDstX, \regSrcX2
.endif
.endm

.macro MacroXorps regX, op2
.if (EnableAvx != 0)
    vxorps \regX, \regX, \op2
.else
    xorps  \regX, \op2
.endif
.endm

.macro MacroPhaddw regX, op2
.if (EnableAvx != 0)
    vphaddw \regX, \regX, \op2
.else
    phaddw  \regX, \op2
.endif
.endm

# 0回シフトを防ぐ
.macro ShlNonZero reg, count
.if (\count != 0)
        shl    \reg, \count
.endif
.endm

.macro ShrNonZero reg, count
.if (\count != 0)
        shr    \reg, \count
.endif
.endm

.macro RolNonZero reg, count
.if (\count != 0)
        rol    \reg, \count
.endif
.endm

.macro PslldqNonZero regX, count
.if (\count != 0)
   .if (EnableAvx != 0)
        vpslldq \regX, \regX, \count
   .else
        pslldq  \regX, \count
   .endif
.endif
.endm

.macro PsrldqNonZero regX, count
.if (\count != 0)
   .if (EnableAvx != 0)
        vpsrldq \regX, \regX, \count
   .else
        psrldq \regX, \count
   .endif
.endif
.endm

# inc/decよりもadd/subの方が、全フラグを書き換えるため早くなる
.macro FastInc reg
#       inc    \reg
        add    \reg, 1
.endm

.macro FastDec reg
#       dec    \reg
        sub    \reg, 1
.endm

# 64bitレジスタの下位32bitの候補だけ残す
.macro MaskLower32bit regDst
        # cdq命令はAレジスタにしか使えない
        ShlNonZero  \regDst, (boxRowByteSize * bitPerByte)
        ShrNonZero  \regDst, (boxRowByteSize * bitPerByte)
.endm

        .global testMaskLower32bit
testMaskLower32bit:
        mov     rbx, rax
        MaskLower32bit rbx
        ret

# 64bitレジスタの上下32bitを分離する
.macro SplitRowLowParts regLow, regSrcAndHigh
        mov    \regLow, \regSrcAndHigh
        MaskLower32bit  \regLow
        shrNonZero  \regSrcAndHigh, (boxRowByteSize * bitPerByte)
.endm

        .global testSplitRowLowParts
testSplitRowLowParts:
        mov     rbx, rax
        SplitRowLowParts rcx, rbx
        ret

# 共通マクロのラベルは3000番台(上から順)
# 0または2のべき乗なら、<=(符号なし:jbe) その反対は >(符号なし:ja) で分岐
.macro IsPowerOf2ToFlags regSrc, regWork1
        popcnt  \regWork1, \regSrc
        cmp     \regWork1, 1
.endm

        .global testIsPowerOf2ToFlags
testIsPowerOf2ToFlags:
        IsPowerOf2ToFlags rax, rbx
        mov     rbx, 0
        ja      testIsPowerOf2ToFlagsA
        FastInc rbx
testIsPowerOf2ToFlagsA:
        ret

# 2のべき乗(0を含む)ならその値、それ以外は全bit0
.macro PowerOf2or0 regDst, regSrc, regWork1
        popcnt  \regWork1, \regSrc
        xor     \regDst,   \regDst
        cmp     \regWork1, 1
        cmovz   \regDst,   \regSrc
.endm

        .global testPowerOf2or0
testPowerOf2or0:
        PowerOf2or0 rbx, rax, rcx
        ret

# 2のべき乗(0を除く)ならその値、それ以外は全bit1
.macro PowerOf2orAll1 regDst, regSrc, regWork1
        # mov -1の方がxor r,rでレジスタをクリアしてnotするより速い
        popcnt  \regWork1, \regSrc
        mov     \regDst,   -1
        cmp     \regWork1, 1
        cmovz   \regDst,   \regSrc
.endm

        .global testPowerOf2orAll1
testPowerOf2orAll1:
        PowerOf2orAll1 rbx, rax, rcx
        ret

# 汎用レジスタの3マス分の候補を1つにまとめる(破壊的)
.macro MergeThreeElements regSum, regWork
        mov    \regWork, \regSum
        ShrNonZero  \regWork, (candidatesNum * 2)
        or     \regSum,  \regWork
        mov    \regWork, \regSum
        ShrNonZero  \regWork, (candidatesNum * 1)
        or     \regSum,  \regWork
        and    \regSum,  gRegBitMask
.endm

        .global testMergeThreeElements
testMergeThreeElements:
        InitMaskRegister
        mov     rbx, rax
        MergeThreeElements rbx, rcx
        ret

# 上と同じだがレジスタを1個多く使える
.macro MergeThreeElements2 regSum, regWork1, regWork2
        mov    \regWork1, \regSum
        mov    \regWork2, \regSum
        ShrNonZero  \regWork1, (candidatesNum * 2)
        ShrNonZero  \regWork2, (candidatesNum * 1)
        or     \regSum,  \regWork1
        or     \regSum,  \regWork2
        and    \regSum,  gRegBitMask
.endm

        .global testMergeThreeElements2
testMergeThreeElements2:
        InitMaskRegister
        mov     rbx, rax
        MergeThreeElements2 rbx, rcx, rdx
        ret

# 3行ごとに候補をまとめる
.macro OrThreeXmmRegs xRegDst, xRegSrc1, xRegSrc2, xRegSrc3
.if (EnableAvx != 0)
        vorps  \xRegDst, \xRegSrc1, \xRegSrc2
        vorps  \xRegDst, \xRegDst,  \xRegSrc3
.else
        MacroMovdqa \xRegDst, \xRegSrc1
        MacroOrps   \xRegDst, \xRegSrc2
        MacroOrps   \xRegDst, \xRegSrc3
.endif
.endm

        .global testOrThreeXmmRegs
testOrThreeXmmRegs:
        OrThreeXmmRegs xRegRow1, xRegRow2, xRegRow3, xRegRow4
        ret

# 横3マスのうちの1マスに対して、唯一の候補を残して他は0にする
.macro FilterUniqueCandidatesInRowPartSub regBitmask, regDst, regRowPart, regWork1, regWork2, regWork3
        ShlNonZero  \regBitmask, candidatesNum
        mov    \regWork1, \regRowPart
        and    \regWork1, \regBitmask
        PowerOf2or0 \regWork2, \regWork1, \regWork3
        or     \regDst, \regWork2
.endm

        .global testFilterUniqueCandidatesInRowPartSub
testFilterUniqueCandidatesInRowPartSub:
        InitMaskRegister
        mov     rdx, rcx
        FilterUniqueCandidatesInRowPartSub rax, rdx, rbx, r12, r13, r14
        ret

# 横3マスにある唯一の候補を残して他は0にする
.macro FilterUniqueCandidatesInRowPart regDst, regRowPart, shift32bit, regWork1, regWork2, regWork3, regWork4
        popcnt \regWork2, \regRowPart
        mov    \regDst,   \regRowPart
        cmp    \regWork2, 3
        jz 3001f

        # できるだけ並列実行できるようにする
        .set   regBitmask,  \regWork1
        mov    regBitmask, gRegBitMask
        ShlNonZero  regBitmask, (\shift32bit * boxRowByteSize * bitPerByte)

        mov    \regWork3, \regRowPart
        and    \regWork3, regBitmask
        PowerOf2or0 \regDst, \regWork3, \regWork4

        FilterUniqueCandidatesInRowPartSub regBitmask, \regDst, \regRowPart, \regWork2, \regWork3, \regWork4
        FilterUniqueCandidatesInRowPartSub regBitmask, \regDst, \regRowPart, \regWork3, \regWork2, \regWork4
3001:
.endm

.macro TestFilterUniqueCandidatesInRowPart shift32bit
        InitMaskRegister
        FilterUniqueCandidatesInRowPart rbx, rax, \shift32bit, r11, r12, r13, r14
        ret
.endm

        .global testFilterUniqueCandidatesInRowPart0
        .global testFilterUniqueCandidatesInRowPart1
testFilterUniqueCandidatesInRowPart0:
        TestFilterUniqueCandidatesInRowPart 0
testFilterUniqueCandidatesInRowPart1:
        TestFilterUniqueCandidatesInRowPart 1

# 3マスにある唯一の候補を集める
.macro CollectUniqueCandidatesInRowPart regSum, regHasZero, regRowPart, regWork1, regWork2, regWork3, regWork4, regWork5
        .set   regZeroChecker, \regWork1
        .set   regElement0Src, \regWork2

        mov    regElement0Src, \regRowPart
        and    regElement0Src, gRegBitMask
        mov    regZeroChecker, regElement0Src
        PowerOf2or0 \regSum, regElement0Src, \regWork3

        # 並列実行できるよう次の3マスを計算する
        .set   regElement1Src, \regWork4
        .set   regElement1Dst, \regWork5
        mov    regElement1Src, \regRowPart
        ShrNonZero  regElement1Src, (candidatesNum * 1)
        and    regElement1Src, gRegBitMask

        # マスが0なら符号ビットが立つ
        FastDec regZeroChecker
        or      \regHasZero, regZeroChecker

        PowerOf2or0 regElement1Dst, regElement1Src, \regWork2
        # 次のマスが0かどうか調べる
        mov     regZeroChecker, regElement1Src
        or      \regSum, regElement1Dst
        FastDec regZeroChecker
        or      \regHasZero, regZeroChecker

        # 前の3マスと並列実行しない方がよい
        .set   regElement2Src, \regWork3
        .set   regElement2Dst, \regWork4
        mov    regElement2Src, \regRowPart
        ShrNonZero  regElement2Src, (candidatesNum * 2)
        # マスク不要

        mov     regZeroChecker, regElement2Src
        PowerOf2or0 regElement2Dst, regElement2Src, \regWork5
        FastDec regZeroChecker
        or      \regHasZero, regZeroChecker
        or      \regSum, regElement2Dst
.endm

        .global testCollectUniqueCandidatesInRowPart
testCollectUniqueCandidatesInRowPart:
        InitMaskRegister
        xor     rbx, rbx
        xor     rcx, rcx
        CollectUniqueCandidatesInRowPart rbx, rcx, rax, r10, r11, r12, r13, r14
        ret

# 1列にある唯一の候補を集める
.macro CollectUniqueCandidatesInLine regDst, regSrcX, regWork1, regWork2, regWork3, regWork4, regWork5, regWork6, regWork7, regWork8, regWork9
        .set   regHasZero,       \regWork1
        .set   regThreeElements, \regWork2
        .set   regSixToThreeElements,  \regWork3
        .set   regUniqueThreeElements, \regWork4

        MacroPextrq regThreeElements, \regSrcX, 1
        xor     regHasZero, regHasZero
        CollectUniqueCandidatesInRowPart \regDst, regHasZero, regThreeElements, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9

        MacroPextrq regSixToThreeElements, \regSrcX, 0
        mov     regThreeElements, regSixToThreeElements
        MaskLower32bit  regThreeElements
        CollectUniqueCandidatesInRowPart regUniqueThreeElements, regHasZero, regThreeElements, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9
        or      \regDst, regUniqueThreeElements

        ShrNonZero  regSixToThreeElements, (boxRowByteSize * bitPerByte)
        CollectUniqueCandidatesInRowPart regUniqueThreeElements, regHasZero, regSixToThreeElements, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9
        or      \regDst, regUniqueThreeElements

        # 空白があったら中断(ブランチは1行分まとめて)
        or      regHasZero, regHasZero
        js      abortFilling
.endm

        .global testCollectUniqueCandidatesInLine
testCollectUniqueCandidatesInLine:
        InitMaskRegister
        xor     rax, rax
        mov     qword ptr [rip + sudokuXmmAborted], 0
        CollectUniqueCandidatesInLine rax, xRegRow1, rbx, rcx, r8, r9, r10, r11, r12, r13, r14
        ret

# 9マスにある唯一の候補を残して他は0にする
.macro FilterUniqueCandidatesInLine regDstX, regSrcX, regWork1, regWork2, regWork3, regWork4, regWork5, regWork6, regWork7, regWorkX
        .set    regRowPartLow,   \regWork1
        .set    regRowPartHigh,  \regWork2
        .set    regElements1,    \regWork3
        .set    regElements2,    \regWork4

        MacroPextrq regRowPartLow, \regSrcX, 1
        FilterUniqueCandidatesInRowPart regElements1, regRowPartLow, 0, \regWork4, \regWork5, \regWork6, \regWork7
        MacroPinsrq \regDstX,  regElements1, 1

        MacroPextrq regRowPartHigh, \regSrcX, 0
        mov     regRowPartLow, regRowPartHigh
        FilterUniqueCandidatesInRowPart regElements1, regRowPartLow,  0, \regWork4, \regWork5, \regWork6, \regWork7
        FilterUniqueCandidatesInRowPart regElements2, regRowPartHigh, 1, regRowPartLow, \regWork5, \regWork6, \regWork7
        or     regElements2, regElements1
        MacroPinsrq \regDstX, regElements2, 0
.endm

        .global testFilterUniqueCandidatesInLine
testFilterUniqueCandidatesInLine:
        InitMaskRegister
        mov     qword ptr [rip + sudokuXmmAborted], 0
        FilterUniqueCandidatesInLine xRegWork2, xRegRow1, r8, r9, r10, r11, r12, r13, r14, xRegWork1
        ret

# 3行の候補をまとめる
.macro CollectUniqueCandidatesInThreeLine regDstX, regSrc1X, regSrc2X, regSrc3X, regWork1, regWork2, regWork3, regWork4, regWork5, regWork6, regWork7, regWork1X, regWork2X
        FilterUniqueCandidatesInLine \regDstX, \regSrc1X, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5, \regWork6, \regWork7, \regWork1X
        FilterUniqueCandidatesInLine \regWork2X, \regSrc2X, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5, \regWork6, \regWork7, \regWork1X
        orps   \regDstX, \regWork2X
        FilterUniqueCandidatesInLine \regWork2X, \regSrc3X, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5, \regWork6, \regWork7, \regWork1X
        orps   \regDstX, \regWork2X
.endm

        .global testCollectUniqueCandidatesInThreeLine
testCollectUniqueCandidatesInThreeLine:
        InitMaskRegister
        mov     qword ptr [rip + sudokuXmmAborted], 0
        CollectUniqueCandidatesInThreeLine xRegRow1to3, xRegRow1, xRegRow2, xRegRow3, r8, r9, r10, r11, r12, r13, r14, xRegWork1, xRegWork2
        ret

# 指定列の3マスを取り出す
.macro SelectRowParts regDst, regRowX, outBoxShift
        .if (\outBoxShift == 0)
        # もっといい方法があるはず
        MacroPextrq \regDst, \regRowX, 0
        MaskLower32bit  \regDst

        .elseif (\outBoxShift == 1)
        MacroPextrq \regDst, \regRowX, 0
        ShrNonZero  \regDst, (boxRowByteSize * bitPerByte)

        .elseif (\outBoxShift == 2)
        MacroPextrq \regDst, \regRowX, 1
        .endif
.endm

.macro TestSelectRowParts outBoxShift
        InitMaskRegister
        SelectRowParts rax, xRegRow1, \outBoxShift
        ret
.endm

        .global testSelectRowParts0
        .global testSelectRowParts1
        .global testSelectRowParts2
testSelectRowParts0:
        TestSelectRowParts 0
testSelectRowParts1:
        TestSelectRowParts 1
testSelectRowParts2:
        TestSelectRowParts 2

# 指定列の行を取り出してすべて確定しているかどうかをフラグを立てる(確定してたらZF=1)
.macro SelectRowAndCount regDstHigh, regDstLow, regRowX, regWork
        MacroPextrq \regDstHigh, \regRowX, 1
        mov     \regWork,    \regDstHigh
        MacroPextrq \regDstLow,  \regRowX, 0
        or      \regWork,    \regDstLow
        popcnt  \regWork,    \regWork
        cmp     \regWork, 9
.endm

        .global testSelectRowAndCount
testSelectRowAndCount:
        InitMaskRegister
        xor     rcx, rcx
        SelectRowAndCount rax, rbx, xRegRow1, r14
        jnz     testSelectRowAndCountA
        FastInc rcx
testSelectRowAndCountA:
        ret

# 指定列の1マスを取り出す
.macro SelectElementInRowParts regDst, regSrc, inBoxShift
        mov    \regDst, \regSrc
        ShrNonZero  \regDst, (candidatesNum * \inBoxShift)
        # 上位32bitにマスがないという前提
        .if (\inBoxShift != 2)
        and    \regDst, gRegBitMask
        .endif
.endm

.macro TestSelectElementInRowParts inBoxShift
        InitMaskRegister
        SelectElementInRowParts rbx, rax, \inBoxShift
        ret
.endm
        .global testSelectElementInRowParts0
        .global testSelectElementInRowParts1
        .global testSelectElementInRowParts2
testSelectElementInRowParts0:
        TestSelectElementInRowParts 0
testSelectElementInRowParts1:
        TestSelectElementInRowParts 1
testSelectElementInRowParts2:
        TestSelectElementInRowParts 2

# マスを埋めるマクロのラベルは1000番台(下から順)
# 唯一の候補があれば1マス埋める、そうでなければ候補を絞る
.macro FillUniqueElement regRowPart, regCandidateSet, regColumn, regMergedBox, regMergedRow, inBoxShift, regWork1, regWork2, regWork3, regWork4, regWork5, regWork1X, regWork2X
        .set   regBitmask,   \regWork1
        .set   regCandidate, \regWork2
        .set   regShiftedBitmask,   \regWork3
        .set   regShiftedCandidate, \regWork4
        .set   regUniqueCandidate,  \regWork5

        # ビットマスクの生成と他の命令を混ぜた方が速い
        mov    regBitmask, gRegBitMask
        mov    regShiftedBitmask, gRegBitMask
        ShlNonZero  regShiftedBitmask, (candidatesNum * \inBoxShift)

        # 行、列、3x3箱で使われている候補を集める
        mov    regCandidate, \regCandidateSet
        not    regCandidate
        and    regCandidate, regBitmask
        not    regBitmask

        # regCandidate = 他で使われていないもの
        or     regBitmask, regCandidate
        RolNonZero  regBitmask, (candidatesNum * \inBoxShift)
        # 他のマスは残す
        and    \regRowPart, regBitmask

        mov    regUniqueCandidate, \regRowPart
        and    regUniqueCandidate, regShiftedBitmask
        PowerOf2or0 regShiftedCandidate, regUniqueCandidate, \regWork1

        # マスク不要
        or     \regColumn, regShiftedCandidate
        # 箱または行のいずれか9マスで使われている候補
        ShrNonZero regShiftedCandidate, (candidatesNum * \inBoxShift)
        or     \regMergedBox, regShiftedCandidate
        or     \regMergedRow, regShiftedCandidate
.endm

.macro testFillUniqueElement inBoxShift
        InitMaskRegister
        mov     rbx, [rip + testFillUniqueElementPreRowPart]
        mov     rcx, [rip + testFillUniqueElementPreColumn]
        mov     rdx, [rip + testFillUniqueElementPreBox]
        mov     rsi, [rip + testFillUniqueElementPreRow]
        FillUniqueElement rbx, rax, rcx, rdx, rsi, \inBoxShift, r10, r11, r12, r13, r14, xRegWork1, xRegWork2
        ret
.endm

        .global testFillUniqueElement0
        .global testFillUniqueElement1
        .global testFillUniqueElement2
testFillUniqueElement0:
        testFillUniqueElement 0
testFillUniqueElement1:
        testFillUniqueElement 1
testFillUniqueElement2:
        testFillUniqueElement 2

.macro FillOneUniqueCandidates regRowPart, regMergedBox, regMergedRow, regColumn, inBoxShift, regWork1, regWork2, regWork3, regWork4, regWork5, regWork6, regWork1X, regWork2X
        .set   regElement, \regWork1
        SelectElementInRowParts regElement, \regRowPart, \inBoxShift
        IsPowerOf2ToFlags regElement, \regWork2
        jbe    1131f

        .set   regCandidateSet, \regWork1
        SelectElementInRowParts regCandidateSet, \regColumn, \inBoxShift
        or     regCandidateSet, \regMergedBox
        or     regCandidateSet, \regMergedRow
        FillUniqueElement \regRowPart, regCandidateSet, \regColumn, \regMergedBox, \regMergedRow, \inBoxShift, \regWork2, \regWork3, \regWork4, \regWork5, \regWork6, \regWork1X, \regWork2X
1131:
.endm

.macro testFillOneUniqueCandidates inBoxShift
        InitMaskRegister
        mov     rax, [rip + testFillUniqueElementPreRowPart]
        mov     rbx, [rip + testFillUniqueElementPreColumn]
        mov     rcx, [rip + testFillUniqueElementPreBox]
        mov     rdx, [rip + testFillUniqueElementPreRow]
        FillOneUniqueCandidates rax, rcx, rdx, rbx, \inBoxShift, r9, r10, r11, r12, r13, r14, xRegWork1, xRegWork2
        ret
.endm

        .global testFillOneUniqueCandidates0
        .global testFillOneUniqueCandidates1
        .global testFillOneUniqueCandidates2
testFillOneUniqueCandidates0:
        testFillOneUniqueCandidates 0
testFillOneUniqueCandidates1:
        testFillOneUniqueCandidates 1
testFillOneUniqueCandidates2:
        testFillOneUniqueCandidates 2

.macro SaveLineSet regNewElement, regResultBoxX, regResultRowX, outBoxShift, regWork1, regWork2, regWork3, regWork4, regWork1X, regWork2X
        .set  regCandidate,  \regWork1
        .set  regBitmask,    \regWork2
        .set  regCandidateX, \regWork1X
        .set  regBitmaskX,   \regWork2X

        # 全bitを1にする
.if (EnableAvx != 0)
        vpcmpeqw \regWork1X, \regWork1X, \regWork1X
.else
        pcmpeqw  \regWork1X, \regWork1X
.endif

        # ビットマスク
        mov    regBitmask,  rowPartBitMask
        MacroMovq regBitmaskX, regBitmask
        PslldqNonZero regBitmaskX, (boxRowByteSize * \outBoxShift)
        MacroAndnps regBitmaskX, \regWork1X

        mov    regCandidate, \regNewElement
        MacroMovq regCandidateX, regCandidate
        PslldqNonZero regCandidateX, (boxRowByteSize * \outBoxShift)
        MacroOrps  regBitmaskX, regCandidateX
        MacroAndps \regResultRowX, regBitmaskX

        popcnt \regWork4, \regNewElement
        cmp    \regWork4, 3
        # cmovで0を代入するより、分岐したほうが速い
        ja     1121f

        MacroMovq regCandidateX, \regNewElement
        PslldqNonZero regCandidateX, (boxRowByteSize * \outBoxShift)
        MacroOrps xRegRowAll, regCandidateX
        MacroOrps \regResultBoxX, regCandidateX
1121:
.endm

.macro TestSaveLineSet inBoxShift
        InitMaskRegister
        SaveLineSet rax, xRegRow1to3, xRegRow1, \inBoxShift, r11, r12, r13, r14, xRegWork1, xRegWork2
        ret
.endm

        .global testSaveLineSet0
        .global testSaveLineSet1
        .global testSaveLineSet2
testSaveLineSet0:
        TestSaveLineSet 0
testSaveLineSet1:
        TestSaveLineSet 1
testSaveLineSet2:
        TestSaveLineSet 2

# 他で使われていない候補が一つしかなければそれに絞る
.macro MaskElementCandidates regRowPart, regOtherCandidates, inBoxShift, regWork1, regWork2, regWork3
        .set regAllCandidates, \regWork1

        mov  regAllCandidates, \regOtherCandidates
        not  regAllCandidates
        and  regAllCandidates, gRegBitMask
        PowerOf2orAll1 \regWork2, regAllCandidates, \regWork3

        mov  \regWork1, gRegBitMask
        not  \regWork1
        or   \regWork1, \regWork2
        RolNonZero  \regWork1, (candidatesNum * \inBoxShift)
        and  \regRowPart, \regWork1
.endm

.macro TestMaskElementCandidates inBoxShift
        InitMaskRegister
        mov     rcx, rax
        MaskElementCandidates rcx, rbx, \inBoxShift, r12, r13, r14
        ret
.endm

        .global testMaskElementCandidates0
        .global testMaskElementCandidates1
        .global testMaskElementCandidates2
testMaskElementCandidates0:
        TestMaskElementCandidates 0
testMaskElementCandidates1:
        TestMaskElementCandidates 1
testMaskElementCandidates2:
        TestMaskElementCandidates 2

# 2マス分の候補を1つにまとめる(破壊的)
.macro MergeTwoElementsImpl regSum, shiftLeft1, shiftLeft2, inBoxShift, regWork
        # マスク並列化
        .if (\inBoxShift == 0)
        mov    \regWork, \regSum
        ShrNonZero  \regSum,  (candidatesNum * 1)
        and    \regSum, gRegBitMask
        ShrNonZero  \regWork, (candidatesNum * 2)
        or     \regSum, \regWork
        .elseif (\inBoxShift == 1)
        mov    \regWork, \regSum
        ShrNonZero  \regWork, (candidatesNum * 2)
        and    \regSum, gRegBitMask
        or     \regSum, \regWork
        .else
        # 一般解
        mov    \regWork, \regSum
        ShrNonZero  \regWork, (candidatesNum * \shiftLeft2)
        ShrNonZero  \regSum,  (candidatesNum * \shiftLeft1)
        or     \regSum, \regWork
        and    \regSum, gRegBitMask
        .endif
.endm

.macro MergeTwoElements regSum, inBoxShift, regWork
        .if (\inBoxShift == 0)
        MergeTwoElementsImpl \regSum, 1, 2, 0, \regWork
        .elseif (\inBoxShift == 1)
        MergeTwoElementsImpl \regSum, 0, 2, 1, \regWork
        .elseif (\inBoxShift == 2)
        MergeTwoElementsImpl \regSum, 0, 1, 2, \regWork
        .endif
.endm

.macro TestMergeTwoElements inBoxShift
        InitMaskRegister
        mov     rbx, rax
        MergeTwoElements rbx, \inBoxShift, r14
        ret
.endm
        .global testMergeTwoElements0
        .global testMergeTwoElements1
        .global testMergeTwoElements2
testMergeTwoElements0:
        TestMergeTwoElements 0
testMergeTwoElements1:
        TestMergeTwoElements 1
testMergeTwoElements2:
        TestMergeTwoElements 2

# 1列の1マスにある候補を集める
.macro FindElementCandidates regRowPart, regRowCandidates, regBox, regColumnRowPart, inBoxShift, regWork1, regWork2, regWork3, regWork4, regWork5, regWork1X, regWork2X
        .set  regTwoElements, \regWork1
        .set  regElement, \regWork2
        SelectElementInRowParts regElement, \regRowPart, \inBoxShift
        IsPowerOf2ToFlags regElement, \regWork3
        jbe 1081f

        # 行
        mov  regElement, \regRowPart
        MergeTwoElements regElement, \inBoxShift, \regWork3
        mov  regTwoElements, regElement
        or   regElement, \regRowCandidates
        MaskElementCandidates \regRowPart, regElement, \inBoxShift, \regWork3, \regWork4, \regWork5

        mov  \regWork3, \regRowPart
        SelectElementInRowParts regElement, \regWork3, \inBoxShift
        IsPowerOf2ToFlags regElement, \regWork3
        jbe 1081f

        # 3x3箱
        mov  regElement, regTwoElements
        or   regElement, \regBox
        MaskElementCandidates \regRowPart, regElement, \inBoxShift, \regWork3, \regWork4, \regWork5

        mov  \regWork3, \regRowPart
        SelectElementInRowParts regElement, \regWork3, \inBoxShift
        IsPowerOf2ToFlags regElement, \regWork3
        jbe 1081f

        # 列
        SelectElementInRowParts regElement, \regColumnRowPart, \inBoxShift
        MaskElementCandidates \regRowPart, regElement, \inBoxShift, \regWork3, \regWork4, \regWork5
1081:
.endm

.macro TestFindElementCandidates inBoxShift
        InitMaskRegister
        mov     rsi, rax
        FindElementCandidates rsi, rbx, rcx, rdx, \inBoxShift, r10, r11, r12, r13, r14, xRegWork1, xRegWor2k
        ret
.endm

        .global testFindElementCandidates0
        .global testFindElementCandidates1
        .global testFindElementCandidates2
testFindElementCandidates0:
        TestFindElementCandidates 0
testFindElementCandidates1:
        TestFindElementCandidates 1
testFindElementCandidates2:
        TestFindElementCandidates 2

# 1列の3マスにある候補を集める
.macro FindThreePartsCandidates regRowPart, regCandidates, regBox, regColumnRowPart, regWork1, regWork2, regWork3, regWork4, regWork5, regWork1X, regWork2X
        FindElementCandidates \regRowPart, \regCandidates, \regBox, \regColumnRowPart, 2, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5, \regWork1X, \regWork2X
        FindElementCandidates \regRowPart, \regCandidates, \regBox, \regColumnRowPart, 1, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5, \regWork1X, \regWork2X
        FindElementCandidates \regRowPart, \regCandidates, \regBox, \regColumnRowPart, 0, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5, \regWork1X, \regWork2X
.endm

        .global testFindThreePartsCandidates
testFindThreePartsCandidates:
        InitMaskRegister
        mov     rsi, rax
        FindThreePartsCandidates rsi, rbx, rcx, rdx, r10, r11, r12, r13, r14, xRegWork1, xRegWork2
        ret

# 1列、1行および3x3箱にある唯一の候補を集める
.macro FillThreePartsUniqueCandidates regMergedRow, regBoxX, regRowX, regColumnX, outBoxShift, regWork1, regWork2, regWork3, regWork4, regWork5, regWork6, regWork7, regWork8, regWork9, regWork1X, regWork2X
        .set  regMergedBox, \regWork1
        .set  regColumn,    \regWork2
        .set  regRowPart,   \regWork3

        SelectRowParts regRowPart, \regRowX, \outBoxShift
        popcnt \regWork4, regRowPart
        cmp    \regWork4, 3
        jbe    1061f

        # 行の結果は汎用レジスタにおいて使いまわす
        SelectRowParts regMergedBox, \regBoxX, \outBoxShift
        SelectRowParts regColumn, \regColumnX, \outBoxShift
        MergeThreeElements2 regMergedBox, \regWork4, \regWork5

        FillOneUniqueCandidates regRowPart, regMergedBox, \regMergedRow, regColumn, 2, \regWork4, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9, \regWork1X, \regWork2X
        FillOneUniqueCandidates regRowPart, regMergedBox, \regMergedRow, regColumn, 1, \regWork4, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9, \regWork1X, \regWork2X
        FillOneUniqueCandidates regRowPart, regMergedBox, \regMergedRow, regColumn, 0, \regWork4, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9, \regWork1X, \regWork2X

        # findThreePartsCandidatesをここで呼び出すと却って遅くなるので使わない
        SaveLineSet regRowPart, \regBoxX, \regRowX, \outBoxShift, \regWork4, \regWork5, \regWork6, \regWork7, \regWork1X, \regWork2X
1061:
.endm

.macro TestFillThreePartsUniqueCandidates outBoxShift
        InitMaskRegister
        FillThreePartsUniqueCandidates rax, xRegRow1to3, xRegRow1, xRegRowAll, \outBoxShift, rdx, rsi, r8, r9, r10, r11, r12, r13, r14, xRegWork1, xRegWork2
        MacroMovdqa ([rip + testFillNineUniqueCandidatesRowX]),    xmm1
        MacroMovdqa ([rip + testFillNineUniqueCandidatesBoxX]),    xmm10
        MacroMovdqa ([rip + testFillNineUniqueCandidatesColumnX]), xmm0
        ret
.endm

        .global testFillThreePartsUniqueCandidates0
        .global testFillThreePartsUniqueCandidates1
        .global testFillThreePartsUniqueCandidates2
testFillThreePartsUniqueCandidates0:
        TestFillThreePartsUniqueCandidates 0
testFillThreePartsUniqueCandidates1:
        TestFillThreePartsUniqueCandidates 1
testFillThreePartsUniqueCandidates2:
        TestFillThreePartsUniqueCandidates 2

.macro FillRowPartCandidates regMergedRow, regBoxX, regRowX, regColumnX, outBoxShiftTarget, outBoxShiftOtherA, outBoxShiftOtherB, regWork1, regWork2, regWork3, regWork4, regWork5, regWork6, regWork7, regWork8, regWork9, regWork10, regWork11, regWork1X, regWork2X
        # 以前はfindThreePartsCandidatesを呼び出すために中間データをここでまとめていたが、今はやらない
        FillThreePartsUniqueCandidates \regMergedRow, \regBoxX, \regRowX, \regColumnX, \outBoxShiftTarget, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9, \regWork1X, \regWork2X
.endm

.macro TestFillRowPartCandidates outBoxShiftTarget, outBoxShiftOtherA, outBoxShiftOtherB
        InitMaskRegister
        mov     rax, [rip + testFillNineUniqueCandidatesPreRow]
        FillRowPartCandidates rax, xRegRow1to3, xRegRow1, xRegRowAll, \outBoxShiftTarget, \outBoxShiftOtherA, \outBoxShiftOtherB, rbx, rcx, rdx, rsi, r8, r9, r10, r11, r12, r13, r14, xRegWork1, xRegWork2
        MacroMovdqa ([rip + testFillNineUniqueCandidatesRowX]),    xmm1
        MacroMovdqa ([rip + testFillNineUniqueCandidatesBoxX]),    xmm10
        MacroMovdqa ([rip + testFillNineUniqueCandidatesColumnX]), xmm0
        ret
.endm

        .global testTestFillRowPartCandidates0
        .global testTestFillRowPartCandidates1
        .global testTestFillRowPartCandidates2
testTestFillRowPartCandidates0:
        TestFillRowPartCandidates 0 1 2
testTestFillRowPartCandidates1:
        TestFillRowPartCandidates 1 2 0
testTestFillRowPartCandidates2:
        TestFillRowPartCandidates 2 0 1

# 1列、1行および3x3箱にある唯一の候補を集める
.macro FillNineUniqueCandidates regBoxX, regRowX, regColumnX, regWork1, regWork2, regWork3, regWork4, regWork5, regWork6, regWork7, regWork8, regWork9, regWork10, regWork11, regWork12, regWork1X, regWork2X
        .set  regMergedRow, \regWork1
        CollectUniqueCandidatesInLine  regMergedRow, \regRowX, \regWork4, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9, \regWork10, \regWork11, \regWork12
        # ここで9bit立っているからといって抜けると正しく動作しないのは、複数マクロで同じラベルを用いていたから
        popcnt \regWork2, regMergedRow
        cmp    \regWork2, 9
        jz     1041f

        FillRowPartCandidates regMergedRow, \regBoxX, \regRowX, \regColumnX, 2, 1, 0, \regWork2, \regWork3, \regWork4, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9, \regWork10, \regWork11, \regWork12, \regWork1X, \regWork2X
        # ここでCollectUniqueCandidatesInLineを呼び出して9bit立っているかどうか調べると却って遅くなる
        FillRowPartCandidates regMergedRow, \regBoxX, \regRowX, \regColumnX, 1, 2, 0, \regWork2, \regWork3, \regWork4, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9, \regWork10, \regWork11, \regWork12, \regWork1X, \regWork2X
        FillRowPartCandidates regMergedRow, \regBoxX, \regRowX, \regColumnX, 0, 2, 1, \regWork2, \regWork3, \regWork4, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9, \regWork10, \regWork11, \regWork12, \regWork1X, \regWork2X
1041:
.endm

        .global testFillNineUniqueCandidates
testFillNineUniqueCandidates:
        InitMaskRegister
        FillNineUniqueCandidates xRegRow1to3, xRegRow1, xRegRowAll, rax, rbx, rcx, rdx, rsi, r8, r9, r10, r11, r12, r13, r14, xRegWork1, xRegWork2
        MacroMovdqa ([rip + testFillNineUniqueCandidatesRowX]),    xmm1
        MacroMovdqa ([rip + testFillNineUniqueCandidatesBoxX]),    xmm10
        MacroMovdqa ([rip + testFillNineUniqueCandidatesColumnX]), xmm0
        ret

.macro Collect27UniqueCandidates regBoxX, regRow1X, regRow2X, regRow3X, regColumnX, regWork1, regWork2, regWork3, regWork4, regWork5, regWork6, regWork7, regWork8, regWork9, regWork10, regWork11, regWork12, regWork13, regWork1X, regWork2X
        FillNineUniqueCandidates \regBoxX, \regRow1X, \regColumnX, \regWork2, \regWork3, \regWork4, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9, \regWork10, \regWork11, \regWork12, \regWork13, \regWork1X, \regWork2X
        FillNineUniqueCandidates \regBoxX, \regRow2X, \regColumnX, \regWork2, \regWork3, \regWork4, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9, \regWork10, \regWork11, \regWork12, \regWork13, \regWork1X, \regWork2X
        FillNineUniqueCandidates \regBoxX, \regRow3X, \regColumnX, \regWork2, \regWork3, \regWork4, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9, \regWork10, \regWork11, \regWork12, \regWork13, \regWork1X, \regWork2X
.endm

# 1列、1行および3x3箱にある唯一の候補を集める
.macro CollectUniqueCandidates regWork1, regWork2, regWork3, regWork4, regWork5, regWork6, regWork7, regWork8, regWork9, regWork10, regWork11, regWork12, regWork13, regWork1X, regWork2X
        Collect27UniqueCandidates xRegRow1to3, xRegRow1, xRegRow2, xRegRow3, xRegRowAll, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9, \regWork10, \regWork11, \regWork12, \regWork13, \regWork1X, \regWork2X
        Collect27UniqueCandidates xRegRow4to6, xRegRow4, xRegRow5, xRegRow6, xRegRowAll, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9, \regWork10, \regWork11, \regWork12, \regWork13, \regWork1X, \regWork2X
        Collect27UniqueCandidates xRegRow7to9, xRegRow7, xRegRow8, xRegRow9, xRegRowAll, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9, \regWork10, \regWork11, \regWork12, \regWork13, \regWork1X, \regWork2X
.endm

.macro FindRowPartCandidates regRowPartTarget, regRowPartLeft, regRowPartCandidatesTarget, regRowPartCandidatesLeft, regRowPartCandidatesOther, regRowCandidates, regBox, regBoxX, regColumnX, outBoxShift, regWork1, regWork2, regWork3, regWork4, regWork5, regWork1X, regWork2X
        SelectRowParts      \regBox, \regBoxX,  \outBoxShift
        MergeThreeElements2 \regBox, \regWork1, \regWork2

        mov  \regRowPartCandidatesLeft, \regRowPartLeft
        MergeThreeElements2 \regRowPartCandidatesLeft, \regWork3, \regWork4

        mov  \regRowCandidates, \regRowPartCandidatesLeft
        or   \regRowCandidates, \regRowPartCandidatesOther
        SelectRowParts \regRowPartCandidatesTarget, \regColumnX, \outBoxShift
        FindThreePartsCandidates \regRowPartTarget, \regRowCandidates, \regBox, \regRowPartCandidatesTarget, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5
.endm

.macro TestFindRowPartCandidates outBoxShift
        InitMaskRegister
        mov     rsi, [rip + testFindRowPartCandidatesPreRowPartTarget]
        mov     r8,  [rip + testFindRowPartCandidatesPreRowCandidates]
        mov     r9,  [rip + testFindRowPartCandidatesPreBox]
        FindRowPartCandidates rsi, rax, rbx, rcx, rdx, r8, r9, xRegRow1to3, xRegRowAll, \outBoxShift, r10, r11, r12, r13, r14, xRegWork1, xRegWork2
        mov     [rip + testFindRowPartCandidatesRowPartTarget], rsi
        mov     [rip + testFindRowPartCandidatesRowCandidates], r8
        mov     [rip + testFindRowPartCandidatesBox], r9
        ret
.endm

        .global testFindRowPartCandidates0
        .global testFindRowPartCandidates1
        .global testFindRowPartCandidates2
testFindRowPartCandidates0:
        TestFindRowPartCandidates 0
testFindRowPartCandidates1:
        TestFindRowPartCandidates 1
testFindRowPartCandidates2:
        TestFindRowPartCandidates 2

# 1列、1行および3x3箱にある候補を絞る
.macro FindNineCandidates regTargetRowX, regRow1X, regRow2X, regBoxX, regThreeRow1X, regThreeRow2X, regColumnX, regWork1, regWork2, regWork3, regWork4, regWork5, regWork6, regWork7, regWork8, regWork9, regWork10, regWork11, regWork12, regWork13, regWork1X, regWork2X
        .set  regRowPart2, \regWork1
        .set  regRowPart1, \regWork2
        .set  regRowPart0, \regWork3
        .set  regRowPartCandidates2, \regWork4
        .set  regRowPartCandidates1, \regWork5
        .set  regRowPartCandidates0, \regWork6
        .set  regRowCandidates, \regWork7
        .set  regBox, \regWork8

        MacroOrps3op \regBoxX, \regRow1X, \regRow2X
        OrThreeXmmRegs \regColumnX, \regBoxX, \regThreeRow1X, \regThreeRow2X

        # selectRowPartsの繰り返しをまとめる
        SelectRowAndCount regRowPart2, regRowPart1, \regTargetRowX, \regWork9
        jz   1001f
        SplitRowLowParts  regRowPart0, regRowPart1

        mov  regRowPartCandidates1, regRowPart1
        mov  regRowPartCandidates0, regRowPart0
        MergeThreeElements2 regRowPartCandidates1, \regWork9, \regWork10
        MergeThreeElements2 regRowPartCandidates0, \regWork9, \regWork10
        mov  regRowCandidates, regRowPartCandidates1
        or   regRowCandidates, regRowPartCandidates0

        SelectRowParts regBox, \regBoxX, 2
        MergeThreeElements2 regBox, \regWork9, \regWork10

        SelectRowParts regRowPartCandidates2, \regColumnX, 2
        # regRowPartCandidates2が伝搬していない?
        FindThreePartsCandidates regRowPart2, regRowCandidates, regBox, regRowPartCandidates2, \regWork9, \regWork10, \regWork11, \regWork12, \regWork13, \regWork1X, \regWork2X
        FindRowPartCandidates  regRowPart1, regRowPart2, regRowPartCandidates1, regRowPartCandidates2, regRowPartCandidates0, regRowCandidates, regBox, \regBoxX, \regColumnX, 1, \regWork9, \regWork10, \regWork11, \regWork12, \regWork13, \regWork1X, \regWork2X
        FindRowPartCandidates  regRowPart0, regRowPart1, regRowPartCandidates0, regRowPartCandidates1, regRowPartCandidates2, regRowCandidates, regBox, \regBoxX, \regColumnX, 0, \regWork9, \regWork10, \regWork11, \regWork12, \regWork13, \regWork1X, \regWork2X

        MacroPinsrq \regTargetRowX, regRowPart2, 1
        ShlNonZero  regRowPart1, (boxRowByteSize * bitPerByte)
        or      regRowPart1, regRowPart0
        MacroPinsrq \regTargetRowX, regRowPart1, 0
1001:
.endm

        .global testFindNineCandidates
testFindNineCandidates:
        InitMaskRegister
        MacroXorps xRegRow4to6, xRegRow4to6
        MacroXorps xRegRow7to9, xRegRow7to9
        FindNineCandidates xRegRow1, xRegRow2, xRegRow3, xRegRow1to3, xRegRow4to6, xRegRow7to9, xRegRowAll, rax, rbx, rcx, rdx, rsi, rdi, r8, r9, r10, r11, r12, r13, r14, xRegWork1, xRegWork2
        MacroMovdqa ([rip + testFillNineUniqueCandidatesRowX]),    xmm1
        MacroMovdqa ([rip + testFillNineUniqueCandidatesBoxX]),    xmm10
        MacroMovdqa ([rip + testFillNineUniqueCandidatesColumnX]), xmm0
        ret

.macro Find27UniqueCandidates regRow1X, regRow2X, regRow3X, regBoxX, regThreeRow1X, regThreeRow2X, regColumnX, regWork1, regWork2, regWork3, regWork4, regWork5, regWork6, regWork7, regWork8, regWork9, regWork10, regWork11, regWork12, regWork13, regWork1X, regWork2X
        FindNineCandidates \regRow1X, \regRow2X, \regRow3X, \regBoxX, \regThreeRow1X, \regThreeRow2X, \regColumnX, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9, \regWork10, \regWork11, \regWork12, \regWork13, \regWork1X, \regWork2X
        FindNineCandidates \regRow2X, \regRow1X, \regRow3X, \regBoxX, \regThreeRow1X, \regThreeRow2X, \regColumnX, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9, \regWork10, \regWork11, \regWork12, \regWork13, \regWork1X, \regWork2X
        FindNineCandidates \regRow3X, \regRow1X, \regRow2X, \regBoxX, \regThreeRow1X, \regThreeRow2X, \regColumnX, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9, \regWork10, \regWork11, \regWork12, \regWork13, \regWork1X, \regWork2X
.endm

# 1列、1行および3x3箱にある候補を絞る
.macro FindCandidates regWork1, regWork2, regWork3, regWork4, regWork5, regWork6, regWork7, regWork8, regWork9, regWork10, regWork11, regWork12, regWork13, regWork1X, regWork2X
        OrThreeXmmRegs xRegRow4to6, xRegRow4, xRegRow5, xRegRow6
        OrThreeXmmRegs xRegRow7to9, xRegRow7, xRegRow8, xRegRow9
        Find27UniqueCandidates xRegRow1, xRegRow2, xRegRow3, xRegRow1to3, xRegRow4to6, xRegRow7to9, xRegRowAll, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9, \regWork10, \regWork11, \regWork12, \regWork13, \regWork1X, \regWork2X

        OrThreeXmmRegs xRegRow1to3, xRegRow1, xRegRow2, xRegRow3
        Find27UniqueCandidates xRegRow4, xRegRow5, xRegRow6, xRegRow4to6, xRegRow1to3, xRegRow7to9, xRegRowAll, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9, \regWork10, \regWork11, \regWork12, \regWork13, \regWork1X, \regWork2X

        OrThreeXmmRegs xRegRow4to6, xRegRow4, xRegRow5, xRegRow6
        Find27UniqueCandidates xRegRow7, xRegRow8, xRegRow9, xRegRow7to9, xRegRow1to3, xRegRow4to6, xRegRowAll, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9, \regWork10, \regWork11, \regWork12, \regWork13, \regWork1X, \regWork2X
.endm

# 何マス埋まったか調べる(前回の結果をregPrevPopcntに退避する)
.macro CountFilledElements reg64LoopCnt, reg64CurrentPopcnt, reg64PrevPopcnt, regWork
        # もしかしたらメモリの方が速いかもしれない
        MacroPextrq \reg64PrevPopcnt, xLoopPopCnt, 0
        MacroPextrq \reg64LoopCnt,    xLoopPopCnt, 1
        MacroPextrq \reg64CurrentPopcnt, xRegRowAll, 0
        MacroPextrq \regWork,            xRegRowAll, 1
        popcnt  \reg64CurrentPopcnt, \reg64CurrentPopcnt
        popcnt  \regWork, \regWork
        add     \reg64CurrentPopcnt, \regWork
.endm

        .global testCountFilledElements
testCountFilledElements:
        CountFilledElements rax, rbx, rcx, r14
        ret

# 前回の結果を保存する
    .macro SaveLoopCnt reg64LoopCnt, regCurrentPopcnt
        MacroPinsrq xLoopPopCnt, \regCurrentPopcnt, 0
        MacroPinsrq xLoopPopCnt, \reg64LoopCnt, 1
.endm

        .global testSaveLoopCnt
testSaveLoopCnt:
        SaveLoopCnt rax, rbx
        ret

# ----------------------------------------------------------------------------------------------------
# バックトラッキングでマスを埋めるマクロのラベルは3桁(下から順)
# 指定ビットより右にマスクを指定位置に設定する

# マスの候補数を数える
# regPopCount = 0または2のべき乗のときは大きな値、それ以外は立っているビット数
# regPopCount += 立っているビット数
.macro PopCountOrPowerOf2 regPopCount, regAccumPopCount, regSrc, inBoxShift, regWork1
        SelectElementInRowParts  \regWork1, \regSrc, \inBoxShift
        popcnt  \regPopCount, \regWork1
        mov     \regWork1, candidatesTooMany
        add     \regAccumPopCount, \regPopCount
        cmp     \regPopCount, 1
        cmovbe  \regPopCount, \regWork1
.endm

.macro TestPopCountOrPowerOf2 inBoxShift
        InitMaskRegister
        mov  rbx, rdx
        PopCountOrPowerOf2 rax, rbx, rcx, \inBoxShift, r8
        ret
.endm

        .global testPopCountOrPowerOf20
        .global testPopCountOrPowerOf21
        .global testPopCountOrPowerOf22
testPopCountOrPowerOf20:
        TestPopCountOrPowerOf2 0

testPopCountOrPowerOf21:
        TestPopCountOrPowerOf2 1

testPopCountOrPowerOf22:
        TestPopCountOrPowerOf2 2

# High:Low(幅はbitWidth)の組について、Left > Rightの辞書順に比較する
.macro CompareRegisterSet regLeftHigh, regLeftLow, regRightHigh, regRightLow, bitWidth, regWork1, regWork2
        mov  \regWork1, \regLeftHigh
        ShlNonZero  \regWork1, \bitWidth
        or   \regWork1, \regLeftLow
        mov  \regWork2, \regRightHigh
        ShlNonZero  \regWork2, \bitWidth
        or   \regWork2, \regRightLow
        cmp  \regWork1, \regWork2
.endm

        .global testCompareRegisterSet
testCompareRegisterSet:
        InitMaskRegister
        xor  rax, rax
        xor  rbx, rbx
        mov  r10, 1
        CompareRegisterSet rcx, rdx, rsi, rdi, 16, r8, r9
        cmovz  rax, r10
        cmovc  rbx, r10
        ret

# マスの候補数がregMinCount以下であれば、そのマスの位置を返す
.macro CountCellCandidates regOutBoxShift, regInBoxShift, regMinCount, regRowPopCount, regSrc, outBoxShift, inBoxShift, regWork1, regWork2, regWork3
        PopCountOrPowerOf2  \regWork1, \regRowPopCount, \regSrc, \inBoxShift, \regWork2
        mov  \regWork2, \outBoxShift
        mov  \regWork3, \inBoxShift
        cmp  \regWork1, \regMinCount
        cmovbe  \regOutBoxShift, \regWork2
        cmovbe  \regInBoxShift, \regWork3
        cmovbe  \regMinCount, \regWork1
.endm

.set  testCountCellCandidatesInvalidShift, 0xffff
.set  testCountCellCandidatesInvalidRowNumber, 0xfffe
.set  testCountCellCandidatesRowNumber, 5

.macro TestCountCellCandidates outBoxShift, inBoxShift
        InitMaskRegister
        mov  rax, testCountCellCandidatesInvalidShift
        mov  rbx, testCountCellCandidatesInvalidShift
        mov  rcx, [rsi]
        mov  rdx, [rsi + 8]
        CountCellCandidates rax, rbx, rcx, rdx, rdi, \outBoxShift, \inBoxShift, r8, r9, r10
        ret
.endm

        .global testCountCellCandidates10
        .global testCountCellCandidates21
        .global testCountCellCandidates02
testCountCellCandidates10:
        TestCountCellCandidates 1 0

testCountCellCandidates21:
        TestCountCellCandidates 2 1

testCountCellCandidates02:
        TestCountCellCandidates 0 2

.macro CountThreeCellCandidates regOutBoxShift, regInBoxShift, regMinCount, regRowPopCount, regSrc, outBoxShift, regWork1, regWork2, regWork3
        CountCellCandidates \regOutBoxShift, \regInBoxShift, \regMinCount, \regRowPopCount, \regSrc, \outBoxShift, 0, \regWork1, \regWork2, \regWork3
        CountCellCandidates \regOutBoxShift, \regInBoxShift, \regMinCount, \regRowPopCount, \regSrc, \outBoxShift, 1, \regWork1, \regWork2, \regWork3
        CountCellCandidates \regOutBoxShift, \regInBoxShift, \regMinCount, \regRowPopCount, \regSrc, \outBoxShift, 2, \regWork1, \regWork2, \regWork3
.endm

        .global testCountThreeCellCandidates
testCountThreeCellCandidates:
        InitMaskRegister
        mov  rax, testCountCellCandidatesInvalidShift
        mov  rbx, testCountCellCandidatesInvalidShift
        mov  rcx, [rsi]
        mov  rdx, [rsi + 8]
        CountThreeCellCandidates rax, rbx, rcx, rdx, rdi, 2, r8, r9, r10
        ret

.macro CountRowCellCandidatesSub regOutBoxShift, regInBoxShift, regMinCount, regRowPopCount, xRegRow, regWork1, regWork2, regWork3, regWork4, regWork5
        xor  \regRowPopCount, \regRowPopCount
        MacroPextrq  \regWork1, \xRegRow, 0
        SplitRowLowParts  \regWork2, \regWork1
        CountThreeCellCandidates \regOutBoxShift, \regInBoxShift, \regMinCount, \regRowPopCount, \regWork2, 0, \regWork3, \regWork4, \regWork5
        CountThreeCellCandidates \regOutBoxShift, \regInBoxShift, \regMinCount, \regRowPopCount, \regWork1, 1, \regWork3, \regWork4, \regWork5
        MacroPextrq  \regWork1, \xRegRow, 1
        CountThreeCellCandidates \regOutBoxShift, \regInBoxShift, \regMinCount, \regRowPopCount, \regWork1, 2, \regWork3, \regWork4, \regWork5
.endm

        .global testCountRowCellCandidatesSub
testCountRowCellCandidatesSub:
        InitMaskRegister
        mov  rax, testCountCellCandidatesInvalidShift
        mov  rbx, testCountCellCandidatesInvalidShift
        mov  rcx, [rip + testCountRowCellCandidatesMinCount]
        MacroMovdqa xRegRow1, [rip + testCountRowCellCandidatesRowX]
        CountRowCellCandidatesSub rax, rbx, rcx, rdx, xRegRow1, r8, r9, r10, r11, r12
        ret

.macro CountRowCellCandidates xRegRowNumber, regOutBoxShift, regInBoxShift, regMinCount, regRowPopCount, xRegRow, rowNumber, regWork1, regWork2, regWork3, regWork4, regWork5, regWork6, regWork7, regWork8, regWork9
        mov  \regWork1, outBoxIndexInvalid
        mov  \regWork2, inBoxIndexInvalid
        mov  \regWork3, \regMinCount
        CountRowCellCandidatesSub  \regWork1, \regWork2, \regWork3, \regWork4, \xRegRow, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9

        xor  \regWork5, \regWork5
        mov  \regWork6, 1
        cmp  \regWork1, outBoxIndexInvalid
        cmovz  \regWork5, \regWork6
        CompareRegisterSet  \regMinCount, \regRowPopCount, \regWork3, \regWork4, 16, \regWork7, \regWork8
        cmovbe \regWork5, \regWork6

        or  \regWork5, \regWork5
        jnz 301f

        mov  \regOutBoxShift, \regWork1
        mov  \regInBoxShift, \regWork2
        mov  \regWork5, \rowNumber
        MacroPinsrq  \xRegRowNumber, \regWork5, 0
        mov \regMinCount, \regWork3
        mov \regRowPopCount, \regWork4
        cmp \regWork4, (candidatesNum + 2)
        jz  302f
301:
.endm

        .global testCountRowCellCandidates
testCountRowCellCandidates:
        InitMaskRegister
        mov  rax, testCountCellCandidatesInvalidRowNumber
        MacroPinsrq xRegWork1, rax, 0
        mov  rax, testCountCellCandidatesInvalidShift
        mov  rbx, testCountCellCandidatesInvalidShift
        mov  rcx, [rip + testCountRowCellCandidatesMinCount]
        mov  rdx, [rip + testCountRowCellCandidatesRowPopCount]
        MacroMovdqa xRegRow6, [rip + testCountRowCellCandidatesRowX]
        CountRowCellCandidates xRegWork1, rax, rbx, rcx, rdx, xRegRow6, testCountCellCandidatesRowNumber, rsi, rdi, r8, r9, r10, r11, r12, r13, r14
        MacroPextrw rsi, xRegWork1, 0
        ret

.macro CountRowSetCell xRegRowNumber, regOutBoxShift, regInBoxShift, regWork1, regWork2, regWork3, regWork4, regWork5, regWork6, regWork7, regWork8, regWork9, regWork10, regWork11
        mov  \regOutBoxShift, outBoxIndexInvalid
        mov  \regInBoxShift, inBoxIndexInvalid
        mov  \regWork1, (numberOfRows + 1)
        MacroPinsrq \xRegRowNumber, \regWork1, 0
        mov  \regWork1, candidatesTooMany
        xor  \regWork2, \regWork2

        CountRowCellCandidates  \xRegRowNumber, \regOutBoxShift, \regInBoxShift, \regWork1, \regWork2, xRegRow1, 0, \regWork3, \regWork4, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9, \regWork10, \regWork11
        CountRowCellCandidates  \xRegRowNumber, \regOutBoxShift, \regInBoxShift, \regWork1, \regWork2, xRegRow2, 1, \regWork3, \regWork4, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9, \regWork10, \regWork11
        CountRowCellCandidates  \xRegRowNumber, \regOutBoxShift, \regInBoxShift, \regWork1, \regWork2, xRegRow3, 2, \regWork3, \regWork4, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9, \regWork10, \regWork11
        CountRowCellCandidates  \xRegRowNumber, \regOutBoxShift, \regInBoxShift, \regWork1, \regWork2, xRegRow4, 3, \regWork3, \regWork4, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9, \regWork10, \regWork11
        CountRowCellCandidates  \xRegRowNumber, \regOutBoxShift, \regInBoxShift, \regWork1, \regWork2, xRegRow5, 4, \regWork3, \regWork4, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9, \regWork10, \regWork11
        CountRowCellCandidates  \xRegRowNumber, \regOutBoxShift, \regInBoxShift, \regWork1, \regWork2, xRegRow6, 5, \regWork3, \regWork4, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9, \regWork10, \regWork11
        CountRowCellCandidates  \xRegRowNumber, \regOutBoxShift, \regInBoxShift, \regWork1, \regWork2, xRegRow7, 6, \regWork3, \regWork4, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9, \regWork10, \regWork11
        CountRowCellCandidates  \xRegRowNumber, \regOutBoxShift, \regInBoxShift, \regWork1, \regWork2, xRegRow8, 7, \regWork3, \regWork4, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9, \regWork10, \regWork11
        CountRowCellCandidates  \xRegRowNumber, \regOutBoxShift, \regInBoxShift, \regWork1, \regWork2, xRegRow9, 8, \regWork3, \regWork4, \regWork5, \regWork6, \regWork7, \regWork8, \regWork9, \regWork10, \regWork11
302:
.endm

        .global searchNextCandidate
searchNextCandidate:
        InitMaskRegister
        CountRowSetCell xRegWork1, rax, rbx, rcx, rdx, rsi, rdi, r8, r9, r10, r11, r12, r13, r14
        MacroPextrq  rcx, xRegWork1, 0
        ret

        .global testFoldRowParts
        .global testCheckRow
        .global testCheckRowSet
        .global testCheckColumn
        .global testCheckBox
        .global testCheckSetBox
        .global testCheckConsistency

.macro FoldRowParts xRegRow, regWork1, regWork2, regWork3, regWork4, regWork5
        MacroPextrq       \regWork2, \xRegRow, 0
        MacroPextrq       \regWork1, \xRegRow, 1
        SplitRowLowParts  \regWork3, \regWork2
        MergeThreeElements2  \regWork1, \regWork4, \regWork5
        MergeThreeElements2  \regWork2, \regWork4, \regWork5
        MergeThreeElements2  \regWork3, \regWork4, \regWork5
.endm

testFoldRowParts:
        InitMaskRegister
        xor  rax, rax
        xor  rbx, rbx
        xor  rcx, rcx
        FoldRowParts  xRegRow1, rax, rbx, rcx, r8, r9
        ret

# 列に一つずつ候補があるかどうか調べて、結果を返す
# そうであればregResultは不変、そうでなければregInvalidを設定する
.macro CheckRow xRegRow, regResult, regInvalid, regWork1, regWork2, regWork3, regWork4, regWork5
        FoldRowParts \xRegRow, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5
        or      \regWork1, \regWork2
        or      \regWork1, \regWork3
        popcnt  \regWork1, \regWork1
        cmp     \regWork1, candidatesNum
        cmovnz  \regResult, \regInvalid
.endm

testCheckRow:
        InitMaskRegister
        xor  rax, rax
        mov  r8, 1
        CheckRow  xRegRow1, rax, r8, r9, r10, r11, r12, r13
        ret

.macro CheckRowSet regResult, regInvalid, regWork1, regWork2, regWork3, regWork4, regWork5
        CheckRow xRegRow1, \regResult, \regInvalid, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5
        CheckRow xRegRow2, \regResult, \regInvalid, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5
        CheckRow xRegRow3, \regResult, \regInvalid, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5
        CheckRow xRegRow4, \regResult, \regInvalid, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5
        CheckRow xRegRow5, \regResult, \regInvalid, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5
        CheckRow xRegRow6, \regResult, \regInvalid, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5
        CheckRow xRegRow7, \regResult, \regInvalid, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5
        CheckRow xRegRow8, \regResult, \regInvalid, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5
        CheckRow xRegRow9, \regResult, \regInvalid, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5
.endm

testCheckRowSet:
        InitMaskRegister
        xor  rax, rax
        mov  r8, 1
        CheckRowSet  rax, r8, r9, r10, r11, r12, r13
        ret

.macro CheckColumn regResult, regInvalid, reg32Work1, reg32Work2, reg32Work3
        MacroPextrd \reg32Work1, xRegRowAll, 0
        MacroPextrd \reg32Work2, xRegRowAll, 1
        MacroPextrd \reg32Work3, xRegRowAll, 2
        popcnt  \reg32Work1, \reg32Work1
        popcnt  \reg32Work2, \reg32Work2
        popcnt  \reg32Work3, \reg32Work3

        cmp     \reg32Work1, (candidatesNum * rowPartNum)
        cmovnz  \regResult, \regInvalid
        cmp     \reg32Work2, (candidatesNum * rowPartNum)
        cmovnz  \regResult, \regInvalid
        cmp     \reg32Work3, (candidatesNum * rowPartNum)
        cmovnz  \regResult, \regInvalid
.endm

testCheckColumn:
        InitMaskRegister
        xor  rax, rax
        mov  r8, 1
        CheckColumn  rax, r8, ebx, ecx, edx
        ret

.macro CheckBox xRegRow, regResult, regInvalid, regWork1, regWork2, regWork3, regWork4, regWork5
        FoldRowParts \xRegRow, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5

        popcnt  \regWork1, \regWork1
        cmp     \regWork1, candidatesNum
        cmovnz  \regResult, \regInvalid

        popcnt  \regWork2, \regWork2
        cmp     \regWork2, candidatesNum
        cmovnz  \regResult, \regInvalid

        popcnt  \regWork3, \regWork3
        cmp     \regWork3, candidatesNum
        cmovnz  \regResult, \regInvalid
.endm

testCheckBox:
        InitMaskRegister
        xor  rax, rax
        mov  r8, 1
        CheckBox  xRegRow1to3, rax, r8, r9, r10, r11, r12, r13
        ret

.macro CheckSetBox regResult, regInvalid, regWork1, regWork2, regWork3, regWork4, regWork5
        CheckBox xRegRow1to3, \regResult, \regInvalid, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5
        CheckBox xRegRow4to6, \regResult, \regInvalid, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5
        CheckBox xRegRow7to9, \regResult, \regInvalid, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5
.endm

testCheckSetBox:
        InitMaskRegister
        xor  rax, rax
        mov  r8, 1
        CheckSetBox  rax, r8, r9, r10, r11, r12, r13
        ret

.macro CheckConsistency regResult, regInvalid, regWork1, regWork2, regWork3, regWork4, regWork5, reg32Work1, reg32Work2, reg32Work3
        xor  \regResult, \regResult
        mov  \regInvalid, 1
        CheckRowSet  \regResult, \regInvalid, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5
        cmp  \regResult, \regInvalid
        jz   20001f

        OrThreeXmmRegs  xRegRow1to3, xRegRow1, xRegRow2, xRegRow3
        OrThreeXmmRegs  xRegRow4to6, xRegRow4, xRegRow5, xRegRow6
        OrThreeXmmRegs  xRegRow7to9, xRegRow7, xRegRow8, xRegRow9
        OrThreeXmmRegs  xRegRowAll, xRegRow1to3, xRegRow4to6, xRegRow7to9
        CheckColumn  \regResult, \regInvalid, \reg32Work1, \reg32Work2, \reg32Work3
        CheckSetBox  \regResult, \regInvalid, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5
20001:
.endm

testCheckConsistency:
        InitMaskRegister
        xor  rax, rax
        mov  r8, 1
        CheckConsistency  rax, r8, r9, r10, rbx, rcx, rdx, ebx, ecx, edx
        ret

# コードを連続した領域に配置したよいので、テストを挟まず一か所にまとめる
# C++のインラインアセンブラからサブルーチンとして呼び出すためのシンボル
        .global solveSudokuAsm
solveSudokuAsm:
        mov    gRegBitMask, elementBitMask
        xorps  xLoopPopCnt, xLoopPopCnt

        # データ構造
        # xmmレジスタ = | 32bit:all 0 | 32bit:1行1..3列 | 同4..6列 | 同7..9列 |
        # xmmレジスタ0..8 = 1..9列目
        # xmmレジスタの各32bit = |22個の0, c_9, c_8, ... , c_1, 0|
        # c_n: マスに数字nが入れば1、そうでなければ0|

loopFilling:
        # 3行ごとに候補をまとめる
        CollectUniqueCandidatesInThreeLine xRegRow1to3, xRegRow1, xRegRow2, xRegRow3, rax, rbx, rcx, rdx, r8, r9, r10, xRegWork1, xRegWork2
        CollectUniqueCandidatesInThreeLine xRegRow4to6, xRegRow4, xRegRow5, xRegRow6, rax, rbx, rcx, rdx, r8, r9, r10, xRegWork1, xRegWork2
        CollectUniqueCandidatesInThreeLine xRegRow7to9, xRegRow7, xRegRow8, xRegRow9, rax, rbx, rcx, rdx, r8, r9, r10, xRegWork1, xRegWork2
        # すべての行をまとめる
        OrThreeXmmRegs xRegRowAll, xRegRow1to3, xRegRow4to6, xRegRow7to9

        # 何マス埋まったか調べる
        .set    regCurrentPopcnt, r8
        .set    regPrevPopcnt,    r9
        .set    regMaxPopcnt,     r10
        .set    regLoopCnt,       r11
        CountFilledElements regLoopCnt, regCurrentPopcnt, regPrevPopcnt, rcx
        mov     regMaxPopcnt, maxElementNumber

        # 全マス埋まったか、マスを埋められなかった(最大のマス数と比較してZF=1)
        cmp     regCurrentPopcnt, regPrevPopcnt
        cmovz   regPrevPopcnt, regMaxPopcnt
        cmp     regPrevPopcnt, regMaxPopcnt
        jnz     keepFilling

exitFilling:
        .set    regResult, r9
        CheckConsistency regResult, r10, r11, r12, rbx, rcx, rdx, ebx, ecx, edx
        mov     qword ptr [rip + sudokuXmmAborted], regResult
        mov     qword ptr [rip + sudokuXmmElementCnt], regCurrentPopcnt
        ret

abortFilling:
        mov     qword ptr [rip + sudokuXmmAborted], 1
        mov     qword ptr [rip + sudokuXmmElementCnt], 0
        ret

keepFilling:
        # 強制終了
        cmp     regLoopCnt, sudokuMaxLoopcnt
        jae     exitFilling
        FastInc regLoopCnt
        SaveLoopCnt regLoopCnt, regCurrentPopcnt

        CollectUniqueCandidates r8, r9, r10, r11, r12, r13, r14, rax, rbx, rcx, rdx, rsi, rdi, xRegWork1, xRegWork2
        FindCandidates r8, r9, r10, r11, r12, r13, r14, rax, rbx, rcx, rdx, rsi, rdi, xRegWork1, xRegWork2
        jmp loopFilling

        .global loadXmmRegisters
loadXmmRegisters:
        MacroMovdqa xmm0,  (xmmword ptr [rsi])
        MacroMovdqa xmm1,  (xmmword ptr [rsi+16])
        MacroMovdqa xmm2,  (xmmword ptr [rsi+32])
        MacroMovdqa xmm3,  (xmmword ptr [rsi+48])
        MacroMovdqa xmm4,  (xmmword ptr [rsi+64])
        MacroMovdqa xmm5,  (xmmword ptr [rsi+80])
        MacroMovdqa xmm6,  (xmmword ptr [rsi+96])
        MacroMovdqa xmm7,  (xmmword ptr [rsi+112])
        MacroMovdqa xmm8,  (xmmword ptr [rsi+128])
        MacroMovdqa xmm9,  (xmmword ptr [rsi+144])
        MacroMovdqa xmm10, (xmmword ptr [rsi+160])
        MacroMovdqa xmm11, (xmmword ptr [rsi+176])
        MacroMovdqa xmm12, (xmmword ptr [rsi+192])
        MacroMovdqa xmm13, (xmmword ptr [rsi+208])
        MacroMovdqa xmm14, (xmmword ptr [rsi+224])
        MacroMovdqa xmm15, (xmmword ptr [rsi+240])
        ret

        .global saveXmmRegisters
saveXmmRegisters:
        MacroMovdqa (xmmword ptr [rdi]),     xmm0
        MacroMovdqa (xmmword ptr [rdi+16]),  xmm1
        MacroMovdqa (xmmword ptr [rdi+32]),  xmm2
        MacroMovdqa (xmmword ptr [rdi+48]),  xmm3
        MacroMovdqa (xmmword ptr [rdi+64]),  xmm4
        MacroMovdqa (xmmword ptr [rdi+80]),  xmm5
        MacroMovdqa (xmmword ptr [rdi+96]),  xmm6
        MacroMovdqa (xmmword ptr [rdi+112]), xmm7
        MacroMovdqa (xmmword ptr [rdi+128]), xmm8
        MacroMovdqa (xmmword ptr [rdi+144]), xmm9
        MacroMovdqa (xmmword ptr [rdi+160]), xmm10
        MacroMovdqa (xmmword ptr [rdi+176]), xmm11
        MacroMovdqa (xmmword ptr [rdi+192]), xmm12
        MacroMovdqa (xmmword ptr [rdi+208]), xmm13
        MacroMovdqa (xmmword ptr [rdi+224]), xmm14
        MacroMovdqa (xmmword ptr [rdi+240]), xmm15
        ret

# 数独の個数を数える
        # 固定でレジスタを割り当てる(rcxはシフト用)
        .set    xRightestColumn, xmm10
        .set    xRegWork3,       xmm11

        .set    gRegSum64,     rax
        .set    gRegTarget64,  rsi
        .set    gRegWork1Byte, bl
        .set    gRegWork2Byte, dl
        .set    gRightBottomElement64,  rdi
        .set    gRegAddrToSaveSum,      r8
        .set    gRegAddrToNextCellFunc, r9
        .set    gRegNewCandidate64,     r10
        .set    gRegWork64, r11
        .set    gRegCount,  r12
        .set    gRegReturn, r13
        .set    gRegOne64,  r14
        .set    gRegFastBitMask64, r15

        .if (UseReg64Most != 0)
        .set    gRegShift,  rcx
        .set    gRegSum,    gRegSum64
        .set    gRegTarget, gRegTarget64
        .set    gRegWork1,  rbx
        .set    gRegWork2,  rdx
        .set    gRightBottomElement, gRightBottomElement64
        .set    gRegNewCandidate, gRegNewCandidate64
        .set    gRegOne,    gRegOne64
        .set    gRegFastBitMask, gRegFastBitMask64

        .else
        .set    gRegShift,  ecx
        .set    gRegSum,    eax
        .set    gRegTarget, esi
        .set    gRegWork1,  ebx
        .set    gRegWork2,  edx
        .set    gRightBottomElement, edi
        .set    gRegNewCandidate, r10d
        .set    gRegOne,    r14d
        .set    gRegFastBitMask, r15d
        .endif

.macro FastInitMaskRegister
        mov     gRegFastBitMask64, elementBitMask
        mov     gRegOne64, 1
.endm

.macro InitRegisterBeforeTesting
        FastInitMaskRegister
        xor     gRightBottomElement64, gRightBottomElement64
        xor     gRegCount, gRegCount
.endm

.macro FastCollectCandidatesAtRow regSum, regTarget, regWork1, regWork2, xRegSrc, columnNumber, rowNumber
        .if ((\columnNumber + 1) == candidatesNum)
         .if (EnableAvx != 0)
        vphaddw xRegWork1, \xRegSrc,  xRegWork2
        vphaddw xRegWork1, xRegWork1, xRegWork2
        vphaddw xRegWork1, xRegWork1, xRegWork2
        MacroPextrw \regSum, xRegWork1, 0
         .else
        movdqa  xRegWork1, \xRegSrc
        phaddw  xRegWork1, xRegWork2
        phaddw  xRegWork1, xRegWork2
        phaddw  xRegWork1, xRegWork2
        pextrw  \regSum, xRegWork1, 0
         .endif

         .if ((\rowNumber + 1) == candidatesNum)
        mov     \regTarget, gRightBottomElement
         .else
        MacroPextrw \regTarget, xRightestColumn, \rowNumber
         .endif

        .else
         .if (EnableAvx != 0)
        vphaddw xRegWork1, \xRegSrc,  xRegWork2
        vphaddw xRegWork1, xRegWork1, xRegWork2
        vphaddw xRegWork1, xRegWork1, xRegWork2
         .else
        MacroMovdqa xRegWork1, \xRegSrc
        MacroPhaddw xRegWork1, xRegWork2
        MacroPhaddw xRegWork1, xRegWork2
        MacroPhaddw xRegWork1, xRegWork2
         .endif

        MacroPextrw \regTarget, \xRegSrc, \columnNumber
         .if ((\rowNumber + 1) == candidatesNum)
        mov     \regSum, gRightBottomElement
         .else
        MacroPextrw \regSum, xRightestColumn, \rowNumber
         .endif

        MacroPextrw \regWork1, xRegWork1, 0
        or      \regSum, \regWork1
        xor     \regSum, \regTarget
        .endif
.endm

.macro testFastCollectCandidatesAtRow xRegSrc, columnNumber, rowNumber
        InitRegisterBeforeTesting
        mov     gRightBottomElement64, rcx
        xor     rax, rax
        xor     rbx, rbx

        .if (UseReg64Most != 0)
        FastCollectCandidatesAtRow rax, rbx, gRegShift, gRegWork2, \xRegSrc, \columnNumber, \rowNumber
        .else
        FastCollectCandidatesAtRow eax, ebx, gRegShift, gRegWork2, \xRegSrc, \columnNumber, \rowNumber
        .endif
        ret
.endm

        .global testFastCollectCandidatesAtRow00
        .global testFastCollectCandidatesAtRow10
        .global testFastCollectCandidatesAtRow20
        .global testFastCollectCandidatesAtRow30
        .global testFastCollectCandidatesAtRow40
        .global testFastCollectCandidatesAtRow50
        .global testFastCollectCandidatesAtRow60
        .global testFastCollectCandidatesAtRow70
        .global testFastCollectCandidatesAtRow80
        .global testFastCollectCandidatesAtRow88
testFastCollectCandidatesAtRow00:
        testFastCollectCandidatesAtRow xRegRow1, 0, 0
testFastCollectCandidatesAtRow10:
        testFastCollectCandidatesAtRow xRegRow1, 1, 0
testFastCollectCandidatesAtRow20:
        testFastCollectCandidatesAtRow xRegRow1, 2, 0
testFastCollectCandidatesAtRow30:
        testFastCollectCandidatesAtRow xRegRow1, 3, 0
testFastCollectCandidatesAtRow40:
        testFastCollectCandidatesAtRow xRegRow1, 4, 0
testFastCollectCandidatesAtRow50:
        testFastCollectCandidatesAtRow xRegRow1, 5, 0
testFastCollectCandidatesAtRow60:
        testFastCollectCandidatesAtRow xRegRow1, 6, 0
testFastCollectCandidatesAtRow70:
        testFastCollectCandidatesAtRow xRegRow1, 7, 0
testFastCollectCandidatesAtRow80:
        testFastCollectCandidatesAtRow xRegRow1, 8, 0
testFastCollectCandidatesAtRow88:
        testFastCollectCandidatesAtRow xRegRow9, 8, 8

.macro FastCollectCandidatesAtBox regSum, xRowSrc, regWork, columnNumber1, columnNumber2, rowNumber
        .if ((\columnNumber2 + 1) == candidatesNum)
        MacroPextrw \regSum,  \xRowSrc, \columnNumber1
         .if ((\rowNumber + 1) == candidatesNum)
        mov     \regWork, gRightBottomElement
         .else
        MacroPextrw \regWork, xRightestColumn, \rowNumber
         .endif

        .else
        MacroPextrw \regSum,  \xRowSrc, \columnNumber1
        MacroPextrw \regWork, \xRowSrc, \columnNumber2
        .endif

        or      \regSum,  \regWork
.endm

.macro testFastCollectCandidatesAtBox xRegSrc, columnNumber1, columnNumber2, rowNumber
        InitRegisterBeforeTesting
        MacroXorps xRightestColumn, xRightestColumn
        mov     gRegWork64, 1
        MacroPinsrq xRightestColumn, gRegWork64, 0
        mov     gRightBottomElement, gRegOne

        xor     rax, rax
        .if (UseReg64Most != 0)
        FastCollectCandidatesAtBox rax, \xRegSrc, gRegWork1, \columnNumber1, \columnNumber2, \rowNumber
        .else
        FastCollectCandidatesAtBox eax, \xRegSrc, gRegWork1, \columnNumber1, \columnNumber2, \rowNumber
        .endif
        ret
.endm

        .global testFastCollectCandidatesAtBox670
        .global testFastCollectCandidatesAtBox780
        .global testFastCollectCandidatesAtBox788
testFastCollectCandidatesAtBox670:
        testFastCollectCandidatesAtBox xRegRow1, 6, 7, 0
testFastCollectCandidatesAtBox780:
        testFastCollectCandidatesAtBox xRegRow1, 7, 8, 0
testFastCollectCandidatesAtBox788:
        testFastCollectCandidatesAtBox xRegRow9, 7, 8, 8

.macro FastCollectCandidatesAtColumn regSum, regWork1, columnNumber, rowNumber
        .if ((\columnNumber + 1) == candidatesNum)
         .if (EnableAvx != 0)
        vphaddw xRegWork1, xRightestColumn, xRegWork2
        vphaddw xRegWork1, xRegWork1, xRegWork2
        vphaddw xRegWork1, xRegWork1, xRegWork2
        MacroPextrw \regSum, xRegWork1, 0
         .else
        MacroMovdqa xRegWork1, xRightestColumn
        MacroPhaddw xRegWork1, xRegWork2
        MacroPhaddw xRegWork1, xRegWork2
        MacroPhaddw xRegWork1, xRegWork2
        MacroPextrw \regSum, xRegWork1, 0
         .endif

         .if ((\rowNumber + 1) < candidatesNum)
        or      \regSum, gRightBottomElement
        MacroPextrw \regWork1, xRightestColumn, \rowNumber
        xor     \regSum, \regWork1
         .endif

        .else
        MacroXorps xRegWork2, xRegWork2
        MacroXorps xRegWork3, xRegWork3

         .if (CellsPacked != 0)
          .if (\rowNumber == 0)
        MacroXorps xRegWork1, xRegWork1
          .elseif (\rowNumber == 1)
        MacroMovdqa xRegWork1, xRegRow1
          .elseif (\rowNumber >= 2)
        MacroOrps3op xRegWork1, xRegRow1, xRegRow2
          .endif

         .else
          .if (\rowNumber == 0)
        movdqa xRegWork1, xRegRow2
          .elseif (\rowNumber == 1)
        movdqa xRegWork1, xRegRow1
          .else
        MacroOrps3op xRegWork1, xRegRow1, xRegRow2
          .endif
         .endif

         .if ((CellsPacked == 0) || (\rowNumber >= 2))
          .if (\rowNumber != 2)
        MacroOrps xRegWork1, xRegRow3
          .endif

          .if ((CellsPacked != 0) && (\rowNumber <= 3))
          .elseif ((CellsPacked != 0) && (\rowNumber == 4))
        MacroMovdqa xRegWork2, xRegRow4
          .elseif ((CellsPacked != 0) && (\rowNumber == 5))
        MacroOrps3op xRegWork2, xRegRow4, xRegRow5
          .else
           .if (\rowNumber == 3)
        MacroMovdqa xRegWork2, xRegRow5
           .elseif (\rowNumber == 4)
        MacroMovdqa xRegWork2, xRegRow4
           .else
        MacroOrps3op xRegWork2, xRegRow4, xRegRow5
           .endif

           .if ((CellsPacked == 0) || (\rowNumber >= 5))
            .if (\rowNumber != 5)
        MacroOrps xRegWork2, xRegRow6
            .endif

            .if ((CellsPacked != 0) && (\rowNumber <= 6))
            .elseif ((CellsPacked != 0) && (\rowNumber == 7))
        MacroMovdqa xRegWork3, xRegRow7
            .elseif ((CellsPacked != 0) && (\rowNumber == 8))
        MacroOrps3op xRegWork3, xRegRow7, xRegRow8
            .else
             .if (\rowNumber == 6)
        MacroMovdqa xRegWork3, xRegRow8
             .elseif (\rowNumber == 7)
        MacroMovdqa xRegWork3, xRegRow7
             .else
        MacroOrps3op xRegWork3, xRegRow7, xRegRow8
             .endif

             .if ((CellsPacked == 0) || (\rowNumber >= 8))
              .if (\rowNumber != 8)
        MacroOrps xRegWork3, xRegRow9
              .endif
             .endif
            .endif
           .endif
          .endif
         .endif

        # xRegWork1も反映させる
        MacroOrps xRegWork1, xRegWork2
        MacroOrps xRegWork1, xRegWork3
        MacroPextrw \regSum, xRegWork1, \columnNumber
        .endif
.endm

.macro testFastCollectCandidatesAtColumn columnNumber, rowNumber
        InitRegisterBeforeTesting
        xor     rax, rax
        mov     gRightBottomElement, 0x100

        MacroXorps xRegRow1, xRegRow1
        mov     gRegWork64, 1
        MacroPinsrq xRegRow1, gRegWork64, 0
        mov     gRegWork64, 0x80000000000000
        MacroPinsrq xRegRow1, gRegWork64, 1
        MacroXorps xRegRow2, xRegRow2

        MacroXorps xRegRow3, xRegRow3
        MacroXorps xRegRow4, xRegRow4
        MacroXorps xRegRow5, xRegRow5
        MacroXorps xRegRow6, xRegRow6
        MacroXorps xRegRow7, xRegRow7
        MacroXorps xRegRow8, xRegRow8
        MacroXorps xRegRow9, xRegRow9
        MacroXorps xRightestColumn, xRightestColumn

        mov     gRegWork64, 0x80004000200010
        MacroPinsrq xRightestColumn, gRegWork64, 1
        mov     gRegWork64, 0x8000400020001
        MacroPinsrq xRightestColumn, gRegWork64, 0

        xor     rax, rax
        .if (UseReg64Most != 0)
        FastCollectCandidatesAtColumn rax, gRegWork1, \columnNumber, \rowNumber
        .else
        FastCollectCandidatesAtColumn eax, gRegWork1, \columnNumber, \rowNumber
        .endif
        ret
.endm

        .global testFastCollectCandidatesAtColumn01
        .global testFastCollectCandidatesAtColumn71
        .global testFastCollectCandidatesAtColumn81
        .global testFastCollectCandidatesAtColumn88
testFastCollectCandidatesAtColumn01:
        testFastCollectCandidatesAtColumn 0, 1
testFastCollectCandidatesAtColumn71:
        testFastCollectCandidatesAtColumn 7, 1
testFastCollectCandidatesAtColumn81:
        testFastCollectCandidatesAtColumn 8, 1
testFastCollectCandidatesAtColumn88:
        testFastCollectCandidatesAtColumn 8, 8

.macro FastCollectCandidatesAtCellSub regSum, regTarget, xRegSrcTarget, xRegSrcOther1, xRegSrcOther2, regWork1, regWork2, columnNumber, otherColumnNumber1, otherColumnNumber2, rowNumber
        FastCollectCandidatesAtRow \regSum, \regTarget, \regWork1, \regWork2, \xRegSrcTarget, \columnNumber, \rowNumber

        FastCollectCandidatesAtColumn \regWork1, \regWork2, \columnNumber, \rowNumber
        or  \regSum, \regWork1

        popcnt  \regWork2, \regSum
        cmp     \regWork2, candidatesNum
        jae     10012f

        # ここで打ち切っても速くならない
        .if ((CellsPacked == 0) || ((\rowNumber % 3) != 0))
        FastCollectCandidatesAtBox \regWork1, \xRegSrcOther1, \regWork2, \otherColumnNumber1, \otherColumnNumber2, \rowNumber
        or  \regSum, \regWork1
        .endif

        .if ((CellsPacked == 0) || ((\rowNumber % 3) == 2))
        FastCollectCandidatesAtBox \regWork1, \xRegSrcOther2, \regWork2, \otherColumnNumber1, \otherColumnNumber2, \rowNumber
        or  \regSum, \regWork1
        .endif

        .endm

.macro FastCollectCandidatesAtCellMain regSum, regTarget, xRegSrcTarget, xRegSrcOther1, xRegSrcOther2, regWork1, regWork2, columnNumber, rowNumber
        .if (columnNumber == 0)
        FastCollectCandidatesAtCellSub \regSum, \regTarget, \xRegSrcTarget, \xRegSrcOther1, \xRegSrcOther2, \regWork1, \regWork2, 0, 1, 2, \rowNumber
        .elseif (columnNumber == 1)
        FastCollectCandidatesAtCellSub \regSum, \regTarget, \xRegSrcTarget, \xRegSrcOther1, \xRegSrcOther2, \regWork1, \regWork2, 1, 0, 2, \rowNumber
        .elseif (columnNumber == 2)
        FastCollectCandidatesAtCellSub \regSum, \regTarget, \xRegSrcTarget, \xRegSrcOther1, \xRegSrcOther2, \regWork1, \regWork2, 2, 0, 1, \rowNumber
        .elseif (columnNumber == 3)
        FastCollectCandidatesAtCellSub \regSum, \regTarget, \xRegSrcTarget, \xRegSrcOther1, \xRegSrcOther2, \regWork1, \regWork2, 3, 4, 5, \rowNumber
        .elseif (columnNumber == 4)
        FastCollectCandidatesAtCellSub \regSum, \regTarget, \xRegSrcTarget, \xRegSrcOther1, \xRegSrcOther2, \regWork1, \regWork2, 4, 3, 5, \rowNumber
        .elseif (columnNumber == 5)
        FastCollectCandidatesAtCellSub \regSum, \regTarget, \xRegSrcTarget, \xRegSrcOther1, \xRegSrcOther2, \regWork1, \regWork2, 5, 3, 4, \rowNumber
        .elseif (columnNumber == 6)
        FastCollectCandidatesAtCellSub \regSum, \regTarget, \xRegSrcTarget, \xRegSrcOther1, \xRegSrcOther2, \regWork1, \regWork2, 6, 7, 8, \rowNumber
        .elseif (columnNumber == 7)
        FastCollectCandidatesAtCellSub \regSum, \regTarget, \xRegSrcTarget, \xRegSrcOther1, \xRegSrcOther2, \regWork1, \regWork2, 7, 6, 8, \rowNumber
        .elseif (columnNumber == 8)
        FastCollectCandidatesAtCellSub \regSum, \regTarget, \xRegSrcTarget, \xRegSrcOther1, \xRegSrcOther2, \regWork1, \regWork2, 8, 6, 7, \rowNumber
        .endif
.endm

.macro FastCollectUniqueCandidatesAtCell regSum, regTarget, regWork1, regWork2, cellCount
        .set columnNumber, (\cellCount % numberOfColumns)
        .set rowNumber,    (\cellCount / numberOfRows)

        .if (rowNumber == 0)
        FastCollectCandidatesAtCellMain \regSum, \regTarget, xRegRow1, xRegRow2, xRegRow3, \regWork1, \regWork2, columnNumber, rowNumber
        .elseif (rowNumber == 1)
        FastCollectCandidatesAtCellMain \regSum, \regTarget, xRegRow2, xRegRow1, xRegRow3, \regWork1, \regWork2, columnNumber, rowNumber
        .elseif (rowNumber == 2)
        FastCollectCandidatesAtCellMain \regSum, \regTarget, xRegRow3, xRegRow1, xRegRow2, \regWork1, \regWork2, columnNumber, rowNumber
        .elseif (rowNumber == 3)
        FastCollectCandidatesAtCellMain \regSum, \regTarget, xRegRow4, xRegRow5, xRegRow6, \regWork1, \regWork2, columnNumber, rowNumber
        .elseif (rowNumber == 4)
        FastCollectCandidatesAtCellMain \regSum, \regTarget, xRegRow5, xRegRow4, xRegRow6, \regWork1, \regWork2, columnNumber, rowNumber
        .elseif (rowNumber == 5)
        FastCollectCandidatesAtCellMain \regSum, \regTarget, xRegRow6, xRegRow4, xRegRow5, \regWork1, \regWork2, columnNumber, rowNumber
        .elseif (rowNumber == 6)
        FastCollectCandidatesAtCellMain \regSum, \regTarget, xRegRow7, xRegRow8, xRegRow9, \regWork1, \regWork2, columnNumber, rowNumber
        .elseif (rowNumber == 7)
        FastCollectCandidatesAtCellMain \regSum, \regTarget, xRegRow8, xRegRow7, xRegRow9, \regWork1, \regWork2, columnNumber, rowNumber
        .elseif (rowNumber == 8)
        FastCollectCandidatesAtCellMain \regSum, \regTarget, xRegRow9, xRegRow7, xRegRow8, \regWork1, \regWork2, columnNumber, rowNumber
        .endif
.endm

.macro FastSetUniqueCandidatesAtCellSub regCandidate, xRegSrcTarget, columnNumber, rowNumber
        .if ((\columnNumber + 1) == candidatesNum)
         .if ((\rowNumber + 1) < candidatesNum)
        MacroPinsrw xRightestColumn, \regCandidate, \rowNumber
         .else
        mov  gRightBottomElement, \regCandidate
         .endif

        .else
        MacroPinsrw \xRegSrcTarget, \regCandidate, \columnNumber
        .endif
.endm

.macro testFastSetUniqueCandidatesAtCellSub xRegSrcTarget, columnNumber, rowNumber
        InitRegisterBeforeTesting
        MacroXorps \xRegSrcTarget, \xRegSrcTarget
        MacroXorps xRightestColumn, xRightestColumn
        mov    gRegWork1, 1
        FastSetUniqueCandidatesAtCellSub gRegWork1, \xRegSrcTarget, \columnNumber, \rowNumber

        MacroPextrq rax, \xRegSrcTarget, 1
        MacroPextrq rbx, \xRegSrcTarget, 0
        MacroPextrq rcx, xRightestColumn, 1
        MacroPextrq rdx, xRightestColumn, 0

        xor     rsi, rsi
        .if (UseReg64Most != 0)
        mov     rsi, gRightBottomElement
        .else
        mov     esi, gRightBottomElement
        .endif
        ret
.endm

        .global testFastSetUniqueCandidatesAtCellSub00
        .global testFastSetUniqueCandidatesAtCellSub80
        .global testFastSetUniqueCandidatesAtCellSub08
        .global testFastSetUniqueCandidatesAtCellSub88
testFastSetUniqueCandidatesAtCellSub00:
        testFastSetUniqueCandidatesAtCellSub xRegRow1, 0, 0
testFastSetUniqueCandidatesAtCellSub80:
        testFastSetUniqueCandidatesAtCellSub xRegRow1, 8, 0
testFastSetUniqueCandidatesAtCellSub08:
        testFastSetUniqueCandidatesAtCellSub xRegRow9, 0, 8
testFastSetUniqueCandidatesAtCellSub88:
        testFastSetUniqueCandidatesAtCellSub xRegRow9, 8, 8

.macro FastSetUniqueCandidatesAtCell regCandidate, cellCount
        .set columnNumber, (\cellCount % numberOfRows)
        .set rowNumber,    (\cellCount / numberOfColumns)

        .if (rowNumber == 0)
        FastSetUniqueCandidatesAtCellSub \regCandidate, xRegRow1, columnNumber, rowNumber
        .elseif (rowNumber == 1)
        FastSetUniqueCandidatesAtCellSub \regCandidate, xRegRow2, columnNumber, rowNumber
        .elseif (rowNumber == 2)
        FastSetUniqueCandidatesAtCellSub \regCandidate, xRegRow3, columnNumber, rowNumber
        .elseif (rowNumber == 3)
        FastSetUniqueCandidatesAtCellSub \regCandidate, xRegRow4, columnNumber, rowNumber
        .elseif (rowNumber == 4)
        FastSetUniqueCandidatesAtCellSub \regCandidate, xRegRow5, columnNumber, rowNumber
        .elseif (rowNumber == 5)
        FastSetUniqueCandidatesAtCellSub \regCandidate, xRegRow6, columnNumber, rowNumber
        .elseif (rowNumber == 6)
        FastSetUniqueCandidatesAtCellSub \regCandidate, xRegRow7, columnNumber, rowNumber
        .elseif (rowNumber == 7)
        FastSetUniqueCandidatesAtCellSub \regCandidate, xRegRow8, columnNumber, rowNumber
        .elseif (rowNumber == 8)
        FastSetUniqueCandidatesAtCellSub \regCandidate, xRegRow9, columnNumber, rowNumber
        .endif
.endm

.macro CallCountUniqueCandidatesAtCell cellCount
        call [rip + sudokuXmmCountFuncTable + \cellCount * funcPtrByteSize]
.endm

.macro JmpCountUniqueCandidatesAtCell cellCount
        jmp [rip + sudokuXmmCountFuncTable + \cellCount * funcPtrByteSize]
.endm

.macro CountUniqueCandidatesAtNextCell cellCount, candidate
        test    gRegSum, \candidate
        jz      11009f
        mov     gRegNewCandidate, \candidate
        FastSetUniqueCandidatesAtCell gRegNewCandidate, \cellCount
        CallCountUniqueCandidatesAtCell (\cellCount + 1)
11009:
        mov     gRegSum64, qword ptr [rip + sudokuXmmCandidateArray + \cellCount * arrayElementByteSize]
.endm

.macro FastPrintAllCells
        MacroMovdqa  (xmmword ptr [rip + sudokuXmmToPrint]),     xmm0
        MacroMovdqa  (xmmword ptr [rip + sudokuXmmToPrint+16]),  xmm1
        MacroMovdqa  (xmmword ptr [rip + sudokuXmmToPrint+32]),  xmm2
        MacroMovdqa  (xmmword ptr [rip + sudokuXmmToPrint+48]),  xmm3
        MacroMovdqa  (xmmword ptr [rip + sudokuXmmToPrint+64]),  xmm4
        MacroMovdqa  (xmmword ptr [rip + sudokuXmmToPrint+80]),  xmm5
        MacroMovdqa  (xmmword ptr [rip + sudokuXmmToPrint+96]),  xmm6
        MacroMovdqa  (xmmword ptr [rip + sudokuXmmToPrint+112]), xmm7
        MacroMovdqa  (xmmword ptr [rip + sudokuXmmToPrint+128]), xmm8
        MacroMovdqa  (xmmword ptr [rip + sudokuXmmToPrint+144]), xmm9
        MacroMovdqa  (xmmword ptr [rip + sudokuXmmToPrint+160]), xmm10
        MacroMovdqa  (xmmword ptr [rip + sudokuXmmToPrint+176]), xmm11
        MacroMovdqa  (xmmword ptr [rip + sudokuXmmToPrint+192]), xmm12
        MacroMovdqa  (xmmword ptr [rip + sudokuXmmToPrint+208]), xmm13
        MacroMovdqa  (xmmword ptr [rip + sudokuXmmToPrint+224]), xmm14
        MacroMovdqa  (xmmword ptr [rip + sudokuXmmToPrint+240]), xmm15

        push    rbp
        mov     qword ptr [rip + sudokuXmmStackPointer], rsp

        # x64のABIは、呼び出し先関数の領域を32 byte分だけ、呼び出し側で確保する
        .set    MinStackSize, 32
        sub     rsp, MinStackSize

        # x64のABIは、rsp を 16 byte境界にアラインメントしないと動作しない
        # 戻りアドレスを積む前の時点で16 byte境界なので、積んだ後は8 mod 16byteである
        # そのため下位ビットを切り捨てて、下位アドレスの16 byte境界にそろえる
        # これをしないと、std::cout << の実行中にC++ライブラリの中で異常終了することになる
        and     rsp, ~0xf
        call    [rip + sudokuXmmPrintFunc]
        mov     rsp, qword ptr [rip + sudokuXmmStackPointer]
        pop     rbp

        MacroMovdqa  xmm0,  (xmmword ptr [rip + sudokuXmmToPrint])
        MacroMovdqa  xmm1,  (xmmword ptr [rip + sudokuXmmToPrint+16])
        MacroMovdqa  xmm2,  (xmmword ptr [rip + sudokuXmmToPrint+32])
        MacroMovdqa  xmm3,  (xmmword ptr [rip + sudokuXmmToPrint+48])
        MacroMovdqa  xmm4,  (xmmword ptr [rip + sudokuXmmToPrint+64])
        MacroMovdqa  xmm5,  (xmmword ptr [rip + sudokuXmmToPrint+80])
        MacroMovdqa  xmm6,  (xmmword ptr [rip + sudokuXmmToPrint+96])
        MacroMovdqa  xmm7,  (xmmword ptr [rip + sudokuXmmToPrint+112])
        MacroMovdqa  xmm8,  (xmmword ptr [rip + sudokuXmmToPrint+128])
        MacroMovdqa  xmm9,  (xmmword ptr [rip + sudokuXmmToPrint+144])
        MacroMovdqa  xmm10, (xmmword ptr [rip + sudokuXmmToPrint+160])
        MacroMovdqa  xmm11, (xmmword ptr [rip + sudokuXmmToPrint+176])
        MacroMovdqa  xmm12, (xmmword ptr [rip + sudokuXmmToPrint+192])
        MacroMovdqa  xmm13, (xmmword ptr [rip + sudokuXmmToPrint+208])
        MacroMovdqa  xmm14, (xmmword ptr [rip + sudokuXmmToPrint+224])
        MacroMovdqa  xmm15, (xmmword ptr [rip + sudokuXmmToPrint+240])
.endm

.macro CountUniqueCandidatesAtCellLoop outBoxShift, inBoxShift, cellCount, xRegSrcTarget, xRegSrcOther1, xRegSrcOther2, rowNumber
        xor     gRegSum64, gRegSum64
        xor     gRegTarget64, gRegTarget64

        # 候補が一つしかないときはここではなく、呼び出し先でretする
        pop     gRegReturn
        FastCollectUniqueCandidatesAtCell gRegSum, gRegTarget, gRegWork2, gRegShift, \cellCount

        .if (TrimRedundancy == 0)
        and     gRegSum,   gRegFastBitMask
        .endif

        popcnt  gRegWork1, gRegSum
        xor     gRegWork2, gRegWork2
        cmp     gRegWork1, candidatesNum

        # 候補があれば1
        # 次の分岐とまとめる
        adc     gRegWork2Byte, 0
10011:
        .if (TrimRedundancy == 0)
        and     gRegTarget, gRegFastBitMask
        .endif

        test    gRegSum, gRegTarget
        setnz   gRegWork1Byte
        add     gRegWork2Byte, gRegWork1Byte

        # 遠くに飛ぶと遅くなるのでできるだけ近くにする
        .if ((LastCellsToFilled != 0) && (\cellCount + LastCellsToFilled) >= maxElementNumber)
        # 前方には分岐しないのがデフォルトの分岐予測
        jz      10012f
        jmp     10013f
        .else
        jnz     10013f
        .endif
10012:
        jmp     gRegReturn

10013:
        .if ((\cellCount + 1) >= maxElementNumber)
        # すべてのマスが埋まった
        add     gRegCount, gRegOne64

        cmp     qword ptr [rip + sudokuXmmPrintAllCandidate], 0
        # 候補を表示しないに分岐しないようにして速くする
        jnz     10021f
        jmp     gRegReturn

10021:
        not     gRegSum
        and     gRegSum, gRegFastBitMask
        mov     qword ptr [rip + sudokuXmmRightBottomSolved], gRegSum64
        mov     qword ptr [rip + sudokuXmmAllPatternCnt], gRegCount
        mov     qword ptr [rip + sudokuXmmReturnAddr], gRegReturn

        FastPrintAllCells
        FastInitMaskRegister
        mov     gRegReturn, qword ptr [rip + sudokuXmmReturnAddr]
        mov     gRightBottomElement64, qword ptr [rip + sudokuXmmRightBottomElement]
        mov     gRegCount, qword ptr [rip + sudokuXmmAllPatternCnt]
        jmp     gRegReturn

        .else
10031:
        push    gRegReturn
        and     gRegTarget, gRegTarget
        jnz     10035f

10032:
        mov     gRegNewCandidate, gRegOne
        # 他の候補に使われていないものが1
        not     gRegSum
        and     gRegSum, gRegFastBitMask

10033:
        # CountUniqueCandidatesAtNextCellを使って展開すると却って遅くなる
        mov     gRegNewCandidate64, gRegOne64
        bsf     gRegShift, gRegSum
        shl     gRegNewCandidate, cl
        xor     gRegSum, gRegNewCandidate

        # push-popの方が固定アドレスより速い
        push    gRegSum64
        FastSetUniqueCandidatesAtCell gRegNewCandidate, \cellCount
        CallCountUniqueCandidatesAtCell (\cellCount + 1)
        pop     gRegSum64

        # 最右列と最下行は候補があったとしても高々一つ
         .if (LastCellsToFilled == 0) || (((\cellCount % numberOfColumns) + 1) != numberOfColumns) || ((\cellCount + LastCellsToFilled) < maxElementNumber)
        and     gRegSum, gRegSum
        jnz     10033b
         .endif
10034:
        # 0に書き戻す
        pop     gRegReturn
        xor     gRegWork1, gRegWork1
        FastSetUniqueCandidatesAtCell gRegWork1, \cellCount
        # retにするとかなり遅くなる
        # latencyはpop reg64=1, jmp reg=1, ret=8なので、pop+jmpの方が速い
        jmp     gRegReturn

10035:
        # 候補があればその値(ジャンプ先でret)
        JmpCountUniqueCandidatesAtCell (\cellCount + 1)
        .endif
.endm

.macro CountUniqueCandidatesAtCell cellCount
        .set outBoxShift, ((rowPartNum - 1) - ((\cellCount % candidatesNum) / rowPartNum))
        .set inBoxShift,  ((candidatesInRowPart - 1) - (\cellCount % candidatesInRowPart))
        .set rowNumber,   (\cellCount / numberOfRows)

        .if (rowNumber == 0)
        CountUniqueCandidatesAtCellLoop outBoxShift, inBoxShift, \cellCount, xRegRow1, xRegRow2, xRegRow3, rowNumber
        .elseif (rowNumber == 1)
        CountUniqueCandidatesAtCellLoop outBoxShift, inBoxShift, \cellCount, xRegRow2, xRegRow1, xRegRow3, rowNumber
        .elseif (rowNumber == 2)
        CountUniqueCandidatesAtCellLoop outBoxShift, inBoxShift, \cellCount, xRegRow3, xRegRow1, xRegRow2, rowNumber
        .elseif (rowNumber == 3)
        CountUniqueCandidatesAtCellLoop outBoxShift, inBoxShift, \cellCount, xRegRow4, xRegRow5, xRegRow6, rowNumber
        .elseif (rowNumber == 4)
        CountUniqueCandidatesAtCellLoop outBoxShift, inBoxShift, \cellCount, xRegRow5, xRegRow4, xRegRow6, rowNumber
        .elseif (rowNumber == 5)
        CountUniqueCandidatesAtCellLoop outBoxShift, inBoxShift, \cellCount, xRegRow6, xRegRow4, xRegRow5, rowNumber
        .elseif (rowNumber == 6)
        CountUniqueCandidatesAtCellLoop outBoxShift, inBoxShift, \cellCount, xRegRow7, xRegRow8, xRegRow9, rowNumber
        .elseif (rowNumber == 7)
        CountUniqueCandidatesAtCellLoop outBoxShift, inBoxShift, \cellCount, xRegRow8, xRegRow7, xRegRow9, rowNumber
        .elseif (rowNumber == 8)
        CountUniqueCandidatesAtCellLoop outBoxShift, inBoxShift, \cellCount, xRegRow9, xRegRow7, xRegRow8, rowNumber
        .endif
.endm

        .global sudokuXmmCountFromCell
sudokuXmmCountFromCell:
        mov     gRegWork64, sudokuMaxLoopcnt
        FastDec gRegWork64
        cmp     rax, gRegWork64
        cmova   rax, gRegWork64

        lea     gRegWork64, [rip + sudokuXmmCountFuncTable]
        ShlNonZero rax, funcPtrByteSizeLog2
        add     rax, gRegWork64

        mov     gRightBottomElement64, qword ptr [rip + sudokuXmmRightBottomElement]
        mov     qword ptr [rip + sudokuXmmDebug], 0
        xor     gRegCount, gRegCount

        FastInitMaskRegister
        call    [rax]
        mov     qword ptr [rip + sudokuXmmAllPatternCnt], gRegCount
        ret

countAt00:
        CountUniqueCandidatesAtCell 0
countAt01:
        CountUniqueCandidatesAtCell 1
countAt02:
        CountUniqueCandidatesAtCell 2
countAt03:
        CountUniqueCandidatesAtCell 3
countAt04:
        CountUniqueCandidatesAtCell 4
countAt05:
        CountUniqueCandidatesAtCell 5
countAt06:
        CountUniqueCandidatesAtCell 6
countAt07:
        CountUniqueCandidatesAtCell 7
countAt08:
        CountUniqueCandidatesAtCell 8
countAt09:
        CountUniqueCandidatesAtCell 9

countAt10:
        CountUniqueCandidatesAtCell 10
countAt11:
        CountUniqueCandidatesAtCell 11
countAt12:
        CountUniqueCandidatesAtCell 12
countAt13:
        CountUniqueCandidatesAtCell 13
countAt14:
        CountUniqueCandidatesAtCell 14
countAt15:
        CountUniqueCandidatesAtCell 15
countAt16:
        CountUniqueCandidatesAtCell 16
countAt17:
        CountUniqueCandidatesAtCell 17
countAt18:
        CountUniqueCandidatesAtCell 18
countAt19:
        CountUniqueCandidatesAtCell 19

countAt20:
        CountUniqueCandidatesAtCell 20
countAt21:
        CountUniqueCandidatesAtCell 21
countAt22:
        CountUniqueCandidatesAtCell 22
countAt23:
        CountUniqueCandidatesAtCell 23
countAt24:
        CountUniqueCandidatesAtCell 24
countAt25:
        CountUniqueCandidatesAtCell 25
countAt26:
        CountUniqueCandidatesAtCell 26
countAt27:
        CountUniqueCandidatesAtCell 27
countAt28:
        CountUniqueCandidatesAtCell 28
countAt29:
        CountUniqueCandidatesAtCell 29

countAt30:
        CountUniqueCandidatesAtCell 30
countAt31:
        CountUniqueCandidatesAtCell 31
countAt32:
        CountUniqueCandidatesAtCell 32
countAt33:
        CountUniqueCandidatesAtCell 33
countAt34:
        CountUniqueCandidatesAtCell 34
countAt35:
        CountUniqueCandidatesAtCell 35
countAt36:
        CountUniqueCandidatesAtCell 36
countAt37:
        CountUniqueCandidatesAtCell 37
countAt38:
        CountUniqueCandidatesAtCell 38
countAt39:
        CountUniqueCandidatesAtCell 39

countAt40:
        CountUniqueCandidatesAtCell 40
countAt41:
        CountUniqueCandidatesAtCell 41
countAt42:
        CountUniqueCandidatesAtCell 42
countAt43:
        CountUniqueCandidatesAtCell 43
countAt44:
        CountUniqueCandidatesAtCell 44
countAt45:
        CountUniqueCandidatesAtCell 45
countAt46:
        CountUniqueCandidatesAtCell 46
countAt47:
        CountUniqueCandidatesAtCell 47
countAt48:
        CountUniqueCandidatesAtCell 48
countAt49:
        CountUniqueCandidatesAtCell 49

countAt50:
        CountUniqueCandidatesAtCell 50
countAt51:
        CountUniqueCandidatesAtCell 51
countAt52:
        CountUniqueCandidatesAtCell 52
countAt53:
        CountUniqueCandidatesAtCell 53
countAt54:
        CountUniqueCandidatesAtCell 54
countAt55:
        CountUniqueCandidatesAtCell 55
countAt56:
        CountUniqueCandidatesAtCell 56
countAt57:
        CountUniqueCandidatesAtCell 57
countAt58:
        CountUniqueCandidatesAtCell 58
countAt59:
        CountUniqueCandidatesAtCell 59

countAt60:
        CountUniqueCandidatesAtCell 60
countAt61:
        CountUniqueCandidatesAtCell 61
countAt62:
        CountUniqueCandidatesAtCell 62
countAt63:
        CountUniqueCandidatesAtCell 63
countAt64:
        CountUniqueCandidatesAtCell 64
countAt65:
        CountUniqueCandidatesAtCell 65
countAt66:
        CountUniqueCandidatesAtCell 66
countAt67:
        CountUniqueCandidatesAtCell 67
countAt68:
        CountUniqueCandidatesAtCell 68
countAt69:
        CountUniqueCandidatesAtCell 69

countAt70:
        CountUniqueCandidatesAtCell 70
countAt71:
        CountUniqueCandidatesAtCell 71
countAt72:
        CountUniqueCandidatesAtCell 72
countAt73:
        CountUniqueCandidatesAtCell 73
countAt74:
        CountUniqueCandidatesAtCell 74
countAt75:
        CountUniqueCandidatesAtCell 75
countAt76:
        CountUniqueCandidatesAtCell 76
countAt77:
        CountUniqueCandidatesAtCell 77
countAt78:
        CountUniqueCandidatesAtCell 78
countAt79:
        CountUniqueCandidatesAtCell 79

# 80=最後のマス以降は同じマクロ
countAt80:
countAt81:
        CountUniqueCandidatesAtCell 80
