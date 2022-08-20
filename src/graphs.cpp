#include "graphs.h"
#include <algorithm>
#include <cassert>
using std::string;
using std::swap;


bool Forest::isAdjacent(int va, int vb) {
    assert (0 <= va && va < V);
    assert (0 <= vb && vb < V);
    if (va > vb) swap(va, vb);
    return edges.contains(pair<int,int>(va,vb));
}

/* returns true iff insertion was successful
    (edge has not been present in the graph
    and does not produce a cycle) */
bool Forest::insertEdge(int va, int vb) {
    assert (0 <= va && va < V);
    assert (0 <= vb && vb < V);
    if (va > vb) swap(va, vb);
    
    if (va == vb) return false;
    if (links.connected(va, vb)) return false;
    
    edges.insert(pair<int,int>(va,vb));
    links.link(va, vb);
    edgeCount++;
    return true;
}

void Forest::deleteEdge(int va, int vb) {
    assert (0 <= va && va < V);
    assert (0 <= vb && vb < V);
    if (va > vb) swap(va, vb);
    
    if (isAdjacent(va, vb)) {
        edges.remove(pair<int,int>(va,vb));
        links.cut(va, vb);
        edgeCount--;
    }
}

// outputs DOT description (graphviz-friendly format)
void Forest::printDescription(ostream &outputStream) {
    outputStream << "graph {" << "\n";
    outputStream << "  node [margin=0 shape=circle style=filled]" << "\n";
    auto allEdges = edges.collectKeys();
    for (pair<int,int> &edge : allEdges) {
        outputStream << "  " << edge.first << " -- " << edge.second << "\n";
    }
    outputStream << "}" << "\n";
}

/* getEdge(i) returns a pair containing i-th edge endpoints
   (numbering starts from 0) */
pair<int,int> Forest::getEdge(int index) {
    assert (0 <= index && index < edgeCount);
    return edges.getNth(index);
}

vector<pair<int,int>> Forest::getAllEdges() {
    return edges.collectKeys();
}

bool BoundedArbGraph::isAdjacent(int va, int vb) {
    bool adjacent = false;
    for (Forest &f : forests) adjacent |= f.isAdjacent(va, vb);
    return adjacent;
}

/* returns true iff insertion was successful */
bool BoundedArbGraph::insertEdge(int forestIndex, int va, int vb) {
    if (isAdjacent(va, vb)) return false;
    else return forests[forestIndex].insertEdge(va, vb);
}

void BoundedArbGraph::deleteEdge(int va, int vb) {
    for (Forest &f : forests) f.deleteEdge(va, vb);
}

int BoundedArbGraph::getEdgeCount() {
    int totalEdges = 0;
    for (Forest &f : forests) totalEdges += f.getEdgeCount();
    return totalEdges;
}

/* getEdge(i) returns a pair containing i-th edge endpoints
   (numbering starts from 0) */
pair<int,int> BoundedArbGraph::getEdge(int index) {
    assert (0 <= index && index < getEdgeCount());
    for (Forest &f : forests) {
        if (index - f.getEdgeCount() >= 0) index -= f.getEdgeCount();
        else return f.getEdge(index);
    }
}

/* outputs DOT description where each forest has a unique color */
void BoundedArbGraph::printDescription(ostream &outputStream) {
    const vector<string> palette = { "navy", "red", "darkgreen",
    "chocolate", "purple", "dimgray", "black" };
    
    outputStream << "graph {" << "\n";
    outputStream << "  node [margin=0 shape=circle style=filled]" << "\n";
    outputStream << "  edge [penwidth=5]" << "\n";
    
    for (int f = 0; f < forests.size(); f++) {
        string currentColor = palette[f % palette.size()];
        outputStream << "  edge [color=" << currentColor << "]" << "\n";
        auto edges = forests[f].getAllEdges();
        for (pair<int,int> &edge : edges) {
            outputStream << "  " << edge.first << " -- " << edge.second << "\n";
        }
    }
    outputStream << "}" << "\n";
}

/* returns the number of edges oriented from v */
int ForestOrientation::getOutdegree(int v) {
    assert (0 <= v && v < V);
    return outdegs[v];
}

/* returns the maximum outdegree */
int ForestOrientation::getMaxOutdegree() {
    return *max_element(outdegs.begin(), outdegs.end());
}

/* is the edge (va, vb) oriented from va to vb? */
bool ForestOrientation::isOriented(int va, int vb) {
    assert (0 <= va && va < V);
    assert (0 <= vb && vb < V);
    return directions.count(pair<int,int>(va, vb)) > 0;
}

/* does the edge (va, vb) exist in the orientation? */
bool ForestOrientation::contains(int va, int vb) {
    return isOriented(va, vb) || isOriented(vb, va);
}

/* insert a new oriented edge */
void ForestOrientation::orientEdge(int from, int to) {
    assert (!contains(from, to));
    
    outdegs[from]++;
    directions.emplace(from, to);
    revDirections[to].insert(from);
}

/* delete provided edge */
void ForestOrientation::removeEdge(int from, int to) {
    assert (isOriented(from, to));
    
    outdegs[from]--;
    directions.erase(pair<int,int>(from, to));
    revDirections[to].erase(from);
}

/* flip provided edge */
void ForestOrientation::flipEdge(int from, int to) {
    removeEdge(from, to);
    orientEdge(to, from);
}

/* returns neighbours towards v */
vector<int> ForestOrientation::getInNeighbours(int v) {
    assert (0 <= v && v < V);
    
    return vector<int>(revDirections[v].begin(), revDirections[v].end());
}

/* returns neighbours from v */
vector<int> ForestOrientation::getOutNeighbours(int v) {
    assert (0 <= v && v < V);
    
    auto lower = directions.lower_bound(pair<int,int>(v,0));
    auto upper = directions.upper_bound(pair<int,int>(v+1,-1));
    
    vector<int> outNeighbours;
    for (auto iter = lower; iter != upper; iter++) {
        outNeighbours.push_back(iter->second);
    }
    return outNeighbours;
}

/* lists all edges with their orientations */
vector<pair<int,int>> ForestOrientation::getAllEdges() {
    return vector<pair<int,int>>(directions.begin(), directions.end());
}

/* description in DOT format */
void ForestOrientation::printDescription(ostream &outputStream) {
    outputStream << "digraph {" << "\n";
    outputStream << "  node [margin=0 shape=circle style=filled]" << "\n";
    for (auto const &edge : directions) {
        outputStream << "  " << edge.first << " -> " << edge.second << "\n";
    }
    outputStream << "}" << "\n";
}

