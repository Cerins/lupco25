#pragma once
#include "representation.hpp"

// Calculate how big an error is in the current graph
int errorScore(const Graph& graph);

// The strategy is the following:
// 1. Go through all counts
// 2. If the neighboring bomb count is less than desired
// Then go through neighboring bombs and arm the unarmed ones until the count is satisfied
void basicFill(Graph& graph);


struct LahcOptions {
    int maxIterations = 10000; // Maximum number of iterations
    int scoreMemorySize = 1000; // How many previous scores to remember
};

void lahcFill(Graph& graph, const LahcOptions& options);