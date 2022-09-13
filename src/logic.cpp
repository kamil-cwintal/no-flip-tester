#include "logic.h"
#include <string>
#include <set>
#include <cmath>
#include <cassert>
#include <algorithm>
using std::to_string;
using std::min;
using std::max;
using std::set;
using std::multiset;
using std::set_difference;


string Literal::printLiteral() {
    if (polarity == POSITIVE) return "x" + to_string(index);
    else return "~x" + to_string(index);
}

/* Pretty-printer of a Valuation object. */
void printValuation(Valuation &val, ostream &outputStream) {
    for (auto const &entry : val) {
        VarIndex index = entry.first;
        bool value = entry.second;
        
        outputStream << "x" << index << " -> ";
        outputStream << (value ? "true" : "false");
        outputStream << "\n";
    }
}

/* add a clause to the formula */
void Formula::addClause(Clause &clause) {
    formula.push_back(clause);
}

/* returns total number of literals in the formula */
unsigned int Formula::literalsCount() {
    unsigned int totalLiterals = 0;
    for (Clause &clause : formula) {
        totalLiterals += clause.size();
    }
    return totalLiterals;
}

/* first step of the Davis-Putnam algorithm: satisfies clauses
   with a single unassigned literal. The valuation is modified,
   while the underlying formula is not. */
void Formula::unitPropagation(Valuation &val) {
    for (Clause &clause : formula) {
        if (clause.size() == 1) { // single literal
            Polarity p = clause[0].polarity;
            VarIndex index = clause[0].index;
            
            // make sure the literal is unassigned
            if (val.count(index)) continue;
            
            bool satisfyingValue = (p == POSITIVE);
            val[index] = satisfyingValue;
        }
    }
}

/* second step of the Davis-Putnam algorithm: seeks literals
   with a single polarity and assigns them appropriately.
   The valuation is modified, the underlying formula is not. */
void Formula::pureLiteralPropagation(Valuation &val) {
    set<VarIndex> positives; // variables with a positive literal
    set<VarIndex> negatives; // variables with a negative literal
    
    for (Clause &clause : formula) {
        for (Literal &lit : clause) {
            VarIndex index = lit.index;
            if (lit.polarity == POSITIVE) positives.insert(index);
            else negatives.insert(index);
        }
    }
    
    // check for pure positive literals
    for (VarIndex p : positives) {
        if (!negatives.count(p) && !val.count(p)) val[p] = true;
    }
    
    // check for pure negative literals
    for (VarIndex n : negatives) {
        if (!positives.count(n) && !val.count(n)) val[n] = false;
    }
}

/* single simplifying step - returns satisfiability verdict */
Verdict Formula::simplifyOnce(Valuation &val) {
    
    unitPropagation(val);
    pureLiteralPropagation(val); // extend valuation (if possible)
    
    // new formula built as a separate object
    vector<Clause> nextFormula;
    
    for (Clause &clause : formula) {
        Clause nextClause;
        bool clauseSatisfied = false;
        
        for (Literal &lit : clause) {
            // unpack current literal
            Polarity plr = lit.polarity;
            VarIndex index = lit.index;
            
            if (!val.count(index)) nextClause.push_back(lit);
            else if ((plr == POSITIVE && val[index]) ||
                     (plr == NEGATIVE && !val[index])) {
                // mark clause as satisfied
                clauseSatisfied = true;
                break;
            }
        }
        
        if (!clauseSatisfied) {
            if (nextClause.empty()) return UNSATISFIABLE;
            else nextFormula.push_back(nextClause);
        }
    }
    
    if (nextFormula.empty()) return SATISFIABLE;
    else {
        formula = nextFormula;
        return NO_VERDICT;
    }
}

/* iterative formula simplification to a fixpoint
   if the returned verdict is either SATISFIABLE or UNSATISFIABLE,
   the underlying formula is guaranteed to be empty. */
Verdict Formula::simplify(Valuation &val) {
    while (true) {
        unsigned int beforeSize = literalsCount();
        Verdict verdict = simplifyOnce(val);
        if (verdict != NO_VERDICT) {
            if (verdict == UNSATISFIABLE) val.clear();
            formula.clear();
            return verdict;
        }
        
        unsigned int afterSize = literalsCount();
        if (beforeSize > afterSize) continue;
        else return NO_VERDICT;
    }
}

/* heuristic to choose a branching literal (Jeroslow-Wang rule) */
VarIndex Formula::getBestBranch() {
    map<VarIndex, double> scores;
    
    for (Clause &clause : formula) {
        unsigned int length = clause.size();
        for (Literal &lit : clause) {
            scores[lit.index] += 1 / pow(2, length);
        }
    }
    
    VarIndex bestBranch = 0;
    for (auto const &s : scores) {
        VarIndex currentBranch = s.first;
        if (scores[currentBranch] > scores[bestBranch]) {
            bestBranch = currentBranch;
        }
    }
    
    return bestBranch;
}

/* A helper function that branches recursively until all possible valuations
   are tested. If vrd is SATISFIABLE, the satisfying valuation is stored
   and propagated up the search tree in the satisfVal parameter. */
void Formula::solveDPHelper(Valuation &currentVal, Verdict &vrd, Valuation &satisfVal) {
    Verdict infer = simplify(currentVal);
    if (infer == UNSATISFIABLE) return;
    else if (infer == SATISFIABLE) {
        // store the satisfying valuation
        vrd = SATISFIABLE;
        satisfVal = currentVal;
        return;
    }
    
    // find best variable to branch further
    VarIndex branch = getBestBranch();
    vector<Clause> formulaCopy = formula;
    
    // TRUE branch
    currentVal[branch] = true;
    solveDPHelper(currentVal, vrd, satisfVal);
    if (vrd == SATISFIABLE) return; // prune search tree on success
    
    formula = formulaCopy;
    currentVal.erase(branch);
    
    // FALSE branch
    currentVal[branch] = false;
    solveDPHelper(currentVal, vrd, satisfVal);
}

/* Does there exist a certain valuation which satisfies the formula?
   If the verdict is SATISFIABLE, then the Valuation object contains
   a satisfying valuation, otherwise it is empty. Note the formula
   gets erased after calling this function. */
Verdict Formula::solveDP(Valuation &val) {
    Verdict verdict = UNSATISFIABLE;
    Valuation satisfVal;
    
    solveDPHelper(val, verdict, satisfVal);
    formula.clear();
    
    if (verdict == UNSATISFIABLE) val.clear();
    return verdict;
}

/* Pretty-printer of a Formula object. */
void Formula::printFormula(ostream &outputStream) {
    for (Clause &clause : formula) {
        for (int p = 0; p < clause.size(); p++) {
            outputStream << clause[p].printLiteral();
            if (p+1 < clause.size()) outputStream << " V ";
        }
        if (clause.empty()) outputStream << "(empty clause)";
        outputStream << "\n";
    }
}

/* Constructs a CNF formula satisfiable iff there exists a solution
   of the IntervalProblemInstance representing a graph orientation
   where each vertex has an outdegree of at most outdegBound. */
Formula convertToSAT(IntervalProblemInstance &ipi, int outdegBound) {
    Formula phi; // reduction result in CNF form
    vector<vector<Interval>::iterator> currentPath; // picked intervals
    pair<int,int> currentTimespan(0, ipi.timeframe); // common timespan of picked intervals
    
    convertToSATHelper(currentPath, currentTimespan, ipi.intervals.begin(),
                       ipi.intervals, outdegBound+1, phi);
    
    return phi;
}

/* recursive check of all possible clauses; prunes search tree for efficiency */
void convertToSATHelper(auto &currentPath, pair<int,int> currentTimespan,
                        auto startIterator, vector<Interval> &intervals,
                        int stepsLeft, Formula &builtFormula) {
    if (stepsLeft == 0) { // new clause found
        Clause newClause = buildClause(currentPath, intervals.begin());
        builtFormula.addClause(newClause);
        return;
    }
    
    for (auto iter = startIterator; iter != intervals.end(); ++iter) {
        // check for timespan intersection
        int newStartTime = max(currentTimespan.first, (int)iter->startTime);
        int newEndTime = min(currentTimespan.second, (int)iter->endTime);
        if (newStartTime > newEndTime) continue;
        
        // check for common node
        bool commonNodeExists;
        if (currentPath.empty()) commonNodeExists = true;
        else if (currentPath.size() == 1) {
            commonNodeExists = (currentPath[0]->nodes.first == iter->nodes.first ||
                                currentPath[0]->nodes.first == iter->nodes.second ||
                                currentPath[0]->nodes.second == iter->nodes.first ||
                                currentPath[0]->nodes.second == iter->nodes.second);
        } else {
            // shared node is uniquely determined
            int commonNode = getCommonNode(currentPath);
            commonNodeExists = (iter->nodes.first == commonNode ||
                                iter->nodes.second == commonNode);
        }
        
        if (!commonNodeExists) continue;
        
        currentPath.push_back(iter);
        convertToSATHelper(currentPath, pair<int,int>(newStartTime, newEndTime),
                           iter + 1, intervals, stepsLeft - 1, builtFormula);
        currentPath.pop_back();
    }
}

/* returns a node shared by all visited intervals */
int getCommonNode(vector<vector<Interval>::iterator> path) {
    assert (!path.empty());
    multiset<int> occurrences;
    
    for (auto intv : path) {
        occurrences.insert(intv->nodes.first);
        occurrences.insert(intv->nodes.second);
    }
    
    for (int x : occurrences) {
        if (occurrences.count(x) == path.size()) return x;
    }
    
    assert (false); // a common node should have existed
}

/* describes visited intervals with a clause */
Clause buildClause(vector<vector<Interval>::iterator> path, auto startIterator) {
    int commonNode = getCommonNode(path);
    Clause newClause;
    
    for (auto intv : path) {
        unsigned int intervalNum = intv - startIterator + 1;
        Polarity plr = (intv->nodes.first == commonNode) ?
            POSITIVE : NEGATIVE;
        newClause.push_back(Literal { plr, intervalNum });
    }
    
    return newClause;
}
