#include "BST.h"

BST::BST()
{
    root = NULL;
}

BST::~BST()
{
    destroyHelper(root);
    root = NULL;
}

void BST::destroyHelper(TreeNode* node)
{
    if (node == NULL)
    {
        return;
    }
    destroyHelper(node->left);
    destroyHelper(node->right);
    delete node;
    node == NULL;
}

void BST::clear()
{
    destroyHelper(root);
    root = NULL;
}

TreeNode* BST::insertHelper(TreeNode* node, int key, bool& inserted)
{
    if (node == NULL)
    {
        TreeNode* newNode = new TreeNode(key, key);
        inserted = true;
        return newNode;
    }

    if (key < node->val)
    {
        node->left = insertHelper(node->left, key, inserted);
    }
    else if (key > node->val)
    {
        node->right = insertHelper(node->right, key, inserted);
    }
    else
    {
        // Duplicate key: rejected
        inserted = false;
    }
    return node;
}

bool BST::insert(int key)
{
    bool inserted = false;
    root = insertHelper(root, key, inserted);
    return inserted;
}

TreeNode* BST::search(int key)
{
    TreeNode* curr = root;
    while (curr != NULL)
    {
        if (key == curr->val)
        {
            return curr;
        }
        else if (key < curr->val)
        {
            curr = curr->left;
        }
        else
        {
            curr = curr->right;
        }
    }
    return NULL;
}

int* BST::getPath(int key, int& length)
{
    length = 0;
    TreeNode* curr = root;
    // Pehle length count karein
    int count = 0;
    bool found = false;
    while (curr != NULL)
    {
        count++;
        if (curr->val == key)
        {
            found = true;
            break;
        }
        else if (key < curr->val)
        {
            curr = curr->left;
        }
        else
        {
            curr = curr->right;
        }
    }

    if (!found)
    {
        length = 0;
        return NULL;
    }

    int* path = new int[count];
    curr = root;
    for (int i = 0; i < count; i++)
    {
        path[i] = curr->val;
        if (curr->val == key)
        {
            break;
        }
        else if (key < curr->val)
        {
            curr = curr->left;
        }
        else
        {
            curr = curr->right;
        }
    }
    length = count;
    return path;
}

int* BST::getInsertPath(int key, int& length)
{
    length = 0;
    if (root == NULL)
    {
        // Root is empty, no parent
        return NULL;
    }

    // Traverse down to future parent
    TreeNode* curr = root;
    int count = 0;
    while (curr != NULL)
    {
        count++;
        if (key < curr->val)
        {
            if (curr->left == NULL)
            {
                break;
            }
            curr = curr->left;
        }
        else if (key > curr->val)
        {
            if (curr->right == NULL)
            {
                break;
            }
            curr = curr->right;
        }
        else
        {
            // Duplicate
            break;
        }
    }

    int* path = new int[count];
    curr = root;
    for (int i = 0; i < count; i++)
    {
        path[i] = curr->val;
        if (key < curr->val)
        {
            if (curr->left == NULL)
            {
                break;
            }
            curr = curr->left;
        }
        else if (key > curr->val)
        {
            if (curr->right == NULL)
            {
                break;
            }
            curr = curr->right;
        }
        else
        {
            break;
        }
    }
    length = count;
    return path;
}

int BST::deleteCase(int key)
{
    TreeNode* node = search(key);
    if (node == NULL)
    {
        return -1;
    }

    if (node->left == NULL && node->right == NULL)
    {
        return 0; // Leaf
    }
    else if (node->left != NULL && node->right != NULL)
    {
        return 2; // Two children
    }
    else
    {
        return 1; // One child
    }
}

TreeNode* BST::inorderSuccessor(TreeNode* node)
{
    if (node == NULL || node->right == NULL)
    {
        return NULL;
    }
    TreeNode* curr = node->right;
    while (curr->left != NULL)
    {
        curr = curr->left;
    }
    return curr;
}

TreeNode* BST::findMin(TreeNode* node) const
{
    if (node == NULL)
    {
        return NULL;
    }
    while (node->left != NULL)
    {
        node = node->left;
    }
    return node;
}

TreeNode* BST::removeHelper(TreeNode* node, int key, bool& removed)
{
    if (node == NULL)
    {
        return NULL;
    }

    if (key < node->val)
    {
        node->left = removeHelper(node->left, key, removed);
    }
    else if (key > node->val)
    {
        node->right = removeHelper(node->right, key, removed);
    }
    else
    {
        removed = true;
        // Case 0: Leaf
        if (node->left == NULL && node->right == NULL)
        {
            delete node;
            return NULL;
        }
        // Case 1: One child (only right)
        else if (node->left == NULL)
        {
            TreeNode* temp = node->right;
            delete node;
            return temp;
        }
        // Case 1: One child (only left)
        else if (node->right == NULL)
        {
            TreeNode* temp = node->left;
            delete node;
            return temp;
        }
        // Case 2: Two children
        else
        {
            TreeNode* succ = findMin(node->right);
            node->val = succ->val;
            node->NodeId = succ->NodeId;
            node->right = removeHelper(node->right, succ->val, removed);
        }
    }
    return node;
}

bool BST::remove(int key)
{
    bool removed = false;
    root = removeHelper(root, key, removed);
    return removed;
}

int BST::countRangeHelper(TreeNode* node, int lo, int hi) const
{
    if (node == NULL)
    {
        return 0;
    }
    int count = 0;
    if (node->val > lo)
    {
        count += countRangeHelper(node->left, lo, hi);
    }
    if (node->val >= lo && node->val <= hi)
    {
        count++;
    }
    if (node->val < hi)
    {
        count += countRangeHelper(node->right, lo, hi);
    }
    return count;
}

void BST::rangeHelper(TreeNode* node, int lo, int hi, int* arr, int& count) const
{
    if (node == NULL)
    {
        return;
    }
    if (node->val > lo)
    {
        rangeHelper(node->left, lo, hi, arr, count);
    }
    if (node->val >= lo && node->val <= hi)
    {
        arr[count] = node->val;
        count++;
    }
    if (node->val < hi)
    {
        rangeHelper(node->right, lo, hi, arr, count);
    }
}

int* BST::rangeToArray(int lo, int hi, int& count)
{
    int total = countRangeHelper(root, lo, hi);
    count = 0;
    if (total == 0)
    {
        return NULL;
    }
    int* arr = new int[total];
    rangeHelper(root, lo, hi, arr, count);
    return arr;
}

int BST::heightHelper(TreeNode* node) const
{
    if (node == NULL)
    {
        return 0;
    }
    int lh = heightHelper(node->left);
    int rh = heightHelper(node->right);
    if (lh > rh)
    {
        return lh + 1;
    }
    return rh + 1;
}

int BST::height()
{
    return heightHelper(root);
}

int BST::sizeHelper(TreeNode* node) const
{
    if (node == NULL)
    {
        return 0;
    }
    return 1 + sizeHelper(node->left) + sizeHelper(node->right);
}

int BST::size()
{
    return sizeHelper(root);
}

TreeNode* BST::getRoot() const
{
    return root;
}
