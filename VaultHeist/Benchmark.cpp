#include "Benchmark.h"
#include "BinaryTree.h"
#include "BST.h"
#include "AVL.h"
#include "MaxHeap.h"
#include "Validator.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <chrono>
#include <string>
#include <algorithm>

namespace {

    // Helper functions for Benchmark internal sorting algorithms
    void insertionSortBench(std::vector<int>& arr, long long& comparisons, long long& swaps)
    {
        comparisons = 0;
        swaps = 0;
        int n = static_cast<int>(arr.size());
        for (int i = 1; i < n; i++)
        {
            int key = arr[i];
            int j = i - 1;
            while (j >= 0)
            {
                comparisons++;
                if (arr[j] > key)
                {
                    arr[j + 1] = arr[j];
                    swaps++;
                    j--;
                }
                else
                {
                    break;
                }
            }
            arr[j + 1] = key;
        }
    }

    void mergeBench(std::vector<int>& arr, int l, int m, int r, long long& comparisons, long long& swaps)
    {
        int n1 = m - l + 1;
        int n2 = r - m;
        std::vector<int> L(n1), R(n2);
        for (int i = 0; i < n1; i++) L[i] = arr[l + i];
        for (int j = 0; j < n2; j++) R[j] = arr[m + 1 + j];

        int i = 0, j = 0, k = l;
        while (i < n1 && j < n2)
        {
            comparisons++;
            if (L[i] <= R[j])
            {
                arr[k++] = L[i++];
            }
            else
            {
                arr[k++] = R[j++];
                swaps++;
            }
        }
        while (i < n1) arr[k++] = L[i++];
        while (j < n2) arr[k++] = R[j++];
    }

    void mergeSortBench(std::vector<int>& arr, int l, int r, long long& comparisons, long long& swaps)
    {
        if (l < r)
        {
            int m = l + (r - l) / 2;
            mergeSortBench(arr, l, m, comparisons, swaps);
            mergeSortBench(arr, m + 1, r, comparisons, swaps);
            mergeBench(arr, l, m, r, comparisons, swaps);
        }
    }

    int partitionBench(std::vector<int>& arr, int low, int high, long long& comparisons, long long& swaps)
    {
        int pivot = arr[high];
        int i = (low - 1);
        for (int j = low; j <= high - 1; j++)
        {
            comparisons++;
            if (arr[j] < pivot)
            {
                i++;
                std::swap(arr[i], arr[j]);
                swaps++;
            }
        }
        std::swap(arr[i + 1], arr[high]);
        swaps++;
        return (i + 1);
    }

    void quickSortBench(std::vector<int>& arr, int low, int high, long long& comparisons, long long& swaps, int currentDepth, int& maxDepth)
    {
        while (low < high)
        {
            if (currentDepth > maxDepth) maxDepth = currentDepth;
            int mid = low + (high - low) / 2;
            std::swap(arr[mid], arr[high]);
            swaps++;

            int pi = partitionBench(arr, low, high, comparisons, swaps);
            if (pi - low < high - pi)
            {
                quickSortBench(arr, low, pi - 1, comparisons, swaps, currentDepth + 1, maxDepth);
                low = pi + 1;
                currentDepth++;
            }
            else
            {
                quickSortBench(arr, pi + 1, high, comparisons, swaps, currentDepth + 1, maxDepth);
                high = pi - 1;
                currentDepth++;
            }
        }
        if (currentDepth > maxDepth) maxDepth = currentDepth;
    }

    // AVL full balanced insert & delete with rotation counter
    TreeNode* benchAvlInsert(AVL& avl, TreeNode* node, int key, bool& inserted, long long& rotationCount)
    {
        if (!node)
        {
            inserted = true;
            return new TreeNode(key);
        }
        if (key == node->val)
        {
            inserted = false;
            return node;
        }
        if (key < node->val)
            node->left = benchAvlInsert(avl, node->left, key, inserted, rotationCount);
        else
            node->right = benchAvlInsert(avl, node->right, key, inserted, rotationCount);

        node->height = 1 + std::max(avl.height(node->left), avl.height(node->right));
        node->size = avl.getSize(node->left) + avl.getSize(node->right) + 1;

        int balance = avl.getBalance(node);

        // LL
        if (balance > 1 && key < node->left->val)
        {
            rotationCount += 1;
            return avl.rightRotate(node);
        }
        // RR
        if (balance < -1 && key > node->right->val)
        {
            rotationCount += 1;
            return avl.leftRotate(node);
        }
        // LR
        if (balance > 1 && key > node->left->val)
        {
            rotationCount += 2;
            node->left = avl.leftRotate(node->left);
            return avl.rightRotate(node);
        }
        // RL
        if (balance < -1 && key < node->right->val)
        {
            rotationCount += 2;
            node->right = avl.rightRotate(node->right);
            return avl.leftRotate(node);
        }

        return node;
    }

    TreeNode* benchAvlDelete(AVL& avl, TreeNode* root, int key, bool& deleted, int& perOpRotations)
    {
        if (!root)
        {
            deleted = false;
            return root;
        }

        if (key < root->val)
            root->left = benchAvlDelete(avl, root->left, key, deleted, perOpRotations);
        else if (key > root->val)
            root->right = benchAvlDelete(avl, root->right, key, deleted, perOpRotations);
        else
        {
            deleted = true;
            if (!root->left || !root->right)
            {
                TreeNode* temp = root->left ? root->left : root->right;
                if (!temp)
                {
                    delete root;
                    return nullptr;
                }
                else
                {
                    *root = *temp;
                    delete temp;
                }
            }
            else
            {
                TreeNode* temp = root->right;
                while (temp->left) temp = temp->left;
                root->val = temp->val;
                root->right = benchAvlDelete(avl, root->right, temp->val, deleted, perOpRotations);
            }
        }

        if (!root) return nullptr;

        root->height = 1 + std::max(avl.height(root->left), avl.height(root->right));
        root->size = avl.getSize(root->left) + avl.getSize(root->right) + 1;

        int balance = avl.getBalance(root);

        if (balance > 1 && avl.getBalance(root->left) >= 0)
        {
            perOpRotations += 1;
            return avl.rightRotate(root);
        }

        if (balance > 1 && avl.getBalance(root->left) < 0)
        {
            perOpRotations += 2;
            root->left = avl.leftRotate(root->left);
            return avl.rightRotate(root);
        }

        if (balance < -1 && avl.getBalance(root->right) <= 0)
        {
            perOpRotations += 1;
            return avl.leftRotate(root);
        }

        if (balance < -1 && avl.getBalance(root->right) > 0)
        {
            perOpRotations += 2;
            root->right = avl.rightRotate(root->right);
            return avl.leftRotate(root);
        }

        return root;
    }

    int countSearchComparisons(TreeNode* root, int target)
    {
        int comps = 0;
        TreeNode* curr = root;
        while (curr)
        {
            comps++;
            if (target == curr->val) break;
            else if (target < curr->val) curr = curr->left;
            else curr = curr->right;
        }
        return comps;
    }
}

bool Benchmark::runAll(const std::string& outputPath)
{
    std::ofstream csv(outputPath);
    if (!csv.is_open()) return false;

    // Header exactly as specified
    csv << "Experiment,Structure,InputType,N,InsertTime_ms,SearchTime_ms,DeleteTime_ms,FinalHeight,AvgComparisons,RotationsOrSwaps,MaxPerOp,Comparisons,Auxiliary\n";
    csv.flush();

    const std::vector<int> N_VALUES = {100, 1000, 10000, 25000};
    const uint32_t SEED = 42;

    // =========================================================================
    // 1. HeightComparison: BST and AVL, N = 100, 1000, 10000, 25000, Sorted and Random
    // =========================================================================
    for (int N : N_VALUES)
    {
        for (const std::string& inType : {"Sorted", "Random"})
        {
            std::vector<int> data(N);
            if (inType == "Sorted")
            {
                for (int i = 0; i < N; i++) data[i] = i + 1;
            }
            else
            {
                std::mt19937 rng(SEED + N);
                for (int i = 0; i < N; i++) data[i] = i + 1;
                std::shuffle(data.begin(), data.end(), rng);
            }

            // BST (avoid call-stack overflow on sorted N >= 10000 if strictly skewed)
            if (!(inType == "Sorted" && N >= 10000))
            {
                BST bst;
                auto tStart = std::chrono::high_resolution_clock::now();
                for (int v : data) bst.insert(v);
                auto tEnd = std::chrono::high_resolution_clock::now();
                double insertMs = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
                int h = Validator::recomputeHeight(bst.getRoot());

                csv << "HeightComparison,BST," << inType << "," << N << ","
                    << insertMs << ",,,," << h << ",,,,,,\n";
            }
            else
            {
                // Note skewed N
                csv << "HeightComparison,BST," << inType << "," << N << ",N/A,,," << N << ",,,,,,\n";
            }

            // AVL
            {
                AVL avl;
                long long dummyRots = 0;
                auto tStart = std::chrono::high_resolution_clock::now();
                for (int v : data)
                {
                    bool ins = false;
                    avl.getRoot() = benchAvlInsert(avl, avl.getRoot(), v, ins, dummyRots);
                }
                auto tEnd = std::chrono::high_resolution_clock::now();
                double insertMs = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
                int h = Validator::recomputeHeight(avl.getRoot());

                csv << "HeightComparison,AVL," << inType << "," << N << ","
                    << insertMs << ",,,," << h << ",,,,,,\n";
            }
        }
    }

    // =========================================================================
    // 2. SearchCost: 10000 random searches on the trees
    // =========================================================================
    const int NUM_SEARCHES = 10000;
    for (int N : N_VALUES)
    {
        for (const std::string& inType : {"Sorted", "Random"})
        {
            std::vector<int> data(N);
            for (int i = 0; i < N; i++) data[i] = i + 1;
            if (inType == "Random")
            {
                std::mt19937 rng(SEED + N);
                std::shuffle(data.begin(), data.end(), rng);
            }

            std::mt19937 searchRng(999);
            std::vector<int> searchTargets(NUM_SEARCHES);
            for (int i = 0; i < NUM_SEARCHES; i++) searchTargets[i] = (searchRng() % (N * 2)) + 1;

            // BST
            if (!(inType == "Sorted" && N >= 10000))
            {
                BST bst;
                for (int v : data) bst.insert(v);

                long long totalComps = 0;
                auto tStart = std::chrono::high_resolution_clock::now();
                for (int t : searchTargets) totalComps += countSearchComparisons(bst.getRoot(), t);
                auto tEnd = std::chrono::high_resolution_clock::now();
                double searchMs = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
                double avgComps = static_cast<double>(totalComps) / NUM_SEARCHES;

                csv << "SearchCost,BST," << inType << "," << N << ",,"
                    << searchMs << ",,," << avgComps << ",,,,\n";
            }

            // AVL
            {
                AVL avl;
                long long dummyRots = 0;
                for (int v : data)
                {
                    bool ins = false;
                    avl.getRoot() = benchAvlInsert(avl, avl.getRoot(), v, ins, dummyRots);
                }

                long long totalComps = 0;
                auto tStart = std::chrono::high_resolution_clock::now();
                for (int t : searchTargets) totalComps += countSearchComparisons(avl.getRoot(), t);
                auto tEnd = std::chrono::high_resolution_clock::now();
                double searchMs = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
                double avgComps = static_cast<double>(totalComps) / NUM_SEARCHES;

                csv << "SearchCost,AVL," << inType << "," << N << ",,"
                    << searchMs << ",,," << avgComps << ",,,,\n";
            }
        }
    }

    // =========================================================================
    // 3. RotationCount: AVL insert and delete, Random and Sorted
    // =========================================================================
    for (int N : N_VALUES)
    {
        for (const std::string& inType : {"Sorted", "Random"})
        {
            std::vector<int> data(N);
            for (int i = 0; i < N; i++) data[i] = i + 1;
            if (inType == "Random")
            {
                std::mt19937 rng(SEED + N);
                std::shuffle(data.begin(), data.end(), rng);
            }

            AVL avl;
            long long insertRotations = 0;
            for (int v : data)
            {
                bool ins = false;
                avl.getRoot() = benchAvlInsert(avl, avl.getRoot(), v, ins, insertRotations);
            }

            long long deleteRotations = 0;
            int maxPerOp = 0;
            auto tStart = std::chrono::high_resolution_clock::now();
            for (int v : data)
            {
                bool del = false;
                int perOp = 0;
                avl.getRoot() = benchAvlDelete(avl, avl.getRoot(), v, del, perOp);
                deleteRotations += perOp;
                if (perOp > maxPerOp) maxPerOp = perOp;
            }
            auto tEnd = std::chrono::high_resolution_clock::now();
            double delMs = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
            double avgDelRotations = static_cast<double>(deleteRotations) / N;

            csv << "RotationCount,AVL," << inType << "," << N << ",,,"
                << delMs << ",," << avgDelRotations << "," << maxPerOp << ",,\n";
        }
    }

    // =========================================================================
    // 4. OrderStatistics: 1000 queries of kth on AVL vs inorder scan on BST
    // =========================================================================
    for (int N : N_VALUES)
    {
        std::vector<int> data(N);
        std::mt19937 rng(SEED + N);
        for (int i = 0; i < N; i++) data[i] = i + 1;
        std::shuffle(data.begin(), data.end(), rng);

        AVL avl;
        BST bst;
        long long dummyRots = 0;
        for (int v : data)
        {
            bool ins = false;
            avl.getRoot() = benchAvlInsert(avl, avl.getRoot(), v, ins, dummyRots);
            bst.insert(v);
        }

        std::vector<int> queries(1000);
        for (int i = 0; i < 1000; i++) queries[i] = (rng() % N) + 1;

        // AVL findKth
        auto tStartA = std::chrono::high_resolution_clock::now();
        for (int k : queries)
        {
            TreeNode* res = avl.findKth(k);
            (void)res;
        }
        auto tEndA = std::chrono::high_resolution_clock::now();
        double avlKthMs = std::chrono::duration<double, std::milli>(tEndA - tStartA).count();

        // BST Inorder scan
        auto tStartB = std::chrono::high_resolution_clock::now();
        for (int k : queries)
        {
            int* inArr = bst.inorderRecursive();
            if (inArr)
            {
                int val = inArr[k - 1];
                (void)val;
                delete[] inArr;
            }
        }
        auto tEndB = std::chrono::high_resolution_clock::now();
        double bstInorderMs = std::chrono::duration<double, std::milli>(tEndB - tStartB).count();

        csv << "OrderStatistics,AVL,Random," << N << ",," << avlKthMs << ",,,,,,,,\n";
        csv << "OrderStatistics,BST,Random," << N << ",," << bstInorderMs << ",,,,,,,,\n";
    }

    // =========================================================================
    // 5. HeapConstruction: MaxHeap bottom-up vs repeated insert
    // =========================================================================
    for (int N : N_VALUES)
    {
        for (const std::string& inType : {"Sorted", "Random"})
        {
            std::vector<int> data(N);
            for (int i = 0; i < N; i++) data[i] = i + 1;
            if (inType == "Random")
            {
                std::mt19937 rng(SEED + N);
                std::shuffle(data.begin(), data.end(), rng);
            }

            // Bottom-up buildMaxHeap O(N)
            MaxHeap hBottomUp(data.data(), N);
            auto tStartB = std::chrono::high_resolution_clock::now();
            hBottomUp.buildMaxHeap();
            auto tEndB = std::chrono::high_resolution_clock::now();
            double bottomUpMs = std::chrono::duration<double, std::milli>(tEndB - tStartB).count();

            // Repeated insert O(N log N)
            std::vector<int> buffer(N);
            auto tStartI = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < N; i++)
            {
                buffer[i] = data[i];
                // SiftUp from index i
                int idx = i;
                while (idx > 0)
                {
                    int parent = (idx - 1) / 2;
                    if (buffer[idx] > buffer[parent])
                    {
                        std::swap(buffer[idx], buffer[parent]);
                        idx = parent;
                    }
                    else break;
                }
            }
            auto tEndI = std::chrono::high_resolution_clock::now();
            double repeatedInsertMs = std::chrono::duration<double, std::milli>(tEndI - tStartI).count();

            csv << "HeapConstruction,MaxHeapBottomUp," << inType << "," << N << ","
                << bottomUpMs << ",,,,,,,,\n";
            csv << "HeapConstruction,MaxHeapRepeatedInsert," << inType << "," << N << ","
                << repeatedInsertMs << ",,,,,,,,\n";
        }
    }

    // =========================================================================
    // 6. HeapSort: Random, Sorted, ReverseSorted, DuplicateHeavy
    // =========================================================================
    for (int N : N_VALUES)
    {
        for (const std::string& inType : {"Random", "Sorted", "ReverseSorted", "DuplicateHeavy"})
        {
            std::vector<int> data(N);
            std::mt19937 rng(SEED + N);
            if (inType == "Sorted")
            {
                for (int i = 0; i < N; i++) data[i] = i + 1;
            }
            else if (inType == "ReverseSorted")
            {
                for (int i = 0; i < N; i++) data[i] = N - i;
            }
            else if (inType == "DuplicateHeavy")
            {
                for (int i = 0; i < N; i++) data[i] = (rng() % 10) + 1;
            }
            else
            {
                for (int i = 0; i < N; i++) data[i] = i + 1;
                std::shuffle(data.begin(), data.end(), rng);
            }

            MaxHeap h(data.data(), N);
            auto tStart = std::chrono::high_resolution_clock::now();
            h.heapSort();
            auto tEnd = std::chrono::high_resolution_clock::now();
            double sortMs = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
            int swaps = h.countSortSwaps();

            csv << "HeapSort,MaxHeap," << inType << "," << N << ","
                << sortMs << ",,,,," << swaps << ",,,\n";
        }
    }

    // =========================================================================
    // 7. SortingComparison: Heap Sort vs Merge Sort, Quick Sort and Insertion Sort
    // =========================================================================
    for (int N : N_VALUES)
    {
        for (const std::string& inType : {"Random", "Sorted", "ReverseSorted", "DuplicateHeavy"})
        {
            std::vector<int> data(N);
            std::mt19937 rng(SEED + N);
            if (inType == "Sorted")
            {
                for (int i = 0; i < N; i++) data[i] = i + 1;
            }
            else if (inType == "ReverseSorted")
            {
                for (int i = 0; i < N; i++) data[i] = N - i;
            }
            else if (inType == "DuplicateHeavy")
            {
                for (int i = 0; i < N; i++) data[i] = (rng() % 10) + 1;
            }
            else
            {
                for (int i = 0; i < N; i++) data[i] = i + 1;
                std::shuffle(data.begin(), data.end(), rng);
            }

            // Heap Sort
            {
                MaxHeap h(data.data(), N);
                auto tStart = std::chrono::high_resolution_clock::now();
                h.heapSort();
                auto tEnd = std::chrono::high_resolution_clock::now();
                double ms = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
                int swaps = h.countSortSwaps();
                csv << "SortingComparison,HeapSort," << inType << "," << N << ","
                    << ms << ",,,,," << swaps << ",,,0\n";
            }

            // Merge Sort
            {
                std::vector<int> copy = data;
                long long comps = 0, swaps = 0;
                auto tStart = std::chrono::high_resolution_clock::now();
                mergeSortBench(copy, 0, N - 1, comps, swaps);
                auto tEnd = std::chrono::high_resolution_clock::now();
                double ms = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
                csv << "SortingComparison,MergeSort," << inType << "," << N << ","
                    << ms << ",,,,," << swaps << ",," << comps << "," << N << "\n";
            }

            // Quick Sort (random pivot to avoid quadratic depth on sorted)
            {
                std::vector<int> copy = data;
                long long comps = 0, swaps = 0;
                int maxDepth = 0;
                auto tStart = std::chrono::high_resolution_clock::now();
                quickSortBench(copy, 0, N - 1, comps, swaps, 1, maxDepth);
                auto tEnd = std::chrono::high_resolution_clock::now();
                double ms = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
                csv << "SortingComparison,QuickSort," << inType << "," << N << ","
                    << ms << ",,,,," << swaps << ",," << comps << "," << maxDepth << "\n";
            }

            // Insertion Sort (up to N <= 10000)
            if (N <= 10000)
            {
                std::vector<int> copy = data;
                long long comps = 0, swaps = 0;
                auto tStart = std::chrono::high_resolution_clock::now();
                insertionSortBench(copy, comps, swaps);
                auto tEnd = std::chrono::high_resolution_clock::now();
                double ms = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
                csv << "SortingComparison,InsertionSort," << inType << "," << N << ","
                    << ms << ",,,,," << swaps << ",," << comps << ",0\n";
            }
        }
    }

    csv.close();
    std::cout << "Benchmark complete, saved to " << outputPath << "\n";
    return true;
}
