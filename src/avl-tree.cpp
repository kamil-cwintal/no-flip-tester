#include "avl-tree.h"
#include <cassert>
#include <stdexcept>
using std::max;
using std::domain_error;


template <typename ElemT>
void AVLTreeNode<ElemT>::printNode(ostream &outputStream) {
    outputStream << key << ",c=" << count << ",h=" << height;
}

/* Empties the tree and frees allocated memory. */
template <typename ElemT>
void AVLTree<ElemT>::clearTree(auto *node) {
    if (node == nullptr) return;
    if (node->left) clearTree(node->left);
    if (node->right) clearTree(node->right);
    delete node;
}

/* Returns a pointer to the node with the smallest key
   located in the subtree rooted in the provided node. */
template <typename ElemT>
AVLTreeNode<ElemT>* AVLTree<ElemT>::getMinNode(auto *node) {
    if (node->left == nullptr) return node;
    else return getMinNode(node->left);
}

/* Returns a pointer to the node with the largest key
   located in the subtree rooted in the provided node. */
template <typename ElemT>
AVLTreeNode<ElemT>* AVLTree<ElemT>::getMaxNode(auto *node) {
    if (node->right == nullptr) return node;
    else return getMaxNode(node->right);
}

/* Returns the smallest key stored in the tree.
   Throws a domain_error exception when the tree is empty. */
template <typename ElemT>
ElemT AVLTree<ElemT>::getMin() {
    if (root == nullptr) throw domain_error("The tree is empty.");
    else return getMinNode(root)->key;
}

/* Returns the largest key stored in the tree.
   Throws a domain_error exception when the tree is empty. */
template <typename ElemT>
ElemT AVLTree<ElemT>::getMax() {
    if (root == nullptr) throw domain_error("The tree is empty.");
    else return getMaxNode(root)->key;
}

/* Returns the n-th smallest key in the tree. The indexing of keys starts
   at zero. May throw a domain_error when the index is out of bounds. */
template <typename ElemT>
ElemT AVLTree<ElemT>::getNth(unsigned int n) {
    if (nodeCount <= n) throw domain_error("Index out of range.");
    else return searchNth(root, n)->key;
}

/* Returns a pointer to the node with the provided key parameter.
   Might return nullptr if there is no element with such key. */
template <typename ElemT>
AVLTreeNode<ElemT>* AVLTree<ElemT>::search(auto *node, ElemT &key) {
    if (node == nullptr) return nullptr;
    if (node->key == key) return node;
    
    // Continue recursively in one of the subtrees.
    if (node->key > key) return search(node->left, key);
    else return search(node->right, key);
}

/* Returns a pointer to the node containing the n-th smallest key.
   The indexing of keys starts at zero. */
template <typename ElemT>
AVLTreeNode<ElemT>* AVLTree<ElemT>::searchNth(auto *node, unsigned int n) {
    assert (node != nullptr);
    int leftCount = getCount(node->left);
    
    if (leftCount == n) return node;
    // Continue searching down the tree.
    if (leftCount > n) return searchNth(node->left, n);
    else return searchNth(node->right, n-leftCount-1);
}

/* Searches for the specified key in the tree. */
template <typename ElemT>
bool AVLTree<ElemT>::contains(ElemT key) {
    return search(root, key) != nullptr;
}

template <typename ElemT>
int AVLTree<ElemT>::getCount(const auto *node) {
    if (node == nullptr) return 0;
    else return node->count;
}

template <typename ElemT>
int AVLTree<ElemT>::getHeight(const auto *node) {
    if (node == nullptr) return 0;
    else return node->height;
}

/* Updates the auxiliary values in the node, assuming the children
   have already been processed. */
template <typename ElemT>
void AVLTree<ElemT>::updateAuxValues(auto *node) {
    node->count = 1 + getCount(node->left) + getCount(node->right);
    node->height = 1 + max(getHeight(node->left), getHeight(node->right));
}

/* AVL right-rotation procedure, called when the node is unbalanced.
   Preserves the correct order of the elements. */
template <typename ElemT>
AVLTreeNode<ElemT>* AVLTree<ElemT>::rotateRight(auto *node) {
    auto root = node->left;
    node->left = root->right;
    root->right = node;
    updateAuxValues(node);
    updateAuxValues(root);
    return root;
}

/* AVL left-rotation procedure, called when the node is unbalanced.
   Preserves the correct order of the elements. */
template <typename ElemT>
AVLTreeNode<ElemT>* AVLTree<ElemT>::rotateLeft(auto *node) {
    auto root = node->right;
    node->right = root->left;
    root->left = node;
    updateAuxValues(node);
    updateAuxValues(root);
    return root;
}

/* Returns a balanced tree rooted in the provided node. */
template <typename ElemT>
AVLTreeNode<ElemT>* AVLTree<ElemT>::balanceTree(auto *root) {
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

/* Adds a new key, allowing multiple occurrences of the same key. */
template <typename ElemT>
void AVLTree<ElemT>::insert(ElemT key) {
    root = insertHelper(root, key);
    nodeCount++;
}

template <typename ElemT>
AVLTreeNode<ElemT>* AVLTree<ElemT>::insertHelper(auto *node, ElemT &key) {
    if (node == nullptr) return new AVLTreeNode<ElemT>(key);
    
    if (node->key >= key) node->left = insertHelper(node->left, key);
    else node->right = insertHelper(node->right, key);
    
    updateAuxValues(node);
    return balanceTree(node);
}

/* Removes one occurrence of the specified key.
   Has no effect if there is no such key. */
template <typename ElemT>
void AVLTree<ElemT>::remove(ElemT key) {
    root = removeHelper(root, key);
}

template <typename ElemT>
AVLTreeNode<ElemT>* AVLTree<ElemT>::removeHelper(auto *node, ElemT &key) {
    if (node == nullptr) return node; // key not present in the tree
    
    // Walk down the tree, searching for the node to be removed.
    if (node->key > key) {
        node->left = removeHelper(node->left, key);
        updateAuxValues(node);
        return balanceTree(node);
    }
    else if (node->key < key) {
        node->right = removeHelper(node->right, key);
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
template <typename ElemT>
AVLTreeNode<ElemT>* AVLTree<ElemT>::dropMinNode(auto *node) {
    if (node->left == nullptr) return node->right;
    node->left = dropMinNode(node->left);
    updateAuxValues(node);
    return balanceTree(node);
}

/* Returns a vector containing all keys stored in the tree. */
template <typename ElemT>
vector<ElemT> AVLTree<ElemT>::collectKeys() {
    vector<ElemT> result;
    collectKeysHelper(root, result);
    return result; // no copy here (move semantics)
}

template <typename ElemT>
void AVLTree<ElemT>::collectKeysHelper(auto *node, vector<ElemT> &result) {
    if (node == nullptr) return;
    result.push_back(node->key);
    collectKeysHelper(node->left, result);
    collectKeysHelper(node->right, result);
}

/* Prints the bracket representation of the tree (for debug purposes) */
template <typename ElemT>
void AVLTree<ElemT>::printTree(ostream &outputStream) {
    printTreeHelper(root, outputStream);
    outputStream << "\n";
}

template <typename ElemT>
void AVLTree<ElemT>::printTreeHelper(auto *node, ostream &outputStream) {
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

