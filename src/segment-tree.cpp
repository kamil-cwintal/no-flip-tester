#include "segment-tree.h"
#include <cassert>
#include <queue>
using std::queue;
using std::make_pair;


template <typename ElemT>
void SegmentTreeNode<ElemT>::printSegment(ostream &outputStream) {
    outputStream << "[" << range.first << ", " << range.second << "] --> " <<
        "value = " << value << ", lazy = " << lazy << "\n";
}

template <typename ElemT>
SegmentTree<ElemT>::SegmentTree(int size, ElemT neutral) :
    size(size), neutral(neutral) {
    ElemT initialValue = ElemT {};
    ElemT initialLazy = ElemT {};
    pair<int,int> rootRange = getRootRange(size);
    root = new SegmentTreeNode<ElemT>(initialValue, initialLazy, rootRange);
}

template <typename ElemT>
void SegmentTree<ElemT>::insert(int leftBound, int rightBound, ElemT value) {
    assert (0 <= leftBound && leftBound < size);
    assert (0 <= rightBound && rightBound < size);
    assert (leftBound <= rightBound);
    
    pair<int,int> insertSegment = {leftBound, rightBound};
    insertHelper(root, insertSegment, value);
}

template <typename ElemT>
ElemT SegmentTree<ElemT>::query(int leftBound, int rightBound) {
    assert (0 <= leftBound && leftBound < size);
    assert (0 <= rightBound && rightBound < size);
    assert (leftBound <= rightBound);
    
    pair<int,int> querySegment = {leftBound, rightBound};
    return queryHelper(root, querySegment);
}

/* Pretty-printer of the entire segment tree, using breadth-first search. */
template <typename ElemT>
void SegmentTree<ElemT>::printTree(ostream &outputStream) {
    using QueueEntry = pair<SegmentTreeNode<ElemT>*, int>; // node pointer and depth
    queue<QueueEntry> pending; // queue for BFS traversal
    pending.emplace(root, 0);
    
    while (!pending.empty()) {
        auto node = pending.front().first;
        int depth = pending.front().second;
        pending.pop();
        
        // Print indentation and segment info.
        for (int indent = 0; indent < depth; indent++) outputStream << "  ";
        node->printSegment(outputStream);
        
        if (node->left) pending.emplace(node->left, depth+1);
        if (node->right) pending.emplace(node->right, depth+1);
    }
}

/* Returns the root range [0, p-1], where p is the smallest power of two
    larger or equal to declared size. */
template <typename ElemT>
pair<int,int> SegmentTree<ElemT>::getRootRange(int size) {
    int p = 1;
    while (p < size) p *= 2;
    return make_pair(0, p-1);
}

template <typename ElemT>
bool SegmentTree<ElemT>::isLeafNode(auto *node) {
    return node->range.first == node->range.second;
}

template <typename ElemT>
int SegmentTree<ElemT>::segmentSize(pair<int,int> &range) {
    return range.second - range.first + 1;
}

/* Is the segB segment fully contained within the segA segment? */
template <typename ElemT>
bool SegmentTree<ElemT>::contains(auto &segA, auto &segB) {
    return segA.first <= segB.first && segB.second <= segA.second;
}

/* Do the two segments have a nonempty overlap? */
template <typename ElemT>
bool SegmentTree<ElemT>::nonemptyOverlap(auto &segA, auto &segB) {
    return max(segA.first, segB.first) <= min(segA.second, segB.second);
}

template <typename ElemT>
int SegmentTree<ElemT>::overlapSize(auto &segA, auto &segB) {
    return min(segA.second, segB.second) - max(segA.first, segB.first) + 1;
}

template <typename ElemT>
void SegmentTree<ElemT>::insertHelper(auto *node, pair<int,int> &query, ElemT &value) {
    if (contains(query, node->range)) {
        node->lazy = update(node->lazy, value);
        int overlap = overlapSize(query, node->range);
        node->value = update(node->value, multiAccumulate(overlap, value));
    }
    
    else if (nonemptyOverlap(query, node->range)) {
        allocateChildren(node);
        propagateDown(node);
        // Continue further down the tree.
        insertHelper(node->left, query, value);
        insertHelper(node->right, query, value);
        node->value = accumulate(node->left->value, node->right->value);
    }
}

template <typename ElemT>
ElemT SegmentTree<ElemT>::queryHelper(auto *node, pair<int,int> &query) {
    if (contains(query, node->range)) {
        return node->value;
    }
    
    else if (nonemptyOverlap(query, node->range)) {
        allocateChildren(node);
        propagateDown(node);
        return accumulate(queryHelper(node->left, query),
                            queryHelper(node->right, query));
    }
    
    else return neutral; // Node doesn't contribute to result.
}

/* Allocates memory for the left and right child nodes. */
template <typename ElemT>
void SegmentTree<ElemT>::allocateChildren(auto *node) {
    if (node->left && node->right) return; // children already exist
    if (isLeafNode(node)) return;
    
    ElemT initialValue = ElemT {};
    ElemT initialLazy = ElemT {};
    int half = (node->range.second - node->range.first) / 2;
    pair<int,int> leftChildRange = {node->range.first, node->range.first + half};
    pair<int,int> rightChildRange = {node->range.first + half + 1, node->range.second};
    node->left = new SegmentTreeNode<ElemT>(initialValue, initialLazy, leftChildRange);
    node->right = new SegmentTreeNode<ElemT>(initialValue, initialLazy, rightChildRange);
}

/* Propagates lazy updates to the children of the node. */
template <typename ElemT>
void SegmentTree<ElemT>::propagateDown(auto *node) {
    if (!isLeafNode(node)) {
        node->left->lazy = update(node->left->lazy, node->lazy);
        node->right->lazy = update(node->right->lazy, node->lazy);
        
        int childSegmentSize = segmentSize(node->range) / 2;
        node->left->value = update(node->left->value,
                            multiAccumulate(childSegmentSize, node->lazy));
        node->right->value = update(node->right->value,
                             multiAccumulate(childSegmentSize, node->lazy));
    }
    node->lazy = ElemT {}; // reset lazy update counter
}

