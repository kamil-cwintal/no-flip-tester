#include "generators.h"
#include <fstream>
using std::min;
using std::swap;
using std::to_string;
using std::make_pair;
using std::uniform_real_distribution;
using std::uniform_int_distribution;
using std::geometric_distribution;


string Command::printCommand() {
    return string(operation == INSERT ? "INSERT" : "DELETE") + " " +
    to_string(nodes.first) + " -- " + to_string(nodes.second) + "\n";
}

// pretty-printer of the entire operation sequence
void OrientationProblemInstance::printSequence(ostream &outputStream) {
    outputStream << "|V| = " << V << ", alpha = " << alpha << "\n";
    for (Command &c : sequence) outputStream << c.printCommand();
}

Generator::Generator(int V, int alpha, random_device &rd) : V(V), alpha(alpha) {
    engine.seed(rd());
}

void Generator::setSeed(long long seed) {
    engine.seed(seed);
}

// "sequenceLen" is the desired number of operations
OrientationProblemInstance Generator::generateInstance(int sequenceLen) {
    
    OrientationProblemInstance instance = {V, alpha};
    BoundedArbGraph graph(V, alpha); // initial empty graph
    uniform_real_distribution<> unit(0, 1);
    int purgeCountdown = 0; // purge phase is active iff purgeCountdown > 0
    
    for (int time = 0; time < sequenceLen; time++) {
        
        OperationType type = unit(engine) < getInsertProbability(graph) ?
            INSERT : DELETE;
        
        // special cases where operation type is enforced
        if (graph.getEdgeCount() == 0) type = INSERT;
        else if (graph.getEdgeCount() == (V-1)*alpha) type = DELETE;
        else if (purgeCountdown > 0) type = DELETE;
        
        pair<int,int> currentEdge;
        if (type == INSERT) currentEdge = insertRandomEdge(graph);
        else if (type == DELETE) currentEdge = deleteRandomEdge(graph);
        Command cmd = {type, currentEdge};
        instance.sequence.push_back(cmd);
        
        bool activatePurge = !purgeCountdown && unit(engine) < getPurgeProbability(graph);
        if (activatePurge) {
            uniform_int_distribution<> purgeLength(0, graph.getEdgeCount() / 2);
            purgeCountdown = purgeLength(engine);
        }
        else if (purgeCountdown > 0) purgeCountdown--;
    }
    return instance;
}

pair<int,int> UniformDistrGenerator::insertRandomEdge(BoundedArbGraph &graph) {
    uniform_int_distribution<> forestDistr(0, alpha-1);
    uniform_int_distribution<> V_Distr(0, V-1);
    int forestIndex, endpointFirst, endpointSecond;
    do {
        forestIndex = forestDistr(engine);
        endpointFirst = V_Distr(engine);
        endpointSecond = V_Distr(engine);
    } while (!graph.insertEdge(forestIndex, endpointFirst, endpointSecond));
    if (endpointFirst > endpointSecond) swap(endpointFirst, endpointSecond);
    return make_pair(endpointFirst, endpointSecond);
}

pair<int,int> UniformDistrGenerator::deleteRandomEdge(BoundedArbGraph &graph) {
    uniform_int_distribution<> E_Distr(0, graph.getEdgeCount()-1);
    int removedEdgeIndex = E_Distr(engine);
    pair<int,int> edgeRemoved = graph.getEdge(removedEdgeIndex);
    graph.deleteEdge(edgeRemoved.first, edgeRemoved.second);
    return edgeRemoved;
}

float UniformDistrGenerator::getInsertProbability(BoundedArbGraph &graph) {
    float currentDensity = graph.getEdgeCount() / (alpha * (V-1.));
    if (currentDensity <= edgeDensity) {
        return 1 - currentDensity / (2 * edgeDensity);
    }
    else return (1 - currentDensity) / (2 - 2 * edgeDensity);
}

pair<int,int> GeomDistrGenerator::insertRandomEdge(BoundedArbGraph &graph) {
    uniform_int_distribution<> forestDistr(0, alpha-1);
    uniform_int_distribution<> Uniform_V_Distr(0, V-1);
    geometric_distribution<> Geom_V_Distr(succProb);
    int forestIndex, endpointFirst, endpointSecond;
    do {
        forestIndex = forestDistr(engine);
        endpointFirst = Uniform_V_Distr(engine);
        endpointSecond = min(Geom_V_Distr(engine), V-1);
        // the entire distribution tail corresponds to the last node
    } while (!graph.insertEdge(forestIndex, endpointFirst, endpointSecond));
    if (endpointFirst > endpointSecond) swap(endpointFirst, endpointSecond);
    return make_pair(endpointFirst, endpointSecond);
}

pair<int,int> GeomDistrGenerator::deleteRandomEdge(BoundedArbGraph &graph) {
    uniform_int_distribution<> E_Distr(0, graph.getEdgeCount()-1);
    int removedEdgeIndex = E_Distr(engine);
    pair<int,int> edgeRemoved = graph.getEdge(removedEdgeIndex);
    graph.deleteEdge(edgeRemoved.first, edgeRemoved.second);
    return edgeRemoved;
}

float GeomDistrGenerator::getInsertProbability(BoundedArbGraph &graph) {
    float currentDensity = graph.getEdgeCount() / (alpha * (V-1.));
    if (currentDensity <= edgeDensity) {
        return 1 - currentDensity / (2 * edgeDensity);
    }
    else return (1 - currentDensity) / (2 - 2 * edgeDensity);
}

