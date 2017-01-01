// Sudoku solver with SSE 4.2 / AVX
// Copyright (C) 2012-2017 Zettsu Tatsuya

#include "sudoku.h"

int main(int argc, char *argv[]) {
    /* 使うCPUを固定すると、processorとcacheのaffinityが上がるが、
     * マルチスレッドもできなくなるので、ここでは設定しない */

    /* 本当はデータのポインタと関数のポインタは互換ではない */
    static_assert(sizeof(uintptr_t) == sizeof(&PrintPattern), "Unexpected uintptr_t size");
    static_assert(sizeof(sudokuXmmPrintFunc) == sizeof(uintptr_t), "Unexpected sudokuXmmPrintFunc size");
    sudokuXmmPrintFunc = reinterpret_cast<uintptr_t>(&PrintPattern);

    if (!SudokuLoader::CanLaunch(argc, argv)) {
        std::cout << "Cannot solve sudoku maps, counting only\n";
        return 1;
    }

    SudokuLoader sudoku(argc, argv, &std::cin, &std::cout);
    return sudoku.Exec();
}

/*
Local Variables:
mode: c++
coding: utf-8-unix
tab-width: nil
c-file-style: "stroustrup"
End:
*/
