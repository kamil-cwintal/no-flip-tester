#include "strategies.h"
#include <cmath>
using std::max;


/* Implementation of Brodal's strategy from the original Brodal and Fagerberg paper
   "Dynamic Representations of Sparse Graphs" (Lemma 3). "outdegBound" is the maximum
   permissible outdegree; for each operation there are at most O(log |V|) flips. */
int orientByBrodalStrategy(OrientationProblemInstance &opi, int outdegBound) {
    assert (opi.alpha == 1); // applicable for forests only
    assert (outdegBound > 1); // Brodal's assumption
    
    const int TIMEFRAME = opi.sequence.size();
    vector<Forest> graphs(TIMEFRAME, Forest(opi.V));
    ForestOrientation orientation(opi.V);
    
    // construct 1-orientation of the last phase
    buildGraphsHistory(opi.sequence, graphs);
    constructOptimalOrientation(graphs.back(), orientation);
    
    // construct previous orientations in decreasing order and count flips
    int totalFlips = 0;
    propagateBack(opi.sequence, orientation, outdegBound, totalFlips);
    return totalFlips;
}

/* Reviews the list of graph operations in reverse chronological order and
   maintains orientations according to Brodal and Fagerberg's construction.
   It also counts performed flips. */
void propagateBack(vector<Command> &sequence, ForestOrientation &orientation,
                   int outdegBound, int &totalFlips) {
    for (int time = sequence.size()-1; time >= 0; time--) {
        OperationType operation = sequence[time].operation;
        int u = sequence[time].nodes.first;
        int v = sequence[time].nodes.second;
        
        if (operation == INSERT) {
            if (orientation.isOriented(u, v)) orientation.removeEdge(u, v);
            else orientation.removeEdge(v, u);
        }
        
        else { // the DELETE operation
            if (orientation.getOutdegree(u) < outdegBound) {
                orientation.orientEdge(u, v); // insert edge safely
            }
            else { // outdegree of u is equal to outdegBound
                flipOnShortPath(orientation, u, outdegBound, totalFlips);
                orientation.orientEdge(u, v);
            }
        }
    }
}

/* Searches for an at-most-logarithmic-length path from vertex "startNode"
   to some vertex v with an outdegree less than "outdegBound" and reverses
   the direction of edges on the path from startNode to v. Such a path
   is guaranteed to exist. */
void flipOnShortPath(ForestOrientation &orientation, int startNode,
                     int outdegBound, int &totalFlips) {
    
    const int V = orientation.getV();
    const int LIMIT = ceil(log2(V) / log2(outdegBound));
    vector<bool> visited(V, false);
    vector<int> currentPath { startNode };
    vector<int> foundPath;
    
    // Walk from startNode in search of v.
    seekShortPath(startNode, LIMIT, outdegBound, visited,
                  orientation, currentPath, foundPath);
    
    // Assert there is such a path.
    assert (!foundPath.empty());
    
    // Flip edges along the path.
    for (int p = 1; p < foundPath.size(); p++) {
        orientation.flipEdge(foundPath[p-1], foundPath[p]);
    }
    totalFlips += foundPath.size() - 1;
}

/* Performs a DFS search for a short path to flip. */
void seekShortPath(int v, int distanceLeft, int outdegBound, vector<bool> &visited,
                   ForestOrientation &orientation, auto &currentPath, auto &foundPath) {
    visited[v] = true;
    
    if (orientation.getOutdegree(v) < outdegBound) {
        // goal found, but prefer shorter paths
        if (foundPath.empty()) foundPath = currentPath;
        else if (currentPath.size() < foundPath.size()) foundPath = currentPath;
    }
    
    if (distanceLeft > 0) { // otherwise, the path would be too long
        for (int neighbour : orientation.getOutNeighbours(v)) {
            if (!visited[neighbour]) {
                currentPath.push_back(neighbour);
                seekShortPath(neighbour, distanceLeft-1, outdegBound, visited,
                              orientation, currentPath, foundPath);
                currentPath.pop_back();
            }
        }
    }
}

/* Implementation of Kowalik's offline orientation strategy. This strategy introduces
   no edge reorientations, however, the bound of maximal outdegree is logarithmic.
   The returned value is the largest outdegree that appears in the dynamic graph. */
int orientByKowalikStrategy(OrientationProblemInstance &opi) {
    assert (opi.alpha == 1); // applicable for forests only
    
    const int TIMEFRAME = opi.sequence.size();
    vector<Forest> graphs(TIMEFRAME, Forest(opi.V));
    vector<ForestOrientation> orientations(TIMEFRAME, ForestOrientation(opi.V));
    
    buildGraphsHistory(opi.sequence, graphs);
    constructOrientations(orientations, graphs, 0, TIMEFRAME-1);
    
    // assert the resulting sequence of orientations incurs zero flips
    assert (countTotalFlips(orientations) == 0);
    
    int maxOutdegree = getMaxOutdegree(orientations);
    
    // assert the theoretical bound is correct
    assert (maxOutdegree <= floor(log2(TIMEFRAME)) + 1);
    return maxOutdegree;
}

/* Auxiliary function for Brodal's and Kowalik's strategies: populates the graph
   history according to a sequence of graph operations in OrientationProblemInstance. */
void buildGraphsHistory(vector<Command> &sequence,
                        vector<Forest> &graphs) {
    for (int t = 0; t < sequence.size(); t++) {
        // copy previous graph
        auto prevEdges = graphs[max(0, t-1)].getAllEdges();
        for(pair<int,int> &edge : prevEdges) {
            graphs[t].insertEdge(edge.first, edge.second);
        }
        
        OperationType type = sequence[t].operation;
        int u = sequence[t].nodes.first;
        int v = sequence[t].nodes.second;
        
        if (type == INSERT) graphs[t].insertEdge(u, v);
        else graphs[t].deleteEdge(u, v);
    }
}

/* Recursive function that constructs a sequence of orientations
   within time interval [startTime, endTime] according to Kowalik's
   algorithm. The sequence does not incur any edge reorientations
   and the maximum outdegree bound is logarithmic. */
void constructOrientations(auto &orientations, auto &graphs,
                           int startTime, int endTime) {
    
    // base case: perform arbitrary 1-orientation
    if (startTime == endTime) {
        constructOptimalOrientation(graphs[startTime], orientations[startTime]);
        return;
    }
    
    // for larger sequences, split recursively
    int midTime = startTime + (endTime - startTime + 1) / 2;
    constructOrientations(orientations, graphs, startTime, midTime - 1);
    if (midTime + 1 <= endTime) {
        constructOrientations(orientations, graphs, midTime + 1, endTime);
    }
    
    // combine both halves into a larger orientation sequence
    constructOptimalOrientation(graphs[midTime], orientations[midTime]);
    for (pair<int,int> &edge : orientations[midTime].getAllEdges()) {
        int from = edge.first;
        int to = edge.second;
        for (int time = startTime; time <= endTime; time++) {
            if (orientations[time].isOriented(to, from)) {
                orientations[time].flipEdge(to, from);
            }
        }
    }
}

/* Orients every edge towards a root (vertex numbered 0)
   which results in an optimal 1-orientation of the provided
   Forest object. */
void constructOptimalOrientation(Forest &forest,
                                 ForestOrientation &orientation) {
    vector<vector<int>> edges(forest.getV(), vector<int>());
    vector<bool> visited(forest.getV(), false);
    
    for (pair<int,int> &edge : forest.getAllEdges()) {
        int u = edge.first;
        int v = edge.second;
        edges[u].push_back(v);
        edges[v].push_back(u);
    }
    
    for (int root = 0; root < forest.getV(); root++) {
        if (!visited[root]) forestTraversal(root, visited, edges, orientation);
    }
}

/* Performs a DFS traversal of a forest.
   It also orients all edges towards the root. */
void forestTraversal(int v, vector<bool> &visited, vector<vector<int>> &edges,
                     ForestOrientation &orientation) {
    visited[v] = true;
    for (int neighbour : edges[v]) {
        if (!visited[neighbour]) {
            forestTraversal(neighbour, visited, edges, orientation);
            orientation.orientEdge(neighbour, v);
        }
    }
}

/* Counts edge flips between two provided orientations. */
int countFlipsBetween(ForestOrientation &o1, ForestOrientation &o2) {
    int flipsCount = 0;
    for (pair<int,int> &edge : o1.getAllEdges()) {
        if (o2.isOriented(edge.second, edge.first)) flipsCount++;
    }
    return flipsCount;
}

/* Counts the total number of flips over the entire sequence of operations. */
int countTotalFlips(vector<ForestOrientation> &orientations) {
    int totalFlips = 0;
    for (int time = 1; time < orientations.size(); time++) {
        totalFlips += countFlipsBetween(orientations[time-1], orientations[time]);
    }
    return totalFlips;
}

/* Returns the largest outdegree that appears over the sequence of operations. */
int getMaxOutdegree(vector<ForestOrientation> &orientations) {
    int maxOutdeg = 0;
    for (ForestOrientation &orientation : orientations) {
        maxOutdeg = max(maxOutdeg, orientation.getMaxOutdegree());
    }
    return maxOutdeg;
}
