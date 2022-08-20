#include "link-cut-tree.h"
#include <algorithm>
using std::swap;


void LinkCutTreeNode::push() {
    if (reversed) {
        reversed = false;
        swap(left, right);
        if (left) left->reversed ^= true;
        if (right) right->reversed ^= true;
    }
}

bool LinkCutTreeNode::isRoot() {
    return parent == nullptr || (parent->left != this && parent->right != this);
}

LinkCutTrees::LinkCutTrees(int V) {
    nodes.resize(V+1);
    for (int i = 1; i <= V; i++) {
        nodes[i].label = i;
    }
}

void LinkCutTrees::rotate(LinkCutTreeNode *child) {
    auto parent = child->parent;
    auto grand = parent->parent;
    
    if (!parent->isRoot()) {
        (grand->right == parent ? grand->right : grand->left) = child;
    }
    
    parent->push();
    child->push();
    
    if (parent->left == child) {
        parent->left = child->right;
        child->right = parent;
        if (parent->left) parent->left->parent = parent;
    }
    else {
        parent->right = child->left;
        child->left = parent;
        if (parent->right) parent->right->parent = parent;
    }
    
    parent->parent = child;
    child->parent = grand;
}

void LinkCutTrees::splay(LinkCutTreeNode *child) {
    while (!child->isRoot()) {
        auto parent = child->parent;
        auto grand = parent->parent;
        if (!parent->isRoot()) {
            rotate((grand->right == parent) == (parent->right == child) ?
                parent : child);
        }
        rotate(child);
    }
    child->push();
}

LinkCutTreeNode* LinkCutTrees::access(int v) {
    LinkCutTreeNode *last = nullptr;
    LinkCutTreeNode *child = &nodes[v];
    for (LinkCutTreeNode *loc = child; loc; loc = loc->parent) {
        splay(loc);
        loc->right = last;
        last = loc;
    }
    splay(child);
    return last;
}

void LinkCutTrees::makeRoot(int v) {
    access(v);
    auto *child = &nodes[v];
    if (child->left) {
        child->left->reversed ^= true, child->left = nullptr;
    }
}

void LinkCutTrees::link(int u, int v) {
    makeRoot(v);
    LinkCutTreeNode *child = &nodes[v];
    child->parent = &nodes[u];
}

void LinkCutTrees::cut(int u, int v) {
    makeRoot(u);
    access(v);
    if (nodes[v].left) {
        nodes[v].left->parent = nullptr;
        nodes[v].left = nullptr;
    }
}

bool LinkCutTrees::connected(int u, int v) {
    access(u);
    access(v);
    return nodes[u].parent != nullptr;
}

