#ifndef GRAPHS_H
#define GRAPHS_H

#include <iostream>
#include <vector>
#include <utility>
#include <set>
#include "avl-tree.h"
#include "avl-tree.cpp"
#include "link-cut-tree.h"
using std::vector;
using std::ostream;
using std::pair;
using std::set;


// undirected graph of arboricity one
class Forest {
    private:
        const int V;
        int edgeCount;
        AVLTree< pair<int,int> > edges;
        LinkCutTrees links;
    
    public:
        Forest(int V) : V(V), edgeCount(0), edges(), links(V) {}
        
        bool isAdjacent(int va, int vb);
        bool insertEdge(int va, int vb); // returns true iff insertion was successful
        void deleteEdge(int va, int vb);
        void printDescription(ostream &outputStream); // description in DOT format
        pair<int,int> getEdge(int index); // edge numbering starts from 0
        vector< pair<int,int> > getAllEdges();
        int getV() { return V; }
        int getEdgeCount() { return edgeCount; }
};


/* bounded arboricity graph (represented as a collection of forests,
   where every edge belongs to one particular forest) */
class BoundedArbGraph {
    private:
        const int V;
        const int alpha; // arboricity upper bound
        vector<Forest> forests;
    
    public:
        BoundedArbGraph(int V, int alpha) :
            V(V),
            alpha(alpha),
            forests(alpha, Forest(V)) {}
        
        bool isAdjacent(int va, int vb);
        bool insertEdge(int forestIndex, int va, int vb); // returns true iff insertion was successful
        void deleteEdge(int va, int vb);
        void printDescription(ostream &outputStream); // description in DOT format
        pair<int,int> getEdge(int index); // edge numbering starts from 0
        int getEdgeCount();
};


/* Auxiliary structure that stores a forest orientation.
   Used in both Brodal and Kowalik's offline strategies. */
class ForestOrientation {
    private:
        const int V;
        vector<int> outdegs; // stores current outdegrees; node numbering starts from 0
        set< pair<int,int> > directions; // pair (u, v) denotes an edge oriented from u to v
        vector< set<int> > revDirections; // allows for reverse lookup
    
    public:
        ForestOrientation(int V) :
            V(V), outdegs(V, 0), revDirections(V, set<int>()) {}
        int getV() { return V; }
        int getOutdegree(int v); // returns the number of edges oriented from v
        int getMaxOutdegree(); // returns the maximum outdegree
        bool isOriented(int va, int vb); // is the edge (va, vb) oriented from va to vb?
        bool contains(int va, int vb); // does the edge (va, vb) exist in the orientation?
        void orientEdge(int from, int to); // insert a new oriented edge
        void removeEdge(int from, int to); // delete provided edge
        void flipEdge(int from, int to); // flip provided edge
        vector<int> getInNeighbours(int v); // returns neighbours towards v
        vector<int> getOutNeighbours(int v); // returns neighbours from v
        vector< pair<int,int> > getAllEdges(); // lists all edges with their orientations
        void printDescription(ostream &outputStream); // description in DOT format
};

#endif

