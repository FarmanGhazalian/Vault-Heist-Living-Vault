#include <iostream>
#include <cmath>
#include "ScoreSystem.h"
using namespace std;

LevelState::LevelState(int optimal)
{
	optimalSteps = optimal;
	alarmCap = ScoreSystem::computeCapacity(optimalSteps);
	steps = 0;
	alarm = 0;

	penaltyCount = 0;
	penaltyPoints = 0;
	bonus = 0;

	hintUsed = false;
	finished = false;
	won = false;
}

bool LevelState::checkGameOver()
{
	if (alarm >= alarmCap)
	{
		finished = true;
		won = false;
		return true;
	}
	return false;
}

void LevelState::addStep()
{
	steps++;
	alarm++;
	checkGameOver();
}

void LevelState::addpenalty(int extraAlarm, int points)
{
	penaltyCount++;
	steps++;
	penaltyPoints += points;
	alarm += extraAlarm;
	checkGameOver();
}

void LevelState::useHint()
{
    if (!hintUsed) {
        // First hint: count as a normal step
        steps++;
        alarm++;
        hintUsed = true;
    } else {
        // Second (or later) hint: penalty of +2 alarm only
        alarm += 2;
    }
    checkGameOver();
}

bool LevelState::isFull() const
{
	return alarm >= alarmCap;
}

int LevelState::remaining() const
{
	return alarmCap - alarm;
}

void LevelState::reduceCapacity(int amount)
{
	alarmCap -= amount;
	if (alarmCap < alarm)
	{
		alarmCap = alarm;
	}
}

int ScoreSystem::computeCapacity(int optimalSteps)
{
	return static_cast<int>(std::ceil(1.5 * optimalSteps)) + 2;
}

int ScoreSystem::stars(const LevelState& state)
{
	if (!state.won || state.alarm >= state.alarmCap)
	{
		return 0;
	}
	int baseStar = 1;
	if (state.penaltyCount == 0 && state.steps <= state.optimalSteps)
	{
		baseStar = 3;
	}
	else if (state.steps <= 1.25 * state.optimalSteps && state.penaltyCount <= 1)
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
	if (!state.won || state.alarm >= state.alarmCap)
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
