// solver with SSE 4.2 / AVX
// Copyright (C) 2012-2016 Zettsu Tatsuya
// Linux依存部

#include <time.h>
#include <sched.h>
#include "sudoku_os_dependent.h"

namespace Sudoku {
    // 関数を使う前に定義しないとエラーになる
    template <> void Timer<TimerPlatform::LINUX, timespec>::reset(void) {
        constexpr decltype(startTimestamp_) zeroTimeStamp {0, 0};
        startTimestamp_ = zeroTimeStamp;
        stopTimestamp_ = zeroTimeStamp;
        BaseTimer::reset();
        return;
    }

    template <> Timer<TimerPlatform::LINUX, timespec>::Timer(void) {
        reset();
        return;
    }

    template <> void Timer<TimerPlatform::LINUX, timespec>::Reset(void) {
        reset();
        return;
    }

    // Linux時刻を取得する
    template <> void Timer<TimerPlatform::LINUX, timespec>::getTimeOfSys(timespec& timestamp) {
        clock_gettime(CLOCK_MONOTONIC, &timestamp);
        return;
    }

    template <> SudokuTime Timer<TimerPlatform::LINUX, timespec>::convertTimeToNum(const timespec& timestamp) {
        SudokuTime timeIn100nsec = timestamp.tv_sec;
        timeIn100nsec *= SudokuTimeUsecPerSec;
        timeIn100nsec *= SudokuTimeUnitInUsec;

        constexpr auto unit = static_cast<decltype(SudokuTimeUnitInUsec)>(1000) / SudokuTimeUnitInUsec;
        timeIn100nsec += timestamp.tv_nsec / unit;
        return timeIn100nsec;
    }

    template <> ProcessorBinder<TimerPlatform::LINUX>::ProcessorBinder(void) {
        /* 使うCPUを固定すると、processorとcacheのaffinityが上がる */
        cpu_set_t mask;
        CPU_ZERO(&mask);
        CPU_SET(0, &mask);

        if (sched_setaffinity(0, sizeof(mask), &mask)) {
            std::cout << "SetProcessAffinityMask failed\n";
            failed_ = true;
        }
        return;
    }

    std::unique_ptr<ITimer> CreateTimerInstance(void) {
        std::unique_ptr<ITimer> pObj(new Timer<TimerPlatform::LINUX, timespec>);
        return pObj;
    }

    std::unique_ptr<IProcessorBinder> CreateProcessorBinder(void) {
        std::unique_ptr<IProcessorBinder> pObj(new ProcessorBinder<TimerPlatform::LINUX>);
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
