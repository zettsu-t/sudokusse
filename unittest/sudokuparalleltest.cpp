// C++11とBoost C++ Librariesによる並列実行をテストする
// Copyright (C) 2012-2017 Zettsu Tatsuya
//
// クラス定義は下記から流用
// http://www.atmarkit.co.jp/fdotnet/cpptest/cpptest02/cpptest02_03.html

#include <cassert>
#include "sudokutest_os_dependent.h"

#ifdef SOLVE_PARALLEL_WITH_BOOST_THREAD
#include <boost/atomic.hpp>
#include <boost/thread/future.hpp>
#define SUDOKU_TEST_ATOMIC boost::atomic
#define SUDOKU_TEST_THREAD_FUTURE boost::unique_future
#define SUDOKU_TEST_THREAD_ASYNC  boost::async
#define SUDOKU_TEST_THREAD_LAUNCH_ASYNC  boost::launch::async
#else
#include <atomic>
#include <future>
#define SUDOKU_TEST_ATOMIC std::atomic
#define SUDOKU_TEST_THREAD_FUTURE std::future
#define SUDOKU_TEST_THREAD_ASYNC  std::async
#define SUDOKU_TEST_THREAD_LAUNCH_ASYNC std::launch::async
#endif

class SudokuBaseParallelRunnerTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(SudokuBaseParallelRunnerTest);
    CPPUNIT_TEST(test_GetHardwareConcurrency);
    CPPUNIT_TEST(test_Add);
    CPPUNIT_TEST(test_Run);
    CPPUNIT_TEST(test_runSequential);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;
protected:
    void test_GetHardwareConcurrency();
    void test_Add();
    void test_Run();
    void test_runSequential();
private:
    using Evaluator = Sudoku::BaseParallelRunner::Evaluator;
    using NumberOfCores = Sudoku::BaseParallelRunner::NumberOfCores;

    bool setupFalseEvaluatorSet();
    bool setupAnyTrueEvaluatorSet();
    bool setupTrueEvaluatorSet();

    std::unique_ptr<Sudoku::BaseParallelRunner> pRunner_;
    Evaluator evaluatorT_;
    Evaluator evaluatorF_;
    std::vector<Evaluator> falseEvaluatorSet_;
    std::vector<Evaluator> anyTrueEvaluatorSet_;
    std::vector<Evaluator> trueEvaluatorSet_;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SudokuBaseParallelRunnerTest);

void SudokuBaseParallelRunnerTest::setUp() {
    pRunner_ = Sudoku::CreateParallelRunner();
    evaluatorT_ = [] { return true; };
    evaluatorF_ = [] { return false; };
    falseEvaluatorSet_ = {evaluatorF_, evaluatorF_, evaluatorF_};
    anyTrueEvaluatorSet_ = {evaluatorF_, evaluatorT_, evaluatorF_};
    trueEvaluatorSet_ = {evaluatorT_, evaluatorT_, evaluatorT_};
    return;
}

void SudokuBaseParallelRunnerTest::tearDown() {
    pRunner_.reset();
    return;
}

void SudokuBaseParallelRunnerTest::test_GetHardwareConcurrency() {
#ifdef NO_PARALLEL
    CPPUNIT_ASSERT(pRunner_->GetHardwareConcurrency() == 1);
#else
    CPPUNIT_ASSERT(pRunner_->GetHardwareConcurrency() >= 1);
#endif
    return;
}

void SudokuBaseParallelRunnerTest::test_Add() {
    CPPUNIT_ASSERT(pRunner_->evaluatorSet_.empty());
    pRunner_->Add(evaluatorT_);
    CPPUNIT_ASSERT(pRunner_->evaluatorSet_.size() == 1);
    pRunner_->Add(evaluatorF_);
    CPPUNIT_ASSERT(pRunner_->evaluatorSet_.size() == 2);
    return;
}

void SudokuBaseParallelRunnerTest::test_Run() {
    constexpr NumberOfCores numberOfCoresSet[] = {1, 2};
    for(auto num : numberOfCoresSet) {
        auto expected = setupFalseEvaluatorSet();
        auto result = pRunner_->Run(num);
        CPPUNIT_ASSERT(pRunner_->evaluatorSet_.empty());
        CPPUNIT_ASSERT_EQUAL(expected, result);

        expected = setupAnyTrueEvaluatorSet();
        result = pRunner_->Run(num);
        CPPUNIT_ASSERT_EQUAL(expected, result);

        expected = setupFalseEvaluatorSet();
        result = pRunner_->Run(num);
        CPPUNIT_ASSERT_EQUAL(expected, result);

        expected = setupTrueEvaluatorSet();
        result = pRunner_->Run(num);
        CPPUNIT_ASSERT_EQUAL(expected, result);
    }

    return;
}

void SudokuBaseParallelRunnerTest::test_runSequential() {
    auto expected = setupFalseEvaluatorSet();
    auto result = pRunner_->runSequential();
    CPPUNIT_ASSERT_EQUAL(expected, result);

    expected = setupAnyTrueEvaluatorSet();
    result = pRunner_->runSequential();
    CPPUNIT_ASSERT_EQUAL(expected, result);

    expected = setupFalseEvaluatorSet();
    result = pRunner_->runSequential();
    CPPUNIT_ASSERT_EQUAL(expected, result);

    expected = setupTrueEvaluatorSet();
    result = pRunner_->runSequential();
    CPPUNIT_ASSERT_EQUAL(expected, result);
    return;
}

bool SudokuBaseParallelRunnerTest::setupFalseEvaluatorSet() {
    pRunner_ = Sudoku::CreateParallelRunner();
    for(auto& f : falseEvaluatorSet_) {
        pRunner_->Add(f);
    }
    return false;
}

bool SudokuBaseParallelRunnerTest::setupAnyTrueEvaluatorSet() {
    pRunner_ = Sudoku::CreateParallelRunner();
    for(auto& f : anyTrueEvaluatorSet_) {
        pRunner_->Add(f);
    }
    return true;
}

bool SudokuBaseParallelRunnerTest::setupTrueEvaluatorSet() {
    pRunner_ = Sudoku::CreateParallelRunner();
    for(auto& f : trueEvaluatorSet_) {
        pRunner_->Add(f);
    }
    return true;
}

class SudokuParallelRunnerTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(SudokuParallelRunnerTest);
    CPPUNIT_TEST(test_GetHardwareConcurrency);
    CPPUNIT_TEST(test_Run);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;
protected:
    void test_GetHardwareConcurrency();
    void test_Run();
private:
    using Evaluator = Sudoku::BaseParallelRunner::Evaluator;
    using EvaluatorSet = std::vector<Evaluator>;
    using NumberOfCores = Sudoku::BaseParallelRunner::NumberOfCores;
    using LoopCount = size_t;

    enum class TestType {
        PUBLIC_FUNC,
        PROTECTED_FUNC,
        IMPL_FUNC,
    };

    void setupEvaluatorSet();
    LoopCount setupEvaluatorSet(NumberOfCores numberOfCores, EvaluatorSet& evaluatorSet);
    void runFutureSet(EvaluatorSet& evaluatorSet);

    std::unique_ptr<Sudoku::BaseParallelRunner> pRunner_;
    Evaluator evaluatorT_;
    Evaluator evaluatorF_;
    SUDOKU_TEST_ATOMIC<LoopCount> countTotal_;
    SUDOKU_TEST_ATOMIC<LoopCount> countT_;
    SUDOKU_TEST_ATOMIC<LoopCount> countF_;
    static constexpr LoopCount loopCount_ = 1000000;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SudokuParallelRunnerTest);

void SudokuParallelRunnerTest::setUp() {
    pRunner_ = Sudoku::CreateParallelRunner();
    return;
}

void SudokuParallelRunnerTest::tearDown() {
    pRunner_.reset();
    return;
}

void SudokuParallelRunnerTest::test_GetHardwareConcurrency() {
    const NumberOfCores publicResult = pRunner_->GetHardwareConcurrency();
#ifdef NO_PARALLEL
    CPPUNIT_ASSERT(publicResult == 1);
#else
    const NumberOfCores privateResult = pRunner_->getHardwareConcurrency();
    CPPUNIT_ASSERT(publicResult >= 1);
    CPPUNIT_ASSERT(privateResult >= 1);
    CPPUNIT_ASSERT(publicResult == privateResult);
#endif
}

void SudokuParallelRunnerTest::test_Run() {
#ifdef NO_PARALLEL
    constexpr TestType testTypeSet[] = {TestType::PUBLIC_FUNC, TestType::IMPL_FUNC};
#else
    constexpr TestType testTypeSet[] = {TestType::PUBLIC_FUNC, TestType::PROTECTED_FUNC, TestType::IMPL_FUNC};
#endif
    for(auto testType : testTypeSet) {
        constexpr NumberOfCores numberOfCoresSet[] = {1, 2, 4, 8};
        for(auto num : numberOfCoresSet) {
            EvaluatorSet evaluatorSet;
            LoopCount count = setupEvaluatorSet(num, evaluatorSet) * num;

            switch(testType) {
            case TestType::PUBLIC_FUNC:
                pRunner_->Run(num);
                CPPUNIT_ASSERT(pRunner_->evaluatorSet_.empty());
                break;
            case TestType::PROTECTED_FUNC:
                pRunner_->runParallel(evaluatorSet);
                break;
            case TestType::IMPL_FUNC:
                runFutureSet(evaluatorSet);
                break;
            default:
                break;
            }

            LoopCount countT = countT_;
            LoopCount countF = countT_;
            LoopCount countTotal = countTotal_;
            CPPUNIT_ASSERT_EQUAL(count, countT);
            CPPUNIT_ASSERT_EQUAL(count, countF);
            CPPUNIT_ASSERT_EQUAL(count * 2, countTotal);
        }
    }

    return;
}

SudokuParallelRunnerTest::LoopCount SudokuParallelRunnerTest::setupEvaluatorSet(NumberOfCores numberOfCores,
                                                                                EvaluatorSet& evaluatorSet) {
    countTotal_ = 0;
    countT_ = 0;
    countF_ = 0;

    evaluatorT_ = [&] {
        for(LoopCount i=0; i<loopCount_; ++i) {
            ++countT_;
            ++countTotal_;
        }
        return true;
    };

    evaluatorF_ = [&] {
        for(LoopCount i=0; i<loopCount_; ++i) {
            ++countF_;
            ++countTotal_;
        }
        return false;
    };

    pRunner_ = Sudoku::CreateParallelRunner();
    for(decltype(numberOfCores) i = 0; i < numberOfCores; ++i) {
        evaluatorSet.push_back(evaluatorT_);
        evaluatorSet.push_back(evaluatorF_);
        pRunner_->Add(evaluatorT_);
        pRunner_->Add(evaluatorF_);
    }

    return loopCount_;
}

void SudokuParallelRunnerTest::runFutureSet(EvaluatorSet& evaluatorSet) {
    std::vector<SUDOKU_TEST_THREAD_FUTURE<Sudoku::BaseParallelRunner::ResultType>> futureSet;
    for(auto& evaluator : evaluatorSet) {
        futureSet.push_back(SUDOKU_TEST_THREAD_ASYNC(SUDOKU_TEST_THREAD_LAUNCH_ASYNC, evaluator));
    }

    pRunner_->runParallelImpl(futureSet);
    return;
}

/*
Local Variables:
mode: c++
coding: utf-8-unix
tab-width: nil
c-file-style: "stroustrup"
End:
*/
