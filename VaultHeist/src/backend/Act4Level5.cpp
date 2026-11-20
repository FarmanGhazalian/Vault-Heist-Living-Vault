#include "backend/Act4Level5.h"

Act4Level5::Act4Level5() : Act4Level3()
{
	building = true;
}

bool Act4Level5::initLevel(const string& jsonPath)
{
	bool ok = Act4Level3::initLevel(jsonPath);
	if (!ok)
	{
		return false;
	}
	building = true;
	int dynSteps = heap.countBuildSwaps() + heap.countSortSwaps();
	if (dynSteps <= 0)
	{
		dynSteps = loader.data.optimalSteps;
	}
	state = LevelState(dynSteps);
	return true;
}

ActionResult Act4Level5::processAction(const PlayerAction& action)
{
	if (building)
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
				result.message = "Hint: Build phase! Swap parent at index " + to_string(pHint) +
					" with larger child at index " + to_string(cHint) + ".";
			}
			else
			{
				result.message = "Hint: Heap built! Ready for extraction.";
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
					building = false; // switch to extract/sort phases!
					result.message = "Max Heap built! Now begin extracting treasures.";
				}
				else
				{
					result.message = "Valid build swap! Continue building heap.";
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
	else
	{
		// Sift / Extract phases handled completely by Act4Level3!
		return Act4Level3::processAction(action);
	}
}

bool Act4Level5::isBuilding() const
{
	return building;
}