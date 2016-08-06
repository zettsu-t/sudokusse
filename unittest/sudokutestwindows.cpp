// Windows依存部をテストする
// Copyright (C) 2012-2016 Zettsu Tatsuya
//
// クラス定義は下記から流用
// http://www.atmarkit.co.jp/fdotnet/cpptest/cpptest02/cpptest02_03.html

#include <cassert>
#include <windows.h>
#include "sudokutest_os_dependent.h"

namespace {
    using TestedTimer = Sudoku::Timer<Sudoku::TimerPlatform::WINDOWS, FILETIME>;
    using ProcessorBinder = Sudoku::ProcessorBinder<Sudoku::TimerPlatform::WINDOWS>;
}

class SudokuWindowsTimerTest : public SudokuTimerTest {
    CPPUNIT_TEST_SUITE(SudokuWindowsTimerTest);
    CPPUNIT_TEST(test_Constructor);
    CPPUNIT_TEST(test_Reset);
    CPPUNIT_TEST(test_SetStartTime);
    CPPUNIT_TEST(test_SetStopTime);
    CPPUNIT_TEST(test_StartClock);
    CPPUNIT_TEST(test_StopClock);
    CPPUNIT_TEST(test_GetElapsedTime);
    CPPUNIT_TEST(test_GetClockInterval);
    CPPUNIT_TEST(test_PrintTime);
    CPPUNIT_TEST(test_convertTimeToNum);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp() override;
    void tearDown() override;
protected:
    void test_Constructor();
    void test_Reset();
    void test_SetStartTime();
    void test_SetStopTime();
    void test_StartClock();
    void test_StopClock();
    void test_GetElapsedTime();
    void test_GetClockInterval();
    void test_PrintTime();
    void test_convertTimeToNum();

    // 派生クラスのメンバを参照できるようにする
    std::shared_ptr<TestedTimer> createTimerInstance(void) {
        std::shared_ptr<Sudoku::ITimer> pTimer = std::move(Sudoku::CreateTimerInstance());
        std::shared_ptr<TestedTimer> pBaseTimer = std::dynamic_pointer_cast<TestedTimer>(pTimer);
        return pBaseTimer;
    }

    // 指定ミリ秒待つ
    MilliSleepFunc createMilliSleepFunc(int timeInMsec) {
        MilliSleepFunc func(std::bind(::Sleep, timeInMsec));
        return func;
    }
};

class SudokuWindowsProcessorBinderTest : public SudokuProcessorBinderTest {
    CPPUNIT_TEST_SUITE(SudokuWindowsProcessorBinderTest);
    CPPUNIT_TEST(test_Constructor);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp() override;
    void tearDown() override;
protected:
    void test_Constructor();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SudokuWindowsTimerTest);
CPPUNIT_TEST_SUITE_REGISTRATION(SudokuWindowsProcessorBinderTest);

// 各テスト・ケースの実行直前に呼ばれる
void SudokuWindowsTimerTest::setUp() {
    return;
}

// 各テスト・ケースの実行直後に呼ばれる
void SudokuWindowsTimerTest::tearDown() {
    return;
}

// これ以降はテスト・ケースの実装内容
void SudokuWindowsTimerTest::test_Constructor() {
    auto pTimer = createTimerInstance();
    checkConstructor(pTimer.get());
    CPPUNIT_ASSERT((pTimer->startTimestamp_.dwLowDateTime == 0) &&
                   (pTimer->startTimestamp_.dwHighDateTime == 0) &&
                   (pTimer->stopTimestamp_.dwLowDateTime == 0) &&
                   (pTimer->stopTimestamp_.dwHighDateTime == 0));
}

void SudokuWindowsTimerTest::test_Reset() {
    auto pTimer = createTimerInstance();
    checkReset(pTimer.get());
    CPPUNIT_ASSERT((pTimer->startTimestamp_.dwLowDateTime == 0) &&
                   (pTimer->startTimestamp_.dwHighDateTime == 0) &&
                   (pTimer->stopTimestamp_.dwLowDateTime == 0) &&
                   (pTimer->stopTimestamp_.dwHighDateTime == 0));
}

void SudokuWindowsTimerTest::test_SetStartTime() {
    auto pTimer = createTimerInstance();
    pTimer->SetStartTime();
    CPPUNIT_ASSERT((pTimer->startTimestamp_.dwLowDateTime != 0) ||
                   (pTimer->startTimestamp_.dwHighDateTime != 0));
}

void SudokuWindowsTimerTest::test_SetStopTime() {
    auto pTimer = createTimerInstance();
    pTimer->SetStopTime();
    CPPUNIT_ASSERT((pTimer->stopTimestamp_.dwLowDateTime != 0) ||
                   (pTimer->stopTimestamp_.dwHighDateTime != 0));
}

void SudokuWindowsTimerTest::test_StartClock() {
    auto pTimer = createTimerInstance();
    checkStartClock(pTimer.get());
}

void SudokuWindowsTimerTest::test_StopClock() {
    auto pTimer = createTimerInstance();
    checkStopClock(pTimer.get());
}

void SudokuWindowsTimerTest::test_GetElapsedTime() {
    auto pTimer = createTimerInstance();
    constexpr SudokuTime msecTime = 50;
    // msec/usec * 100nsec解像度
    constexpr SudokuTime timeInUnit = msecTime * 1000 * Sudoku::BaseTimer::SudokuTimeUnitInUsec;
    MilliSleepFunc func = createMilliSleepFunc(msecTime);
    checkGetElapsedTime(pTimer.get(), func, timeInUnit);
}

void SudokuWindowsTimerTest::test_GetClockInterval() {
    auto pTimer = createTimerInstance();
    constexpr SudokuTime msecTime = 1;
    // 1GHzなら、1clock = 1nsec : 最低0.2GHzで動作すると仮定する
    constexpr SudokuTime clockCount = msecTime * 200000;
    MilliSleepFunc func = createMilliSleepFunc(msecTime);
    checkGetClockInterval(pTimer.get(), func, clockCount);
}

void SudokuWindowsTimerTest::test_PrintTime() {
    std::unique_ptr<SudokuOutStream> pOutStream(new SudokuOutStream());
    auto pTimer = createTimerInstance();
    {
        constexpr FILETIME startTimestamp  {0x3e8UL, 0UL};  // 1000 * 100nsec
        constexpr FILETIME stopTimestamp   {0xbb8UL, 0UL};  // 3000 * 100nsec
        pTimer->startTimestamp_ = startTimestamp;
        pTimer->stopTimestamp_ = stopTimestamp;
        setUpPrintTime1(pTimer.get(), pOutStream.get());
    }

    {
        constexpr FILETIME startTimestamp  {0x0, 0UL};
        constexpr FILETIME stopTimestamp   {0x754d4c0, 0UL};  // 123000000 * 100nsec
        pTimer->startTimestamp_ = startTimestamp;
        pTimer->stopTimestamp_ = stopTimestamp;
        setUpPrintTime2(pTimer.get(), pOutStream.get());
    }

    {
        constexpr FILETIME startTimestamp  {0x4e72a000, 0xfffff918};  //  10000000000000 (0が13個) * 100nsec
        constexpr FILETIME stopTimestamp   {0x5eece003, 0x0000540b};  // 110000000000003 (0が間に12個) * 100nsec
        pTimer->startTimestamp_ = startTimestamp;
        pTimer->stopTimestamp_ = stopTimestamp;
        setUpAndCheckPrintTime(pTimer.get(), pOutStream.get());
    }

    return;
}

void SudokuWindowsTimerTest::test_convertTimeToNum() {
    struct TestSet {
        FILETIME   filetime; // low, highの順
        SudokuTime sudokutime;
    };

    constexpr TestSet testSet[] {
        {{0UL,0UL}, 0ULL},
        {{1UL,0UL}, 1ULL},
        {{0UL,1UL}, 0x100000000ULL},
        {{2UL,1UL}, 0x100000002ULL},
        {{0xffffffffUL,1UL}, 0x1ffffffffULL},
        {{0xffffffffUL,0xffffffffUL}, 0xffffffffffffffffULL}
    };

    auto pTimer = createTimerInstance();
    for(const auto& test : testSet) {
        CPPUNIT_ASSERT_EQUAL(test.sudokutime, pTimer->convertTimeToNum(test.filetime));
    }
    return;
}

// 各テスト・ケースの実行直前に呼ばれる
void SudokuWindowsProcessorBinderTest::setUp() {
    return;
}

// 各テスト・ケースの実行直後に呼ばれる
void SudokuWindowsProcessorBinderTest::tearDown() {
    return;
}

// これ以降はテスト・ケースの実装内容
void SudokuWindowsProcessorBinderTest::test_Constructor() {
    auto pBinder = Sudoku::CreateProcessorBinder();
    ProcessorBinder* pConcreteBinder = dynamic_cast<decltype(pConcreteBinder)>(pBinder.get());
    CPPUNIT_ASSERT(!pConcreteBinder->failed_);
}

/*
Local Variables:
mode: c++
coding: utf-8-unix
tab-width: nil
c-file-style: "stroustrup"
End:
*/
