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
    opts.maxIterations = 2 * g.bombs.size();
    opts.scoreMemorySize = (int)((float)g.bombs.size() * 0.2);
    int score = errorScore(g);
    lahcFill(g, opts);
    int endScore = errorScore(g);
    dumpGraph(g);
    cout << "---" << endl;
    cout << "Initial score: " << score << endl;
    cout << "Final score: " << endScore << endl;
    return 0;
}