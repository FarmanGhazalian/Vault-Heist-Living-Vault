#pragma once
#include <string>

class Benchmark
{
public:
    static bool runAll(const std::string& outputPath = "benchmark_results.csv");
};
