#include "MaxHeap.h"

MaxHeap::MaxHeap()
{
	data = NULL;
	count = 0;
	heapSize = 0;
}

MaxHeap::MaxHeap(int* values, int n)
{
	count = n;
	heapSize = n;
	if (n > 0 && values != NULL)
	{
		data = new int[n];
		for (int i = 0; i < n; i++)
		{
			data[i] = values[i];
		}
	}
	else
	{
		data = NULL;
	}
}

MaxHeap::MaxHeap(const MaxHeap& other)
{
	count = other.count;
	heapSize = other.heapSize;
	if (count > 0 && other.data != NULL)
	{
		data = new int[count];
		for (int i = 0; i < count; i++)
		{
			data[i] = other.data[i];
		}
	}
	else
	{
		data = NULL;
	}
}

MaxHeap& MaxHeap::operator=(const MaxHeap& other)
{
	if (this != &other)
	{
		if (data != NULL)
		{
			delete[] data;
			data = NULL;
		}

		count = other.count;
		heapSize = other.heapSize;
		if (count > 0 && other.data != NULL)
		{
			data = new int[count];
			for (int i = 0; i < count; i++)
			{
				data[i] = other.data[i];
			}
		}
		else
		{
			data = NULL;
		}
	}
	return *this;
}

MaxHeap::~MaxHeap()
{
	if (data != NULL)
	{
		delete[] data;
		data = NULL;
	}
}

int MaxHeap::getAt(int i) const
{
	if (i >= 0 && i < count && data != NULL)
	{
		return data[i];
	}
	return -1;
}

int MaxHeap::getCount() const
{
	return count;
}

int MaxHeap::getHeapSize() const
{
	return heapSize;
}

void MaxHeap::swapValues(int i, int j)
{
	if (data == NULL || i < 0 || i >= count || j < 0 || j >= count)
	{
		return;
	}
	int temp = data[i];
	data[i] = data[j];
	data[j] = temp;
}

void MaxHeap::siftDown(int i, int size)
{
	if (data == NULL)
	{
		return;
	}

	while (true)
	{
		int largest = i;
		int left = 2 * i + 1;
		int right = 2 * i + 2;

		if (left < size && data[left] > data[largest])
		{
			largest = left;
		}
		if (right < size && data[right] > data[largest])
		{
			largest = right;
		}

		if (largest != i)
		{
			swapValues(i, largest);
			i = largest;
		}
		else
		{
			break;
		}
	}
}

void MaxHeap::siftUp(int i)
{
	if (data == NULL)
	{
		return;
	}

	while (i > 0)
	{
		int parent = (i - 1) / 2;
		if (data[i] > data[parent])
		{
			swapValues(i, parent);
			i = parent;
		}
		else
		{
			break;
		}
	}
}

void MaxHeap::buildMaxHeap()
{
	if (data == NULL || count <= 1)
	{
		return;
	}
	for (int i = count / 2 - 1; i >= 0; i--)
	{
		siftDown(i, count);
	}
	heapSize = count;
}

int MaxHeap::extractMax()
{
	if (data == NULL || heapSize <= 0)
	{
		return -1;
	}

	int maxVal = data[0];
	swapValues(0, heapSize - 1);
	heapSize--;
	siftDown(0, heapSize);

	return maxVal;
}

void MaxHeap::heapSort()
{
	if (data == NULL || count <= 1)
	{
		return;
	}

	buildMaxHeap();

	for (int i = count - 1; i > 0; i--)
	{
		swapValues(0, i);
		siftDown(0, i);
	}
	heapSize = 0;
}

bool MaxHeap::isMaxHeap() const
{
	if (data == NULL || heapSize <= 1)
	{
		return true;
	}

	for (int i = 0; i < heapSize / 2; i++)
	{
		int left = 2 * i + 1;
		int right = 2 * i + 2;

		if (left < heapSize && data[left] > data[i])
		{
			return false;
		}
		if (right < heapSize && data[right] > data[i])
		{
			return false;
		}
	}
	return true;
}

bool MaxHeap::isValidSwap(int i, int j) const
{
	if (data == NULL || i < 0 || i >= heapSize || j < 0 || j >= heapSize || i == j)
	{
		return false;
	}

	int p = -1;
	int c = -1;

	if ((i - 1) / 2 == j)
	{
		p = j;
		c = i;
	}
	else if ((j - 1) / 2 == i)
	{
		p = i;
		c = j;
	}
	else
	{
		return false;
	}

	if (data[c] <= data[p])
	{
		return false;
	}

	// Check sibling condition: child c must be >= sibling
	int left = 2 * p + 1;
	int right = 2 * p + 2;
	int sibling = (c == left) ? right : left;

	if (sibling < heapSize && data[sibling] > data[c])
	{
		return false;
	}

	return true;
}

bool MaxHeap::isExtractSwap(int i, int j) const
{
	if (heapSize < 2)
	{
		return false;
	}
	return (i == 0 && j == heapSize - 1) || (i == heapSize - 1 && j == 0);
}

void MaxHeap::extractSwap()
{
	if (heapSize >= 2)
	{
		swapValues(0, heapSize - 1);
		heapSize--;
	}
}

bool MaxHeap::isSorted() const
{
	if (data == NULL || count <= 1)
	{
		return true;
	}
	for (int i = 0; i < count - 1; i++)
	{
		if (data[i] > data[i + 1])
		{
			return false;
		}
	}
	return true;
}
