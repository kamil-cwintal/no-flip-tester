#include "interval-tree.h"
#include <cassert>
using std::max;


void IntervalTreeNode::printNode(ostream &outputStream) {
    outputStream << "[" << getLow() << "," << getHigh() << "],"
    << "h=" << height << ",hs=" << highest;
}

/* Empties the tree and frees allocated memory. */
void IntervalTree::clearTree(IntervalTreeNode *node) {
    if (node == nullptr) return;
    if (node->left) clearTree(node->left);
    if (node->right) clearTree(node->right);
    delete node;
}

/* Returns a pointer to the node with the smallest key
   located in the subtree rooted in the provided node. */
IntervalTreeNode* IntervalTree::getMinNode(auto *node) const {
    if (node->left == nullptr) return node;
    else return getMinNode(node->left);
}

/* Returns a pointer to the node with the largest key
   located in the subtree rooted in the provided node. */
IntervalTreeNode* IntervalTree::getMaxNode(auto *node) const {
    if (node->right == nullptr) return node;
    else return getMaxNode(node->right);
}

/* Returns a pointer to the node with the provided interval.
   Might return nullptr if there is no such interval. */
IntervalTreeNode* IntervalTree::search(auto *node, pair<int,int> &target) const {
    if (node == nullptr) return nullptr;
    if (node->range == target) return node;
    
    // Continue recursively in one of the subtrees.
    if (node->range > target) return search(node->left, target);
    else return search(node->right, target);
}

/* Searches for the specified interval in the tree. */
bool IntervalTree::contains(int low, int high) const {
    assert (low <= high);
    pair<int,int> target(low, high);
    return search(root, target) != nullptr;
}

/* Do the two intervals have a nonempty overlap? */
bool IntervalTree::areIntervalsClashing(auto &intA, auto &intB) const {
    return intA.first <= intB.second && intB.first <= intA.second;
}

using IntervalList = vector<pair<int,int>>;

/* Returns all intervals stored in the tree that overlap
   with the query interval. */
IntervalList IntervalTree::getClashes(int low, int high) const {
    assert (low <= high);
    IntervalList result;
    pair<int,int> query(low, high);
    collectClashes(root, query, result);
    return result; // no copy here (move semantics)
}

/* Returns the number of intervals stored in the tree
   that overlap with the query interval. */
int IntervalTree::countClashes(int low, int high) const {
    IntervalList clashes = getClashes(low, high);
    return clashes.size();
}

void IntervalTree::collectClashes(auto *node, pair<int,int> &query, IntervalList &result) const {
    if (node == nullptr) return;
    // Skip further search if no intervals in the subtree may clash.
    if (node->highest < query.first) return;
    
    collectClashes(node->left, query, result);
    if (areIntervalsClashing(node->range, query)) {
        result.push_back(node->range);
    }
    // Another possible skip, using the BST property.
    if (node->range.first <= query.second) {
        collectClashes(node->right, query, result);
    }
}

int IntervalTree::getHighest(const auto *node) const {
    if (node == nullptr) return 0;
    else return node->highest;
}

int IntervalTree::getHeight(const auto *node) const {
    if (node == nullptr) return 0;
    else return node->height;
}

/* Updates the auxiliary values in the node, assuming the children
   have already been processed. */
void IntervalTree::updateAuxValues(auto *node) {
    node->height = 1 + max(getHeight(node->left), getHeight(node->right));
    node->highest = max(node->getHigh(),
                    max(getHighest(node->left), getHighest(node->right)));
}

/* AVL right-rotation procedure, called when the node is unbalanced.
   Preserves the correct order of the elements. */
IntervalTreeNode* IntervalTree::rotateRight(auto *node) {
    auto root = node->left;
    node->left = root->right;
    root->right = node;
    updateAuxValues(node);
    updateAuxValues(root);
    return root;
}

/* AVL left-rotation procedure, called when the node is unbalanced.
   Preserves the correct order of the elements. */
IntervalTreeNode* IntervalTree::rotateLeft(auto *node) {
    auto root = node->right;
    node->right = root->left;
    root->left = node;
    updateAuxValues(node);
    updateAuxValues(root);
    return root;
}

/* Returns a balanced tree rooted in the provided node. */
IntervalTreeNode* IntervalTree::balanceTree(auto *root) {
    int rootBalance = getHeight(root->left) - getHeight(root->right);
    assert (abs(rootBalance) <= 2);
    
    if (rootBalance == 2) { // Tree is leaning left.
        auto leftChild = root->left;
        int leftChildBalance = getHeight(leftChild->left) - getHeight(leftChild->right);
        if (leftChildBalance < 0) root->left = rotateLeft(leftChild); // double rotation
        return rotateRight(root);
    }
    
    else if (rootBalance == -2) { // Tree is leaning right.
        auto rightChild = root->right;
        int rightChildBalance = getHeight(rightChild->left) - getHeight(rightChild->right);
        if (rightChildBalance > 0) root->right = rotateRight(rightChild); // double rotation
        return rotateLeft(root);
    }
    
    return root;
}

/* Inserts a new [low, high] interval. The tree can store
   multiple intervals that have the same endpoints. */
void IntervalTree::insert(int low, int high) {
    assert (low <= high);
    pair<int,int> addend(low, high);
    root = insertHelper(root, addend);
    nodeCount++;
}

IntervalTreeNode* IntervalTree::insertHelper(auto *node, pair<int,int> &addend) {
    if (node == nullptr) return new IntervalTreeNode(addend);
    
    if (node->range >= addend) node->left = insertHelper(node->left, addend);
    else node->right = insertHelper(node->right, addend);
    
    updateAuxValues(node);
    return balanceTree(node);
}

/* Removes one occurrence of the specified interval.
   Has no effect if there is no such interval. */
void IntervalTree::remove(int low, int high) {
    assert (low <= high);
    pair<int,int> target(low, high);
    root = removeHelper(root, target);
}

IntervalTreeNode* IntervalTree::removeHelper(auto *node, pair<int,int> &target) {
    if (node == nullptr) return node; // not present in the tree
    
    // Walk down the tree, searching for the node to be removed.
    if (node->range > target) {
        node->left = removeHelper(node->left, target);
        updateAuxValues(node);
        return balanceTree(node);
    }
    else if (node->range < target) {
        node->right = removeHelper(node->right, target);
        updateAuxValues(node);
        return balanceTree(node);
    }
    else { // found the target node
        nodeCount--;
        auto leftChild = node->left;
        auto rightChild = node->right;
        
        /* Case #1. Target node has at most one child,
                    replace it with the child subtree. */
        if (leftChild == nullptr || rightChild == nullptr) {
            delete node;
            return leftChild == nullptr ? rightChild : leftChild;
        }
        
        /* Case #2. Target node has both children - substitute
           it with its successor (wrt the order on the keys). */
        auto successor = getMinNode(rightChild);
        successor->right = dropMinNode(rightChild);
        successor->left = leftChild;
        delete node;
        updateAuxValues(successor);
        return balanceTree(successor);
    }
}

/* Helper function that returns the subtree rooted in
   the specified node, but without the smallest key. */
IntervalTreeNode* IntervalTree::dropMinNode(auto *node) {
    if (node->left == nullptr) return node->right;
    node->left = dropMinNode(node->left);
    updateAuxValues(node);
    return balanceTree(node);
}

/* Prints the bracket representation of the tree (for debug purposes) */
void IntervalTree::printTree(ostream &outputStream) {
    printTreeHelper(root, outputStream);
    outputStream << "\n";
}

void IntervalTree::printTreeHelper(auto *node, ostream &outputStream) {
    if(node == nullptr) {
        outputStream << "n"; // nullptr symbol
        return;
    }
    outputStream << "(";
    printTreeHelper(node->left, outputStream);
    outputStream << ",";
    node->printNode(outputStream);
    outputStream << ",";
    printTreeHelper(node->right, outputStream);
    outputStream << ")";
}

