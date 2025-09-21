#include "optimization.hpp"
#include "representation.hpp"
#include <unordered_map>
#include "bitset.hpp"
#include <iostream>
#include <numeric>
#include <random>

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

// For optimization we need to quickly calculate error score
static int lahcErrorScore(
    const Graph& graph,
    const BitSet& bitset, 
    const int* countNeighborLookup
) {
    int totalError = 0;
    for (size_t i = 0; i < graph.counts.size(); i++) {
        const Count& count = graph.counts[i];
        unsigned int armedNeighbors = 0;
        for (size_t j = 0; j < 8; j++) {
            int bitsetIndex = countNeighborLookup[i * 8 + j];
            if (bitsetIndex != -1 && bitset.at(bitsetIndex)) {
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


struct FlipResult {
    int flipIndex;
    float scoreDiff;
};

int flipIndex(
    const Graph& graph,
    BitSet& bitset,
    const int* countNeighborLookup
) {
    int currentScore = lahcErrorScore(graph, bitset, countNeighborLookup);
    int bitAmount = graph.bombs.size();
    std::vector<float> weights(bitAmount);
    float minWeight = 0.0f;

    for (int i = 0; i < bitAmount; i++) {
        bitset.set(i, !bitset.at(i));
        int newScore = lahcErrorScore(graph, bitset, countNeighborLookup);
        float diff = float(currentScore - newScore);
        if (diff < 0) diff = 0;
        weights[i] = diff;
        bitset.set(i, !bitset.at(i));
        if (weights[i] < minWeight) {
            minWeight = weights[i];
        }
    }
    // Ensure that weights are non-negative
    for (auto& w : weights) {
        w -= minWeight;
    }


    static thread_local std::mt19937 rng{std::random_device{}()};
    if (std::all_of(weights.begin(), weights.end(), [](float w){ return w <= 0.0f; })) {
        std::uniform_int_distribution<int> uni(0, bitAmount - 1);
        return uni(rng);
    }

    std::discrete_distribution<int> dist(weights.begin(), weights.end());
    return dist(rng);
}



void lahcFill(Graph& graph, const LahcOptions& options) {
    // Allocate memory for previous scores
    int* previousScores = new int[options.scoreMemorySize];
    // First we need an inital solution
    basicFill(graph);
    // TODO: Implement the rest of the algorithm
    int bombCount = graph.bombs.size();
    if(bombCount != 0) {
        BitSet current(bombCount);
        unordered_map<i64, int> bombIndexMap;
        int cbitSetIndex = 0;
        int currentScore = errorScore(graph);
        int bestScore = currentScore;
        for(int i = 0; i < options.scoreMemorySize; i++) {
            previousScores[i] = currentScore;
        }
        // We need to a way so that each count can quickly look up the neighboring bombs in the bitset
        // This will require an array of ints of size 8 since 0-7 are for count 0, 8-15 for count 1 and so on
        int countNeighborLookupSize = 8 * graph.counts.size();
        int* countNeighborLookup = new int[countNeighborLookupSize];
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
                    countNeighborLookup[i * 8 + j] = bombIndexMap[key];
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
            int fli = flipIndex(graph, current, countNeighborLookup);
            current.set(fli, !current.at(fli));
            int newScore = lahcErrorScore(graph, current, countNeighborLookup);
            // cout << "Iteration " << iteration << " score: " << newScore << endl;
            if(newScore <= bestScore) {
                bestScore = newScore;
                best = current;
            }
            if(newScore <= currentScore || newScore <= previousScores[k]) {
                // Accept new state
                currentScore = newScore;
            } else {
                // Revert the flip
                current.set(fli, !current.at(fli));
            }
            previousScores[k] = newScore;
            // If we reached perfect score, stop
            if(bestScore == 0) {
                break;
            }
            k = (k + 1) % options.scoreMemorySize;
        }
        // Now we need to apply the best solution to the graph
        for(const auto& [key, index] : bombIndexMap) {
            auto it = graph.bombs.find(key);
            if(it != graph.bombs.end()) {
                it->second.armed = best.at(index);
            }
        }
        delete[] countNeighborLookup;
    }
    // Nothing else to do
    delete[] previousScores;
}