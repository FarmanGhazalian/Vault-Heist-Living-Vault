#include<iostream>
#include"Act1Level4.h"

void L4_data::generate()
{
	preArr = new int[totalNodes];
	inArr = new int[totalNodes];
}

bool Level4DataLoader::loadJSON(const string& filePath)
{
	ifstream file(filePath);
	if (!file.is_open())
	{
		return false;
	}

	json j;
	file >> j;

	data.levelId = j["levelId"];
	data.levelName = j["levelName"];
	data.totalNodes = j["totalNodes"];
	data.optimalSteps = j["optimalSteps"];
	data.generate();

	for (int i = 0; i < data.totalNodes; i++)
	{
		data.inArr[i] = j["inorderSequence"][i].get<int>();
	}

	for (int i = 0; i < data.totalNodes; i++)
	{
		data.preArr[i] = j["preorderSequence"][i].get<int>();
	}
	data.roomA = j["roomA"];
	data.roomB = j["roomB"];
	file.close();
	return true;
}


string Act1Level4::getHint()
{
	return "Hint: First click the Lowest Common Ancestor (where paths from Room #" + to_string(roomA) + " and Room #" + std::to_string(roomB) + " meet), then click the path nodes step-by-step from Room #" + to_string(roomA) + " to Room #" + std::to_string(roomB) + "!";
}

void Act1Level4::onValidStep()
{
	state.addStep();
}

void Act1Level4::onMistake(int extraAlarm, int penaltyPoints)
{
	state.addpenalty(extraAlarm, penaltyPoints);
}

void Act1Level4::onHintRequested()
{
	state.useHint();
}

bool Act1Level4::initLevel(const std::string& jsonPath)
{
	if (!loader.loadJSON(jsonPath)) return false;

	roomA = loader.data.roomA;
	roomB = loader.data.roomB;
	actualLCA = -1;
	pathCount = 0;
	currentPathIndex = 0;
	expectedPathLength = 0;
	lcaFound = false;

	solTree.buildFromPreIn(loader.data.preArr, loader.data.inArr, loader.data.totalNodes);
	state = LevelState(loader.data.optimalSteps);

	TreeNode* lcaNode = solTree.lca(roomA, roomB);
	if (lcaNode != nullptr)
	{
		actualLCA = lcaNode->val;
	}
	if (expectedPath != nullptr)
	{
		delete[] expectedPath;
	}
	expectedPath = solTree.getPath(lcaNode, roomA, roomB, expectedPathLength);
	pathCount = expectedPathLength;

	return true;
}

ActionResult Act1Level4::processAction(const PlayerAction& action)
{
	ActionResult res;
	if (state.finished)
	{
		return res;
	}

	if (action.type == CLICK_HINT)
	{
		if (state.hintUsed)
		{
			res.correct = false;
			res.accepted = false;
			res.message = "Hint already Used!";
			onMistake(2, 50);
			res.penaltyPoints = 50;
			res.alarmDelta = 2;
		}
		else
		{
			res.correct = true;
			res.accepted = false;
			res.message = getHint();
			onHintRequested();
		}

		if (!state.won && state.alarm >= state.alarmCap) {
			state.finished = true;
			state.won = false;
			res.message = "Alarm Triggered! Vault Lockdown!";
		}
		return res;
	}

	if (!lcaFound  && action.type == CLICK_LCA)
	{
		if (action.nodeA == actualLCA) {
			lcaFound = true;
			currentPathIndex = 0;
			onValidStep();
			res.correct = true;
			res.accepted = true;
			res.message = "Meeting Room(LCA #" + to_string(actualLCA) + ") verified!Now trace the path from Room #" + to_string(roomA) + " to Room #" + to_string(roomB) + ".";
		}
		else {
			res.message = "Wrong Meeting Room! Find where the two escape routes intersect!";
		}
	}
	else if(action.type == TRACE_PATH)
	{
		if (action.nodeA == expectedPath[currentPathIndex])
		{
			currentPathIndex++;

			if (currentPathIndex == pathCount) {
				res.message = "Escape Path Verified! Path Length: " + to_string(expectedPathLength) + ". Sector Cleared!";
				state.finished = true;
				state.won = true;
			}
			else
			{
				res.message = "Path Step " + to_string(currentPathIndex) + "/" + to_string(pathCount) + " verified!";
			}
		}
		else
		{
			res.message = "Wrong Room on Path! Follow the shortest path between Room #" + to_string(roomA) + " and Room #" + to_string(roomB) + "!";
		}
	}
	if (!isWon() && state.alarm >= state.alarmCap) {
		state.finished = true;
		state.won = false;
		res.message = "Alarm Reached Maximum Capacity! Vault Lockdown!";
	}

	return res;
}
