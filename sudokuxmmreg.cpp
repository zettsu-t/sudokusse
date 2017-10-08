// Sudoku solver with SSE 4.2
// Copyright (C) 2012-2015 Zettsu Tatsuya

#include "sudoku.h"

namespace Sudoku {
    // Loads XMM registers from main memory
    void LoadXmmRegistersFromMem(const xmmRegister *pData) {
        asm volatile (
            "call loadXmmRegisters\n\t"
            ::"S"(pData):);
        return;
    }

    // Saves XMM registers to main memory
    void SaveXmmRegistersToMem(xmmRegister *pData) {
        asm volatile (
            "call saveXmmRegisters\n\t"
            ::"D"(pData):);
        return;
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
