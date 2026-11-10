#pragma once
#include"ILevelLogic.h"


class Act1Level1 : public ILevelLogic
{
private:
	int currentNodeId;
	int targetNodeId;
public:

	Act1Level1(int currentNode, int targetNode, int optimalstep) : ILevelLogic(optimalstep), currentNodeId(currentNode), targetNodeId(targetNode){}

	void onValidStep()
	{
		state.addStep();
		if()
	}
};