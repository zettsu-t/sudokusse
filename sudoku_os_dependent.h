// Sudoku solver with SSE 4.2 / AVX
// Copyright (C) 2012-2017 Zettsu Tatsuya
// WindowsとLinuxの違いを吸収する

#ifndef SUDOKU_OS_DEPENDENT_H_INCLUDED
#define SUDOKU_OS_DEPENDENT_H_INCLUDED

#include <iostream>
#include <iomanip>
#include <memory>

using SudokuTime = unsigned long long;   // 時刻と時間
static_assert(sizeof(SudokuTime) == 8, "Unexpected SudokuTime size");

// unit test
class SudokuTimerTest;
class SudokuWindowsTimerTest;
class SudokuLinuxTimerTest;
class SudokuWindowsProcessorBinderTest;
class SudokuLinuxProcessorBinderTest;

// 読み込みと実行時間測定
namespace Sudoku {
    // OSから時刻を取得する方法
    enum class TimerPlatform {
        WINDOWS,
        LINUX
    };

    // OSから時刻を取得するクラス
    class ITimer {
    protected:
        ITimer(void) = default;
    public:
        virtual ~ITimer(void) = default;
    public:
        virtual void Reset(void) = 0;         // 保存している時刻を破棄する
        virtual void SetStartTime(void) = 0;  // 開始時刻を打つ
        virtual void SetStopTime(void) = 0;   // 終了時刻を打つ
        // OSの時刻を取得するのは時間がかかるので、
        // クロックだけ短時間で取得したい時があるため別I/Fにする
        virtual void StartClock(void) = 0;    // 開始クロックを打つ
        virtual void StopClock(void) = 0;     // 終了クロックを打つ
        virtual SudokuTime GetElapsedTime(void)  = 0;   // 開始から終了までの経過時間を取得する
        virtual SudokuTime GetClockInterval(void) = 0;  // 開始から終了までの経過クロック数を取得する
        virtual void PrintTime(std::ostream* pOutStream, SudokuTime count,
                               SudokuTime leastClock, bool showAverage) = 0;  // 経過時間とクロック数を表示する
    };

    // 使用するプロセッサを固定してProcessor Affinityを向上するよう設定するクラス
    class IProcessorBinder {
    public:
        IProcessorBinder(void) = default;
        virtual ~IProcessorBinder(void) = default;
    };

    // OSから時刻を取得するクラス(CPUクロックは64bit固定)
    class BaseTimer : public ITimer {
        // unit test
        friend class ::SudokuTimerTest;
    protected:
        BaseTimer(void) = default;
    public:
        virtual ~BaseTimer(void) = default;
        BaseTimer(const BaseTimer&) = delete;
        BaseTimer& operator =(const BaseTimer&) = delete;
        virtual void StartClock(void) override { getTimeOfClock(startClockCount_); }
        virtual void StopClock(void) override { getTimeOfClock(stopClockCount_); }
        virtual SudokuTime GetClockInterval(void) override {
            return stopClockCount_ - startClockCount_;
        }
    protected:
        using ClockCount = uint64_t;  // CPUから取得するクロック

        void reset(void) {
            startClockCount_ = 0;
            stopClockCount_ = 0;
        }

        // CPUクロックカウンタを取得する
        void getTimeOfClock(ClockCount& timestamp) {
            auto pTime = &timestamp;
            asm volatile (
                "RDTSC\n\t"
                "mov [%0], eax\n\t"
                "mov [%0+4], edx\n\t"
                ::"r"(pTime):"eax", "edx"
                );
            return;
        }

        static constexpr SudokuTime SudokuTimeUnitInUsec = 10;       // 1usecの時間単位(100nsec*10単位)
        static constexpr SudokuTime SudokuTimeUsecPerSec = 1000000;  // 1秒当たり1usec
        static constexpr SudokuTime SudokuTimeSecPerMinute = 60;     // 1分当たり秒
        ClockCount startClockCount_ {0};
        ClockCount stopClockCount_ {0};
    };

    // OSから時刻を取得する具象クラス
    template <TimerPlatform timerPlatform, typename TimeSpecType>
    class Timer : public BaseTimer {
        // Unit test
        friend class ::SudokuWindowsTimerTest;
        friend class ::SudokuLinuxTimerTest;
    public:
        Timer(void);
        virtual ~Timer(void) = default;
        virtual void Reset(void) override;
        virtual void SetStartTime(void) override { getTimeOfSys(startTimestamp_); }
        virtual void SetStopTime(void) override { getTimeOfSys(stopTimestamp_); }
        virtual SudokuTime GetElapsedTime(void) override {
            return convertTimeToNum(stopTimestamp_) - convertTimeToNum(startTimestamp_);
        }

        // 処理時間を表示する
        virtual void PrintTime(std::ostream* pOutStream, SudokuTime count,
                               SudokuTime leastClock, bool showAverage) override {
            if (pOutStream == nullptr) {
                return;
            }

            // 解の数だけ求める場合
            const SudokuTime actualCount = (count) ? count : 1;
            const SudokuTime usecTime = GetElapsedTime() / SudokuTimeUnitInUsec;
            const SudokuTime clockElapsed = GetClockInterval();
            const double usecOnceTime = static_cast<decltype(usecOnceTime)>(usecTime) /
                static_cast<decltype(usecOnceTime)>(actualCount);
            const SudokuTime clockOnce = clockElapsed / actualCount;
            const double leastUsecOnceTime = static_cast<decltype(leastUsecOnceTime)>(leastClock  * usecTime) /
                static_cast<decltype(leastUsecOnceTime)>(clockElapsed);

            SudokuTime secTime = usecTime / SudokuTimeUsecPerSec;
            const SudokuTime minTime = secTime / SudokuTimeSecPerMinute;
            secTime = secTime % SudokuTimeSecPerMinute;

            (*pOutStream) << std::dec;
            (*pOutStream) << "Total : ";
            if (minTime > 0) {
                (*pOutStream) << minTime << "min ";
            }
            if ((minTime > 0) || (secTime > 0)) {
                (*pOutStream) << secTime << "sec, ";
            }
            (*pOutStream) << std::dec << usecTime << "usec, ";
            (*pOutStream) << std::dec << clockElapsed << "clock\n";
            if (showAverage) {
                (*pOutStream) << "Average : " << std::fixed << std::setprecision(3) << usecOnceTime << "usec, ";
                (*pOutStream) << std::dec << clockOnce << "clock\n";
                (*pOutStream) << "Once least : " << std::fixed << std::setprecision(3) << leastUsecOnceTime << "usec, ";
                (*pOutStream) << std::dec << leastClock << "clock\n\n";
            }
            return;
        }
    private:
        // Windows/Linuxのシステム時刻を取得する
        void getTimeOfSys(TimeSpecType& timestamp);
        // 時間構造体の値を整数にする
        SudokuTime convertTimeToNum(const TimeSpecType& timestamp);
        void reset(void);
        TimeSpecType startTimestamp_;
        TimeSpecType stopTimestamp_;
    };

    template <TimerPlatform timerPlatform>
    class ProcessorBinder : public IProcessorBinder {
        friend class ::SudokuWindowsProcessorBinderTest;
        friend class ::SudokuLinuxProcessorBinderTest;
    public:
        ProcessorBinder(void);  // 使うCPUを固定する
        virtual ~ProcessorBinder(void) = default;  // 使うCPUを固定したら戻さない
        ProcessorBinder(const ProcessorBinder&) = delete;
        ProcessorBinder& operator =(const ProcessorBinder&) = delete;
    private:
        bool failed_ {false};  // 設定に失敗した
    };

    // WindowsとLinuxに応じたインスタンスを取得する
    extern std::unique_ptr<ITimer> CreateTimerInstance(void);
    extern std::unique_ptr<IProcessorBinder> CreateProcessorBinder(void);
}

#endif // SUDOKU_OS_DEPENDENT_H_INCLUDED

/*
Local Variables:
mode: c++
coding: utf-8-unix
tab-width: nil
c-file-style: "stroustrup"
End:
*/
