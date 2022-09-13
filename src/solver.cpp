#include "solver.h"
#include <cassert>
#include <set>
#include <utility>
using std::set;
using std::pair;
using std::max;


/* Efficient implementation of the IntervalProblemInstance
   solving algorithm (Adaptive Minimize Collisions). During
   the process, all interval statuses are set to either
   FIRST_NODE_SELECTED or SECOND_NODE_SELECTED.
   "maxOutdegree" denotes the largest outdegree that appeared. */
void solveInstance(IntervalProblemInstance &ipi, int &maxOutdegree) {
    
    vector<IntervalTree> setIntervals = buildEmptyIntervalTrees(ipi);
    vector<IntervalTree> notsetIntervals = buildFullIntervalTrees(ipi);
    OutdegManager outdeg = buildOutdegManager(ipi);
    IntervalDict dict = constructIntervalDict(ipi);
    
    // Initialize a priority queue of unprocessed intervals.
    set<Interval*, decltype(ScoreComparator)> queue(ScoreComparator);
    for (Interval &intv : ipi.intervals) {
        queue.insert(&intv);
    }
    
    while (!queue.empty()) {
        /* Select the interval with the highest score.
           Interval score is defined as the number of clashes
           with intervals that already have a node assigned. */
        Interval *current = *queue.begin();
        assert (current->status == NOT_SET);
        
        notsetIntervals[current->nodes.first]
            .remove(current->startTime, current->endTime);
        notsetIntervals[current->nodes.second]
            .remove(current->startTime, current->endTime);
        
        const int fstCollisions = setIntervals[current->nodes.first]
            .countClashes(current->startTime, current->endTime);
        const int sndCollisions = setIntervals[current->nodes.second]
            .countClashes(current->startTime, current->endTime);
        
        if (fstCollisions > sndCollisions) current->status = SECOND_NODE_SELECTED;
        else current->status = FIRST_NODE_SELECTED;
        
        // Update outdeg manager.
        const int assignedNode = current->getAssignedNode();
        outdeg[assignedNode].insert(current->startTime, current->endTime, +1);
        const int currentMaxOutdegree = outdeg[assignedNode]
            .query(current->startTime, current->endTime);
        
        maxOutdegree = max(currentMaxOutdegree, maxOutdegree);
        
        setIntervals[assignedNode]
            .insert(current->startTime, current->endTime);
		
        /* Increment the score of unprocessed intervals that clash
           with the current interval. */
        auto clashesList = notsetIntervals[assignedNode]
            .getClashes(current->startTime, current->endTime);
        for (pair<int,int> &clash : clashesList) {
            Interval* tag = findIntervalWithTimeBounds(
                clash.first, clash.second, dict);
            queue.erase(tag); // old key is invalidated
            tag->score++;
            queue.insert(tag);
        }
        
        queue.erase(current);
    }
}

/* Collects all intervals into a vector of interval trees. Each vertex
   has a separate tree with intervals for which it can be selected. */
vector<IntervalTree> buildEmptyIntervalTrees(const IntervalProblemInstance &ipi) {
    vector<IntervalTree> intTrees(ipi.V);
    return intTrees;
}

vector<IntervalTree> buildFullIntervalTrees(const IntervalProblemInstance &ipi) {
    vector<IntervalTree> intTrees(ipi.V);
    for (const Interval &intv : ipi.intervals) {
        intTrees[intv.nodes.first].insert(intv.startTime, intv.endTime);
        intTrees[intv.nodes.second].insert(intv.startTime, intv.endTime);
    }
    return intTrees;
}

/* Let "outdeg" be an OutdegManager object. Then, outdeg[v][t]
   denotes the current outdegree of vertex v at time t. */
OutdegManager buildOutdegManager(const IntervalProblemInstance &ipi) {
    OutdegManager manager;
    manager.reserve(ipi.V); // separate segment tree for every vertex
    for (int v = 0; v < ipi.V; v++) {
        manager.emplace_back(ipi.timeframe); // span over timeframe
    }
    return manager;
}

/* The IntervalDict structure is introduced for efficient lookup. */
IntervalDict constructIntervalDict(IntervalProblemInstance &ipi) {
    IntervalDict dict(TimeBoundsComparator);
    for (Interval &intv : ipi.intervals) {
        dict.emplace(&intv);
    }
    return dict;
}

/* Allows for interval lookup by time bounds. */
Interval* findIntervalWithTimeBounds(int startTime, int endTime,
                                     const IntervalDict &dict) {
    /* We take advantage of the fact that IntervalDict uses
       a TimeBoundsComparator for ordering - we simply need
       to search for a mock interval with given time bounds. */
    Interval mock = {static_cast<unsigned int>(startTime),
                     static_cast<unsigned int>(endTime)};
    return *dict.find(&mock);
}
