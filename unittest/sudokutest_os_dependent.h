// Testing platform dependent implementation
// Copyright (C) 2012-2017 Zettsu Tatsuya
//
// I use CppUnit code on the website.
// http://www.atmarkit.co.jp/fdotnet/cpptest/cpptest02/cpptest02_03.html

#include <functional>
#include <cppunit/extensions/HelperMacros.h>
#include "sudoku.h"
#include "sudokutest.h"

// Common for Windows and Linux
class SudokuTimerTest : public CPPUNIT_NS::TestFixture {
protected:
    // We inject a function to test cases to sleep 1 msec and more
    using MilliSleepFunc = std::function<void(void)>;

    // Called from derives classes
    void checkConstructor(Sudoku::BaseTimer* pTimer) {
        CPPUNIT_ASSERT(!pTimer->startClockCount_);
        CPPUNIT_ASSERT(!pTimer->stopClockCount_);

        auto expectedElapsedTime = pTimer->GetElapsedTime();
        auto expectedClockInterval = pTimer->GetClockInterval();
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(expectedElapsedTime)>(0), expectedElapsedTime);
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(expectedClockInterval)>(0), expectedClockInterval);
    }

    void checkReset(Sudoku::BaseTimer* pTimer) {
        pTimer->SetStartTime();
        CPPUNIT_ASSERT(pTimer->GetElapsedTime());
        pTimer->StartClock();
        CPPUNIT_ASSERT(pTimer->GetClockInterval());

        pTimer->Reset();
        CPPUNIT_ASSERT(!pTimer->startClockCount_);
        CPPUNIT_ASSERT(!pTimer->stopClockCount_);
        CPPUNIT_ASSERT(!pTimer->GetElapsedTime());
        CPPUNIT_ASSERT(!pTimer->GetClockInterval());
    }

    void checkStartClock(Sudoku::BaseTimer* pTimer) {
        pTimer->StartClock();
        CPPUNIT_ASSERT(pTimer->startClockCount_);
        CPPUNIT_ASSERT(!pTimer->stopClockCount_);
    }

    void checkStopClock(Sudoku::BaseTimer* pTimer) {
        pTimer->StopClock();
        CPPUNIT_ASSERT(!pTimer->startClockCount_);
        CPPUNIT_ASSERT(pTimer->stopClockCount_);
    }

    void checkGetElapsedTime(Sudoku::ITimer* pTimer, MilliSleepFunc& milliSleep, SudokuTime leastSleepTime) {
        pTimer->SetStartTime();
        milliSleep();
        pTimer->SetStopTime();
        // Sleep() does not sleep exact specified time. It may get longer.
        CPPUNIT_ASSERT(pTimer->GetElapsedTime() >= leastSleepTime);
        // We assume this test is failed if it sleeps too long time.
        CPPUNIT_ASSERT(pTimer->GetElapsedTime() < (leastSleepTime * 10));
    }

    void checkGetClockInterval(Sudoku::ITimer* pTimer, MilliSleepFunc& milliSleep, SudokuTime leastClockCount) {
        pTimer->StartClock();
        milliSleep();
        pTimer->StopClock();
        // We do not expect wraparound of CPU clock.
        CPPUNIT_ASSERT(pTimer->GetClockInterval() >= leastClockCount);
    }

    void setUpPrintTime1(Sudoku::BaseTimer* pTimer, SudokuOutStream* pOutStream) {
        constexpr Sudoku::BaseTimer::ClockCount startClockCount {0x4b};    // 75
        constexpr Sudoku::BaseTimer::ClockCount stopClockCount  {0xe1};    // 225 : 200/150 = 1.333 usec/clock
        pTimer->startClockCount_ = startClockCount;
        pTimer->stopClockCount_ = stopClockCount;

        // Total time : 200 usec (divide by 10 to convert the time unit from 100nsec to 1usec)
        // once          66.667 usec
        // least         40.000 usec (1.333 * 30 clock)
        pTimer->PrintTime(pOutStream, 3, 30, true);
    }

    void setUpPrintTime2(Sudoku::BaseTimer* pTimer, SudokuOutStream* pOutStream) {
        constexpr Sudoku::BaseTimer::ClockCount startClockCount {0};       // 75
        constexpr Sudoku::BaseTimer::ClockCount stopClockCount  {12300};   // 225 : 200/150 = 1.333 usec/clock
        pTimer->startClockCount_ = startClockCount;
        pTimer->stopClockCount_ = stopClockCount;

        // Total time :  12 300 000 usec (divide by 10)
        // once          66.667 usec
        // least         40.000 usec (1.333 * 30 clock)
        pTimer->PrintTime(pOutStream, 3, 300, true);
    }

    void setUpAndCheckPrintTime(Sudoku::BaseTimer* pTimer, SudokuOutStream* pOutStream) {
        constexpr Sudoku::BaseTimer::ClockCount startClockCount {0xff00000000000000ULL};  // 0.000123456 usec/clock
        constexpr Sudoku::BaseTimer::ClockCount stopClockCount  {0x001fc58ad60d6015ULL};  // 81000518403317781.233
        pTimer->startClockCount_ = startClockCount;
        pTimer->stopClockCount_ = stopClockCount;

        // Total 10000000000000 usec (13 zeros)
        // once         2000000 usec (6 zeros)
        // least        12.3456 usec
        pTimer->PrintTime(pOutStream, 5000000, 100000, true);
        pTimer->PrintTime(pOutStream, 5000000, 100000, false);

        // Confirm a string that we can see on terminals
        std::string expectedstr;
        expectedstr = "Total : 200usec, 150clock\nAverage : 66.667usec, 50clock\n";
        expectedstr += "Once least : 40.000usec, 30clock\n\n";
        expectedstr += "Total : 12sec, 12300000usec, 12300clock\nAverage : 4100000.000usec, 4100clock\n";
        expectedstr += "Once least : 300000.000usec, 300clock\n\n";
        expectedstr += "Total : 166666min 40sec, 10000000000000usec, 81000518403317781clock\n";
        expectedstr += "Average : 2000000.000usec, 16200103680clock\nOnce least : 12.346usec, 100000clock\n\n";
        expectedstr += "Total : 166666min 40sec, 10000000000000usec, 81000518403317781clock\n";

        const std::string actualstr = pOutStream->str();
        CPPUNIT_ASSERT(actualstr == expectedstr);
    }
};

class SudokuProcessorBinderTest : public CPPUNIT_NS::TestFixture {};

/*
Local Variables:
mode: c++
coding: utf-8-unix
tab-width: nil
c-file-style: "stroustrup"
End:
*/
