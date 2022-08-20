#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include "graphs.h"
#include "generators.h"
#include "converter.h"
#include "solver.h"
#include "strategies.h"
using std::cout;
using std::ofstream;
using std::setprecision;
using namespace std::chrono;


// Provides seed for the problem instance generator.
long long getMillisSinceEpoch();

// Saves a counterexample problem instance for further reference.
void saveInstanceDescription(IntervalProblemInstance &ipi);

int main(int argc, char *argv[]) {
    
    const int NODES = 40;             // number of nodes in the graph
    const int ALPHA = 1;              // upper bound for arboricity
    const int MAX_OUTDEG = 2;         // largest allowable outdegree
    const float EDGE_DENSITY = 0.8;   // expected fraction of possible edges
    const float PURGE_PROB = 0.00;    // purge probability (see Generator class)
    const int INSTANCE_LEN = 1000;    // number of Insert/Delete operations
    random_device rd {};              // initialized with random seed
    
    const int ATTEMPTS_TARGET = 100;  // total number of generated instances
    const int STATS_CHECKPOINT = 10;  // print statistics after STATS_CHECKPOINT attempts
    
    cout << "Launched testing: \n";
    cout << "|V| = " << NODES << ", ";
    cout << "arboricity <= " << ALPHA << ", ";
    cout << "instance length = " << INSTANCE_LEN << "\n\n";
    
    double avgKowalik = 0;
    double avgCustom = 0;
    
    UniformDistrGenerator gen(NODES, ALPHA, rd, EDGE_DENSITY, PURGE_PROB);
    gen.setSeed(getMillisSinceEpoch());
    
    for (int attempt = 1; attempt <= ATTEMPTS_TARGET; attempt++) {
        
        OrientationProblemInstance opi = gen.generateInstance(INSTANCE_LEN);
        avgKowalik += orientByKowalikStrategy(opi);
        IntervalProblemInstance ipi = convertInstance(opi);
        
        int maxOutdegCustomStrategy = 0;
        solveInstance(ipi, MAX_OUTDEG, maxOutdegCustomStrategy);
        avgCustom += maxOutdegCustomStrategy;
        
        if (attempt % STATS_CHECKPOINT == 0) {
            // Print current statistics.
            cout << attempt << " / " << ATTEMPTS_TARGET << " attempts done." << "\n";
            cout << "Avg. Kowalik outdeg: " << setprecision(6) << avgKowalik / attempt << "\n";
            cout << "Avg. custom outdeg: " << setprecision(6) << avgCustom / attempt << "\n\n";
        }
    }
}

long long getMillisSinceEpoch() {
    auto elapsedTime = system_clock::now().time_since_epoch();
    return duration_cast<milliseconds>(elapsedTime).count();
}

void saveInstanceDescription(IntervalProblemInstance &ipi) {
    ofstream targetFile("error-instance.txt");
    ipi.printIntervals(targetFile);
    targetFile.close();
}

