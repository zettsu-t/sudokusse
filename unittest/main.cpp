// Launching CppTest
// Copyright (C) 2012-2017 Zettsu Tatsuya
//
// I use CppUnit code on the website.
// http://www.atmarkit.co.jp/fdotnet/cpptest/cpptest02/cpptest02_03.html

#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>

int main(int argc, char* argv[] ) {
    // Create an event manager and a test controller
    CPPUNIT_NS::TestResult controller;

    // Attach a listener that collects results of tests to the controller
    CPPUNIT_NS::TestResultCollector result;
    controller.addListener(&result);

    // Attach a progress listener that prints '.'s to the controller
    CPPUNIT_NS::BriefTestProgressListener progress;
    controller.addListener(&progress);

    // Set tests to a test runner and run the tests
    CPPUNIT_NS::TestRunner runner;
    runner.addTest(CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest());
    runner.run(controller);

    // Write results of the tests to stdout
    CPPUNIT_NS::CompilerOutputter outputter(&result, CPPUNIT_NS::stdCOut());
    outputter.write();

    return result.wasSuccessful() ? 0 : 1;
}

/*
Local Variables:
mode: c++
coding: utf-8-unix
tab-width: nil
c-file-style: "stroustrup"
End:
*/
