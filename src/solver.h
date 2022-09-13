#ifndef SOLVER_H
#define SOLVER_H

#include "converter.h"
#include "interval-tree.h"
#include "segment-tree.h"
#include "segment-tree.cpp"
#include <cstddef>
#include <vector>
#include <set>
using std::vector;
using std::set;


/* Efficient implementation of the IntervalProblemInstance
   solving algorithm (Adaptive Minimize Collisions). During
   the process, all interval statuses are set to either
   FIRST_NODE_SELECTED or SECOND_NODE_SELECTED.
   "maxOutdegree" denotes the largest outdegree that appeared. */
void solveInstance(IntervalProblemInstance &ipi, int &maxOutdegree);

/* Collects all intervals into a vector of interval trees. Each vertex
   has a separate tree with intervals for which it can be selected. */
vector<IntervalTree> buildEmptyIntervalTrees(const IntervalProblemInstance &ipi);
vector<IntervalTree> buildFullIntervalTrees(const IntervalProblemInstance &ipi);

/* Let "outdeg" be an OutdegManager object. Then, outdeg[v][t]
   denotes the current outdegree of vertex v at time t. */
using OutdegManager = vector<SegmentTreePlusMax<uint8_t>>;
OutdegManager buildOutdegManager(const IntervalProblemInstance &ipi);

/* Compares intervals with respect to their time bounds (startTime, endTime).
   Note that no two intervals can have the exact same timestamps. */
auto TimeBoundsComparator = [](const Interval *intA, const Interval *intB) {
    return (*intA) < (*intB);
};

/* Compares intervals according to their current score (highest score first).
   Time bounds comparison serves as a tiebreaker. */
auto ScoreComparator = [](const Interval *intA, const Interval *intB) {
    return intA->score > intB->score ||
          (intA->score == intB->score && (*intA) < (*intB));
};

/* The IntervalDict structure is introduced for efficient lookup. */
using IntervalDict = set<Interval*, decltype(TimeBoundsComparator)>;
IntervalDict constructIntervalDict(IntervalProblemInstance &ipi);

/* Allows for interval lookup by time bounds. */
Interval* findIntervalWithTimeBounds(int startTime, int endTime,
                                     const IntervalDict &dict);

#endif
