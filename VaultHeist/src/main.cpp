#include "gui/GameEngine.h"
#include "backend/TestSuite.h"
#include "backend/Benchmark.h"
#include <string>

int main(int argc, char* argv[])
{
	for (int i = 1; i < argc; i++)
	{
		std::string arg = argv[i];
		if (arg == "--test")
		{
			int p = 0, f = 0;
			bool ok = TestSuite::runAll("test_report.txt", &p, &f);
			return ok ? 0 : 1;
		}
		else if (arg == "--benchmark")
		{
			bool ok = Benchmark::runAll("benchmark_results.csv");
			return ok ? 0 : 1;
		}
	}

	GameEngine engine;
	if (!engine.init())
	{
		return -1;
	}
	engine.run();
	return 0;
}
