#include <unordered_map>
#include <iostream>
#include <numeric>
#include <random>
#include "optimization.hpp"
#include "representation.hpp"
#include "bitset.hpp"

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
    for (int slot = 0; slot < 8; ++slot) {
        int countIndex = bitsetImpactLookup[rowStart + slot];
        if (countIndex == -1) {
            // No more counts affected by this bomb
            break;
        }
        const Count& count = graph.counts[countIndex];
        unsigned int armedNeighbors = 0;
        for (int j = 0; j < 8; ++j) {
            int idx = countNeighborLookup[countIndex * 8 + j];
            if (idx != -1 && bitset.at(idx)) {
                ++armedNeighbors;
            }
        }
        const int currentError = abs(int(count.count) - int(armedNeighbors));
        if (bitset.at(flipIndex)) {
            --armedNeighbors;
        } else {
            ++armedNeighbors;
        }
        const int newError = abs(int(count.count) - int(armedNeighbors));
        delta += (newError - currentError); // IMPORTANT: new - current
    }
    return delta;
}


struct FlipResult {
    int flipIndex;
    float scoreDiff;
};

int flipIndex(
    const Graph& graph,
    BitSet& bitset,
    const int* countNeighborLookup,
    const int* bitsetImpactLookup
) {
    // int currentScore = lahcErrorScore(graph, bitset, countNeighborLookup);
    int bitAmount = graph.bombs.size();
    std::vector<float> weights(bitAmount);
    float minWeight = 0.0f;

    for (int i = 0; i < bitAmount; i++) {
        bitset.set(i, !bitset.at(i));
        // int newScore = lahcErrorScore(graph, bitset, countNeighborLookup);
        float diff = lahcFlipScoreImpact(graph, bitset, countNeighborLookup, bitsetImpactLookup, i);
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

static void randomFill(Graph& graph) {
    for (auto& [key, bomb] : graph.bombs) {
        float r  = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        bomb.armed = (r < 0.5f);
    }
}

void lahcFill(Graph& graph, const LahcOptions& options) {
    // Allocate memory for previous scores
    int* previousScores = new int[options.scoreMemorySize];
    // Initial setup -> random fill
    randomFill(graph);
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
        int bitsetImpactLookupSize = 8 * bombCount ;
        // Lookup the neighboring numbers in the bitset, init with -1 to indicate no neighbor
        int* bitsetImpactLookup = new int[bitsetImpactLookupSize];
        fill(bitsetImpactLookup, bitsetImpactLookup + bitsetImpactLookupSize, -1);
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
            int newScore = currentScore + lahcFlipScoreImpact(graph, current, countNeighborLookup, bitsetImpactLookup, fli);
            current.set(fli, !current.at(fli));
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
        delete[] bitsetImpactLookup;
    }
    // Nothing else to do
    delete[] previousScores;
}