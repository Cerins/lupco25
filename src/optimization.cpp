#include <unordered_map>
#include <iostream>
#include <numeric>
#include <random>
#include "optimization.hpp"
#include "representation.hpp"
#include "bitset.hpp"

// The error gets calculated as the sum of 
// all differences between counts expected value and surrounding armed bombs
int errorScore(const Graph& graph) {
    int totalError = 0;
    for (const auto& count : graph.counts) {
        unsigned int armedNeighbors = 0;
        for (const auto& neighbor : count.neighbors) {
            if (neighbor != nullptr && neighbor->armed) {
                armedNeighbors++;
            }
        }
        if (armedNeighbors < count.count) {
            totalError += (count.count - armedNeighbors);
        } else if (armedNeighbors > count.count) {
            totalError += (armedNeighbors - count.count);
        }
    }
    return totalError;
}


// Basic algorithms to fill the graph
// Really inaccurate, but fast
// The idea is that ir arms the neighbors of a count until the count is satisfied
// If its already satisfied, it does nothing
// If it is oversatisfied, it does nothing
// Mostly was done for initial testing
void basicFill(Graph& graph) {
    for (auto& count : graph.counts) {
        unsigned int armedNeighbors = 0;
        for (const auto& neighbor : count.neighbors) {
            if (neighbor != nullptr && neighbor->armed) {
                armedNeighbors++;
            }
        }
        if (armedNeighbors < count.count) {
            for (auto& neighbor : count.neighbors) {
                if (neighbor != nullptr && !neighbor->armed) {
                    neighbor->armed = true;
                    armedNeighbors++;
                    if (armedNeighbors >= count.count) {
                        break;
                    }
                }
            }
        }
    }
}

// Calculate the impact of flipping a mine of the total error score
static int lahcFlipScoreImpact(
    const Graph& graph,
    const BitSet& bitset,
    const int* countNeighborLookup,
    const int* bitsetImpactLookup,
    int flipIndex
) {
    // newError - currentError
    int delta = 0;
    const int rowStart = flipIndex * 8;
    // Each bomb has at max 8 neighboring counts
    for (int slot = 0; slot < 8; ++slot) {
        int countIndex = bitsetImpactLookup[rowStart + slot];
        // -1 means not connected
        if (countIndex == -1) {
            // No more counts affected by this bomb
            break;
        }
        // Finds neighbor
        const Count& count = graph.counts[countIndex];
        unsigned int armedNeighbors = 0;
        for (int j = 0; j < 8; ++j) {
            // Calcs its amount of armedNeighbors
            int idx = countNeighborLookup[countIndex * 8 + j];
            if (idx != -1 && bitset.at(idx)) {
                armedNeighbors++;
            }
        }
        // Calculates the impact
        const int currentError = abs(int(count.count) - int(armedNeighbors));
        if (bitset.at(flipIndex)) {
            armedNeighbors--;
        } else {
            armedNeighbors++;
        }
        const int newError = abs(int(count.count) - int(armedNeighbors));
        delta += (newError - currentError); // new - current
    }
    return delta;
}


// Find a "random" index to flip
// The randomness is not truly uniform
// but weighted by the impact of flipping each bit
// The higher positive impact, the more likely it is to be chosen
// This is done so that LAHC can converge faster
int flipIndex(
    const Graph& graph,
    BitSet& bitset,
    const int* countNeighborLookup,
    const int* bitsetImpactLookup
) {
    int bitAmount = graph.bombs.size();
    vector<float> weights(bitAmount);
    float minWeight = 0.0f;
    for (int i = 0; i < bitAmount; i++) {
        bitset.set(i, !bitset.at(i));
        float diff = lahcFlipScoreImpact(graph, bitset, countNeighborLookup, bitsetImpactLookup, i);
        // Uniform_int_dist expects non-negative weights
        if (diff < 0) diff = 0;
        weights[i] = diff;
        bitset.set(i, !bitset.at(i));
        if (weights[i] < minWeight) {
            minWeight = weights[i];
        }
    }
    // Not abandoned since this improves the convergence
    for (auto& w : weights) {
        w -= minWeight;
    }
    // Fairer pseudo-random
    static thread_local mt19937 rng{random_device{}()};
    //  If all are 0 then just return a random index
    if (all_of(weights.begin(), weights.end(), [](float w){ return w <= 0.0f; })) {
        uniform_int_distribution<int> uni(0, bitAmount - 1);
        return uni(rng);
    }
    // Choose the index based on weights which are the difference on total error
    discrete_distribution<int> dist(weights.begin(), weights.end());
    return dist(rng);
}

// Algorithm which is used at the start of LAHC to fill the graph randomly
// The hope is that random filling will give the ability to traverse the solution space better
static void randomFill(Graph& graph) {
    for (auto& [key, bomb] : graph.bombs) {
        // Here rand() is good enough
        float r  = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        bomb.armed = (r < 0.5f);
    }
}

// Find the solution using the LAHC algorithm
void lahcFill(Graph& graph, const LahcOptions& options) {
    // Allocate memory for previous scores
    int* previousScores = new int[options.scoreMemorySize];
    // Initial setup -> random fill
    randomFill(graph);
    int bombCount = graph.bombs.size();
    // If there are no bombs do nothing
    if(bombCount != 0) {
        // The solution state can be represented as bitset
        BitSet current(bombCount);
        // Map bomb coordinates to bitset index
        unordered_map<i64, int> bombIndexMap;
        int cbitSetIndex = 0;
        int currentScore = errorScore(graph);
        int bestScore = currentScore;
        // Fill up the lahc memory with the initial score
        for(int i = 0; i < options.scoreMemorySize; i++) {
            previousScores[i] = currentScore;
        }
        // We need to a way so that each count can quickly look up the neighboring bombs in the bitset
        // This will require an array of ints of size 8 since 0-7 are for count 0, 8-15 for count 1 and so on
        int countNeighborLookupSize = 8 * graph.counts.size();
        int* countNeighborLookup = new int[countNeighborLookupSize];
        // The same for the bitset to counts, since we want to fastly calculate the impact of flipping a bit
        int bitsetImpactLookupSize = 8 * bombCount ;
        // Lookup the neighboring numbers in the bitset, init with -1 to indicate no neighbor
        int* bitsetImpactLookup = new int[bitsetImpactLookupSize];
        // Initially the bitset has no count neighbors, -1 represents that
        fill(bitsetImpactLookup, bitsetImpactLookup + bitsetImpactLookupSize, -1);
        // Correctly setup the datastructures
        for(size_t i = 0; i < graph.counts.size(); i++) {
            const Count& count = graph.counts[i];
            for(size_t j = 0; j < 8; j++) {
                if(count.neighbors[j] != nullptr) {
                    i64 key = bombKey(count.neighbors[j]->x, count.neighbors[j]->y);
                    if(bombIndexMap.find(key) == bombIndexMap.end()) {
                        bombIndexMap[key] = cbitSetIndex;
                        if(count.neighbors[j]->armed) {
                            current.set(cbitSetIndex, true);
                        }
                        cbitSetIndex++;
                    }
                    int index = bombIndexMap[key];
                    countNeighborLookup[i * 8 + j] = index;
                    int row = index * 8;
                    int slot = 0;
                    while (slot < 8 && bitsetImpactLookup[row + slot] != -1) {
                        ++slot;
                    }
                    if (slot < 8) {
                        bitsetImpactLookup[row + slot] = static_cast<int>(i); // this bomb affects count i
                    }
                } else {
                    // No neighbor
                    countNeighborLookup[i * 8 + j] = -1;
                }
            }
        }
        // Now starts the fun part
        // We already init the "best, i"
        // now we init k, current
        int k = 0;
        BitSet best = current;
        for(int iteration = 0; iteration < options.maxIterations; iteration++) {
            // Flip a random bit
            int fli = flipIndex(graph, current, countNeighborLookup, bitsetImpactLookup);
            // Calculate the new score
            int newScore = currentScore + lahcFlipScoreImpact(graph, current, countNeighborLookup, bitsetImpactLookup, fli);
            // Update bitmap based on the flip
            current.set(fli, !current.at(fli));
            // cout << "Iteration " << iteration << " score: " << newScore << endl;
            // Apply the score if it is or equal to the best score
            // (The pseucode has < but it's not that important)
            if(newScore <= bestScore) {
                bestScore = newScore;
                best = current;
            }
            // Here is do prefer <= since it makes
            // an actual permutation of the solution if it is equal
            // which i theorize will help more diverse traversal
            if(newScore <= currentScore || newScore <= previousScores[k]) {
                // Accept new state
                currentScore = newScore;
            } else {
                // Revert the flip if not accepted
                current.set(fli, !current.at(fli));
            }
            // Record the score in the memory
            previousScores[k] = newScore;
            // If we reached perfect score, stop
            if(bestScore == 0) {
                break;
            }
            // Next solution in the memory
            k = (k + 1) % options.scoreMemorySize;
        }
        // Now we need to apply the best solution to the graph
        // Since before we used the bitset
        for(const auto& [key, index] : bombIndexMap) {
            auto it = graph.bombs.find(key);
            if(it != graph.bombs.end()) {
                it->second.armed = best.at(index);
            }
        }
        // Cleanup
        delete[] countNeighborLookup;
        delete[] bitsetImpactLookup;
    }
    // Nothing else to do
    delete[] previousScores;
}