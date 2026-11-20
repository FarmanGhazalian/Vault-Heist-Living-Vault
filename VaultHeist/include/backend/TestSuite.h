#pragma once
#include <string>

class TestSuite
{
public:
    static bool runAll(const std::string& reportPath = "test_report.txt", int* outPassed = nullptr, int* outFailed = nullptr);
};
