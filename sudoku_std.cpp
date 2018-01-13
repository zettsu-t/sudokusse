// solver with SSE 4.2 / AVX
// Copyright (C) 2012-2016 Zettsu Tatsuya
// C++11 standard libraries dependent implementation

#include "sudoku_os_dependent.h"

#ifndef SOLVE_PARALLEL_WITH_BOOST_THREAD
#ifndef NO_PARALLEL
#include <future>

namespace Sudoku {
    class CppParallelRunner : public BaseParallelRunner {
    public:
        CppParallelRunner(void) = default;
        virtual ~CppParallelRunner(void) = default;
    protected:
        virtual NumberOfCores getHardwareConcurrency(void) override;
        virtual ResultType runParallel(EvaluatorSet& evaluatorSet) override;
    };

    CppParallelRunner::NumberOfCores CppParallelRunner::getHardwareConcurrency(void) {
        return std::thread::hardware_concurrency();
    }

    CppParallelRunner::ResultType CppParallelRunner::runParallel(EvaluatorSet& evaluatorSet) {
        std::vector<std::future<ResultType>> futureSet;
        for(auto& evaluator : evaluatorSet) {
            futureSet.push_back(std::async(std::launch::async, evaluator));
        }
        return runParallelImpl(futureSet);
    }

    std::unique_ptr<BaseParallelRunner> CreateParallelRunner(void) {
        std::unique_ptr<BaseParallelRunner> pObj(new CppParallelRunner);
        return pObj;
    }
}
#else // NO_PARALLEL
namespace Sudoku {
    class NullParallelRunner : public BaseParallelRunner {
    public:
        NullParallelRunner(void) = default;
        virtual ~NullParallelRunner(void) = default;
    protected:
        virtual NumberOfCores getHardwareConcurrency(void) override { return 0; }
        virtual ResultType runParallel(EvaluatorSet& evaluatorSet) override { return true; }
    };

    std::unique_ptr<BaseParallelRunner> CreateParallelRunner(void) {
        std::unique_ptr<BaseParallelRunner> pObj(new NullParallelRunner);
        return pObj;
    }
}
#endif // NO_PARALLEL
#endif // SOLVE_PARALLEL_WITH_BOOST_THREAD

/*
Local Variables:
mode: c++
coding: utf-8-unix
tab-width: nil
c-file-style: "stroustrup"
End:
*/
