#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include "representation.hpp"
#include "optimization.hpp"

using namespace std;

int main() {
    Board b;
    // From stdin fill up the board
    bool first = true;
    while (true) {
        string line;
        if (!getline(cin, line)) break;
        // Assume empty lines are not part of the board
        if(line.empty()) continue;
        if(first) {
            first = false;
            b.width = line.size();
        } else {
            if (line.size() != b.width) {
                cerr << "Inconsistent row width! Line: " << line << endl;
                return 1;
            }
        }
        b.field.push_back(line);
    }
    b.height = b.field.size();
    Graph g = fromBoard(b);
    LahcOptions opts;
    // Scale iterations with number of bombs
    // Each bomb has k iterations
    int k = 50;
    float toMemory = 0.25f;
    opts.maxIterations = k * g.bombs.size();
    opts.scoreMemorySize = (int)((float)g.bombs.size() * (float)k * toMemory);
    lahcFill(g, opts);
    int endScore = errorScore(g);
    dumpGraph(g);
    cout << "---" << endl;
    // Graph e = fromBoard(b);
    // basicFill(e);
    // int naiveScore = errorScore(e);
    // cout << "Naive score: " << naiveScore << endl;
    cout << "LAHC score: " << endScore << endl;
    return 0;
}