#pragma once
#include <cmath>

struct LevelState;

class ScoreSystem {
public:
	static int computeCapacity(int optimalSteps);
	static int stars(const LevelState& state);
	static int score(const LevelState& state);
};

struct LevelState
{
	int alarmCap;
	int optimalSteps;
	int steps;
	int alarm;

	int penaltyCount;
	int penaltyPoints;
	int bonus;

	bool hintUsed;
	bool finished;
	bool won;

	LevelState(int optimal = 0);

	void addStep();
	void addpenalty(int extraAlarm = 1, int points = 0);
	void useHint();
	bool isFull() const;
	bool checkGameOver();
	int remaining() const;
	void reduceCapacity(int amount);
};
