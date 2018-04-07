// solver with SSE 4.2 / AVX
// Copyright (C) 2012-2018 Zettsu Tatsuya
// Workaround for missing getpagesize()

#ifndef SUDOKU_BOOST_FUTURE_H_INCLUDED
#define SUDOKU_BOOST_FUTURE_H_INCLUDED

#ifdef SOLVE_PARALLEL_WITH_BOOST_THREAD

#ifdef __CYGWIN__
#define getpagesize() sysconf(_SC_PAGESIZE)
#endif

#include <boost/thread/future.hpp>

#endif // SOLVE_PARALLEL_WITH_BOOST_THREAD
#endif // SUDOKU_BOOST_FUTURE_H_INCLUDED

/*
Local Variables:
mode: c++
coding: utf-8-unix
tab-width: nil
c-file-style: "stroustrup"
End:
*/
