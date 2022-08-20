#ifndef STRATEGIES_H
#define STRATEGIES_H

#include <vector>
#include "graphs.h"
#include "generators.h"


/* Implementation of Brodal's strategy from the original Brodal and Fagerberg paper
   "Dynamic Representations of Sparse Graphs" (Lemma 3). "outdegBound" is the maximum
   permissible outdegree; for each operation there are at most O(log |V|) flips. */
int orientByBrodalStrategy(OrientationProblemInstance &opi, int outdegBound);

/* Implementation of Kowalik's offline orientation strategy. This strategy introduces
   no edge reorientations, however, the bound of maximal outdegree is logarithmic.
   The returned value is the largest outdegree that appears in the dynamic graph. */
int orientByKowalikStrategy(OrientationProblemInstance &opi);

/* Reviews the list of graph operations in reverse chronological order and
   maintains orientations according to Brodal and Fagerberg's construction.
   It also counts performed flips. */
void propagateBack(vector<Command> &sequence, ForestOrientation &orientation,
                   int outdegBound, int &totalFlips);

/* Searches for an at-most-logarithmic-length path from vertex "startNode"
   to some vertex v with an outdegree less than "outdegBound" and reverses
   the direction of edges on the path from startNode to v. Such a path
   is guaranteed to exist. */
void flipOnShortPath(ForestOrientation &orientation, int startNode,
                     int outdegBound, int &totalFlips);

/* Performs a DFS search for a short path to flip. */
void seekShortPath(int v, int distanceLeft, int outdegBound, vector<bool> &visited,
                   ForestOrientation &orientation, auto &currentPath, auto &foundPath);

/* Auxiliary function for Brodal's and Kowalik's strategies: populates the graph
   history according to a sequence of graph operations in OrientationProblemInstance. */
void buildGraphsHistory(vector<Command> &sequence,
                        vector<Forest> &graphs);

/* Recursive function that constructs a sequence of orientations
   within time interval [startTime, endTime] according to Kowalik's
   algorithm. The sequence does not incur any edge reorientations
   and the maximum outdegree bound is logarithmic. */
void constructOrientations(auto &orientations, auto &graphs,
                           int startTime, int endTime);

/* Orients every edge towards a root (vertex numbered 0)
   which results in an optimal 1-orientation of the provided
   Forest object. */
void constructOptimalOrientation(Forest &forest,
                                 ForestOrientation &orientation);

/* Performs a DFS traversal of a forest.
   It also orients all edges towards the root. */
void forestTraversal(int v, vector<bool> &visited, vector<vector<int>> &edges,
                     ForestOrientation &orientation);

/* Counts edge flips between two provided orientations. */
int countFlipsBetween(ForestOrientation &o1, ForestOrientation &o2);

/* Counts the total number of flips over the entire sequence of operations. */
int countTotalFlips(vector<ForestOrientation> &orientations);

/* Returns the largest outdegree that appears over the sequence of operations. */
int getMaxOutdegree(vector<ForestOrientation> &orientations);

#endif

