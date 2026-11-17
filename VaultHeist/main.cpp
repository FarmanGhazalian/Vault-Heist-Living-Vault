#include "GameEngine.h"

int main()
{
	GameEngine engine;
	if (!engine.init())
	{
		return -1;
	}
	engine.run();
	return 0;
}
