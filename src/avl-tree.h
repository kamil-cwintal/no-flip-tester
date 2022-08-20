#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <iostream>
#include <vector>
using std::ostream;
using std::vector;


template <typename ElemT>
struct AVLTreeNode {
    ElemT key;
    unsigned int count;         // total number of nodes in the subtree
    unsigned int height;        // node count on the longest root-leaf path
    AVLTreeNode *left, *right;  // pointers to subtrees
    
    AVLTreeNode(ElemT key) : key(key), count(1), height(1),
        left(nullptr), right(nullptr) {};
    
    // Succinct text representation of the node.
    void printNode(ostream &outputStream);
};

template <typename ElemT>
class AVLTree {
    private:
        AVLTreeNode<ElemT> *root;
        unsigned int nodeCount;
    
    public:
        AVLTree() : root(nullptr), nodeCount(0) {}
        
        ~AVLTree() {
            clearTree(root);
        }
        
        unsigned int getNodeCount() {
            return nodeCount;
        }
        
        /* Returns the smallest key stored in the tree.
           Throws a domain_error exception when the tree is empty. */
        ElemT getMin();
        
        /* Returns the largest key stored in the tree.
           Throws a domain_error exception when the tree is empty. */
        ElemT getMax();
        
        /* Returns the n-th smallest key in the tree. The indexing of keys starts
           at zero. May throw a domain_error when the index is out of bounds. */
        ElemT getNth(unsigned int n);
        
        /* Searches for the specified key in the tree. */
        bool contains(ElemT key);
        
        /* Adds a new key, allowing multiple occurrences of the same key. */
        void insert(ElemT key);
        
        /* Removes one occurrence of the specified key. Has no effect
           if there is no such key. */
        void remove(ElemT key);
        
        /* Returns a vector containing all keys stored in the tree. */
        vector<ElemT> collectKeys();
        
        /* Prints the bracket representation of the tree (for debug purposes) */
        void printTree(ostream &outputStream);
    
    private:
        void clearTree(auto *node);
        AVLTreeNode<ElemT>* getMinNode(auto *node);
        AVLTreeNode<ElemT>* getMaxNode(auto *node);
        AVLTreeNode<ElemT>* search(auto *node, ElemT &key);
        AVLTreeNode<ElemT>* searchNth(auto *node, unsigned int n);
        
        int getCount(const auto *node);
        int getHeight(const auto *node);
        void updateAuxValues(auto *node);
        AVLTreeNode<ElemT>* rotateRight(auto *node);
        AVLTreeNode<ElemT>* rotateLeft(auto *node);
        AVLTreeNode<ElemT>* balanceTree(auto *root);
        
        AVLTreeNode<ElemT>* insertHelper(auto *node, ElemT &key);
        AVLTreeNode<ElemT>* removeHelper(auto *node, ElemT &key);
        AVLTreeNode<ElemT>* dropMinNode(auto *node);
        
        void collectKeysHelper(auto *node, vector<ElemT> &result);
        void printTreeHelper(auto *node, ostream &outputStream);
};

#endif

