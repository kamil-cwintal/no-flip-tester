#include "converter.h"
#include <map>
#include <cassert>
using std::to_string;
using std::ostream;
using std::map;


int Interval::getAssignedNode() const {
    assert (status != NOT_SET);
    if (status == FIRST_NODE_SELECTED) return nodes.first;
    else return nodes.second;
}

string Interval::printInterval() const {
    string description = "{" + to_string(nodes.first) + ", " + to_string(nodes.second)
        + "} FROM " + to_string(startTime) + " TO " + to_string(endTime);
    switch (status) {
        case NOT_SET:
            return description + " (NOT SET)" + "\n";
        case FIRST_NODE_SELECTED:
            return description + " (SET " + to_string(nodes.first) + ")" + "\n";
        case SECOND_NODE_SELECTED:
            return description + " (SET " + to_string(nodes.second) + ")" + "\n";
    }
}

// pretty-printer of the entire intervals set
void IntervalProblemInstance::printIntervals(ostream &outputStream) {
    outputStream << "|V| = " << V << ", alpha = " << alpha <<
        ", timeframe = " << timeframe << "\n";
    for (Interval &i : intervals) outputStream << i.printInterval();
}

/* This function is responsible for translating the initial dynamic
   graph orientation problem to interval-based setting. */
IntervalProblemInstance convertInstance(OrientationProblemInstance &opi) {
    const int timeframe = opi.sequence.size() + 1;
    IntervalProblemInstance ipi = {opi.V, opi.alpha, timeframe};
    
    /* Stores matching between edges (represented as pairs of nodes)
       and vectors with timestamps when the edge is considered. */
    map< pair<int,int>, vector<int> > history;
    
    int currentTime = 0;
    for (Command &cmd : opi.sequence) {
        pair<int,int> nodes = cmd.nodes;
        auto nodesIter = history.find(nodes);
        if (nodesIter != history.end()) { // key exists
            vector<int> &timestamps = nodesIter->second;
            timestamps.push_back(currentTime);
        }
        else history.emplace(nodes, vector<int>{currentTime});
        currentTime++;
    }
    
    /* Timestamps at even indices correspond to edge insertions,
       while timestamps at odd indices represent edge deletions. */
    for (auto &elem : history) {
        pair<int,int> nodes = elem.first;
        vector<int> &timestamps = elem.second;
        
        for (int i = 0; i < timestamps.size() / 2; i++) {
            unsigned int startTime = timestamps[2*i];
            unsigned int endTime = timestamps[2*i+1];
            Interval intv = {startTime, endTime, nodes, NOT_SET, 0};
            ipi.intervals.push_back(intv);
        }
        
        // Include edges that remain in the graph after all operations.
        if (timestamps.size() % 2 == 1) {
            unsigned int startTime = timestamps.back(); // last insertion timestamp
            unsigned int endTime = opi.sequence.size(); // "artificial" largest timestamp
            Interval intv = {startTime, endTime, nodes, NOT_SET, 0};
            ipi.intervals.push_back(intv);
        }
    }
    return ipi;
}

