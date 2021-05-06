// Sudoku solver with SSE 4.2 / AVX
// Copyright (C) 2012-2018 Zettsu Tatsuya
// Absorb platform dependencies between Windows and Linux, C++11 and Boost C++ Libraries

#ifndef SUDOKU_OS_DEPENDENT_H_INCLUDED
#define SUDOKU_OS_DEPENDENT_H_INCLUDED

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <functional>
#include <memory>
#include <vector>

using SudokuTime = unsigned long long;   // time (timestamp and duration)
static_assert(sizeof(SudokuTime) == 8, "Unexpected SudokuTime size");

// unit tests
class SudokuTimerTest;
class SudokuWindowsTimerTest;
class SudokuLinuxTimerTest;
class SudokuWindowsProcessorBinderTest;
class SudokuLinuxProcessorBinderTest;
class SudokuBaseParallelRunnerTest;
class SudokuParallelRunnerTest;

namespace Sudoku {
    // Operating Systems
    enum class TimerPlatform {
        WINDOWS,
        LINUX
    };

    // Time and clock
    class ITimer {
    protected:
        ITimer(void) = default;
    public:
        virtual ~ITimer(void) = default;
    public:
        virtual void Reset(void) = 0;         // resets internal timestamps
        virtual void SetStartTime(void) = 0;  // called after starting something
        virtual void SetStopTime(void) = 0;   // called after ending something already started

        // Getting a timestamp via OS takes long and getting CPU clock is used to avoid the delay.
        virtual void StartClock(void) = 0;    // called after starting something
        virtual void StopClock(void) = 0;     // called after ending something already started
        virtual SudokuTime GetElapsedTime(void)  = 0;   // returns elapsed time from SetStartTime to SetStopTime
        virtual SudokuTime GetClockInterval(void) = 0;  // returns elapsed CPU clock counts from StartClock to StopClock

        // This writes time and CPU clock counts to pOutStream
        virtual void PrintTime(std::ostream* pOutStream, SudokuTime count,
                               SudokuTime leastClock, bool showAverage) = 0;
    };

    // Binding a CPU thread and a kernel thread to improve processor affinity
    class IProcessorBinder {
    public:
        IProcessorBinder(void) = default;
        virtual ~IProcessorBinder(void) = default;
    };

    // Implementation to get x64 CPU clock
    class BaseTimer : public ITimer {
        // unit tests
        friend class ::SudokuTimerTest;
        friend class ::SudokuWindowsTimerTest;
        friend class ::SudokuLinuxTimerTest;
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
        using ClockCount = uint64_t;  // x64 CPU clock

        void reset(void) {
            startClockCount_ = 0;
            stopClockCount_ = 0;
        }

        // Get CPU clock via an instruction
        void getTimeOfClock(ClockCount& timestamp) {
            auto pTime = &timestamp;
            // We cannot use %reg in clang inline assembly
            asm volatile (
                ".intel_syntax noprefix\n\t"
                "RDTSC\n\t"
                "mov [rbx], eax\n\t"
                "mov [rbx + 4], edx\n\t"
                ::"b"(pTime):"eax", "edx"
                );
            return;
        }

        static constexpr SudokuTime SudokuTimeUnitInUsec = 10;       // microsecond per 100 nanoseconds (Windows time resolution)
        static constexpr SudokuTime SudokuTimeUsecPerSec = 1000000;  // second per microsecond
        static constexpr SudokuTime SudokuTimeSecPerMinute = 60;     // minute per second
        ClockCount startClockCount_ {0};
        ClockCount stopClockCount_ {0};
    };

    // Implementation to get OS timestamp
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

        virtual void PrintTime(std::ostream* pOutStream, SudokuTime count,
                               SudokuTime leastClock, bool showAverage) override {
            if (pOutStream == nullptr) {
                return;
            }

            const SudokuTime actualCount = (count) ? count : 1;  // 1 for counting solutions
            const SudokuTime usecTime = GetElapsedTime() / SudokuTimeUnitInUsec;
            const SudokuTime clockElapsed = GetClockInterval();
            using FloatTime = double;

            FloatTime usecOnceTime = static_cast<FloatTime>(usecTime);
            usecOnceTime /= static_cast<FloatTime>(actualCount);

            const SudokuTime clockOnce = clockElapsed / actualCount;
            FloatTime leastUsecOnceTime = static_cast<FloatTime>(leastClock * usecTime);
            leastUsecOnceTime /= static_cast<FloatTime>(clockElapsed);

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
        // Get Windows/Linux system time
        void getTimeOfSys(TimeSpecType& timestamp);
        // Convert a time struct into an integer
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
        ProcessorBinder(void);
        virtual ~ProcessorBinder(void) = default;  // does not resolve CPU binding
        ProcessorBinder(const ProcessorBinder&) = delete;
        ProcessorBinder& operator =(const ProcessorBinder&) = delete;
    private:
        bool failed_ {false};  // true if initialization failed
    };

    // This runs "bool f(void)" parallel and reduces their results.
    class BaseParallelRunner {
        // unit test
        friend class ::SudokuBaseParallelRunnerTest;
        friend class ::SudokuParallelRunnerTest;
    public:
        using ResultType = bool;
        using Evaluator = std::function<ResultType(void)>;
        using NumberOfCores = unsigned int;
        BaseParallelRunner(void) = default;
        virtual ~BaseParallelRunner(void) = default;
        BaseParallelRunner(const BaseParallelRunner&) = delete;
        BaseParallelRunner& operator =(const BaseParallelRunner&) = delete;

        NumberOfCores GetHardwareConcurrency(void) {
#ifdef NO_PARALLEL
            return 1;
#else
            auto numberOfCores = getHardwareConcurrency();
            return std::max(numberOfCores, static_cast<decltype(numberOfCores)>(1));
#endif
        }

        void Add(Evaluator& evaluator) {
            evaluatorSet_.push_back(evaluator);
            return;
        }

        ResultType Run(NumberOfCores numberOfCores) {
#ifdef NO_PARALLEL
            ResultType result = runSequential();
#else
            ResultType result = (numberOfCores <= 1) ? runSequential() : runParallel(evaluatorSet_);
#endif
            // Need to clear to prevent using dangling pointers that
            // the function objects destroy after returning this function.
            evaluatorSet_.clear();
            return result;
        }

    protected:
        using EvaluatorSet = std::vector<Evaluator>;
        virtual NumberOfCores getHardwareConcurrency(void) = 0;
        virtual ResultType runParallel(EvaluatorSet& evaluatorSet) = 0;

        template<typename FutureSet> ResultType runParallelImpl(FutureSet& futureSet) {
            ResultType result = false;
            for (auto& f : futureSet) {
                result |= f.get();
            }
            return result;
        }

    private:
        ResultType runSequential(void) {
            ResultType result = false;
            for(auto& evaluator : evaluatorSet_) {
                result |= evaluator();
            }
            return result;
        }

        EvaluatorSet evaluatorSet_;
    };

    // Create an instance for using Windows or Linux
    extern std::unique_ptr<ITimer> CreateTimerInstance(void);
    extern std::unique_ptr<IProcessorBinder> CreateProcessorBinder(void);

    // Create an instance for using C++11 or Boost C++ Libraries
    extern std::unique_ptr<BaseParallelRunner> CreateParallelRunner(void);
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
