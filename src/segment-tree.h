#ifndef SEGMENT_TREE_H
#define SEGMENT_TREE_H

#include <iostream>
#include <utility>
#include <limits>
using std::ostream;
using std::pair;
using std::numeric_limits;
using std::min;
using std::max;


template <typename ElemT>
struct SegmentTreeNode {
    ElemT value;                   // cumulative value of the node
    ElemT lazy;                    // not yet propagated down the tree
    pair<int,int> range;           // range endpoints (from, to)
    SegmentTreeNode *left, *right; // pointers to subtrees
    
    SegmentTreeNode(ElemT value, ElemT lazy, pair<int,int> &range) :
        value(value), lazy(lazy), range(range), left(nullptr), right(nullptr) {};
    
    ~SegmentTreeNode() {
        delete left;
        delete right;
    }
    
    void printSegment(ostream &outputStream);
};

/* Segment tree structure with lazy propagation, allowing for insert and query operations.
   The elements in the tree are of parametrized type ElemT. It is necessary to specify
   the update and accumulation binary functions, usually + or max (associativity required).
   The segment tree is dynamic - memory is allocated only for accessed nodes. */
template <typename ElemT>
class SegmentTree {
    private:
        const int size;                // specifies the index range
        SegmentTreeNode<ElemT> *root;  // pointer to root
        const ElemT neutral;           // value of an empty segment
        
        virtual ElemT update(const ElemT&, const ElemT&) = 0;     // called on insert
        virtual ElemT accumulate(const ElemT&, const ElemT&) = 0; // called on query
        
        /* applies the accumulation function multiple times to neutral */
        virtual ElemT multiAccumulate(const int times, const ElemT &value) = 0;
    
    public:
        SegmentTree(int size, ElemT neutral);
        
        ~SegmentTree() {
            delete root; // triggers cascade deletion
        }
        
        void insert(int leftBound, int rightBound, ElemT value);
        ElemT query(int leftBound, int rightBound);
        
        void printTree(ostream &outputStream);
    
    private:
        pair<int,int> getRootRange(int size);
        bool isLeafNode(auto *node);
        int segmentSize(pair<int,int> &range);
        bool contains(auto &segA, auto &segB);
        bool nonemptyOverlap(auto &segA, auto &segB);
        int overlapSize(auto &segA, auto &segB);
        
        void insertHelper(auto *node, pair<int,int> &query, ElemT &value);
        ElemT queryHelper(auto *node, pair<int,int> &query);
        void allocateChildren(auto *node);
        void propagateDown(auto *node);
};

/* (+, +) segment tree */
template<typename ElemT>
class SegmentTreePlusPlus : public SegmentTree<ElemT> {
    
    private:
        ElemT update(const ElemT &x, const ElemT &y) { return x + y; }
        ElemT accumulate(const ElemT &x, const ElemT &y) { return x + y; }
        ElemT multiAccumulate(const int times, const ElemT &value) { return times * value; }
    
    public:
        SegmentTreePlusPlus(int size, ElemT neutral = ElemT {}) :
            SegmentTree<ElemT>(size, neutral) {}
};

/* (+, max) segment tree */
template<typename ElemT>
class SegmentTreePlusMax : public SegmentTree<ElemT> {
    
    private:
        ElemT update(const ElemT &x, const ElemT &y) { return x + y; }
        ElemT accumulate(const ElemT &x, const ElemT &y) { return max(x, y); }
        ElemT multiAccumulate(const int times, const ElemT &value) { return value; }
    
    public:
        SegmentTreePlusMax(int size, ElemT neutral = numeric_limits<ElemT>::min()) :
            SegmentTree<ElemT>(size, neutral) {}
};

#endif

