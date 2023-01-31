// Copyright (c) 2022, Stefano Cristiano
//
// All Rights Reserved. Reproduction is not allowed.
#pragma once
#include "Console.h" // SC_BREAK_DEBUGGER
#include "StringView.h"

namespace SC
{
struct TestCase;

struct TestReport
{
    bool       abortOnFirstFailedTest = true;
    bool       debugBreakOnFailedTest = true;
    uint32_t   numTestsSucceeded      = 0;
    uint32_t   numTestsFailed         = 0;
    StringView currentSection;
    StringView firstFailedTest;
    StringView testToRun;
    StringView sectionToRun;

    TestReport(int argc, const char** argv);
    ~TestReport();

    void testCaseFinished(TestCase& testCase);
    void printSectionResult(TestCase& testCase);

    [[nodiscard]] bool isTestEnabled(StringView testName) const;
    [[nodiscard]] bool isSectionEnabled(StringView sectionName) const;
    [[nodiscard]] int  getTestReturnCode() const;
};

struct TestCase
{
    TestCase(TestReport& report, StringView testName);
    ~TestCase();
    bool recordExpectation(StringView expression, bool status, StringView detailedError = StringView());
    bool recordExpectation(StringView expression, ReturnCode status);

    [[nodiscard]] bool test_section(StringView sectionName);

    const StringView testName;
    uint32_t         numTestsSucceeded;
    uint32_t         numSectionTestsFailed;
    uint32_t         numTestsFailed;
    TestReport&      report;
    bool             printedSection;
};
} // namespace SC

#define SC_TEST_EXPECT(e)                                                                                              \
    recordExpectation(StringView(#e##_a8), (e))                                                                        \
        ? (void)0                                                                                                      \
        : (TestCase::report.debugBreakOnFailedTest ? SC_BREAK_DEBUGGER : (void)0);
