#ifndef GENERATORS_H
#define GENERATORS_H

#include <iostream>
#include <vector>
#include <random>
#include "graphs.h"
using std::string;
using std::vector;
using std::pair;
using std::random_device;


enum OperationType { INSERT, DELETE };

struct Command {
    OperationType operation;
    pair<int,int> nodes;
    
    string printCommand();
};

struct OrientationProblemInstance {
    const int V;
    const int alpha;
    vector<Command> sequence;
    
    // pretty-printer of the entire operation sequence
    void printSequence(std::ostream &outputStream);
};

/* Creates various instances of the graph orientation problem.
   The Generator inserts a new edge with certain probability, otherwise
   removes one existing edge. To prevent too many edges in the graph,
   a purge phase may be triggered (continuous sequence of deletions) */
class Generator {
    
    protected:
        const int V;
        const int alpha; // arboricity upper bound
        std::mt19937 engine; // Mersenne twister engine
    
    public:
        Generator(int V, int alpha, random_device &rd);
        
        void setSeed(long long seed);
        
        // "sequenceLen" is the desired number of operations
        OrientationProblemInstance generateInstance(int sequenceLen);
        
        /* Every Generator needs to provide custom implementations
            of the "insertRandomEdge" and "deleteRandomEdge" methods. */
        virtual pair<int,int> insertRandomEdge(BoundedArbGraph&) = 0;
        virtual pair<int,int> deleteRandomEdge(BoundedArbGraph&) = 0;
        
        /* Edge insertion and purge probabilities may also depend
           on the current graph structure. */
        virtual float getInsertProbability(BoundedArbGraph&) = 0;
        virtual float getPurgeProbability(BoundedArbGraph&) = 0;
};

/* Both new edge endpoints are sampled from a uniform distribution.
   Similarly, every edge in the graph has equal probability to be deleted.
   edgeDensity is the expected fraction of edges present in the graph.
   The purge probability (purgeProb) is constant. */
class UniformDistrGenerator : public Generator {
    private:
        const float edgeDensity;
        const float purgeProb;
    
    public:
        UniformDistrGenerator(int V, int alpha, random_device &rd,
        float edgeDensity, float purgeProb) :
            Generator(V, alpha, rd), edgeDensity(edgeDensity), purgeProb(purgeProb) {}
    
    private:
        pair<int,int> insertRandomEdge(BoundedArbGraph&);
        pair<int,int> deleteRandomEdge(BoundedArbGraph&);
        float getInsertProbability(BoundedArbGraph&);
        float getPurgeProbability(BoundedArbGraph&) { return purgeProb; }
};

/* Uses a geometric distribution to select one of the endpoints.
   In expectation, this strategy should result in several nodes
   with high degrees. */
class GeomDistrGenerator : public Generator {
    private:
        const float edgeDensity;
        const float purgeProb;
        const float succProb; // distribution parameter (success probability)
    
    public:
        GeomDistrGenerator(int V, int alpha, random_device &rd,
        float edgeDensity, float purgeProb, float succProb) :
            Generator(V, alpha, rd), edgeDensity(edgeDensity),
            purgeProb(purgeProb), succProb(succProb) {}
    
    private:
        pair<int,int> insertRandomEdge(BoundedArbGraph&);
        pair<int,int> deleteRandomEdge(BoundedArbGraph&);
        float getInsertProbability(BoundedArbGraph&);
        float getPurgeProbability(BoundedArbGraph&) { return purgeProb; }
};

#endif

