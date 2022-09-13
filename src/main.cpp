#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include "graphs.h"
#include "generators.h"
#include "converter.h"
#include "solver.h"
#include "logic.h"
#include "strategies.h"
using std::cout;
using std::setprecision;
using namespace std::chrono;


// Provides seed for the problem instance generator.
long long getMillisSinceEpoch();

int main(int argc, char *argv[]) {
    
    const int NODES = 40;             // number of nodes in the graph
    const int ALPHA = 1;              // upper bound for arboricity
    const float EDGE_DENSITY = 0.8;   // expected fraction of possible edges
    const float PURGE_PROB = 0.00;    // purge probability (see Generator class)
    const int INSTANCE_LEN = 1000;    // number of Insert/Delete operations
    random_device rd {};              // initialized with random seed
    
    const int ATTEMPTS_TARGET = 100;  // total number of generated instances
    const int STATS_CHECKPOINT = 10;  // print statistics after STATS_CHECKPOINT attempts
    
    double avgKowalik = 0;
    double avgCustom = 0;
    
    cout << "Launched testing: \n";
    cout << "|V| = " << NODES << ", ";
    cout << "arboricity <= " << ALPHA << ", ";
    cout << "instance length = " << INSTANCE_LEN << "\n\n";
    
    UniformDistrGenerator gen(NODES, ALPHA, rd, EDGE_DENSITY, PURGE_PROB);
    gen.setSeed(getMillisSinceEpoch());
    
    for (int attempt = 1; attempt <= ATTEMPTS_TARGET; attempt++) {
        
        OrientationProblemInstance opi = gen.generateInstance(INSTANCE_LEN);
        IntervalProblemInstance ipi = convertInstance(opi);
        
        // Launch strategy provided by Kowalik.
        avgKowalik += orientByKowalikStrategy(opi);
        
        // Launch user-defined strategy.
        int maxOutdegCustomStrategy = 0;
        solveInstance(ipi, maxOutdegCustomStrategy);
        avgCustom += maxOutdegCustomStrategy;
        
        /* Example usage of SAT-solving capabilities:
         * 
         * const int MAX_OUTDEG = 2; // largest allowable outdegree
         * Formula phi = convertToSAT(ipi, MAX_OUTDEG);
         * Valuation val; // meant to store satisfying valuation
         * Verdict verdict = phi.solveDP(val);
         * if (verdict == SATISFIABLE) cout << "SAT\n";
         */
        
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
