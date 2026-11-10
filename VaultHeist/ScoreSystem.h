#pragma once
#include<cmath>
using namespace std;
struct LevelState
{
	int capacity;
	int optimaSteps;
	int steps;
	int alarm;

	int penaltyCount;
	int penaltyPoints;
	int bonus;

	bool hintUsed;
	bool finished;
	bool won;

	LevelState(int optimal)
	{
		optimaSteps = optimal;
		capacity = ceil(1.5 * optimal) + 2;
		steps = 0;
		alarm =0;

		penaltyCount =0;
		penaltyPoints =0;
		bonus =0;

		hintUsed = false;
		finished = false;
		won = false;
	}

	void addStep();
	void addpenalty();
	void useHint();
	bool isFull();
	int remianing();
};

class ScoreSystem
{
private:
	int capacity;
public:
	ScoreSystem(int cap)
	{
		capacity = cap;
	}

	s
};