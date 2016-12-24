# Sudoku solver with SSE 4.2 / AVX
# Copyright (C) 2012-2016 Zettsu Tatsuya

.intel_syntax noprefix
.file   "sudokusse.s"
        # Makefile designates these flags and they should not be hard-coded here.
        # Set EnableAvx to 1 for using AVX, 0 for SSE
        # .set    EnableAvx, 1

        # Set CellsPacked to 1 to assume that initial cells in a
        # sudoku puzzle are packed in its top-left.
        # .set    CellsPacked, 0

        # Set LastCellsToFilled to 10 to assume each cell in the
        # bottom row in a puzzle has a unique number.
        .set    LastCellsToFilled, 10

        # Set TrimRedundancy to 1 to eliminate verifications in macros
        .set    TrimRedundancy, 1

        # Set UseReg64Most to 1 to use 64-bit registers most, 0 to use
        # 32-bit registers in counting solutions of a puzzle
        .set    UseReg64Most, 1

        # Variables that this assembly and C++ code share
        .global sudokuXmmAborted
        .global sudokuXmmNextCellFound
        .global sudokuXmmNextOutBoxShift
        .global sudokuXmmNextInBoxShift
        .global sudokuXmmNextRowNumber
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

        # Variables that this assembly and unit tests share
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

# Variables that this assembly and C++ code share
.set  sudokuMaxLoopcnt,         81         # The upper limit of number of iterations in filling cells
sudokuXmmAborted:               .quad 0    # Non-zero if there is inconsistency of cells
sudokuXmmNextCellFound:         .quad 0    # Non-zero if guessing a candidate in a cell for backtracking
sudokuXmmNextOutBoxShift:       .quad 0    # Box number in a row of the cell in the guess (0..2)
sudokuXmmNextInBoxShift:        .quad 0    # Horizontal cell number in the box in the guess (0..2)
sudokuXmmNextRowNumber:         .quad 0    # Row number of the cell in the guess (0..8)
sudokuXmmElementCnt:            .quad 0    # Number of the filled cells (cells with unique candidates) (0..81)
sudokuXmmPrintAllCandidate:     .quad 0    # Non-zero if printing candidates of a puzzle
sudokuXmmRightBottomElement:    .quad 0    # Initial candidates of the bottom-right cell in a puzzle
sudokuXmmRightBottomSolved:     .quad 0    # Solved candidate of the bottom-right cell in a puzzle
sudokuXmmAllPatternCnt:         .quad 0    # Number of solutions in a puzzle
sudokuXmmPrintFunc:             .quad 0    # Address of a C++ function to print a puzzle
sudokuXmmStackPointer:          .quad 0    # RSP register value before calling sudokuXmmPrintFunc
sudokuXmmReturnAddr:            .quad 0    # Address to return after counting solutions
sudokuXmmAssumeCellsPacked:     .quad CellsPacked
sudokuXmmUseAvx:                .quad EnableAvx
sudokuXmmDebug:                 .quad 0    # Value for debugging assembly macros

# Candidates of 81 cells
.set  arrayElementByteSize, 8
.align 16
sudokuXmmCandidateArray:  .quad 0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0
                          .quad 0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0
                          .quad 0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0 ,0

# Symbol table to count candidates at each cell
.set  funcPtrByteSize, 8        # Size in bytes of address to jump
.set  funcPtrByteSizeLog2, 3    # Log2 of funcPtrByteSize (i.e. number of left bit-shift)

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

# Variables that this assembly and unit tests share
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

# 128-bit XMM registers
.align 16
testFillNineUniqueCandidatesRowX:             .quad 0, 0
testFillNineUniqueCandidatesBoxX:             .quad 0, 0
testFillNineUniqueCandidatesColumnX:          .quad 0, 0
testCountRowCellCandidatesRowX:               .quad 0, 0


.text
# Constants
.set    maxElementNumber,   81  # Number of candidates of a solved puzzle
.set    boxRowByteSize,      4  # Size in bytes for a row of a 3x3 cell box
.set    bitPerByte,          8  # Size in bits per byte
.set    candidatesNum,       9  # Maximum number of candidates in a cell
.set    candidatesTooMany,  10  # Indicating an out-of-bounds number of candidates
.set    numberOfCellsInRow,  9  # Number of cells in a row
.set    numberOfRows,        9  # Number of rows
.set    numberOfColumns,     9  # Number of columns
.set    candidatesInRowPart, 3  # Number of cells in a row of a box
.set    rowPartNum,          3  # Number of boxes in a row

.set    inBoxIndexInvalid,  (candidatesInRowPart + 1)  # Indicating an out-of-bounds index of cells in a box
.set    outBoxIndexInvalid, (rowPartNum + 1)           # Indicating an out-of-bounds index of boxes
.set    rowNumberInvalid,   (numberOfRows + 1)         # Indicating an out-of-bounds index of rows
.set    nextCellFound,            1  # Indicating it found a cell and guess a its candidate for backtracking
.set    minCandidatesNumToSearch, 2  # Minimum number of cadidates to guess a candidate in a cell

.set    elementBitMask, 0x1ff      # Bit mask for candidates of a cell
.set    rowPartBitMask, 0x7ffffff  # Bit mask for candidates of three cells in a row of a box
.set    rowPartBitMask0, ((elementBitMask << (candidatesNum * 2)) | (elementBitMask << candidatesNum))
.set    rowPartBitMask1, ((elementBitMask << (candidatesNum * 2)) | elementBitMask)
.set    rowPartBitMask2, ((elementBitMask << candidatesNum) | elementBitMask)

# Assigned registers
# Bitwise OR of all rows
.set    xRegRowAll, xmm0
# Rows 1 to 9
.set    xRegRow1, xmm1
.set    xRegRow2, xmm2
.set    xRegRow3, xmm3
.set    xRegRow4, xmm4
.set    xRegRow5, xmm5
.set    xRegRow6, xmm6
.set    xRegRow7, xmm7
.set    xRegRow8, xmm8
.set    xRegRow9, xmm9
# Bitwise OR of rows in a box
.set    xRegRow1to3, xmm10
.set    xRegRow4to6, xmm11
.set    xRegRow7to9, xmm12
# Others
.set    xRegWork1,     xmm13
.set    xRegWork2,     xmm14
.set    xLoopPopCnt,   xmm15
.set    xRegColumnAll, xmm13
 # Bitmask for candidates in a cell and its 32-bit register alias
.set    gRegBitMask,  r15
.set    gRegBitMaskD, r15d

# NOTICE : every unit test must begin with InitMaskRegister. Almost macros need it.
# Labels in general macros must have number 3000 or later

.macro InitMaskRegister
        mov  gRegBitMaskD, elementBitMask
.endm

# Macros that switches SSE and AVX
.macro MacroMovd op1, op2
.if (EnableAvx != 0)
    vmovd \op1, \op2
.else
    movd  \op1, \op2
.endif
.endm

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

# AVX instructions can take three operands
.macro MacroPinsrq regDstX, regSrcX, op3
.if (EnableAvx != 0)
    vpinsrq \regDstX, \regDstX, \regSrcX, \op3
.else
    pinsrq  \regDstX, \regSrcX, \op3
.endif
.endm

.macro MacroPinsrd regDstX, regSrcX, op3
.if (EnableAvx != 0)
    vpinsrd \regDstX, \regDstX, \regSrcX, \op3
.else
    pinsrd  \regDstX, \regSrcX, \op3
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

# Comparing G/LE (signed) has low latency than A/BE (unsigned).
# If signedness of integers does not matter (i.e. less than 0x7fffffff),
# we can choose faster instructions for jump and CMOVcc.
.macro CmovLessEqual op1, op2
    cmovle  \op1, \op2
.endm

.macro JumpIfGreater op1
    jg   \op1
.endm

.macro JumpIfGreaterEqual op1
    jge  \op1
.endm

.macro JumpIfLessEqual op1
    jle  \op1
.endm

    .global testCmovLessEqual
    .global testJumpIfGreater
    .global testJumpIfGreaterEqual
    .global testJumpIfLessEqual
testCmovLessEqual:
    xor  rax, rax
    mov  rdx, 1
    cmp  rbx, rcx
    CmovLessEqual rax, rdx
    ret

testJumpHit:
    mov  rax, 1
    ret

.macro TestJumpIf op1
    xor  rax, rax
    cmp  rbx, rcx
    \op1 testJumpHit
    ret
.endm

testJumpIfGreater:
    TestJumpIf JumpIfGreater

testJumpIfGreaterEqual:
    TestJumpIf JumpIfGreaterEqual

testJumpIfLessEqual:
    TestJumpIf JumpIfLessEqual

# Eliminate 0-times shift that is useless and just wastes I-cache.
.macro ShlNonZero reg, count
.if (\count != 0)
        shl  \reg, \count
.endif
.endm

.macro ShrNonZero reg, count
.if (\count != 0)
        shr  \reg, \count
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

# It is better using ADD/SUB 1 instead of INC/DEC because the former
# updates all processor flags and avoids partial flag dependency.
.macro FastInc reg
        add    \reg, 1
.endm

.macro FastDec reg
        sub    \reg, 1
.endm

.macro IsPowerOf2ToFlags regSrc, regWork1
        popcnt  \regWork1, \regSrc
        cmp     \regWork1, 1
.endm

        .global testIsPowerOf2ToFlags
testIsPowerOf2ToFlags:
        mov     rbx, 0
        IsPowerOf2ToFlags rax, rcx
        JumpIfGreater testIsPowerOf2ToFlagsA
        FastInc rbx
testIsPowerOf2ToFlagsA:
        IsPowerOf2ToFlags eax, ecx
        JumpIfGreater testIsPowerOf2ToFlagsB
        FastInc rbx
testIsPowerOf2ToFlagsB:
        ret

# Set regSrc to regDst if regSrc is power of 2, otherwise set 0 to regDst
# All registers must be 64-bit or 32-bit width.
.macro PowerOf2or0 regDst, regSrc, regWork1
        popcnt  \regWork1, \regSrc
        xor     \regDst,   \regDst
        cmp     \regWork1, 1
        cmovz   \regDst,   \regSrc
.endm

        .global testPowerOf2or064
        .global testPowerOf2or032
testPowerOf2or064:
        PowerOf2or0 rbx, rax, rcx
        ret

testPowerOf2or032:
        PowerOf2or0 ebx, eax, ecx
        ret

# Set regSrc to regDst if regSrc is power of 2 and non-zero, otherwise set all 1's to regDst
.macro PowerOf2orAll1 regDst, regSrc, regWork1
        # mov -1 is faster than xor reg, reg and not
        popcnt  \regWork1, \regSrc
        mov     \regDst,   -1
        cmp     \regWork1, 1
        cmovz   \regDst,   \regSrc
.endm

        .global testPowerOf2orAll164
        .global testPowerOf2orAll132
testPowerOf2orAll164:
        PowerOf2orAll1 rbx, rax, rcx
        ret

testPowerOf2orAll132:
        PowerOf2orAll1 ebx, eax, ecx
        ret

# Fold candidates in three cells
# 64-bit registers are faster than 32-bit in this macro
.macro MergeThreeElementsCommon regSum, regWork1, regWork2, regBitmask
        mov    \regWork1, \regSum
        mov    \regWork2, \regSum
        ShrNonZero  \regWork1, (candidatesNum * 2)
        ShrNonZero  \regWork2, (candidatesNum * 1)
        or     \regSum,  \regWork1
        or     \regSum,  \regWork2
        and    \regSum,  \regBitmask
.endm

.macro MergeThreeElements64 regSum, regWork1, regWork2
        MergeThreeElementsCommon \regSum, \regWork1, \regWork2, gRegBitMask
.endm

.macro MergeThreeElements32 regSumD, regWork1D, regWork2D
        MergeThreeElementsCommon \regSumD, \regWork1D, \regWork2D, gRegBitMaskD
.endm

        .global testMergeThreeElements64
        .global testMergeThreeElements32
testMergeThreeElements64:
        InitMaskRegister
        mov     rbx, rax
        MergeThreeElements64 rbx, rcx, rdx
        ret

testMergeThreeElements32:
        InitMaskRegister
        mov     rbx, rax
        MergeThreeElements32 ebx, ecx, edx
        ret

# Fold candidates in three rows
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

# For a cell in contiguous three cells, clear candidates of the cell if its candidate is not unique.
# All registers are 64-bit registers.
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

# Leave unique candidates and clear others in consecutive three cells.
# regRowPart is a 64-bit registger and must have only three cells,
# cannot contain other three cells in its upper/lower 32 bits.
.macro FilterUniqueCandidatesInRowPart regDst, regRowPart, shift32bit, regWork1, regWork2, regWork3, regWork4
        popcnt \regWork2, \regRowPart
        mov    \regDst,   \regRowPart
        cmp    \regWork2, candidatesInRowPart
        jz 3001f

        .set   regBitmask, \regWork1
        mov    regBitmask, gRegBitMask

        # Filter right
        ShlNonZero  regBitmask, (\shift32bit * boxRowByteSize * bitPerByte)
        mov    \regWork3, \regRowPart
        and    \regWork3, regBitmask
        PowerOf2or0 \regDst, \regWork3, \regWork4

        # Filter middle
        FilterUniqueCandidatesInRowPartSub regBitmask, \regDst, \regRowPart, \regWork2, \regWork3, \regWork4
        # Filter left
        FilterUniqueCandidatesInRowPartSub regBitmask, \regDst, \regRowPart, \regWork2, \regWork3, \regWork4
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

# Collect unique candidates in consecutive three cells
# All registers are 32-bit registers
.macro CollectUniqueCandidatesInRowPart regSumD, regHasZeroD, regRowPartD, regWork1D, regWork2D, regWork3D, regWork4D, regWork5D
        # Collect for the right cell parallel
        .set   regZeroCheckerD, \regWork1D
        .set   regElement0SrcD, \regWork2D

        mov    regElement0SrcD, \regRowPartD
        and    regElement0SrcD, gRegBitMaskD
        mov    regZeroCheckerD, regElement0SrcD
        PowerOf2or0 \regSumD, regElement0SrcD, \regWork3D

        # Collect for the middle cell parallel
        .set   regElement1SrcD, \regWork4D
        .set   regElement1DstD, \regWork5D

        mov    regElement1SrcD, \regRowPartD
        ShrNonZero  regElement1SrcD, (candidatesNum * 1)
        and    regElement1SrcD, gRegBitMaskD

        # Interleaving for the right and middle cells
        # Set all 1's if the cell has zero (0xffffffff, not 0xffffffffffffffff)
        FastDec regZeroCheckerD
        or      \regHasZeroD, regZeroCheckerD
        PowerOf2or0 regElement1DstD, regElement1SrcD, \regWork2D

        mov     regZeroCheckerD, regElement1SrcD
        or      \regSumD, regElement1DstD
        FastDec regZeroCheckerD
        or      \regHasZeroD, regZeroCheckerD

        # Collect for the left cell
        .set   regElement2SrcD, \regWork3D
        .set   regElement2DstD, \regWork4D
        mov    regElement2SrcD, \regRowPartD
        ShrNonZero  regElement2SrcD, (candidatesNum * 2)

        mov     regZeroCheckerD, regElement2SrcD
        PowerOf2or0 regElement2DstD, regElement2SrcD, \regWork5D
        FastDec regZeroCheckerD
        or      \regHasZeroD, regZeroCheckerD
        or      \regSumD, regElement2DstD
.endm

        .global testCollectUniqueCandidatesInRowPart
testCollectUniqueCandidatesInRowPart:
        InitMaskRegister
        xor     rbx, rbx
        xor     rcx, rcx
        CollectUniqueCandidatesInRowPart ebx, ecx, eax, r10d, r11d, r12d, r13d, r14d
        ret

# Collect unique candidates in a row
# All registers are 32-bit registers
.macro CollectUniqueCandidatesInLine regDstD, regSrcX, regWork1D, regWork2D, regWork3D, regWork4D, regWork5D, regWork6D, regWork7D, regWork8D
        .set   regHasZeroD, \regWork1D
        .set   regThreeElementsD, \regWork2D
        .set   regUniqueThreeElementsD, \regWork3D

        MacroPextrd regThreeElementsD, \regSrcX, 2
        xor    regHasZeroD, regHasZeroD
        CollectUniqueCandidatesInRowPart \regDstD, regHasZeroD, regThreeElementsD, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D

        # clear upper 32 bits
        MacroPextrd regThreeElementsD, \regSrcX, 0
        CollectUniqueCandidatesInRowPart regUniqueThreeElementsD, regHasZeroD, regThreeElementsD, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D

        # clear upper 32 bits
        MacroPextrd regThreeElementsD, \regSrcX, 1
        or     \regDstD, regUniqueThreeElementsD
        CollectUniqueCandidatesInRowPart regUniqueThreeElementsD, regHasZeroD, regThreeElementsD, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D
        or      \regDstD, regUniqueThreeElementsD

        # Break if a cell has no candidates
        or      regHasZeroD, regHasZeroD
        js      abortFilling
.endm

        .global testCollectUniqueCandidatesInLine
testCollectUniqueCandidatesInLine:
        InitMaskRegister
        xor     rax, rax
        mov     qword ptr [rip + sudokuXmmAborted], 0
        CollectUniqueCandidatesInLine eax, xRegRow1, ebx, ecx, r8d, r9d, r10d, r11d, r12d, r13d
        ret

# Leave unique candidates and clear others in consecutive nine cells.
# All registers are 64-bit registers except regWork1D
.macro FilterUniqueCandidatesInLine regDstX, regSrcX, regWork1, regWork2, regWork3, regWork4, regWork5, regWork6, regWork7, regWork1D, regWorkX
        .set    regRowPart,   \regWork1
        .set    regRowPartD,  \regWork1D
        .set    regElements1, \regWork2
        .set    regElements2, \regWork3

        MacroPextrd regRowPartD, \regSrcX, 2
        FilterUniqueCandidatesInRowPart regElements1, regRowPart, 0, \regWork4, \regWork5, \regWork6, \regWork7
        MacroPinsrq \regDstX,  regElements1, 1

        MacroPextrd regRowPartD, \regSrcX, 0
        FilterUniqueCandidatesInRowPart regElements1, regRowPart, 0, \regWork4, \regWork5, \regWork6, \regWork7

        MacroPextrd regRowPartD, \regSrcX, 1
        ShlNonZero  regRowPart, (boxRowByteSize * bitPerByte)
        FilterUniqueCandidatesInRowPart regElements2, regRowPart, 1, \regWork4, \regWork5, \regWork6, \regWork7
        or          regElements2, regElements1
        MacroPinsrq \regDstX, regElements2, 0
.endm

        .global testFilterUniqueCandidatesInLine
testFilterUniqueCandidatesInLine:
        InitMaskRegister
        mov     qword ptr [rip + sudokuXmmAborted], 0
        FilterUniqueCandidatesInLine xRegWork2, xRegRow1, r8, r9, r10, r11, r12, r13, r14, r8d, xRegWork1
        ret

# Fold candidates in three rows
.macro CollectUniqueCandidatesInThreeLine regDstX, regSrc1X, regSrc2X, regSrc3X, regWork1, regWork2, regWork3, regWork4, regWork5, regWork6, regWork7, regWork1D, regWork1X, regWork2X
        FilterUniqueCandidatesInLine \regDstX,   \regSrc1X, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5, \regWork6, \regWork7, \regWork1D, \regWork1X
        FilterUniqueCandidatesInLine \regWork2X, \regSrc2X, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5, \regWork6, \regWork7, \regWork1D, \regWork1X
        orps   \regDstX, \regWork2X
        FilterUniqueCandidatesInLine \regWork2X, \regSrc3X, \regWork1, \regWork2, \regWork3, \regWork4, \regWork5, \regWork6, \regWork7, \regWork1D, \regWork1X
        orps   \regDstX, \regWork2X
.endm

        .global testCollectUniqueCandidatesInThreeLine
testCollectUniqueCandidatesInThreeLine:
        InitMaskRegister
        mov     qword ptr [rip + sudokuXmmAborted], 0
        CollectUniqueCandidatesInThreeLine xRegRow1to3, xRegRow1, xRegRow2, xRegRow3, r8, r9, r10, r11, r12, r13, r14, r8d, xRegWork1, xRegWork2
        ret

# Extract a contiguous three cell set in a row
.macro SelectRowParts regDstD, regRowX, outBoxShift
        # Zero-extended for upper bits
        MacroPextrd \regDstD, \regRowX, \outBoxShift
.endm

.macro TestSelectRowParts outBoxShift
        InitMaskRegister
        SelectRowParts  eax, xRegRow1, \outBoxShift
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

# Extract nine cells in a row and set ZF to 1 if all cells have unique candidates
.macro SelectRowAndCount regDstHigh, regDstLow, regRowX, regWork
        MacroPextrq \regDstHigh, \regRowX, 1
        mov     \regWork,    \regDstHigh
        MacroPextrq \regDstLow,  \regRowX, 0
        or      \regWork, \regDstLow
        popcnt  \regWork, \regWork
        cmp     \regWork, numberOfCellsInRow
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

# Extract a cell in a register
.macro SelectElementInRowParts regDstD, regSrcD, inBoxShift
        mov    \regDstD, \regSrcD
        ShrNonZero  \regDstD, (candidatesNum * \inBoxShift)
        .if (\inBoxShift != 2)
        and    \regDstD, gRegBitMaskD
        .endif
.endm

.macro TestSelectElementInRowParts inBoxShift
        InitMaskRegister
        SelectElementInRowParts ebx, eax, \inBoxShift
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

# Labels in macros that fill cells must have number 1000 or later.
# Fill a cell with its unique candidate if any, otherwise decrease its candidates.
.macro FillUniqueElement regRowPartD, regCandidateSetD, regColumnD, regMergedBoxD, regMergedRowD, inBoxShift, regWork1D, regWork2D, regWork3D, regWork4D, regWork5D, regWork1X, regWork2X
        .set   regBitmaskD,   \regWork1D
        .set   regCandidateD, \regWork2D
        .set   regShiftedBitmaskD,   \regWork3D
        .set   regShiftedCandidateD, \regWork4D
        .set   regUniqueCandidateD,  \regWork5D

        # Interleave making bit masks and others
        mov    regBitmaskD, gRegBitMaskD
        mov    regShiftedBitmaskD, gRegBitMaskD
        ShlNonZero  regShiftedBitmaskD, (candidatesNum * \inBoxShift)

        # Collect candidates in a row, column and box
.if (EnableAvx != 0)
        andn   regCandidateD, \regCandidateSetD, regBitmaskD
.else
        mov    regCandidateD, \regCandidateSetD
        not    regCandidateD
        and    regCandidateD, regBitmaskD
.endif
        not    regBitmaskD

        # regCandidate = numbers not used in other cells
        or     regBitmaskD, regCandidateD
        RolNonZero  regBitmaskD, (candidatesNum * \inBoxShift)
        # Leave candidates in other cells
        and    \regRowPartD, regBitmaskD

        mov    regUniqueCandidateD, \regRowPartD
        and    regUniqueCandidateD, regShiftedBitmaskD
        PowerOf2or0 regShiftedCandidateD, regUniqueCandidateD, \regWork1D

        or     \regColumnD, regShiftedCandidateD
        # Numbers which is used in a box or row
        ShrNonZero regShiftedCandidateD, (candidatesNum * \inBoxShift)
        or     \regMergedBoxD, regShiftedCandidateD
        or     \regMergedRowD, regShiftedCandidateD
.endm

.macro testFillUniqueElement inBoxShift
        InitMaskRegister
        mov     rbx, [rip + testFillUniqueElementPreRowPart]
        mov     rcx, [rip + testFillUniqueElementPreColumn]
        mov     rdx, [rip + testFillUniqueElementPreBox]
        mov     rsi, [rip + testFillUniqueElementPreRow]
        FillUniqueElement ebx, eax, ecx, edx, esi, \inBoxShift, r10d, r11d, r12d, r13d, r14d, xRegWork1, xRegWork2
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

.macro FillOneUniqueCandidates regRowPartD, regMergedBoxD, regMergedRowD, regColumnD, inBoxShift, regWork1D, regWork2D, regWork3D, regWork4D, regWork5D, regWork6D, regWork1X, regWork2X
        .set   regElementD, \regWork1D
        SelectElementInRowParts regElementD, \regRowPartD, \inBoxShift
        IsPowerOf2ToFlags regElementD, \regWork2D
        JumpIfLessEqual 1131f

        .set   regCandidateSetD, \regWork1D
        SelectElementInRowParts regCandidateSetD, \regColumnD, \inBoxShift
        or     regCandidateSetD, \regMergedBoxD
        or     regCandidateSetD, \regMergedRowD
        FillUniqueElement \regRowPartD, regCandidateSetD, \regColumnD, \regMergedBoxD, \regMergedRowD, \inBoxShift, \regWork2D, \regWork3D, \regWork4D, \regWork5D, \regWork6D, \regWork1X, \regWork2X
1131:
.endm

.macro testFillOneUniqueCandidates inBoxShift
        InitMaskRegister
        mov     rax, [rip + testFillUniqueElementPreRowPart]
        mov     rbx, [rip + testFillUniqueElementPreColumn]
        mov     rcx, [rip + testFillUniqueElementPreBox]
        mov     rdx, [rip + testFillUniqueElementPreRow]
        FillOneUniqueCandidates eax, ecx, edx, ebx, \inBoxShift, r9d, r10d, r11d, r12d, r13d, r14d, xRegWork1, xRegWork2
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

.macro SaveLineSet regNewElementD, regResultBoxX, regResultRowX, outBoxShift, regWork1D, regWork2D, regWork3D, regWork4D, regWork1X, regWork2X
        .set  regCandidateD, \regWork1D
        .set  regBitmaskD,   \regWork2D
        .set  regCandidateX, \regWork1X
        .set  regBitmaskX,   \regWork2X

        # Set all bits to 1
.if (EnableAvx != 0)
        vpcmpeqw \regWork1X, \regWork1X, \regWork1X
.else
        pcmpeqw  \regWork1X, \regWork1X
.endif

        # Bit masks
        mov    regBitmaskD, rowPartBitMask
        MacroMovd regBitmaskX, regBitmaskD
        PslldqNonZero regBitmaskX, (boxRowByteSize * \outBoxShift)
        MacroAndnps regBitmaskX, \regWork1X

        mov    regCandidateD, \regNewElementD
        MacroMovd regCandidateX, regCandidateD
        PslldqNonZero regCandidateX, (boxRowByteSize * \outBoxShift)
        MacroOrps  regBitmaskX, regCandidateX
        MacroAndps \regResultRowX, regBitmaskX

        popcnt \regWork4D, \regNewElementD
        cmp    \regWork4D, 3
        # Jump is faster than CMOVcc reg, 0 in this case
        JumpIfGreater 1121f

        MacroMovd regCandidateX, \regNewElementD
        PslldqNonZero regCandidateX, (boxRowByteSize * \outBoxShift)
        MacroOrps xRegRowAll, regCandidateX
        MacroOrps \regResultBoxX, regCandidateX
1121:
.endm

.macro TestSaveLineSet inBoxShift
        InitMaskRegister
        SaveLineSet eax, xRegRow1to3, xRegRow1, \inBoxShift, r11d, r12d, r13d, r14d, xRegWork1, xRegWork2
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

# Set a unique candidate to a cell if it is not used for other cells
.macro MaskElementCandidates regRowPartD, regOtherCandidatesD, inBoxShift, regWork1D, regWork2D, regWork3D
        .set regAllCandidatesD, \regWork1D

.if (EnableAvx != 0)
        andn regAllCandidatesD, \regOtherCandidatesD, gRegBitMaskD
.else
        mov  regAllCandidatesD, \regOtherCandidatesD
        not  regAllCandidatesD
        and  regAllCandidatesD, gRegBitMaskD
.endif
        PowerOf2orAll1 \regWork2D, regAllCandidatesD, \regWork3D

        mov  \regWork1D, gRegBitMaskD
        not  \regWork1D
        or   \regWork1D, \regWork2D
        RolNonZero  \regWork1D, (candidatesNum * \inBoxShift)
        and  \regRowPartD, \regWork1D
.endm

.macro TestMaskElementCandidates inBoxShift
        InitMaskRegister
        mov  rcx, rax
        MaskElementCandidates ecx, ebx, \inBoxShift, r12d, r13d, r14d
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

# Merge candidates in two cells (overwriting regSum)
.macro MergeTwoElementsImpl regSumD, shiftLeft1, shiftLeft2, inBoxShift, regWorkD
        # Parrnell for each bit mask
        .if (\inBoxShift == 0)
        mov    \regWorkD, \regSumD
        ShrNonZero  \regSumD,  (candidatesNum * 1)
        and    \regSumD, gRegBitMaskD
        ShrNonZero  \regWorkD, (candidatesNum * 2)
        or     \regSumD, \regWorkD
        .elseif (\inBoxShift == 1)
        mov    \regWorkD, \regSumD
        ShrNonZero  \regWorkD, (candidatesNum * 2)
        and    \regSumD, gRegBitMaskD
        or     \regSumD, \regWorkD
        .else
        # Generic
        mov    \regWorkD, \regSumD
        ShrNonZero  \regWorkD, (candidatesNum * \shiftLeft2)
        ShrNonZero  \regSumD,  (candidatesNum * \shiftLeft1)
        or     \regSumD, \regWorkD
        and    \regSumD, gRegBitMaskD
        .endif
.endm

.macro MergeTwoElements regSumD, inBoxShift, regWorkD
        .if (\inBoxShift == 0)
        MergeTwoElementsImpl \regSumD, 1, 2, 0, \regWorkD
        .elseif (\inBoxShift == 1)
        MergeTwoElementsImpl \regSumD, 0, 2, 1, \regWorkD
        .elseif (\inBoxShift == 2)
        MergeTwoElementsImpl \regSumD, 0, 1, 2, \regWorkD
        .endif
.endm

.macro TestMergeTwoElements inBoxShift
        InitMaskRegister
        mov     rbx, rax
        MergeTwoElements ebx, \inBoxShift, r14d
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

# Find candidates in a cell on a row
.macro FindElementCandidates regRowPartD, regRowCandidatesD, regBoxD, regColumnRowPartD, inBoxShift, regWork1D, regWork2D, regWork3D, regWork4D, regWork5D, regWork1X, regWork2X
        .set  regTwoElementsD, \regWork1D
        .set  regElementD, \regWork2D
        SelectElementInRowParts regElementD, \regRowPartD, \inBoxShift
        IsPowerOf2ToFlags regElementD, \regWork3D
        JumpIfLessEqual 1081f

        # Row
        mov  regElementD, \regRowPartD
        MergeTwoElements regElementD, \inBoxShift, \regWork3D
        mov  regTwoElementsD, regElementD
        or   regElementD, \regRowCandidatesD
        MaskElementCandidates \regRowPartD, regElementD, \inBoxShift, \regWork3D, \regWork4D, \regWork5D

        mov  \regWork3D, \regRowPartD
        SelectElementInRowParts regElementD, \regWork3D, \inBoxShift
        IsPowerOf2ToFlags regElementD, \regWork3D
        JumpIfLessEqual 1081f

        # Box
        mov  regElementD, regTwoElementsD
        or   regElementD, \regBoxD
        MaskElementCandidates \regRowPartD, regElementD, \inBoxShift, \regWork3D, \regWork4D, \regWork5D

        mov  \regWork3D, \regRowPartD
        SelectElementInRowParts regElementD, \regWork3D, \inBoxShift
        IsPowerOf2ToFlags regElementD, \regWork3D
        JumpIfLessEqual 1081f

        # Column
        SelectElementInRowParts regElementD, \regColumnRowPartD, \inBoxShift
        MaskElementCandidates \regRowPartD, regElementD, \inBoxShift, \regWork3D, \regWork4D, \regWork5D
1081:
.endm

.macro TestFindElementCandidates inBoxShift
        InitMaskRegister
        mov     rsi, rax
        FindElementCandidates esi, ebx, ecx, edx, \inBoxShift, r10d, r11d, r12d, r13d, r14d, xRegWork1, xRegWor2k
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

# Find candidates in consecutive three cells on a row
.macro FindThreePartsCandidates regRowPartD, regCandidatesD, regBoxD, regColumnRowPartD, regWork1D, regWork2D, regWork3D, regWork4D, regWork5D, regWork1X, regWork2X
        FindElementCandidates \regRowPartD, \regCandidatesD, \regBoxD, \regColumnRowPartD, 2, \regWork1D, \regWork2D, \regWork3D, \regWork4D, \regWork5D, \regWork1X, \regWork2X
        FindElementCandidates \regRowPartD, \regCandidatesD, \regBoxD, \regColumnRowPartD, 1, \regWork1D, \regWork2D, \regWork3D, \regWork4D, \regWork5D, \regWork1X, \regWork2X
        FindElementCandidates \regRowPartD, \regCandidatesD, \regBoxD, \regColumnRowPartD, 0, \regWork1D, \regWork2D, \regWork3D, \regWork4D, \regWork5D, \regWork1X, \regWork2X
.endm

        .global testFindThreePartsCandidates
testFindThreePartsCandidates:
        InitMaskRegister
        mov     rsi, rax
        FindThreePartsCandidates esi, ebx, ecx, edx, r10d, r11d, r12d, r13d, r14d, xRegWork1, xRegWork2
        ret

# Collect unique candidates for three cells in a row, column and box
.macro FillThreePartsUniqueCandidates regMergedRowD, regBoxX, regRowX, regColumnX, outBoxShift, regWork1D, regWork2D, regWork3D, regWork4D, regWork5D, regWork6D, regWork7D, regWork8D, regWork9D, regWork1X, regWork2X
        .set  regMergedBoxD, \regWork1D
        .set  regColumnD,    \regWork2D
        .set  regRowPartD,   \regWork3D

        SelectRowParts regRowPartD, \regRowX, \outBoxShift
        popcnt \regWork4D, regRowPartD
        cmp    \regWork4D, 3
        JumpIfLessEqual 1061f

        SelectRowParts regMergedBoxD, \regBoxX, \outBoxShift
        SelectRowParts regColumnD, \regColumnX, \outBoxShift
        MergeThreeElements32  regMergedBoxD, \regWork4D, \regWork5D

        FillOneUniqueCandidates regRowPartD, regMergedBoxD, \regMergedRowD, regColumnD, 2, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D, \regWork9D, \regWork1X, \regWork2X
        FillOneUniqueCandidates regRowPartD, regMergedBoxD, \regMergedRowD, regColumnD, 1, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D, \regWork9D, \regWork1X, \regWork2X
        FillOneUniqueCandidates regRowPartD, regMergedBoxD, \regMergedRowD, regColumnD, 0, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D, \regWork9D, \regWork1X, \regWork2X

        # Do not findThreePartsCandidates here. It is slow to solve.
        SaveLineSet regRowPartD, \regBoxX, \regRowX, \outBoxShift, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork1X, \regWork2X
1061:
.endm

.macro TestFillThreePartsUniqueCandidates outBoxShift
        InitMaskRegister
        FillThreePartsUniqueCandidates eax, xRegRow1to3, xRegRow1, xRegRowAll, \outBoxShift, edx, esi, r8d, r9d, r10d, r11d, r12d, r13d, r14d, xRegWork1, xRegWork2
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

.macro FillRowPartCandidates regMergedRowD, regBoxX, regRowX, regColumnX, outBoxShiftTarget, regWork1D, regWork2D, regWork3D, regWork4D, regWork5D, regWork6D, regWork7D, regWork8D, regWork9D, regWork1X, regWork2X
        FillThreePartsUniqueCandidates \regMergedRowD, \regBoxX, \regRowX, \regColumnX, \outBoxShiftTarget, \regWork1D, \regWork2D, \regWork3D, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D, \regWork9D, \regWork1X, \regWork2X
.endm

.macro TestFillRowPartCandidates outBoxShiftTarget
        InitMaskRegister
        mov     rax, [rip + testFillNineUniqueCandidatesPreRow]
        FillRowPartCandidates eax, xRegRow1to3, xRegRow1, xRegRowAll, \outBoxShiftTarget, ebx, ecx, edx, esi, r8d, r9d, r10d, r11d, r12d, xRegWork1, xRegWork2
        MacroMovdqa ([rip + testFillNineUniqueCandidatesRowX]),    xmm1
        MacroMovdqa ([rip + testFillNineUniqueCandidatesBoxX]),    xmm10
        MacroMovdqa ([rip + testFillNineUniqueCandidatesColumnX]), xmm0
        ret
.endm

        .global testTestFillRowPartCandidates0
        .global testTestFillRowPartCandidates1
        .global testTestFillRowPartCandidates2
testTestFillRowPartCandidates0:
        TestFillRowPartCandidates 0
testTestFillRowPartCandidates1:
        TestFillRowPartCandidates 1
testTestFillRowPartCandidates2:
        TestFillRowPartCandidates 2

# Collect unique candidates for nine cells in a row, column and box
.macro FillNineUniqueCandidates regBoxX, regRowX, regColumnX, regWork1D, regWork2D, regWork3D, regWork4D, regWork5D, regWork6D, regWork7D, regWork8D, regWork9D, regWork10D, regWork11D, regWork1X, regWork2X
        .set  regMergedRowD, \regWork1D
        CollectUniqueCandidatesInLine  regMergedRowD, \regRowX, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D, \regWork9D, \regWork10D, \regWork11D
        popcnt \regWork2D, regMergedRowD
        cmp    \regWork2D, 9
        jz     1041f

        FillRowPartCandidates regMergedRowD, \regBoxX, \regRowX, \regColumnX, 2, \regWork2D, \regWork3D, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D, \regWork9D, \regWork10D, \regWork1X, \regWork2X

        # Do not CollectUniqueCandidatesInLine here. It is slow to solve.
        FillRowPartCandidates regMergedRowD, \regBoxX, \regRowX, \regColumnX, 1, \regWork2D, \regWork3D, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D, \regWork9D, \regWork10D, \regWork1X, \regWork2X
        FillRowPartCandidates regMergedRowD, \regBoxX, \regRowX, \regColumnX, 0, \regWork2D, \regWork3D, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D, \regWork9D, \regWork10D, \regWork1X, \regWork2X
1041:
.endm

        .global testFillNineUniqueCandidates
testFillNineUniqueCandidates:
        InitMaskRegister
        FillNineUniqueCandidates xRegRow1to3, xRegRow1, xRegRowAll, eax, ebx, ecx, edx, esi, r8d, r9d, r10d, r11d, r12d, r13d, xRegWork1, xRegWork2
        MacroMovdqa ([rip + testFillNineUniqueCandidatesRowX]),    xmm1
        MacroMovdqa ([rip + testFillNineUniqueCandidatesBoxX]),    xmm10
        MacroMovdqa ([rip + testFillNineUniqueCandidatesColumnX]), xmm0
        ret

.macro Collect27UniqueCandidates regBoxX, regRow1X, regRow2X, regRow3X, regColumnX, regWork1D, regWork2D, regWork3D, regWork4D, regWork5D, regWork6D, regWork7D, regWork8D, regWork9D, regWork10D, regWork11D, regWork1X, regWork2X
        FillNineUniqueCandidates \regBoxX, \regRow1X, \regColumnX, \regWork1D, \regWork2D, \regWork3D, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D, \regWork9D, \regWork10D, \regWork11D, \regWork1X, \regWork2X
        FillNineUniqueCandidates \regBoxX, \regRow2X, \regColumnX, \regWork1D, \regWork2D, \regWork3D, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D, \regWork9D, \regWork10D, \regWork11D, \regWork1X, \regWork2X
        FillNineUniqueCandidates \regBoxX, \regRow3X, \regColumnX, \regWork1D, \regWork2D, \regWork3D, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D, \regWork9D, \regWork10D, \regWork11D, \regWork1X, \regWork2X
.endm

# Collect unique candidates for each cell
.macro CollectUniqueCandidates regWork1D, regWork2D, regWork3D, regWork4D, regWork5D, regWork6D, regWork7D, regWork8D, regWork9D, regWork10D, regWork11D, regWork1X, regWork2X
        Collect27UniqueCandidates xRegRow1to3, xRegRow1, xRegRow2, xRegRow3, xRegRowAll, \regWork1D, \regWork2D, \regWork3D, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D, \regWork9D, \regWork10D, \regWork11D, \regWork1X, \regWork2X
        Collect27UniqueCandidates xRegRow4to6, xRegRow4, xRegRow5, xRegRow6, xRegRowAll, \regWork1D, \regWork2D, \regWork3D, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D, \regWork9D, \regWork10D, \regWork11D, \regWork1X, \regWork2X
        Collect27UniqueCandidates xRegRow7to9, xRegRow7, xRegRow8, xRegRow9, xRegRowAll, \regWork1D, \regWork2D, \regWork3D, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D, \regWork9D, \regWork10D, \regWork11D, \regWork1X, \regWork2X
.endm

.macro FindRowPartCandidates regRowPartTargetD, regRowPartLeftD, regRowPartCandidatesTargetD, regRowPartCandidatesLeftD, regRowPartCandidatesOtherD, regRowCandidatesD, regBoxD, regBoxX, regColumnX, outBoxShift, regWork1D, regWork2D, regWork3D, regWork4D, regWork5D, regWork1X, regWork2X
        SelectRowParts        \regBoxD, \regBoxX, \outBoxShift
        MergeThreeElements32  \regBoxD, \regWork1D, \regWork2D

        mov  \regRowPartCandidatesLeftD, \regRowPartLeftD
        MergeThreeElements32  \regRowPartCandidatesLeftD, \regWork3D, \regWork4D

        mov  \regRowCandidatesD, \regRowPartCandidatesLeftD
        or   \regRowCandidatesD, \regRowPartCandidatesOtherD
        SelectRowParts  \regRowPartCandidatesTargetD, \regColumnX, \outBoxShift
        FindThreePartsCandidates \regRowPartTargetD, \regRowCandidatesD, \regBoxD, \regRowPartCandidatesTargetD, \regWork1D, \regWork2D, \regWork3D, \regWork4D, \regWork5D
.endm

.macro TestFindRowPartCandidates outBoxShift
        InitMaskRegister
        mov     rsi, [rip + testFindRowPartCandidatesPreRowPartTarget]
        mov     r8,  [rip + testFindRowPartCandidatesPreRowCandidates]
        mov     r9,  [rip + testFindRowPartCandidatesPreBox]
        FindRowPartCandidates esi, eax, ebx, ecx, edx, r8d, r9d, xRegRow1to3, xRegRowAll, \outBoxShift, r10d, r11d, r12d, r13d, r14d, xRegWork1, xRegWork2
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

.macro FindNineCandidates regTargetRowX, regRow1X, regRow2X, regBoxX, regThreeRow1X, regThreeRow2X, regColumnX, regWork1, regWork2, regWork3, regWork4, regWork1D, regWork2D, regWork3D, regWork4D, regWork5D, regWork6D, regWork7D, regWork8D, regWork9D, regWork10D, regWork11D, regWork12D, regWork13D, regWork1X, regWork2X
        .set  regRowPart2,  \regWork1
        .set  regRowPart2D, \regWork1D
        .set  regRowPart1,  \regWork2
        .set  regRowPart1D, \regWork2D
        .set  regRowPart0,  \regWork3
        .set  regRowPart0D, \regWork3D
        .set  regBox,       \regWork4
        .set  regBoxD,      \regWork4D
        .set  regRowPartCandidates2D, \regWork5D
        .set  regRowPartCandidates1D, \regWork6D
        .set  regRowPartCandidates0D, \regWork7D
        .set  regRowCandidatesD,      \regWork8D

        MacroOrps3op \regBoxX, \regRow1X, \regRow2X
        OrThreeXmmRegs \regColumnX, \regBoxX, \regThreeRow1X, \regThreeRow2X

        SelectRowAndCount regRowPart2, regRowPart1, \regTargetRowX, regRowPart0
        jz   1001f

        # Clear upper 32 bits
        mov  regRowPart0D, regRowPart1D
        mov  regRowPartCandidates0D, regRowPart1D
        ShrNonZero  regRowPart1, (boxRowByteSize * bitPerByte)
        mov  regRowPartCandidates1D, regRowPart1D

        MergeThreeElements32  regRowPartCandidates1D, \regWork9D, \regWork10D
        MergeThreeElements32  regRowPartCandidates0D, \regWork9D, \regWork10D
        mov  regRowCandidatesD, regRowPartCandidates1D
        or   regRowCandidatesD, regRowPartCandidates0D

        SelectRowParts  regBoxD, \regBoxX, 2
        MergeThreeElements32  regBoxD, \regWork9D, \regWork10D

        SelectRowParts  regRowPartCandidates2D, \regColumnX, 2
        FindThreePartsCandidates regRowPart2D, regRowCandidatesD, regBoxD, regRowPartCandidates2D, \regWork9D, \regWork10D, \regWork11D, \regWork12D, \regWork13D, \regWork1X, \regWork2X
        FindRowPartCandidates  regRowPart1D, regRowPart2D, regRowPartCandidates1D, regRowPartCandidates2D, regRowPartCandidates0D, regRowCandidatesD, regBoxD, \regBoxX, \regColumnX, 1, \regWork9D, \regWork10D, \regWork11D, \regWork12D, \regWork13D, \regWork1X, \regWork2X
        FindRowPartCandidates  regRowPart0D, regRowPart1D, regRowPartCandidates0D, regRowPartCandidates1D, regRowPartCandidates2D, regRowCandidatesD, regBoxD, \regBoxX, \regColumnX, 0, \regWork9D, \regWork10D, \regWork11D, \regWork12D, \regWork13D, \regWork1X, \regWork2X

        MacroPinsrq \regTargetRowX, regRowPart2,  1
        MacroPinsrd \regTargetRowX, regRowPart0D, 0
        MacroPinsrd \regTargetRowX, regRowPart1D, 1
1001:
.endm

        .global testFindNineCandidates
testFindNineCandidates:
        InitMaskRegister
        MacroXorps xRegRow4to6, xRegRow4to6
        MacroXorps xRegRow7to9, xRegRow7to9
        FindNineCandidates xRegRow1, xRegRow2, xRegRow3, xRegRow1to3, xRegRow4to6, xRegRow7to9, xRegRowAll, rax, rbx, rcx, rdx, eax, ebx, ecx, edx, esi, edi, r8d, r9d, r10d, r11d, r12d, r13d, r14d, xRegWork1, xRegWork2
        MacroMovdqa ([rip + testFillNineUniqueCandidatesRowX]),    xmm1
        MacroMovdqa ([rip + testFillNineUniqueCandidatesBoxX]),    xmm10
        MacroMovdqa ([rip + testFillNineUniqueCandidatesColumnX]), xmm0
        ret

.macro Find27UniqueCandidates regRow1X, regRow2X, regRow3X, regBoxX, regThreeRow1X, regThreeRow2X, regColumnX, regWork1, regWork2, regWork3, regWork4, regWork1D, regWork2D, regWork3D, regWork4D, regWork5D, regWork6D, regWork7D, regWork8D, regWork9D, regWork10D, regWork11D, regWork12D, regWork13D, regWork1X, regWork2X
         FindNineCandidates \regRow1X, \regRow2X, \regRow3X, \regBoxX, \regThreeRow1X, \regThreeRow2X, \regColumnX, \regWork1, \regWork2, \regWork3, \regWork4, \regWork1D, \regWork2D, \regWork3D, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D, \regWork9D, \regWork10D, \regWork11D, \regWork12D, \regWork13D, \regWork1X, \regWork2X
         FindNineCandidates \regRow2X, \regRow1X, \regRow3X, \regBoxX, \regThreeRow1X, \regThreeRow2X, \regColumnX, \regWork1, \regWork2, \regWork3, \regWork4, \regWork1D, \regWork2D, \regWork3D, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D, \regWork9D, \regWork10D, \regWork11D, \regWork12D, \regWork13D, \regWork1X, \regWork2X
         FindNineCandidates \regRow3X, \regRow1X, \regRow2X, \regBoxX, \regThreeRow1X, \regThreeRow2X, \regColumnX, \regWork1, \regWork2, \regWork3, \regWork4, \regWork1D, \regWork2D, \regWork3D, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D, \regWork9D, \regWork10D, \regWork11D, \regWork12D, \regWork13D, \regWork1X, \regWork2X
.endm

.macro FindCandidates regWork1, regWork2, regWork3, regWork4, regWork1D, regWork2D, regWork3D, regWork4D, regWork5D, regWork6D, regWork7D, regWork8D, regWork9D, regWork10D, regWork11D, regWork12D, regWork13D, regWork1X, regWork2X
        OrThreeXmmRegs xRegRow4to6, xRegRow4, xRegRow5, xRegRow6
        OrThreeXmmRegs xRegRow7to9, xRegRow7, xRegRow8, xRegRow9
        Find27UniqueCandidates xRegRow1, xRegRow2, xRegRow3, xRegRow1to3, xRegRow4to6, xRegRow7to9, xRegRowAll, \regWork1, \regWork2, \regWork3, \regWork4, \regWork1D \regWork2D, \regWork3D, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D, \regWork9D, \regWork10D, \regWork11D, \regWork12D, \regWork13D, \regWork1X, \regWork2X

        OrThreeXmmRegs xRegRow1to3, xRegRow1, xRegRow2, xRegRow3
        Find27UniqueCandidates xRegRow4, xRegRow5, xRegRow6, xRegRow4to6, xRegRow1to3, xRegRow7to9, xRegRowAll, \regWork1, \regWork2, \regWork3, \regWork4, \regWork1D \regWork2D, \regWork3D, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D, \regWork9D, \regWork10D, \regWork11D, \regWork12D, \regWork13D, \regWork1X, \regWork2X

        OrThreeXmmRegs xRegRow4to6, xRegRow4, xRegRow5, xRegRow6
        Find27UniqueCandidates xRegRow7, xRegRow8, xRegRow9, xRegRow7to9, xRegRow1to3, xRegRow4to6, xRegRowAll, \regWork1, \regWork2, \regWork3, \regWork4, \regWork1D \regWork2D, \regWork3D, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D, \regWork9D, \regWork10D, \regWork11D, \regWork12D, \regWork13D, \regWork1X, \regWork2X
.endm

# Count how many cells are filled after moving previous count to regPrevPopcnt
.macro CountFilledElements reg64LoopCnt, reg64CurrentPopcnt, reg64PrevPopcnt, regWork
        MacroPextrq  \reg64PrevPopcnt, xLoopPopCnt, 0
        MacroPextrq  \reg64LoopCnt,    xLoopPopCnt, 1
        MacroPextrq  \reg64CurrentPopcnt, xRegRowAll, 0
        MacroPextrq  \regWork,            xRegRowAll, 1
        popcnt  \reg64CurrentPopcnt, \reg64CurrentPopcnt
        popcnt  \regWork, \regWork
        add     \reg64CurrentPopcnt, \regWork
.endm

        .global testCountFilledElements
testCountFilledElements:
        CountFilledElements rax, rbx, rcx, r14
        ret

.macro SaveLoopCnt reg64LoopCnt, regCurrentPopcnt
        MacroPinsrq xLoopPopCnt, \regCurrentPopcnt, 0
        MacroPinsrq xLoopPopCnt, \reg64LoopCnt, 1
.endm

        .global testSaveLoopCnt
testSaveLoopCnt:
        SaveLoopCnt rax, rbx
        ret

# ----------------------------------------------------------------------------------------------------
# Labels in backtracking must have 3-digits number

# Count a number of candidates in a cell and set it to regPopCount
# regPopCount = an out-of-range large number if the cell holds 0 or a power-of-2 number
#               otherwise population (number of 1-bits) of the cell
# regAccumPopCount += population of the cell
.macro PopCountOrPowerOf2 regPopCountD, regAccumPopCountD, regSrcD, inBoxShift, regWork1D
        SelectElementInRowParts  \regWork1D, \regSrcD, \inBoxShift
        popcnt  \regPopCountD, \regWork1D
        mov     \regWork1D, candidatesTooMany
        add     \regAccumPopCountD, \regPopCountD
        cmp     \regPopCountD, 1
        CmovLessEqual  \regPopCountD, \regWork1D
.endm

.macro TestPopCountOrPowerOf2 inBoxShift
        InitMaskRegister
        mov  rbx, rdx
        PopCountOrPowerOf2 eax, ebx, ecx, \inBoxShift, r8d
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

# Compare regLeft > regRight in a dictionary order which contain High:Low bitset.
# [MSB..bitWidth] of regLow and upper bitWidth-bits of bitHigh must be 0.
.macro CompareRegisterSet regLeftHigh, regLeftLow, regRightHigh, regRightLow, bitWidth, regWork1, regWork2
        mov  \regWork1, \regLeftHigh
        ShlNonZero  \regWork1, \bitWidth
        or   \regWork1, \regLeftLow
        mov  \regWork2, \regRightHigh
        ShlNonZero  \regWork2, \bitWidth
        or   \regWork2, \regRightLow
        cmp  \regWork1, \regWork2
.endm

.macro testCompareRegisterSet regLeftHigh, regLeftLow, regRightHigh, regRightLow, regWork1, regWork2
        InitMaskRegister
        xor  rax, rax
        xor  rbx, rbx
        mov  r10, 1
        CompareRegisterSet \regLeftHigh, \regLeftLow, \regRightHigh, \regRightLow, 16, \regWork1, \regWork2
        cmovz  rax, r10
        cmovc  rbx, r10
        ret
.endm

        .global testCompareRegisterSet64
        .global testCompareRegisterSet32
testCompareRegisterSet64:
        testCompareRegisterSet rcx, rdx, rsi, rdi, r8, r9
testCompareRegisterSet32:
        testCompareRegisterSet ecx, edx, esi, edi, r8d, r9d

# Set the position of a cell if the cell has candidates less than or equal to regMinCount
.macro CountCellCandidates regOutBoxShiftD, regInBoxShiftD, regMinCountD, regRowPopCountD, regSrcD, outBoxShift, inBoxShift, regWork1D, regWork2D, regWork3D
        PopCountOrPowerOf2  \regWork1D, \regRowPopCountD, \regSrcD, \inBoxShift, \regWork2D
        mov  \regWork2D, \outBoxShift
        mov  \regWork3D, \inBoxShift
        cmp  \regWork1D, \regMinCountD
        CmovLessEqual  \regOutBoxShiftD, \regWork2D
        CmovLessEqual  \regInBoxShiftD, \regWork3D
        CmovLessEqual  \regMinCountD, \regWork1D
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
        CountCellCandidates eax, ebx, ecx, edx, edi, \outBoxShift, \inBoxShift, r8d, r9d, r10d
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

.macro CountThreeCellCandidates regOutBoxShiftD, regInBoxShiftD, regMinCountD, regRowPopCountD, regSrcD, outBoxShift, regWork1D, regWork2D, regWork3D
        CountCellCandidates \regOutBoxShiftD, \regInBoxShiftD, \regMinCountD, \regRowPopCountD, \regSrcD, \outBoxShift, 0, \regWork1D, \regWork2D, \regWork3D
        CountCellCandidates \regOutBoxShiftD, \regInBoxShiftD, \regMinCountD, \regRowPopCountD, \regSrcD, \outBoxShift, 1, \regWork1D, \regWork2D, \regWork3D
        CountCellCandidates \regOutBoxShiftD, \regInBoxShiftD, \regMinCountD, \regRowPopCountD, \regSrcD, \outBoxShift, 2, \regWork1D, \regWork2D, \regWork3D
.endm

        .global testCountThreeCellCandidates
testCountThreeCellCandidates:
        InitMaskRegister
        mov  rax, testCountCellCandidatesInvalidShift
        mov  rbx, testCountCellCandidatesInvalidShift
        mov  rcx, [rsi]
        mov  rdx, [rsi + 8]
        CountThreeCellCandidates eax, ebx, ecx, edx, edi, 2, r8d, r9d, r10d
        ret

.macro CountRowCellCandidatesSub regOutBoxShiftD, regInBoxShiftD, regMinCountD, regRowPopCountD, xRegRow, regWork1D, regWork2D, regWork3D, regWork4D, regWork5D
        xor  \regRowPopCountD, \regRowPopCountD
        MacroPextrd  \regWork2D, \xRegRow, 0
        CountThreeCellCandidates  \regOutBoxShiftD, \regInBoxShiftD, \regMinCountD, \regRowPopCountD, \regWork2D, 0, \regWork3D, \regWork4D, \regWork5D
        MacroPextrd  \regWork1D, \xRegRow, 1
        CountThreeCellCandidates  \regOutBoxShiftD, \regInBoxShiftD, \regMinCountD, \regRowPopCountD, \regWork1D, 1, \regWork3D, \regWork4D, \regWork5D
        MacroPextrd  \regWork1D, \xRegRow, 2
        CountThreeCellCandidates  \regOutBoxShiftD, \regInBoxShiftD, \regMinCountD, \regRowPopCountD, \regWork1D, 2, \regWork3D, \regWork4D, \regWork5D
.endm

        .global testCountRowCellCandidatesSub
testCountRowCellCandidatesSub:
        InitMaskRegister
        mov  rax, testCountCellCandidatesInvalidShift
        mov  rbx, testCountCellCandidatesInvalidShift
        mov  rcx, [rip + testCountRowCellCandidatesMinCount]
        MacroMovdqa xRegRow1, [rip + testCountRowCellCandidatesRowX]
        CountRowCellCandidatesSub eax, ebx, ecx, edx, xRegRow1, r8d, r9d, r10d, r11d, r12d
        ret

.macro CountRowCellCandidates xRegRowNumber, regOutBoxShiftD, regInBoxShiftD, regMinCountD, regRowPopCountD, xRegRow, rowNumber, regWork1D, regWork2D, regWork3D, regWork4D, regWork5D, regWork6D, regWork7D, regWork8D, regWork9D
        mov  \regWork1D, outBoxIndexInvalid
        mov  \regWork2D, inBoxIndexInvalid
        mov  \regWork3D, \regMinCountD
        CountRowCellCandidatesSub  \regWork1D, \regWork2D, \regWork3D, \regWork4D, \xRegRow, \regWork5D, \regWork6D, \regWork7D, \regWork8D, \regWork9D

        xor  \regWork5D, \regWork5D
        mov  \regWork6D, 1
        cmp  \regWork1D, outBoxIndexInvalid
        cmovz  \regWork5D, \regWork6D
        CompareRegisterSet  \regMinCountD, \regRowPopCountD, \regWork3D, \regWork4D, 16, \regWork7D, \regWork8D
        CmovLessEqual \regWork5D, \regWork6D

        or  \regWork5D, \regWork5D
        jnz 301f

        mov  \regOutBoxShiftD, \regWork1D
        mov  \regInBoxShiftD, \regWork2D
        mov  \regWork5D, \rowNumber
        MacroPinsrd  \xRegRowNumber, \regWork5D, 0
        mov  \regMinCountD, \regWork3D
        mov  \regRowPopCountD, \regWork4D
        cmp  \regWork4D, (candidatesNum + 2)
        jz  302f
301:
.endm

        .global testCountRowCellCandidates
testCountRowCellCandidates:
        InitMaskRegister
        mov  rax, testCountCellCandidatesInvalidRowNumber
        MacroPinsrd xRegWork1, eax, 0
        mov  rax, testCountCellCandidatesInvalidShift
        mov  rbx, testCountCellCandidatesInvalidShift
        mov  rcx, [rip + testCountRowCellCandidatesMinCount]
        mov  rdx, [rip + testCountRowCellCandidatesRowPopCount]
        MacroMovdqa xRegRow6, [rip + testCountRowCellCandidatesRowX]
        CountRowCellCandidates xRegWork1, eax, ebx, ecx, edx, xRegRow6, testCountCellCandidatesRowNumber, esi, edi, r8d, r9d, r10d, r11d, r12d, r13d, r14d
        MacroPextrd esi, xRegWork1, 0
        ret

.macro CountRowSetCell xRegRowNumber, regOutBoxShiftD, regInBoxShiftD, regWork1D, regWork2D, regWork3D, regWork4D, regWork5D, regWork6D, regWork7D, regWork8D, regWork9D, regWork10D, regWork11D
        mov  \regOutBoxShiftD, outBoxIndexInvalid
        mov  \regInBoxShiftD, inBoxIndexInvalid
        mov  \regWork1D, rowNumberInvalid
        MacroPinsrd  \xRegRowNumber, \regWork1D, 0
        mov  \regWork1D, candidatesTooMany
        xor  \regWork2D, \regWork2D

        CountRowCellCandidates  \xRegRowNumber, \regOutBoxShiftD, \regInBoxShiftD, \regWork1D, \regWork2D, xRegRow1, 0, \regWork3D, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D, \regWork9D, \regWork10D, \regWork11D
        CountRowCellCandidates  \xRegRowNumber, \regOutBoxShiftD, \regInBoxShiftD, \regWork1D, \regWork2D, xRegRow2, 1, \regWork3D, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D, \regWork9D, \regWork10D, \regWork11D
        CountRowCellCandidates  \xRegRowNumber, \regOutBoxShiftD, \regInBoxShiftD, \regWork1D, \regWork2D, xRegRow3, 2, \regWork3D, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D, \regWork9D, \regWork10D, \regWork11D
        CountRowCellCandidates  \xRegRowNumber, \regOutBoxShiftD, \regInBoxShiftD, \regWork1D, \regWork2D, xRegRow4, 3, \regWork3D, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D, \regWork9D, \regWork10D, \regWork11D
        CountRowCellCandidates  \xRegRowNumber, \regOutBoxShiftD, \regInBoxShiftD, \regWork1D, \regWork2D, xRegRow5, 4, \regWork3D, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D, \regWork9D, \regWork10D, \regWork11D
        CountRowCellCandidates  \xRegRowNumber, \regOutBoxShiftD, \regInBoxShiftD, \regWork1D, \regWork2D, xRegRow6, 5, \regWork3D, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D, \regWork9D, \regWork10D, \regWork11D
        CountRowCellCandidates  \xRegRowNumber, \regOutBoxShiftD, \regInBoxShiftD, \regWork1D, \regWork2D, xRegRow7, 6, \regWork3D, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D, \regWork9D, \regWork10D, \regWork11D
        CountRowCellCandidates  \xRegRowNumber, \regOutBoxShiftD, \regInBoxShiftD, \regWork1D, \regWork2D, xRegRow8, 7, \regWork3D, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D, \regWork9D, \regWork10D, \regWork11D
        CountRowCellCandidates  \xRegRowNumber, \regOutBoxShiftD, \regInBoxShiftD, \regWork1D, \regWork2D, xRegRow9, 8, \regWork3D, \regWork4D, \regWork5D, \regWork6D, \regWork7D, \regWork8D, \regWork9D, \regWork10D, \regWork11D
302:
.endm

.macro SearchNextCandidate
        CountRowSetCell xRegWork1, eax, ebx, ecx, edx, esi, edi, r8d, r9d, r10d, r11d, r12d, r13d, r14d
        MacroPextrd  ecx, xRegWork1, 0
        mov  r8, nextCellFound
        xor  rdx, rdx
        cmp  rcx, rowNumberInvalid
        cmovnz  rdx, r8
        mov  qword ptr [rip + sudokuXmmNextCellFound], rdx
        mov  qword ptr [rip + sudokuXmmNextOutBoxShift], rax
        mov  qword ptr [rip + sudokuXmmNextInBoxShift], rbx
        mov  qword ptr [rip + sudokuXmmNextRowNumber], rcx
.endm

        .global testSearchNextCandidate
testSearchNextCandidate:
        InitMaskRegister
        SearchNextCandidate
        ret

        .global testFoldRowParts
        .global testCheckRow
        .global testCheckRowSet
        .global testCheckColumn
        .global testCheckBox
        .global testCheckSetBox
        .global testCheckConsistency

.macro FoldRowParts xRegRow, regWork1D, regWork2D, regWork3D, regWork4D, regWork5D
        MacroPextrd  \regWork1D, \xRegRow, 2
        MergeThreeElements32  \regWork1D, \regWork4D, \regWork5D
        MacroPextrd  \regWork2D, \xRegRow, 1
        MergeThreeElements32  \regWork2D, \regWork4D, \regWork5D
        MacroPextrd  \regWork3D, \xRegRow, 0
        MergeThreeElements32  \regWork3D, \regWork4D, \regWork5D
.endm

testFoldRowParts:
        InitMaskRegister
        xor  rax, rax
        xor  rbx, rbx
        xor  rcx, rcx
        FoldRowParts  xRegRow1, eax, ebx, ecx, r8d, r9d
        ret

# Leave regResult unchanged if all the row contains all 9 candidates.
# Otherwise set regInvalid to regResult.
.macro CheckRow xRegRow, regResultD, regInvalidD, regWork1D, regWork2D, regWork3D, regWork4D, regWork5D
        FoldRowParts \xRegRow, \regWork1D, \regWork2D, \regWork3D, \regWork4D, \regWork5D
        or      \regWork1D, \regWork2D
        or      \regWork1D, \regWork3D
        popcnt  \regWork1D, \regWork1D
        cmp     \regWork1D, candidatesNum
        cmovnz  \regResultD, \regInvalidD
.endm

testCheckRow:
        InitMaskRegister
        xor  rax, rax
        mov  r8, 1
        CheckRow  xRegRow1, eax, r8d, r9d, r10d, r11d, r12d, r13d
        ret

.macro CheckRowSet regResultD, regInvalidD, regWork1D, regWork2D, regWork3D, regWork4D, regWork5D
        CheckRow xRegRow1, \regResultD, \regInvalidD, \regWork1D, \regWork2D, \regWork3D, \regWork4D, \regWork5D
        CheckRow xRegRow2, \regResultD, \regInvalidD, \regWork1D, \regWork2D, \regWork3D, \regWork4D, \regWork5D
        CheckRow xRegRow3, \regResultD, \regInvalidD, \regWork1D, \regWork2D, \regWork3D, \regWork4D, \regWork5D
        CheckRow xRegRow4, \regResultD, \regInvalidD, \regWork1D, \regWork2D, \regWork3D, \regWork4D, \regWork5D
        CheckRow xRegRow5, \regResultD, \regInvalidD, \regWork1D, \regWork2D, \regWork3D, \regWork4D, \regWork5D
        CheckRow xRegRow6, \regResultD, \regInvalidD, \regWork1D, \regWork2D, \regWork3D, \regWork4D, \regWork5D
        CheckRow xRegRow7, \regResultD, \regInvalidD, \regWork1D, \regWork2D, \regWork3D, \regWork4D, \regWork5D
        CheckRow xRegRow8, \regResultD, \regInvalidD, \regWork1D, \regWork2D, \regWork3D, \regWork4D, \regWork5D
        CheckRow xRegRow9, \regResultD, \regInvalidD, \regWork1D, \regWork2D, \regWork3D, \regWork4D, \regWork5D
.endm

testCheckRowSet:
        InitMaskRegister
        xor  rax, rax
        mov  r8, 1
        CheckRowSet  eax, r8d, r9d, r10d, r11d, r12d, r13d
        ret

.macro CheckColumn regResultD, regInvalidD, regWork1D, regWork2D, regWork3D
        MacroPextrd \regWork1D, xRegRowAll, 0
        MacroPextrd \regWork2D, xRegRowAll, 1
        MacroPextrd \regWork3D, xRegRowAll, 2
        popcnt  \regWork1D, \regWork1D
        popcnt  \regWork2D, \regWork2D
        popcnt  \regWork3D, \regWork3D

        cmp     \regWork1D, (candidatesNum * rowPartNum)
        cmovnz  \regResultD, \regInvalidD
        cmp     \regWork2D, (candidatesNum * rowPartNum)
        cmovnz  \regResultD, \regInvalidD
        cmp     \regWork3D, (candidatesNum * rowPartNum)
        cmovnz  \regResultD, \regInvalidD
.endm

testCheckColumn:
        InitMaskRegister
        xor  rax, rax
        mov  r8, 1
        CheckColumn  eax, r8d, ebx, ecx, edx
        ret

.macro CheckBox xRegRow, regResultD, regInvalidD, regWork1D, regWork2D, regWork3D, regWork4D, regWork5D
        FoldRowParts \xRegRow, \regWork1D, \regWork2D, \regWork3D, \regWork4D, \regWork5D

        popcnt  \regWork1D, \regWork1D
        cmp     \regWork1D, candidatesNum
        cmovnz  \regResultD, \regInvalidD

        popcnt  \regWork2D, \regWork2D
        cmp     \regWork2D, candidatesNum
        cmovnz  \regResultD, \regInvalidD

        popcnt  \regWork3D, \regWork3D
        cmp     \regWork3D, candidatesNum
        cmovnz  \regResultD, \regInvalidD
.endm

testCheckBox:
        InitMaskRegister
        xor  rax, rax
        mov  r8, 1
        CheckBox  xRegRow1to3, eax, r8d, r9d, r10d, r11d, r12d, r13d
        ret

.macro CheckSetBox regResultD, regInvalidD, regWork1D, regWork2D, regWork3D, regWork4D, regWork5D
        CheckBox xRegRow1to3, \regResultD, \regInvalidD, \regWork1D, \regWork2D, \regWork3D, \regWork4D, \regWork5D
        CheckBox xRegRow4to6, \regResultD, \regInvalidD, \regWork1D, \regWork2D, \regWork3D, \regWork4D, \regWork5D
        CheckBox xRegRow7to9, \regResultD, \regInvalidD, \regWork1D, \regWork2D, \regWork3D, \regWork4D, \regWork5D
.endm

testCheckSetBox:
        InitMaskRegister
        xor  rax, rax
        mov  r8, 1
        CheckSetBox  eax, r8d, r9d, r10d, r11d, r12d, r13d
        ret

.macro CheckConsistency regResultD, regInvalidD, regWork1D, regWork2D, regWork3D, regWork4D, regWork5D, reg32Work1, reg32Work2, reg32Work3
        xor  \regResultD, \regResultD
        mov  \regInvalidD, 1
        CheckRowSet  \regResultD, \regInvalidD, \regWork1D, \regWork2D, \regWork3D, \regWork4D, \regWork5D
        cmp  \regResultD, \regInvalidD
        jz   20001f

        OrThreeXmmRegs  xRegRow1to3, xRegRow1, xRegRow2, xRegRow3
        OrThreeXmmRegs  xRegRow4to6, xRegRow4, xRegRow5, xRegRow6
        OrThreeXmmRegs  xRegRow7to9, xRegRow7, xRegRow8, xRegRow9
        OrThreeXmmRegs  xRegRowAll, xRegRow1to3, xRegRow4to6, xRegRow7to9
        CheckColumn  \regResultD, \regInvalidD, \reg32Work1, \reg32Work2, \reg32Work3
        CheckSetBox  \regResultD, \regInvalidD, \regWork1D, \regWork2D, \regWork3D, \regWork4D, \regWork5D
20001:
.endm

testCheckConsistency:
        InitMaskRegister
        xor  rax, rax
        mov  r8, 1
        CheckConsistency  eax, r8d, r9d, r10d, ebx, ecx, edx, ebx, ecx, edx
        ret

# Assign code (text) in a consecutive memory area which its tests do not divide.
# The symbol that C++ code calls via inline assembly.
        .global solveSudokuAsm
solveSudokuAsm:
        mov    gRegBitMask, elementBitMask
        xorps  xLoopPopCnt, xLoopPopCnt
        mov    qword ptr [rip + sudokuXmmNextCellFound], 0

        # Data structure holding sudoku cells
        # XMM registers = 1st .. 9th rows
        # An XMM register = | 32bit: all 0 | 32bit: 1st row, 1st .. 3rd columns | 32bit: 4..6 | 32bit: 7..9 |
        # Set of 9 bits in a XMM register = | c_9, c_8, ... , c_1 |
        # c_n: 1 if n is a candidate of the cell c, 0 otherwise

loopFilling:
        # Find cadidates per 3 rows
        CollectUniqueCandidatesInThreeLine xRegRow1to3, xRegRow1, xRegRow2, xRegRow3, rax, rbx, rcx, rdx, r8, r9, r10, eax, xRegWork1, xRegWork2
        CollectUniqueCandidatesInThreeLine xRegRow4to6, xRegRow4, xRegRow5, xRegRow6, rax, rbx, rcx, rdx, r8, r9, r10, eax, xRegWork1, xRegWork2
        CollectUniqueCandidatesInThreeLine xRegRow7to9, xRegRow7, xRegRow8, xRegRow9, rax, rbx, rcx, rdx, r8, r9, r10, eax, xRegWork1, xRegWork2
        # Hold 3 * 3 rows
        OrThreeXmmRegs xRegRowAll, xRegRow1to3, xRegRow4to6, xRegRow7to9

        # Count how many cells are filled
        .set    regCurrentPopcnt,  r8
        .set    regCurrentPopcntD, r8d
        .set    regPrevPopcnt,     r9
        .set    regPrevPopcntD,    r9d
        .set    regMaxPopcnt,      r10
        .set    regMaxPopcntD,     r10d
        .set    regLoopCnt,        r11
        .set    regLoopCntD,       r11d
        CountFilledElements regLoopCnt, regCurrentPopcnt, regPrevPopcnt, rcx
        mov     regMaxPopcntD, maxElementNumber

        # ZF=0 if all cells are filled or no cell is filled in this iteration
        # ZF=1 otherwise
        cmp     regCurrentPopcntD, regPrevPopcntD
        cmovz   regPrevPopcntD, regMaxPopcntD
        cmp     regPrevPopcntD, regMaxPopcntD
        jnz     keepFilling

exitFilling:
        .set    regResult,  r9
        .set    regResultD, r9d
        mov     qword ptr [rip + sudokuXmmElementCnt], regCurrentPopcnt
        CheckConsistency regResultD, r10d, r11d, r12d, ebx, ecx, edx, ebx, ecx, edx
        mov     qword ptr [rip + sudokuXmmAborted], regResult
        cmp     regCurrentPopcntD, regMaxPopcntD
        jz      exitFillingCells

        # Find a cell to guess its candidate if not all cells are filled
        SearchNextCandidate
exitFillingCells:
        ret

abortFilling:
        mov     qword ptr [rip + sudokuXmmAborted], 1
        mov     qword ptr [rip + sudokuXmmElementCnt], 0
        ret

keepFilling:
        # Abort this loop
        cmp     regLoopCntD, sudokuMaxLoopcnt
        JumpIfGreaterEqual exitFilling
        FastInc regLoopCntD
        SaveLoopCnt regLoopCnt, regCurrentPopcnt

        CollectUniqueCandidates r8d, r9d, r10d, r11d, r12d, r13d, r14d, eax, ebx, ecx, edx, xRegWork1, xRegWork2
        FindCandidates r8, r9, r10, r11, r8d, r9d, r10d, r11d, r12d, r13d, r14d, eax, ebx, ecx, edx, esi, edi, xRegWork1, xRegWork2
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

# 64bitレジスタではなく、32bitでレジスタ計算できるところはそうする、という変更はここまで

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
        JumpIfGreaterEqual 10012f

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
