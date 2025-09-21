#pragma once
#include <string>
#include <vector>
#include <unordered_map>
using namespace std;
#define i64 long long

struct Board {
    vector<string> field; // each string is a row
    int width;
    int height;
};

struct Bomb {
    int x;
    int y;
    bool armed; // true if bomb is present in the cell, false otherwise
};

struct Count {
    int x;
    int y;
    unsigned int count; // number of surrounding bombs
    Bomb* neighbors[8] = {nullptr}; // pointers to neighboring bomb nodes
};

struct Graph {
    vector<Count> counts; // all count nodes
    unordered_map<i64, Bomb> bombs; // quick access to bombs
    int width; // original board width
    int height; // original board height
};

i64 bombKey(int x, int y);
// Build a graph from the given board
Graph fromBoard(const Board& board);

// Dump a representation of the graph to stdout
void dumpGraph(const Graph& graph);
