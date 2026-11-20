#include "backend/TestSuite.h"
#include "backend/Validator.h"
#include "backend/BinaryTree.h"
#include "backend/BST.h"
#include "backend/AVL.h"
#include "backend/MaxHeap.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <random>
#include <algorithm>

namespace {
    static int g_passCount = 0;
    static int g_failCount = 0;
    static std::ofstream* g_outReport = nullptr;

    void logTest(const std::string& name, bool passed)
    {
        if (passed)
        {
            g_passCount++;
            std::cout << "[PASS] " << name << "\n";
            if (g_outReport && g_outReport->is_open())
            {
                (*g_outReport) << "[PASS] " << name << "\n";
                g_outReport->flush();
            }
        }
        else
        {
            g_failCount++;
            std::cout << "[FAIL] " << name << "\n";
            if (g_outReport && g_outReport->is_open())
            {
                (*g_outReport) << "[FAIL] " << name << "\n";
                g_outReport->flush();
            }
        }
    }

    void logHeader(const std::string& header)
    {
        std::cout << "\n=== " << header << " ===\n";
        if (g_outReport && g_outReport->is_open())
        {
            (*g_outReport) << "\n=== " << header << " ===\n";
            g_outReport->flush();
        }
    }

#define CHECK(expr, name) logTest(name, (expr))

    // Helper functions for traversals comparison
    bool compareArrays(int* a, int* b, int n)
    {
        if (!a && !b) return true;
        if (!a || !b) return false;
        for (int i = 0; i < n; i++)
        {
            if (a[i] != b[i]) return false;
        }
        return true;
    }

    // Custom serialization using preorder with null marker
    void serializeHelper(TreeNode* node, std::ostringstream& oss)
    {
        if (!node)
        {
            oss << "# ";
            return;
        }
        oss << node->val << " ";
        serializeHelper(node->left, oss);
        serializeHelper(node->right, oss);
    }

    std::string serializeTree(TreeNode* root)
    {
        std::ostringstream oss;
        serializeHelper(root, oss);
        return oss.str();
    }

    TreeNode* deserializeHelper(std::istringstream& iss)
    {
        std::string token;
        if (!(iss >> token)) return nullptr;
        if (token == "#") return nullptr;
        int val = std::stoi(token);
        TreeNode* node = new TreeNode(val);
        node->left = deserializeHelper(iss);
        node->right = deserializeHelper(iss);
        return node;
    }

    TreeNode* deserializeTree(const std::string& data)
    {
        std::istringstream iss(data);
        return deserializeHelper(iss);
    }

    // AVL full balancing insert & delete helpers for testing
    TreeNode* avlInsertNode(AVL& avl, TreeNode* node, int key, bool& inserted)
    {
        if (!node)
        {
            inserted = true;
            TreeNode* newNode = new TreeNode(key);
            newNode->height = 1;
            newNode->size = 1;
            return newNode;
        }
        if (key == node->val)
        {
            inserted = false;
            return node;
        }
        if (key < node->val)
            node->left = avlInsertNode(avl, node->left, key, inserted);
        else if (key > node->val)
            node->right = avlInsertNode(avl, node->right, key, inserted);
        else
        {
            inserted = false;
            return node;
        }

        int lh = node->left ? node->left->height : 0;
        int rh = node->right ? node->right->height : 0;
        node->height = 1 + std::max(lh, rh);
        node->size = avl.getSize(node->left) + avl.getSize(node->right) + 1;

        int balance = lh - rh;

        // LL
        if (balance > 1 && node->left && key < node->left->val)
        {
            return avl.rightRotate(node);
        }
        // RR
        if (balance < -1 && node->right && key > node->right->val)
            return avl.leftRotate(node);
        // LR
        if (balance > 1 && node->left && key > node->left->val)
        {
            node->left = avl.leftRotate(node->left);
            return avl.rightRotate(node);
        }
        // RL
        if (balance < -1 && node->right && key < node->right->val)
        {
            node->right = avl.rightRotate(node->right);
            return avl.leftRotate(node);
        }

        return node;
    }

    TreeNode* avlDeleteNode(AVL& avl, TreeNode* root, int key, bool& deleted)
    {
        if (!root)
        {
            deleted = false;
            return root;
        }

        if (key < root->val)
            root->left = avlDeleteNode(avl, root->left, key, deleted);
        else if (key > root->val)
            root->right = avlDeleteNode(avl, root->right, key, deleted);
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
                root->right = avlDeleteNode(avl, root->right, temp->val, deleted);
            }
        }

        if (!root) return nullptr;

        root->height = 1 + std::max(avl.height(root->left), avl.height(root->right));
        root->size = avl.getSize(root->left) + avl.getSize(root->right) + 1;

        int balance = avl.getBalance(root);

        if (balance > 1 && avl.getBalance(root->left) >= 0)
            return avl.rightRotate(root);

        if (balance > 1 && avl.getBalance(root->left) < 0)
        {
            root->left = avl.leftRotate(root->left);
            return avl.rightRotate(root);
        }

        if (balance < -1 && avl.getBalance(root->right) <= 0)
            return avl.leftRotate(root);

        if (balance < -1 && avl.getBalance(root->right) > 0)
        {
            root->right = avl.rightRotate(root->right);
            return avl.leftRotate(root);
        }

        return root;
    }
}

static void testBinaryTree()
{
    logHeader("BinaryTree");

    // 1. Empty tree traversals
    {
        BinaryTree bt;
        CHECK(bt.getRoot() == nullptr, "Empty tree root is null");
        int* preI = bt.preorderIterative();
        int* inI = bt.inorderIterative();
        CHECK(preI == nullptr && inI == nullptr, "Empty tree iterative traversals return null");
        CHECK(bt.height() == 0, "Empty tree height == 0");
        CHECK(Validator::isBinaryTreeValid(bt.getRoot()), "Empty tree Validator check");
    }

    // 2. Single node tree
    {
        BinaryTree bt;
        bt.insert(42);
        int* preR = bt.preorderRecursive();
        int* preI = bt.preorderIterative();
        int* inR = bt.inorderRecursive();
        int* inI = bt.inorderIterative();
        int* postR = bt.postorderRecursive();
        int* postI = bt.postorderIterative();
        int* lvl = bt.levelOrder();

        CHECK(preR && preI && preR[0] == 42 && preI[0] == 42, "Single node preorder rec == iter");
        CHECK(inR && inI && inR[0] == 42 && inI[0] == 42, "Single node inorder rec == iter");
        CHECK(postR && postI && postR[0] == 42 && postI[0] == 42, "Single node postorder rec == iter");
        CHECK(lvl && lvl[0] == 42, "Single node levelOrder");
        CHECK(bt.height() == 1, "Single node height == 1");
        CHECK(bt.diameter() >= 0, "Single node diameter");
        CHECK(Validator::isBinaryTreeValid(bt.getRoot()), "Single node Validator check");

        delete[] preR; delete[] preI;
        delete[] inR; delete[] inI;
        delete[] postR; delete[] postI;
        delete[] lvl;
    }

    // 3. Skewed tree (left skewed)
    {
        BinaryTree bt;
        bt.insert(3);
        bt.insert(2);
        bt.insert(1);

        int* preR = bt.preorderRecursive();
        int* preI = bt.preorderIterative();
        int* inR = bt.inorderRecursive();
        int* inI = bt.inorderIterative();
        int* postR = bt.postorderRecursive();
        int* postI = bt.postorderIterative();

        CHECK(compareArrays(preR, preI, 3), "Left-skewed preorder rec == iter");
        CHECK(compareArrays(inR, inI, 3), "Left-skewed inorder rec == iter");
        CHECK(compareArrays(postR, postI, 3), "Left-skewed postorder rec == iter");
        CHECK(bt.height() >= 1, "Skewed tree height check");
        CHECK(Validator::isBinaryTreeValid(bt.getRoot()), "Left-skewed Validator check");

        delete[] preR; delete[] preI;
        delete[] inR; delete[] inI;
        delete[] postR; delete[] postI;
    }

    // 4. Full binary tree & round trip reconstruction
    {
        // Tree:
        //       1
        //     /   \
        //    2     3
        //   / \   / \
        //  4   5 6   7
        int pre[] = {1, 2, 4, 5, 3, 6, 7};
        int in[] = {4, 2, 5, 1, 6, 3, 7};
        int post[] = {4, 5, 2, 6, 7, 3, 1};

        BinaryTree btPreIn;
        btPreIn.buildFromPreIn(pre, in, 7);
        CHECK(Validator::isBinaryTreeValid(btPreIn.getRoot()), "buildFromPreIn valid structure");

        BinaryTree btPostIn;
        btPostIn.buildFromPosIn(post, in, 7);
        CHECK(Validator::isBinaryTreeValid(btPostIn.getRoot()), "buildFromPosIn valid structure");

        CHECK(btPreIn.isSame(btPostIn), "Reconstruction round trip: pre/in vs post/in are identical");
        CHECK(btPreIn.height() == 3, "Full tree height == 3");
        CHECK(btPreIn.diameter() >= 4, "Full tree diameter check");

        TreeNode* lca1 = btPreIn.lca(4, 5);
        CHECK(lca1 && lca1->val == 2, "LCA(4, 5) == 2");
        TreeNode* lca2 = btPreIn.lca(4, 7);
        CHECK(lca2 && lca2->val == 1, "LCA(4, 7) == 1");

        int* path = btPreIn.pathTo(5);
        CHECK(path && path[0] == 1 && path[1] == 2 && path[2] == 5, "pathTo(5) correct path");
        delete[] path;

        // Mirror test: mirror twice gives original
        BinaryTree btOriginal = btPreIn.clone();
        btPreIn.Mirror();
        CHECK(!btPreIn.isSame(btOriginal), "Mirrored tree differs from original");
        btPreIn.Mirror();
        CHECK(btPreIn.isSame(btOriginal), "Double mirrored tree equals original");
        btOriginal.destroyTree();
    }

    // 5. isSymmetric test
    {
        BinaryTree sym;
        TreeNode* r = new TreeNode(1);
        r->left = new TreeNode(2);
        r->right = new TreeNode(2);
        r->left->left = new TreeNode(3);
        r->left->right = new TreeNode(4);
        r->right->left = new TreeNode(4);
        r->right->right = new TreeNode(3);
        sym.setRoot(r);
        CHECK(sym.isSymmetric(), "Symmetric tree returns true");

        r->right->right->val = 99;
        CHECK(!sym.isSymmetric(), "Asymmetric tree returns false");
    }
}

static void testBST()
{
    logHeader("BST");

    BST bst;
    // 1. Insert & duplicate rejected
    CHECK(bst.insert(50), "BST insert 50");
    CHECK(Validator::isBST(bst.getRoot()), "BST valid after insert 50");

    int keys[] = {30, 70, 20, 40, 60, 80};
    for (int k : keys)
    {
        CHECK(bst.insert(k), std::string("BST insert ") + std::to_string(k));
        CHECK(Validator::isBST(bst.getRoot()), std::string("BST property holds after inserting ") + std::to_string(k));
    }
    CHECK(!bst.insert(50), "BST duplicate rejected (50)");
    CHECK(!bst.insert(20), "BST duplicate rejected (20)");

    // 2. Search
    CHECK(bst.findNode(40) != nullptr && bst.findNode(40)->val == 40, "BST search found key 40");
    CHECK(bst.findNode(999) == nullptr, "BST search not found key 999");

    // 3. Min, Max, Successor, Predecessor
    TreeNode* minNode = bst.getRoot();
    while (minNode && minNode->left) minNode = minNode->left;
    CHECK(minNode && minNode->val == 20, "BST findMin == 20");

    TreeNode* maxNode = bst.getRoot();
    while (maxNode && maxNode->right) maxNode = maxNode->right;
    CHECK(maxNode && maxNode->val == 80, "BST findMax == 80");

    TreeNode* succ = bst.inorderSuccessor(bst.findNode(50));
    CHECK(succ && succ->val == 60, "BST inorderSuccessor(50) == 60");

    // Predecessor of 50 is max of left subtree (40)
    TreeNode* predNode = bst.findNode(50)->left;
    while (predNode && predNode->right) predNode = predNode->right;
    CHECK(predNode && predNode->val == 40, "BST predecessor(50) == 40");

    // 4. Range query
    int count = 0;
    int* range = bst.rangeToArray(25, 65, count);
    CHECK(count == 4, "BST rangeQuery [25, 65] count == 4 (30, 40, 50, 60)");
    delete[] range;

    range = bst.rangeToArray(100, 200, count);
    CHECK(count == 0, "BST empty rangeQuery [100, 200] count == 0");
    delete[] range;

    range = bst.rangeToArray(0, 100, count);
    CHECK(count == 7, "BST full rangeQuery [0, 100] count == 7");
    delete[] range;

    // 5. kthSmallest using inorder
    int* inArr = bst.inorderRecursive();
    CHECK(inArr[0] == 20, "BST 1st smallest == 20");
    CHECK(inArr[3] == 50, "BST 4th smallest == 50");
    delete[] inArr;

    // 6. Delete cases
    // Delete missing key
    CHECK(!bst.remove(999), "BST delete missing key returns false");

    // Delete leaf (20)
    CHECK(bst.deleteCase(20) == 0, "BST deleteCase(20) identified as leaf (case 0)");
    CHECK(bst.remove(20), "BST delete leaf (20)");
    CHECK(Validator::isBST(bst.getRoot()), "BST valid after leaf delete");

    // Delete node with one child
    // In bst: 70 has 60,80; 30 has 40. 20 was deleted, so 30 only has right child 40!
    CHECK(bst.deleteCase(30) == 1, "BST deleteCase(30) identified as one child (case 1)");
    CHECK(bst.remove(30), "BST delete node with one child (30)");
    CHECK(Validator::isBST(bst.getRoot()), "BST valid after one child delete");

    // Delete node with two children (70 has children 60, 80)
    CHECK(bst.deleteCase(70) == 2, "BST deleteCase(70) identified as two children (case 2)");
    CHECK(bst.remove(70), "BST delete node with two children (70)");
    CHECK(Validator::isBST(bst.getRoot()), "BST valid after two children delete");

    // Delete root (50)
    CHECK(bst.remove(50), "BST delete root (50)");
    CHECK(Validator::isBST(bst.getRoot()), "BST valid after root delete");
}

static void testAVL()
{
    logHeader("AVLTree");

    AVL avl;

    // 1. LL Rotation Test: Insert 30, 20, 10
    {
        AVL llTree;
        bool ins = false;
        llTree.getRoot() = avlInsertNode(llTree, llTree.getRoot(), 30, ins);
        llTree.getRoot() = avlInsertNode(llTree, llTree.getRoot(), 20, ins);
        llTree.getRoot() = avlInsertNode(llTree, llTree.getRoot(), 10, ins);

        CHECK(llTree.getRoot()->val == 20, "LL rotation root == 20");
        CHECK(llTree.getRoot()->left->val == 10 && llTree.getRoot()->right->val == 30, "LL rotation children (10, 30)");
        CHECK(Validator::isAVL(llTree.getRoot()), "LL rotation Validator::isAVL passed");
    }

    // 2. RR Rotation Test: Insert 10, 20, 30
    {
        AVL rrTree;
        bool ins = false;
        rrTree.getRoot() = avlInsertNode(rrTree, rrTree.getRoot(), 10, ins);
        rrTree.getRoot() = avlInsertNode(rrTree, rrTree.getRoot(), 20, ins);
        rrTree.getRoot() = avlInsertNode(rrTree, rrTree.getRoot(), 30, ins);

        CHECK(rrTree.getRoot()->val == 20, "RR rotation root == 20");
        CHECK(rrTree.getRoot()->left->val == 10 && rrTree.getRoot()->right->val == 30, "RR rotation children (10, 30)");
        CHECK(Validator::isAVL(rrTree.getRoot()), "RR rotation Validator::isAVL passed");
    }

    // 3. LR Rotation Test: Insert 30, 10, 20
    {
        AVL lrTree;
        bool ins = false;
        lrTree.getRoot() = avlInsertNode(lrTree, lrTree.getRoot(), 30, ins);
        lrTree.getRoot() = avlInsertNode(lrTree, lrTree.getRoot(), 10, ins);
        lrTree.getRoot() = avlInsertNode(lrTree, lrTree.getRoot(), 20, ins);

        CHECK(lrTree.getRoot()->val == 20, "LR rotation root == 20");
        CHECK(lrTree.getRoot()->left->val == 10 && lrTree.getRoot()->right->val == 30, "LR rotation children (10, 30)");
        CHECK(Validator::isAVL(lrTree.getRoot()), "LR rotation Validator::isAVL passed");
    }

    // 4. RL Rotation Test: Insert 10, 30, 20
    {
        AVL rlTree;
        bool ins = false;
        rlTree.getRoot() = avlInsertNode(rlTree, rlTree.getRoot(), 10, ins);
        rlTree.getRoot() = avlInsertNode(rlTree, rlTree.getRoot(), 30, ins);
        rlTree.getRoot() = avlInsertNode(rlTree, rlTree.getRoot(), 20, ins);

        CHECK(rlTree.getRoot()->val == 20, "RL rotation root == 20");
        CHECK(rlTree.getRoot()->left->val == 10 && rlTree.getRoot()->right->val == 30, "RL rotation children (10, 30)");
        CHECK(Validator::isAVL(rlTree.getRoot()), "RL rotation Validator::isAVL passed");
    }

    // 5. Delete causing rotation & heavy child BF = 0
    {
        AVL delTree;
        bool ins = false;
        // Build tree where deleting 10 causes right rotation at root, and left child has BF = 0
        int vals[] = {30, 20, 40, 15, 25};
        for (int v : vals)
            delTree.getRoot() = avlInsertNode(delTree, delTree.getRoot(), v, ins);

        bool del = false;
        delTree.getRoot() = avlDeleteNode(delTree, delTree.getRoot(), 40, del);
        CHECK(del, "Deleted key 40");
        CHECK(Validator::isAVL(delTree.getRoot()), "AVL balanced after deletion causing rotation (heavy child BF=0)");
    }

    // 6. 200 Random Operations with fixed seed (Fixed Seed determinism)
    {
        AVL randomTree;
        std::mt19937 rng(1337);
        std::vector<int> sortedArr;
        bool allBalanced = true;
        bool allHeightsValid = true;
        bool allSizesValid = true;
        bool rankAgree = true;

        for (int op = 0; op < 200; op++)
        {
            int action = rng() % 3; // 0,1: insert, 2: delete
            int val = (rng() % 100) + 1;

            if (action < 2)
            {
                bool inserted = false;
                randomTree.getRoot() = avlInsertNode(randomTree, randomTree.getRoot(), val, inserted);
                if (inserted)
                {
                    sortedArr.push_back(val);
                    std::sort(sortedArr.begin(), sortedArr.end());
                }
            }
            else
            {
                if (!sortedArr.empty())
                {
                    int delIdx = rng() % sortedArr.size();
                    int delVal = sortedArr[delIdx];
                    bool deleted = false;
                    randomTree.getRoot() = avlDeleteNode(randomTree, randomTree.getRoot(), delVal, deleted);
                    if (deleted)
                    {
                        sortedArr.erase(sortedArr.begin() + delIdx);
                    }
                }
            }

            if (!Validator::isAVL(randomTree.getRoot())) allBalanced = false;
            if (randomTree.getRoot())
            {
                if (randomTree.getRoot()->height != Validator::recomputeHeight(randomTree.getRoot()))
                    allHeightsValid = false;
                if (randomTree.getRoot()->size != Validator::recomputeSize(randomTree.getRoot()))
                    allSizesValid = false;

                // Check kthSmallest using findKth
                if (!sortedArr.empty())
                {
                    int k = (rng() % sortedArr.size()) + 1;
                    TreeNode* kth = randomTree.findKth(k);
                    if (!kth || kth->val != sortedArr[k - 1])
                    {
                        rankAgree = false;
                    }
                }
            }
        }

        CHECK(allBalanced, "AVL isBalanced maintained across 200 random operations");
        CHECK(allHeightsValid, "AVL stored heights match recomputed heights");
        CHECK(allSizesValid, "AVL stored sizes match recomputed sizes");
        CHECK(rankAgree, "AVL findKth agrees with sorted reference array");
    }

    // 7. Duplicates rejected & empty / single node
    {
        AVL edgeTree;
        CHECK(edgeTree.getRoot() == nullptr, "Empty AVL tree root is null");
        CHECK(Validator::isAVL(edgeTree.getRoot()), "Empty AVL is valid");

        bool ins = false;
        edgeTree.getRoot() = avlInsertNode(edgeTree, edgeTree.getRoot(), 10, ins);
        CHECK(ins, "Single node inserted");
        CHECK(edgeTree.getRoot()->val == 10, "Single node root == 10");
        CHECK(Validator::isAVL(edgeTree.getRoot()), "Single node AVL valid");

        edgeTree.getRoot() = avlInsertNode(edgeTree, edgeTree.getRoot(), 10, ins);
        CHECK(!ins, "Duplicate key 10 rejected");
    }
}

static void testMaxHeap()
{
    logHeader("MaxHeap");

    // 1. buildMaxHeap cases
    {
        MaxHeap emptyH(nullptr, 0);
        emptyH.buildMaxHeap();
        CHECK(emptyH.isMaxHeap(), "Empty buildMaxHeap isMaxHeap == true");

        int singleVal[] = {42};
        MaxHeap singleH(singleVal, 1);
        singleH.buildMaxHeap();
        CHECK(singleH.isMaxHeap() && singleH.getAt(0) == 42, "Single element buildMaxHeap");

        int dupVals[] = {5, 5, 5, 5, 5};
        MaxHeap dupH(dupVals, 5);
        dupH.buildMaxHeap();
        CHECK(dupH.isMaxHeap(), "Duplicates buildMaxHeap isMaxHeap == true");

        int heapVals[] = {50, 30, 40, 10, 20};
        MaxHeap alreadyH(heapVals, 5);
        alreadyH.buildMaxHeap();
        CHECK(alreadyH.isMaxHeap(), "Already-a-heap buildMaxHeap isMaxHeap == true");

        int revVals[] = {1, 2, 3, 4, 5, 6, 7};
        MaxHeap revH(revVals, 7);
        revH.buildMaxHeap();
        CHECK(revH.isMaxHeap(), "Reverse-sorted buildMaxHeap isMaxHeap == true");

        int randVals[] = {17, 3, 45, 99, 12, 64, 30};
        MaxHeap randH(randVals, 7);
        randH.buildMaxHeap();
        CHECK(randH.isMaxHeap(), "Random buildMaxHeap isMaxHeap == true");
    }

    // 2. insert & siftUp
    {
        int arr[10] = {0};
        int count = 0;
        MaxHeap heap(arr, 0);
        bool allSiftUpValid = true;

        int toInsert[] = {10, 20, 5, 30, 25, 40};
        // Simulated incremental insertion by copying array and calling siftUp
        std::vector<int> dynamicBuf;
        for (int v : toInsert)
        {
            dynamicBuf.push_back(v);
            MaxHeap stepHeap(dynamicBuf.data(), static_cast<int>(dynamicBuf.size()));
            stepHeap.siftUp(static_cast<int>(dynamicBuf.size()) - 1);
            for (size_t i = 0; i < dynamicBuf.size(); i++)
            {
                dynamicBuf[i] = stepHeap.getAt(static_cast<int>(i));
            }
            if (!stepHeap.isMaxHeap()) allSiftUpValid = false;
        }
        CHECK(allSiftUpValid, "Heap property holds after each incremental insert & siftUp");
    }

    // 3. extractMax
    {
        int vals[] = {10, 50, 30, 20, 40};
        MaxHeap h(vals, 5);
        h.buildMaxHeap();

        int m1 = h.extractMax();
        CHECK(m1 == 50, "extractMax returned 50");
        CHECK(h.isMaxHeap(), "Remaining active heap is still valid after 1st extractMax");

        int m2 = h.extractMax();
        CHECK(m2 == 40, "extractMax returned 40");
        CHECK(h.isMaxHeap(), "Remaining active heap is still valid after 2nd extractMax");

        MaxHeap emptyH(nullptr, 0);
        int emptyExt = emptyH.extractMax();
        CHECK(emptyExt == -1, "extractMax from empty heap handled safely (returns -1)");
    }

    // 4. heapSort compared with std::sort
    {
        auto testSort = [](std::vector<int> input, const std::string& label) {
            std::vector<int> expected = input;
            std::sort(expected.begin(), expected.end());

            MaxHeap h(input.data(), static_cast<int>(input.size()));
            h.heapSort();

            bool match = true;
            for (size_t i = 0; i < input.size(); i++)
            {
                if (h.getAt(static_cast<int>(i)) != expected[i]) match = false;
            }
            CHECK(match, "heapSort: " + label);
        };

        testSort({}, "Empty array");
        testSort({42}, "Single element");
        testSort({5, 5, 5, 5, 5}, "Duplicates");
        testSort({-10, -5, -20, 0, 15, -1}, "Negatives included");
        testSort({1, 2, 3, 4, 5, 6, 7}, "Already sorted");
        testSort({7, 6, 5, 4, 3, 2, 1}, "Reverse sorted");
        testSort({45, 12, 85, 32, 89, 39, 69, 44, 42, 1, 40}, "Random elements");
    }
}

static void testSaveAndLoad()
{
    logHeader("Save and load");

    // 1. Empty tree serialisation
    {
        std::string s = serializeTree(nullptr);
        TreeNode* loaded = deserializeTree(s);
        CHECK(loaded == nullptr, "Empty tree serialize/deserialize round trip");
    }

    // 2. Single node
    {
        TreeNode* single = new TreeNode(42);
        std::string s = serializeTree(single);
        TreeNode* loaded = deserializeTree(s);
        CHECK(loaded && loaded->val == 42 && !loaded->left && !loaded->right, "Single node serialize/deserialize round trip");
        delete single;
        delete loaded;
    }

    // 3. Skewed tree
    {
        TreeNode* root = new TreeNode(10);
        root->left = new TreeNode(20);
        root->left->left = new TreeNode(30);
        std::string s = serializeTree(root);
        TreeNode* loaded = deserializeTree(s);

        CHECK(loaded && loaded->val == 10 && loaded->left && loaded->left->val == 20 &&
              loaded->left->left && loaded->left->left->val == 30, "Skewed tree serialize/deserialize preserves structure");

        delete root->left->left; delete root->left; delete root;
        delete loaded->left->left; delete loaded->left; delete loaded;
    }

    // 4. Random balanced tree
    {
        BinaryTree bt;
        TreeNode* r = new TreeNode(50);
        r->left = new TreeNode(25);
        r->right = new TreeNode(75);
        r->left->left = new TreeNode(10);
        r->left->right = new TreeNode(35);
        bt.setRoot(r);

        std::string s = serializeTree(bt.getRoot());
        TreeNode* loaded = deserializeTree(s);
        BinaryTree loadedBt;
        loadedBt.setRoot(loaded);

        CHECK(bt.isSame(loadedBt), "Random tree serialize/deserialize matches original structure exactly");
    }

    // 5. Heap array plus active size round trip
    {
        int arr[] = {90, 80, 70, 60, 50, 40, 30};
        MaxHeap h(arr, 7);
        h.extractMax(); // active heapSize becomes 6

        // Serialise
        std::ostringstream oss;
        oss << h.getHeapSize() << " " << h.getCount() << " ";
        for (int i = 0; i < h.getCount(); i++) oss << h.getAt(i) << " ";

        // Load back
        std::istringstream iss(oss.str());
        int loadedHeapSize, loadedCount;
        iss >> loadedHeapSize >> loadedCount;
        std::vector<int> loadedData(loadedCount);
        for (int i = 0; i < loadedCount; i++) iss >> loadedData[i];

        MaxHeap loadedH(loadedData.data(), loadedHeapSize);
        CHECK(loadedHeapSize == 6 && loadedCount == 7, "Heap active size & count preserved");
        CHECK(Validator::isHeap(loadedH), "Heap active region round trip is valid");
    }

    // 6. AVL height and size recomputed after load & Validator passes
    {
        TreeNode* r = new TreeNode(20);
        r->left = new TreeNode(10);
        r->right = new TreeNode(30);

        std::string s = serializeTree(r);
        TreeNode* loaded = deserializeTree(s);

        AVL avl;
        avl.setRoot(loaded);
        avl.updateHeights();
        avl.updateSizes();

        CHECK(Validator::isAVL(avl.getRoot()), "Loaded AVL tree passes Validator after updateHeights & updateSizes");
        CHECK(avl.getRoot()->height == 2, "Loaded AVL root height == 2");
        CHECK(avl.getRoot()->size == 3, "Loaded AVL root size == 3");

        delete r->left; delete r->right; delete r;
    }
}

bool TestSuite::runAll(const std::string& reportPath, int* outPassed, int* outFailed)
{
    g_passCount = 0;
    g_failCount = 0;

    std::ofstream reportFile(reportPath);
    g_outReport = &reportFile;

    std::cout << "Starting TestSuite execution...\n";
    if (reportFile.is_open()) {
        reportFile << "Starting TestSuite execution...\n";
        reportFile.flush();
    }

    testBinaryTree();
    testBST();
    testAVL();
    testMaxHeap();
    testSaveAndLoad();

    std::string summary = "Tests: " + std::to_string(g_passCount) + " passed, " + std::to_string(g_failCount) + " failed.";
    std::cout << "\n" << summary << "\n";
    if (reportFile.is_open())
    {
        reportFile << "\n" << summary << "\n";
        reportFile.close();
    }
    g_outReport = nullptr;

    if (outPassed) *outPassed = g_passCount;
    if (outFailed) *outFailed = g_failCount;

    return (g_failCount == 0);
}
