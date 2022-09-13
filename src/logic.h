#ifndef LOGIC_H
#define LOGIC_H

#include <iostream>
#include <vector>
#include <map>
#include "converter.h"
using std::ostream;
using std::vector;
using std::string;
using std::map;


using VarIndex = unsigned int;
enum Polarity { POSITIVE, NEGATIVE };

// basic component of logical formulas
struct Literal {
    Polarity polarity;
    VarIndex index; // variable ordering starts at 1
    
    string printLiteral();
};

using Clause = vector<Literal>;

// Valuation stores a mapping between variable indexes and their boolean values.
using Valuation = map<VarIndex, bool>;
void printValuation(Valuation &val, ostream &outputStream);

// satisfiability verdict
enum Verdict { SATISFIABLE, UNSATISFIABLE, NO_VERDICT };

// Representation of a formula in conjunctive normal form (CNF).
class Formula {
    private:
        vector<Clause> formula;     // CNF formula is a collection of clauses
        
        /* first step of the Davis-Putnam algorithm: satisfies clauses
           with a single unassigned literal. The valuation is modified,
           while the underlying formula is not. */
        void unitPropagation(Valuation &val);
        
        /* second step of the Davis-Putnam algorithm: seeks literals
           with a single polarity and assigns them appropriately.
           The valuation is modified, the underlying formula is not. */
        void pureLiteralPropagation(Valuation &val);
        
        /* single simplifying step - returns satisfiability verdict */
        Verdict simplifyOnce(Valuation &val);
        
        /* heuristic to choose a branching literal (Jeroslow-Wang rule) */
        VarIndex getBestBranch();
        
        /* A helper function that branches recursively until all possible valuations
           are tested. If vrd is SATISFIABLE, the satisfying valuation is stored
           and propagated up the search tree in the satisfVal parameter. */
        void solveDPHelper(Valuation &currentVal, Verdict &vrd, Valuation &satisfVal);
    
    public:
        /* add a clause to the formula */
        void addClause(Clause &clause);
        
        /* returns total number of literals in the formula */
        unsigned int literalsCount();
        
        /* iterative formula simplification to a fixpoint
           if the returned verdict is either SATISFIABLE or UNSATISFIABLE,
           the underlying formula is guaranteed to be empty. */
        Verdict simplify(Valuation &val);
        
        /* Does there exist a certain valuation which satisfies the formula?
           If the verdict is SATISFIABLE, then the Valuation object contains
           a satisfying valuation, otherwise it is empty. Note the formula
           gets erased after calling this function. */
        Verdict solveDP(Valuation &val);
        
        /* pretty-printer */
        void printFormula(ostream &outputStream);
};

/* Constructs a CNF formula satisfiable iff there exists a solution
   of the IntervalProblemInstance representing a graph orientation
   where each vertex has an outdegree of at most outdegBound. */
Formula convertToSAT(IntervalProblemInstance &ipi, int outdegBound);

/* recursive check of all possible clauses; prunes search tree for efficiency */
void convertToSATHelper(auto &currentPath, pair<int,int> currentTimespan,
                        auto startIterator, vector<Interval> &intervals,
                        int stepsLeft, Formula &builtFormula);

/* returns a node shared by all visited intervals */
int getCommonNode(vector<vector<Interval>::iterator> path);

/* describes visited intervals with a clause */
Clause buildClause(vector<vector<Interval>::iterator> path, auto startIterator);

#endif
