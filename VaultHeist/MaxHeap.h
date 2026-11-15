#pragma once
#include <iostream>

class MaxHeap
{
private:
	int* data;
	int count;
	int heapSize;

public:
	MaxHeap();
	MaxHeap(int* values, int n);
	MaxHeap(const MaxHeap& other);
	MaxHeap& operator=(const MaxHeap& other);
	~MaxHeap();

	int getAt(int i) const;
	int getCount() const;
	int getHeapSize() const;

	void swapValues(int i, int j);
	void siftDown(int i, int size);
	void siftUp(int i);
	void buildMaxHeap();
	int extractMax();
	void heapSort();
	bool isMaxHeap() const;

	bool isValidSwap(int i, int j) const;
	bool isExtractSwap(int i, int j) const;
	void extractSwap();
	bool isSorted() const;
};
