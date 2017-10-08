// solver with SSE 4.2 / AVX
// Copyright (C) 2012-2016 Zettsu Tatsuya
// Boost C++ Libraries dependent implementation

#ifdef SOLVE_PARALLEL_WITH_BOOST_THREAD

#include <boost/thread/future.hpp>
#include "sudoku_os_dependent.h"

namespace Sudoku {
    class BoostParallelRunner : public BaseParallelRunner {
    public:
        BoostParallelRunner(void) = default;
        virtual ~BoostParallelRunner(void) = default;
    protected:
        virtual NumberOfCores getHardwareConcurrency(void) override;
        virtual ResultType runParallel(EvaluatorSet& evaluatorSet) override;
    };

    BoostParallelRunner::NumberOfCores BoostParallelRunner::getHardwareConcurrency(void) {
        return boost::thread::hardware_concurrency();
    }

    BoostParallelRunner::ResultType BoostParallelRunner::runParallel(EvaluatorSet& evaluatorSet) {
        std::vector<boost::unique_future<ResultType>> futureSet;
        for(auto& evaluator : evaluatorSet) {
            futureSet.push_back(boost::async(boost::launch::async, evaluator));
        }
        return runParallelImpl(futureSet);
    }

    std::unique_ptr<BaseParallelRunner> CreateParallelRunner(void) {
        std::unique_ptr<BaseParallelRunner> pObj(new BoostParallelRunner);
        return pObj;
    }
}

#endif // SOLVE_PARALLEL_WITH_BOOST_THREAD

/*
Local Variables:
mode: c++
coding: utf-8-unix
tab-width: nil
c-file-style: "stroustrup"
End:
*/
