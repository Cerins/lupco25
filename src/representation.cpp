#include "representation.hpp"
#include <iostream>
using namespace std;

// The hashmap key which represents a bomb at (x, y)
i64 bombKey(int x, int y) {
    return (static_cast<i64>(x) << 32) | static_cast<i64>(y);
}

// Check if coordinates are in range of the board
static bool inRange(const Board& board, int x, int y) {
    return x >= 0 && x < board.width && y >= 0 && y < board.height;
}

// Check if the item at the coordinates is a number
static bool numberAt(const Board& board, int x, int y) {
    if (!inRange(board, x, y)) return false;
    char cell = board.field[y][x];
    return cell >= '0' && cell <= '9';
}

// From a board definition, create a graph representation
Graph fromBoard(const Board& board) {
    Graph graph;
    graph.width = board.width;
    graph.height = board.height;
    // First pass: find bombs and counts
    for (int y = 0; y < board.height; y++) {
        const string& row = board.field[y];
        for (int x = 0; x < board.width; x++) {
            char cell = row[x];
            // If the cell is an X its a bomb, and put it in the bomb map
            if (cell == 'X') {
                i64 key = bombKey(x, y);
                Bomb& bomb = graph.bombs[key]; // inserts if missing
                bomb.x = x;
                bomb.y = y;
                bomb.armed = true;
            } 
            // Otherwise if its a number, create a count
            else if (cell >= '0' && cell <= '9') {
                Count count;
                count.x = x;
                count.y = y;
                count.count = cell - '0';
                // set neighbors
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        if (dx == 0 && dy == 0) continue;
                        int nx = x + dx;
                        int ny = y + dy;
                        if (!inRange(board, nx, ny)) continue;
                        if (numberAt(board, nx, ny)) continue; // skip if neighbor is a number
                        i64 key = bombKey(nx, ny);
                        Bomb& bomb = graph.bombs[key]; // inserts if missing
                        bomb.x = nx;
                        bomb.y = ny;
                        bomb.armed = (board.field[ny][nx] == 'X');
                        int idx = (dy + 1) * 3 + (dx + 1);
                        if (idx > 4) idx--; // skip center
                        count.neighbors[idx] = &bomb;
                    }
                }
                graph.counts.push_back(count);
            }
        }
    }
    return graph;
}

// Outs the graph to the stdout
void dumpGraph(const Graph& graph) {
    for (int y = 0; y < graph.height; y++) {
        for (int x = 0; x < graph.width; x++) {
            i64 key = bombKey(x, y);
            auto it = graph.bombs.find(key);
            if (it != graph.bombs.end()) {
                cout << (it->second.armed ? 'X' : '.');
                continue;
            }
            bool printed = false;
            for (const auto& count : graph.counts) {
                if (count.x == x && count.y == y) {
                    cout << count.count;
                    printed = true;
                    break;
                }
            }
            if (!printed) {
                cout << '.';
            }
        }
        cout << '\n';
    }
}
