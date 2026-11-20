#include "backend/Act4Level3.h"
#include "backend/LevelGenerator.h"

bool Level4_3DataLoader::loadJSON(const string& filePath)
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
		data.levelId = 43;
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

Act4Level3::Act4Level3() : ILevelLogic(0)
{
	sortedRef = NULL;
	phase = 0;
}

Act4Level3::~Act4Level3()
{
	if (sortedRef != NULL)
	{
		delete[] sortedRef;
		sortedRef = NULL;
	}
}

bool Act4Level3::isFullySorted() const
{
	if (sortedRef == NULL)
	{
		return false;
	}
	for (int i = 0; i < loader.data.totalValues; i++)
	{
		if (heap.getAt(i) != sortedRef[i])
		{
			return false;
		}
	}
	return true;
}

bool Act4Level3::initLevel(const string& jsonPath)
{
	if (!loader.loadJSON(jsonPath))
	{
		return false;
	}

	heap = MaxHeap(loader.data.values, loader.data.totalValues);

	if (sortedRef != NULL)
	{
		delete[] sortedRef;
		sortedRef = NULL;
	}

	// Compute sorted reference via a copy
	MaxHeap copyHeap(loader.data.values, loader.data.totalValues);
	copyHeap.heapSort();

	sortedRef = new int[loader.data.totalValues];
	for (int i = 0; i < loader.data.totalValues; i++)
	{
		sortedRef[i] = copyHeap.getAt(i);
	}

	phase = 0; // start in EXTRACT phase
	int dynSteps = heap.countSortSwaps();
	if (dynSteps <= 0)
	{
		dynSteps = loader.data.optimalSteps;
	}
	state = LevelState(dynSteps);

	return true;
}

void Act4Level3::onValidStep()
{
	state.addStep();
}

void Act4Level3::onMistake(int extraAlarm, int penaltyPoints)
{
	state.addpenalty(extraAlarm, penaltyPoints);
}

void Act4Level3::onHintRequested()
{
	state.useHint();
}

ActionResult Act4Level3::processAction(const PlayerAction& action)
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

		if (phase == 0)
		{
			result.message = "Hint: Extract phase! Swap root (index 0) with last element of active heap (index " +
				to_string(heap.getHeapSize() - 1) + ").";
		}
		else
		{
			result.message = "Hint: Sift phase! Swap parent with larger child to restore max heap property.";
		}
		return result;
	}

	if (action.type == SWAP_ELEMENTS)
	{
		result.accepted = true;

		if (phase == 0) // EXTRACT phase
		{
			if (heap.isExtractSwap(action.nodeA, action.nodeB))
			{
				result.correct       = true;
				result.alarmDelta = 2;
				result.penaltyPoints = 0;

				heap.extractSwap();
				onValidStep();

				if (heap.getHeapSize() <= 1)
				{
					if (isFullySorted())
					{
						state.finished = true;
						state.won      = true;
						result.message = "All treasures extracted and sorted! Level Cleared.";
					}
					else
					{
						result.message = "Final extraction complete!";
					}
				}
				else
				{
					if (heap.isMaxHeap())
					{
						phase = 0; // already a max heap, stay in extract
						result.message = "Root extracted! Heap property holds, ready for next extract.";
					}
					else
					{
						phase = 1; // need to sift down
						result.message = "Root extracted! Now sift down to restore max heap.";
					}
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
					result.message = "Invalid extract swap! Must swap root (0) with last active heap element.";
				}
			}
			return result;
		}
		else // SIFT phase
		{
			if (heap.isValidSwap(action.nodeA, action.nodeB))
			{
				result.correct       = true;
				result.alarmDelta = 2;
				result.penaltyPoints = 0;

				heap.swapValues(action.nodeA, action.nodeB);
				onValidStep();

				if (heap.isMaxHeap())
				{
					phase = 0; // property restored, ready for extract
					result.message = "Max heap restored! Ready for next extraction.";
				}
				else
				{
					result.message = "Valid sift swap! Continue restoring heap.";
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
					result.message = "Invalid sift swap! Must swap parent with strictly larger child.";
				}
			}
			return result;
		}
	}

	result.accepted = false;
	result.message  = "Unsupported action type!";
	return result;
}

const MaxHeap& Act4Level3::getHeap() const
{
	return heap;
}

int Act4Level3::getPhase() const
{
	return phase;
}

int Act4Level3::getHeapSize() const
{
	return heap.getHeapSize();
}