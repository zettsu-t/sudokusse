// solver with SSE 4.2 / AVX
// Copyright (C) 2012-2016 Zettsu Tatsuya
// Windows依存部

#include "sudoku_os_dependent.h"
#include <windows.h>

namespace Sudoku {
    // 関数を使う前に定義しないとエラーになる
    template <> void Timer<TimerPlatform::WINDOWS, FILETIME>::reset(void) {
        constexpr decltype(startTimestamp_) zeroTimeStamp {0, 0};
        startTimestamp_ = zeroTimeStamp;
        stopTimestamp_ = zeroTimeStamp;
        BaseTimer::reset();
        return;
    }

    template <> Timer<TimerPlatform::WINDOWS, FILETIME>::Timer(void) {
        reset();
        return;
    }

    template <> void Timer<TimerPlatform::WINDOWS, FILETIME>::Reset(void) {
        reset();
        return;
    }

    // Windows時刻を取得する
    template <> void Timer<TimerPlatform::WINDOWS, FILETIME>::getTimeOfSys(FILETIME& timestamp) {
        GetSystemTimeAsFileTime(&timestamp);
        return;
    }

    template <> SudokuTime Timer<TimerPlatform::WINDOWS, FILETIME>::convertTimeToNum(const FILETIME& timestamp) {
        SudokuTime timeIn100nsec = timestamp.dwHighDateTime;

        // 一度に32bitまとめてシフトしない(x86のシフト命令はシフト幅が5bitしかない)
        for(size_t i=0;i<sizeof(timestamp.dwLowDateTime); ++i) {
            timeIn100nsec <<= 8;
        }
        timeIn100nsec += timestamp.dwLowDateTime;

        return timeIn100nsec;
    }

    template <> ProcessorBinder<TimerPlatform::WINDOWS>::ProcessorBinder(void) {
        /* 使うCPUを固定すると、processorとcacheのaffinityが上がる */
        DWORD_PTR procMask = 1;
        if (!SetProcessAffinityMask(GetCurrentProcess(), procMask)) {
            std::cout << "SetProcessAffinityMask failed\n";
            failed_ = true;
        }
        return;
    }

    std::unique_ptr<ITimer> CreateTimerInstance(void) {
        std::unique_ptr<ITimer> pObj(new Timer<TimerPlatform::WINDOWS, FILETIME>);
        return pObj;
    }

    std::unique_ptr<IProcessorBinder> CreateProcessorBinder(void) {
        std::unique_ptr<IProcessorBinder> pObj(new ProcessorBinder<TimerPlatform::WINDOWS>);
        return pObj;
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
