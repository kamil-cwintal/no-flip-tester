#ifndef CONVERTER_H
#define CONVERTER_H

#include <iostream>
#include <vector>
#include <utility>
#include "generators.h"
using std::string;
using std::vector;
using std::pair;


enum IntervalStatus { NOT_SET, FIRST_NODE_SELECTED, SECOND_NODE_SELECTED };

// Singular edge occurrence, interpreted as an interval.
struct Interval {
    unsigned int startTime; // timestamp of Insert
    unsigned int endTime;   // timestamp of Delete
    pair<int,int> nodes;    // edge endpoints
    IntervalStatus status;
    unsigned int score;     // current interval score
    
    /* No two intervals have the same time bounds. */
    bool operator==(const Interval &ref) const {
        return startTime == ref.startTime && endTime == ref.endTime;
    }
    
    bool operator<(const Interval &ref) const {
        return startTime < ref.startTime ||
              (startTime == ref.startTime && endTime < ref.endTime);
    }
    
    int getAssignedNode() const;
    string printInterval() const;
};

struct IntervalProblemInstance {
    const int V;
    const int alpha;
    const int timeframe; // largest timestamp + 1
    vector<Interval> intervals;
    
    // pretty-printer of the entire intervals set
    void printIntervals(std::ostream &outputStream);
};

/* This function is responsible for translating the initial dynamic
   graph orientation problem to interval-based setting. */
IntervalProblemInstance convertInstance(OrientationProblemInstance &opi);

#endif

