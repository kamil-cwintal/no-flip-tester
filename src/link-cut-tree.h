#ifndef LINK_CUT_TREE_H
#define LINK_CUT_TREE_H

#include <vector>
using std::vector;

/* Link/cut trees structure implementation. Used mainly during graph generation
   for testing connectivity. Adapted from Bassel Bakr (github.com/Bassel-Bakr) */

struct LinkCutTreeNode {
    int label;
    LinkCutTreeNode *left, *right;
    LinkCutTreeNode *parent;
    bool reversed;
    
    LinkCutTreeNode() = default;
    
    LinkCutTreeNode(int v) : label(v), left(nullptr), right(nullptr),
        parent(nullptr), reversed(false) {}
    
    void push();
    bool isRoot();
};

class LinkCutTrees {
    private:
        vector<LinkCutTreeNode> nodes;
    
    public:
        LinkCutTrees(int V);
        void link(int u, int v);
        void cut(int u, int v);
        bool connected(int u, int v);
    
    private:
        void rotate(LinkCutTreeNode *child);
        void splay(LinkCutTreeNode *child);
        LinkCutTreeNode* access(int v);
        void makeRoot(int v);
};

#endif

