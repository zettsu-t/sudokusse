// Sudoku solver with SSE 4.2
// Copyright (C) 2012-2013 Zettsu Tatsuya

#include "sudoku.h"

int main(int argc, char *argv[]) {
    /* サイズ検査(static_assertが使えるときはそうする) */
    if ((sizeof(SudokuTime) != 8) ||
        (sizeof(SudokuSseElement) != 4) ||
        (sizeof(__m128) != 16) ||
        (sizeof(sudokuXmmPrintFunc) < sizeof(uintptr_t))) {
        std::cerr << "type size error" << std::endl;
        return 1;
    }

    /* 使うCPUを固定すると、processorとcacheのaffinityが上がる */
    DWORD_PTR procMask = 1;
    if (!SetProcessAffinityMask(GetCurrentProcess(), procMask)) {
        std::cout << "SetProcessAffinityMask failed\n";
    }

    /* 本当はデータのポインタと関数のポインタは互換ではない */
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
