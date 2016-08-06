// 機種依存部をテストする
// Copyright (C) 2012-2016 Zettsu Tatsuya
//
// クラス定義は下記から流用
// http://www.atmarkit.co.jp/fdotnet/cpptest/cpptest02/cpptest02_03.html

#include <functional>
#include <cppunit/extensions/HelperMacros.h>
#include "sudoku.h"
#include "sudokutest.h"

// Windows/Linux共通
class SudokuTimerTest : public CPPUNIT_NS::TestFixture {
protected:
    // 1ミリ秒以上待つ関数を注入できるようにする
    using MilliSleepFunc = std::function<void(void)>;

    // 派生クラスから呼び出して検査する
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
        // 指定時間以上sleepする
        CPPUNIT_ASSERT(pTimer->GetElapsedTime() >= leastSleepTime);
        // sleepが長すぎるのも設定値が間違っている
        CPPUNIT_ASSERT(pTimer->GetElapsedTime() < (leastSleepTime * 10));
    }

    void checkGetClockInterval(Sudoku::ITimer* pTimer, MilliSleepFunc& milliSleep, SudokuTime leastClockCount) {
        pTimer->StartClock();
        milliSleep();
        pTimer->StopClock();
        // 指定時間以上sleepしたが、クロックは進んでいる
        CPPUNIT_ASSERT(pTimer->GetClockInterval() >= leastClockCount);
    }

    void setUpPrintTime1(Sudoku::BaseTimer* pTimer, SudokuOutStream* pOutStream) {
        constexpr Sudoku::BaseTimer::ClockCount startClockCount {0x4b};    // 75
        constexpr Sudoku::BaseTimer::ClockCount stopClockCount  {0xe1};    // 225 : 200/150 = 1.333 usec/clock
        pTimer->startClockCount_ = startClockCount;
        pTimer->stopClockCount_ = stopClockCount;

        // Total時間は     200 usec(100ns単位->usecになるので1/10)
        // 1回の時間は     66.667 usec
        // 1回の最小時間は 40.000 usec (1.333 * 30 clock)
        pTimer->PrintTime(pOutStream, 3, 30, true);
    }

    void setUpPrintTime2(Sudoku::BaseTimer* pTimer, SudokuOutStream* pOutStream) {
        constexpr Sudoku::BaseTimer::ClockCount startClockCount {0};       // 75
        constexpr Sudoku::BaseTimer::ClockCount stopClockCount  {12300};   // 225 : 200/150 = 1.333 usec/clock
        pTimer->startClockCount_ = startClockCount;
        pTimer->stopClockCount_ = stopClockCount;

        // Total時間は     12 300 000 usec(100ns単位->usecになるので1/10)
        // 1回の時間は     66.667 usec
        // 1回の最小時間は 40.000 usec (1.333 * 30 clock)
        pTimer->PrintTime(pOutStream, 3, 300, true);
    }

    void setUpAndCheckPrintTime(Sudoku::BaseTimer* pTimer, SudokuOutStream* pOutStream) {
        constexpr Sudoku::BaseTimer::ClockCount startClockCount {0xff00000000000000ULL};  // 0.000123456 usec/clock
        constexpr Sudoku::BaseTimer::ClockCount stopClockCount  {0x001fc58ad60d6015ULL};  // 81000518403317781.233
        pTimer->startClockCount_ = startClockCount;
        pTimer->stopClockCount_ = stopClockCount;

        // Total時間は 10000000000000 usec (0が13個)
        // 1回の時間は        2000000 usec (0が6個)
        // 1回の最小時間は 12.3456usec
        pTimer->PrintTime(pOutStream, 5000000, 100000, true);
        pTimer->PrintTime(pOutStream, 5000000, 100000, false);

        // 画面に表示するはずの内容を確認する
        std::string expectedstr;
        expectedstr = "Total : 200usec, 150clock\naverage : 66.667usec, 50clock\n";
        expectedstr += "Once least : 40.000usec, 30clock\n\n";
        expectedstr += "Total : 12sec, 12300000usec, 12300clock\naverage : 4100000.000usec, 4100clock\n";
        expectedstr += "Once least : 300000.000usec, 300clock\n\n";
        expectedstr += "Total : 166666min 40sec, 10000000000000usec, 81000518403317781clock\n";
        expectedstr += "average : 2000000.000usec, 16200103680clock\nOnce least : 12.346usec, 100000clock\n\n";
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
