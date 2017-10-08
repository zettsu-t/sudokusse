// Testing Linux dependent implementation
// Copyright (C) 2012-2017 Zettsu Tatsuya
//
// I use CppUnit code on the website.
// http://www.atmarkit.co.jp/fdotnet/cpptest/cpptest02/cpptest02_03.html

#include <cassert>
#include <sched.h>
#include <time.h>
#include "sudokutest_os_dependent.h"

namespace {
    using TestedTimer = Sudoku::Timer<Sudoku::TimerPlatform::LINUX, timespec>;
    using ProcessorBinder = Sudoku::ProcessorBinder<Sudoku::TimerPlatform::LINUX>;
}

class SudokuLinuxTimerTest : public SudokuTimerTest {
    CPPUNIT_TEST_SUITE(SudokuLinuxTimerTest);
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

    // Access to members of derived classes of ITimer
    std::shared_ptr<TestedTimer> createTimerInstance(void) {
        std::shared_ptr<Sudoku::ITimer> pTimer = std::move(Sudoku::CreateTimerInstance());
        std::shared_ptr<TestedTimer> pBaseTimer = std::dynamic_pointer_cast<TestedTimer>(pTimer);
        return pBaseTimer;
    }

    // Waits specified time 'timeInMsec'
    MilliSleepFunc createMilliSleepFunc(int timeInMsec) {
        auto funcLambda = [=](decltype(timeInMsec) argTimeInMsec) {
            // Do not consider 1 second and over
            timespec req {0, argTimeInMsec * 1000000};
            timespec rem {0, 0};
            while(::nanosleep(&req, &rem) == EINTR) {
                req = rem;
            }
            return;
        };

        MilliSleepFunc func(std::bind(funcLambda, timeInMsec));
        return func;
    }
};

class SudokuLinuxProcessorBinderTest : public SudokuProcessorBinderTest {
    CPPUNIT_TEST_SUITE(SudokuLinuxProcessorBinderTest);
    CPPUNIT_TEST(test_Constructor);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;
protected:
    void test_Constructor();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SudokuLinuxTimerTest);
CPPUNIT_TEST_SUITE_REGISTRATION(SudokuLinuxProcessorBinderTest);

// Call before running a test
void SudokuLinuxTimerTest::setUp() {
    return;
}

// Call after running a test
void SudokuLinuxTimerTest::tearDown() {
    return;
}

// Test cases
void SudokuLinuxTimerTest::test_Constructor() {
    auto pTimer = createTimerInstance();
    checkConstructor(pTimer.get());
    CPPUNIT_ASSERT((pTimer->startTimestamp_.tv_sec == 0) &&
                   (pTimer->startTimestamp_.tv_nsec == 0) &&
                   (pTimer->stopTimestamp_.tv_sec == 0) &&
                   (pTimer->stopTimestamp_.tv_nsec == 0));
}

void SudokuLinuxTimerTest::test_Reset() {
    auto pTimer = createTimerInstance();
    checkReset(pTimer.get());
    CPPUNIT_ASSERT((pTimer->startTimestamp_.tv_sec == 0) &&
                   (pTimer->startTimestamp_.tv_nsec == 0) &&
                   (pTimer->stopTimestamp_.tv_sec == 0) &&
                   (pTimer->stopTimestamp_.tv_nsec == 0));
}

void SudokuLinuxTimerTest::test_SetStartTime() {
    auto pTimer = createTimerInstance();
    pTimer->SetStartTime();
    CPPUNIT_ASSERT((pTimer->startTimestamp_.tv_sec != 0) ||
                   (pTimer->startTimestamp_.tv_nsec != 0));
}

void SudokuLinuxTimerTest::test_SetStopTime() {
    auto pTimer = createTimerInstance();
    pTimer->SetStopTime();
    CPPUNIT_ASSERT((pTimer->stopTimestamp_.tv_sec != 0) ||
                   (pTimer->stopTimestamp_.tv_nsec != 0));
}

void SudokuLinuxTimerTest::test_StartClock() {
    auto pTimer = createTimerInstance();
    checkStartClock(pTimer.get());
}

void SudokuLinuxTimerTest::test_StopClock() {
    auto pTimer = createTimerInstance();
    checkStopClock(pTimer.get());
}

void SudokuLinuxTimerTest::test_GetElapsedTime() {
    auto pTimer = createTimerInstance();
    constexpr int msecTime = 50;
    // resolution in msec/usec * 100 nsec
    constexpr SudokuTime timeInUnit = msecTime * 1000 * Sudoku::BaseTimer::SudokuTimeUnitInUsec;
    MilliSleepFunc func = createMilliSleepFunc(msecTime);
    checkGetElapsedTime(pTimer.get(), func, timeInUnit);
}

void SudokuLinuxTimerTest::test_GetClockInterval() {
    auto pTimer = createTimerInstance();
    constexpr int msecTime = 1;
    // 1 clock = 1nsec at 1GHz. We expect x64 CPUs run at 0.2GHz or over.
    constexpr SudokuTime clockCount = msecTime * 200000;
    MilliSleepFunc func = createMilliSleepFunc(msecTime);
    checkGetClockInterval(pTimer.get(), func, clockCount);
}

void SudokuLinuxTimerTest::test_PrintTime() {
    std::unique_ptr<SudokuOutStream> pOutStream(new SudokuOutStream());
    auto pTimer = createTimerInstance();
    {
        constexpr timespec startTimestamp {0, 100000UL};  // 1000 * 100nsec
        constexpr timespec stopTimestamp  {0, 300000UL};  // 3000 * 100nsec
        pTimer->startTimestamp_ = startTimestamp;
        pTimer->stopTimestamp_ = stopTimestamp;
        setUpPrintTime1(pTimer.get(), pOutStream.get());
    }

    {
        constexpr timespec startTimestamp {0x0, 0UL};
        constexpr timespec stopTimestamp  {12, 300000000UL};  // 123000000 * 100nsec
        pTimer->startTimestamp_ = startTimestamp;
        pTimer->stopTimestamp_ = stopTimestamp;
        setUpPrintTime2(pTimer.get(), pOutStream.get());
    }

    {
        constexpr timespec startTimestamp { 1000000,   0};  //  1000000'0000000 (13 zeros) * 100 nsec
        constexpr timespec stopTimestamp  {11000000, 300};  // 11000000'0000003 (12 zeros) * 100 nsec
        pTimer->startTimestamp_ = startTimestamp;
        pTimer->stopTimestamp_ = stopTimestamp;
        setUpAndCheckPrintTime(pTimer.get(), pOutStream.get());
    }

    return;
}

void SudokuLinuxTimerTest::test_convertTimeToNum() {
    struct TestSet {
        timespec   timestamp;
        SudokuTime sudokutime;
    };

    constexpr TestSet testSet[] {
        {{0UL,0UL}, 0ULL},
        {{0UL,1UL}, 0ULL},
        {{0UL,1200UL}, 12ULL},
        {{1UL,0UL}, 10000000ULL},
        {{1UL,2300UL}, 10000023ULL},
        {{4UL,3000001298UL}, 70000012ULL}
    };

    auto pTimer = createTimerInstance();
    for(const auto& test : testSet) {
        CPPUNIT_ASSERT_EQUAL(test.sudokutime, pTimer->convertTimeToNum(test.timestamp));
    }
    return;
}

// Call before running a test
void SudokuLinuxProcessorBinderTest::setUp() {
    return;
}

// Call after running a test
void SudokuLinuxProcessorBinderTest::tearDown() {
    return;
}

// Test cases
void SudokuLinuxProcessorBinderTest::test_Constructor() {
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
