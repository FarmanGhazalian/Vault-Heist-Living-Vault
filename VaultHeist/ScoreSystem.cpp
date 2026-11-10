#include<iostream>
#include"ScoreSystem.h"
using namespace std;


void LevelState::addStep()
{
	steps++;
	alarm++;
	return;
}

void LevelState::addpenalty(int extraAlarm = 1, int points = 0)
{
	penaltyCount++;
	penaltyPoints += points;
	alarm += extraAlarm;
	return;
}

void LevelState::useHint()
{
	hintUsed = true;
}

bool LevelState::isFull()
{
	if (alarm >= capacity)
	{
		return true;
	}
	return false;
}

int LevelState::remaining() const
{
	return capacity - alarm;
}

int  ScoreSystem::computeCapacity(int optimalSteps)
{
	return ceil(1.5 * optimalSteps) + 2;
}

int ScoreSystem::stars(const LevelState& state)
{
	if (!state.won || state.alarm >= state.capacity)
	{
		return 0;
	}
	int baseStar = 1;
	if (state.penaltyCount == 0 && state.steps <= state.optimalSteps)
	{
		baseStar = 3;
	}
	if (state.steps <= 1.5 * state.optimalSteps && state.penaltyCount <= 1)
	{
		baseStar = 2;
	}
	if (state.hintUsed && baseStar > 2)
	{
		baseStar = 2;
	}
	return baseStar;
}

int ScoreSystem::score(const LevelState& state)
{
	if (!state.won && state.alarm >= state.penaltyCount)
	{
		return 0;
	}
	int baseScore = 1000 - (10 * state.alarm) - state.penaltyPoints + state.bonus;
	if (baseScore < 0)
	{
		return 0;
	}
	return baseScore;
}