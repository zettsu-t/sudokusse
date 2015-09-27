// Sudoku solver with SSE 4.2 / AVX
// Copyright (C) 2012-2015 Zettsu Tatsuya

#include "sudoku.h"

int main(int argc, char *argv[]) {
    /* 使うCPUを固定すると、processorとcacheのaffinityが上がる */
    DWORD_PTR procMask = 1;
    if (!SetProcessAffinityMask(GetCurrentProcess(), procMask)) {
        std::cout << "SetProcessAffinityMask failed\n";
    }

    /* 本当はデータのポインタと関数のポインタは互換ではない */
    static_assert(sizeof(uintptr_t) == sizeof(&PrintPattern), "Unexpected uintptr_t size");
    static_assert(sizeof(sudokuXmmPrintFunc) == sizeof(uintptr_t), "Unexpected sudokuXmmPrintFunc size");
    sudokuXmmPrintFunc = reinterpret_cast<uintptr_t>(&PrintPattern);

    if (!SudokuLoader::CanLaunch(argc, argv)) {
        std::cout << "Cannot solve sudoku maps, counting only\n";
        return 1;
    }

    SudokuLoader sudoku(argc, argv, &std::cin, &std::cout);
    sudoku.Exec();
    return 0;
}

/*
Local Variables:
mode: c++
coding: utf-8-unix
tab-width: nil
c-file-style: "stroustrup"
End:
*/
