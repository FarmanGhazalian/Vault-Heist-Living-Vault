#pragma once
#include"ScoreSystem.h"

class ILevelLogic
{
public:
	LevelState state;

	ILevelLogic(int optimalSteps) : state(optimalSteps) {}
	virtual ~ILevelLogic() = default;


	virtual void onValidStep() = 0;
	virtual void onMistake(int extraAlarm = 1, int penaltyPoints = 0) = 0;
	virtual void onHintRequested() = 0;

	const LevelState& getState() const 
	{
		return state; 
	}
	bool isFinished() const 
	{
		return state.finished;
	}
	bool isWon() const 
	{
		return state.won;
	}
	int getRemainingAlarm() const 
	{
		return state.remaining();
	}


	int getCalculatedScore() const
	{
		return ScoreSystem::score(state);
	}

	int getCalculatedStars() const
	{
		return ScoreSystem::stars(state);
	}

};