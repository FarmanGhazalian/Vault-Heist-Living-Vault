#include "backend/Act4Level1.h"
#include "backend/LevelGenerator.h"

bool Level4_1DataLoader::loadJSON(const string& filePath)
{
	ifstream file(filePath);
	if (!file.is_open())
	{
		string fallback = "VaultHeist/" + filePath;
		file.open(fallback);
		if (!file.is_open())
		{
			return false;
		}
	}

	json j;
	file >> j;
	file.close();
	LevelGenerator::processJson(j);

	if (j["levelId"].is_string())
	{
		data.levelId = 41;
	}
	else
	{
		data.levelId = j["levelId"];
	}

	data.levelName    = j["levelName"];
	data.optimalSteps = j["optimalSteps"];

	const json& valsArr = j["values"];
	data.totalValues = (int)valsArr.size();

	if (data.values != NULL)
	{
		delete[] data.values;
		data.values = NULL;
	}

	data.values = new int[data.totalValues];
	for (int i = 0; i < data.totalValues; i++)
	{
		data.values[i] = valsArr[i];
	}

	return true;
}

Act4Level1::Act4Level1() : ILevelLogic(0)
{
}

bool Act4Level1::initLevel(const string& jsonPath)
{
	if (!loader.loadJSON(jsonPath))
	{
		return false;
	}

	heap = MaxHeap(loader.data.values, loader.data.totalValues);
	int dynSteps = heap.countBuildSwaps();
	if (dynSteps <= 0)
	{
		dynSteps = loader.data.optimalSteps;
	}
	state = LevelState(dynSteps);

	return true;
}

void Act4Level1::onValidStep()
{
	state.addStep();
}

void Act4Level1::onMistake(int extraAlarm, int penaltyPoints)
{
	state.addpenalty(extraAlarm, penaltyPoints);
}

void Act4Level1::onHintRequested()
{
	state.useHint();
}

ActionResult Act4Level1::processAction(const PlayerAction& action)
{
	ActionResult result;

	if (isFinished() || state.isFull())
	{
		result.accepted = false;
		result.correct  = false;
		result.message  = "Level locked or already finished!";
		return result;
	}

	if (action.type == CLICK_HINT)
	{
		result.accepted = true;

		if (state.hintUsed)
		{
			result.correct       = false;
			result.alarmDelta = 2;
			result.penaltyPoints = 0;
			onMistake(2, 0);

			if (state.isFull())
			{
				state.finished = true;
				state.won      = false;
				result.message = "Hint already used. ALARM CAP REACHED!";
			}
			else
			{
				result.message = "Hint already used.";
			}
			return result;
		}

		result.correct = true;
		onHintRequested();

		// Find first valid swap from bottom up
		int pHint = -1;
		int cHint = -1;
		int sz = heap.getHeapSize();
		for (int p = sz / 2 - 1; p >= 0; p--)
		{
			int left = 2 * p + 1;
			int right = 2 * p + 2;
			int best = p;
			if (left < sz && heap.getAt(left) > heap.getAt(best))
			{
				best = left;
			}
			if (right < sz && heap.getAt(right) > heap.getAt(best))
			{
				best = right;
			}
			if (best != p)
			{
				pHint = p;
				cHint = best;
				break;
			}
		}

		if (pHint != -1)
		{
			result.message = "Hint: Swap parent at index " + to_string(pHint) +
				" (" + to_string(heap.getAt(pHint)) + ") with larger child at index " +
				to_string(cHint) + " (" + to_string(heap.getAt(cHint)) + ").";
		}
		else
		{
			result.message = "Hint: Heap is already balanced!";
		}
		return result;
	}

	if (action.type == SWAP_ELEMENTS)
	{
		result.accepted = true;

		if (heap.isValidSwap(action.nodeA, action.nodeB))
		{
			result.correct       = true;
			result.alarmDelta = 2;
			result.penaltyPoints = 0;

			heap.swapValues(action.nodeA, action.nodeB);
			onValidStep();

			if (heap.isMaxHeap())
			{
				state.finished = true;
				state.won      = true;
				result.message = "Max Heap property established! Vault Activated.";
			}
			else
			{
				result.message = "Valid swap! Continue heapifying.";
			}
		}
		else
		{
			result.correct       = false;
			result.alarmDelta = 2;
			result.penaltyPoints = 0;
			onMistake(1, 0);

			if (state.isFull())
			{
				state.finished = true;
				state.won      = false;
				result.message = "ALARM CAP REACHED! Vault Lockdown Initiated.";
			}
			else
			{
				result.message = "Invalid swap! Must swap parent with strictly larger child.";
			}
		}
		return result;
	}

	result.accepted = false;
	result.message  = "Unsupported action type!";
	return result;
}

const MaxHeap& Act4Level1::getHeap() const
{
	return heap;
}