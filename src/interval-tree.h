#ifndef INTERVAL_TREE_H
#define INTERVAL_TREE_H

#include <iostream>
#include <vector>
#include <utility>
using std::ostream;
using std::vector;
using std::pair;


struct IntervalTreeNode {
    pair<int,int> range;            // describes the [low, high] interval
    int highest;                    // maximum "high" value in the subtree
    unsigned int height;            // node count on the longest root-leaf path
    IntervalTreeNode *left, *right; // pointers to subtrees
    
    IntervalTreeNode(pair<int,int> &range) : range(range), highest(getHigh()),
        height(1), left(nullptr), right(nullptr) {};
    
    int getLow() { return range.first; }
    int getHigh() { return range.second; }
    
    // Succinct text representation of the node.
    void printNode(ostream &outputStream);
};

/* Interval tree structure, implemented as an augmented AVL tree,
   as described in "Introduction to Algorithms" by Cormen et al. */
class IntervalTree {
    private:
        IntervalTreeNode *root;
        unsigned int nodeCount;
    
    public:
        IntervalTree() : root(nullptr), nodeCount(0) {}
        
        ~IntervalTree() {
            clearTree(root);
        }
        
        unsigned int getIntervalCount() {
            return nodeCount;
        }
        
        /* Searches for the specified interval in the tree. */
        bool contains(int low, int high) const;
        
        /* Returns all intervals stored in the tree that overlap
           with the query interval. */
        using IntervalList = vector<pair<int,int>>;
        IntervalList getClashes(int low, int high) const;
        
        /* Returns the number of intervals stored in the tree
           that overlap with the query interval. */
        int countClashes(int low, int high) const;
        
        /* Inserts a new [low, high] interval. The tree can store
           multiple intervals that have the same endpoints. */
        void insert(int low, int high);
        
        /* Removes one occurrence of the specified interval.
           Has no effect if there is no such interval. */
        void remove(int low, int high);
        
        /* Prints the bracket representation of the tree (for debug purposes) */
        void printTree(ostream &outputStream);
    
    private:
        void clearTree(IntervalTreeNode *node);
        IntervalTreeNode* getMinNode(auto *node) const;
        IntervalTreeNode* getMaxNode(auto *node) const;
        IntervalTreeNode* search(auto *node, pair<int,int> &target) const;
        
        bool areIntervalsClashing(auto &intA, auto &intB) const;
        void collectClashes(auto *node, pair<int,int> &query, IntervalList &result) const;
        
        int getHighest(const auto *node) const;
        int getHeight(const auto *node) const;
        void updateAuxValues(auto *node);
        IntervalTreeNode* rotateRight(auto *node);
        IntervalTreeNode* rotateLeft(auto *node);
        IntervalTreeNode* balanceTree(auto *root);
        
        IntervalTreeNode* insertHelper(auto *node, pair<int,int> &addend);
        IntervalTreeNode* removeHelper(auto *node, pair<int,int> &target);
        IntervalTreeNode* dropMinNode(auto *node);
        
        void printTreeHelper(auto *node, ostream &outputStream);
};

#endif

