#define CATCH_CONFIG_MAIN
#include "representation.hpp"
#include <catch.hpp>
#include <iostream>
#include <sstream>
using namespace std;

TEST_CASE("fromBoard: number board") {
    Board board;
    board.width = 3;
    board.height = 3;
    board.field = {
        ".11",
        "...",
        "..."
    };

    Graph graph = fromBoard(board);

    REQUIRE(graph.width == 3);
    REQUIRE(graph.height == 3);
    REQUIRE(graph.bombs.size() == 4); 
    REQUIRE(graph.counts.size() == 2); 
    // Expect that one count has four numbers around it
    // While another has four numbers around it
    int firstCount = 0;
    int secondCount = 0;
    for(const auto& neighbor : graph.counts[0].neighbors) {
        if(neighbor != nullptr) {
            firstCount++;
        }
    }
    for(const auto& neighbor : graph.counts[1].neighbors) {
        if(neighbor != nullptr) {
            secondCount++;
        }
    }
    REQUIRE((firstCount != secondCount));
    REQUIRE((firstCount == 4 || secondCount == 4));
    REQUIRE((firstCount == 2 || secondCount == 2));
}

TEST_CASE("fromBoard: all bombs") {
    Board board;
    board.width = 2;
    board.height = 2;
    board.field = {
        "XX",
        "XX"
    };

    Graph graph = fromBoard(board);

    REQUIRE(graph.bombs.size() == 4);
    for (const auto& [key, bomb] : graph.bombs) {
        REQUIRE(bomb.armed);
    }
    REQUIRE(graph.counts.empty());
}

TEST_CASE("fromBoard: numeric counts only") {
    Board board;
    board.width = 3;
    board.height = 1;
    board.field = {
        "123"
    };

    Graph graph = fromBoard(board);
    REQUIRE(graph.counts.size() == 3);
    REQUIRE(graph.counts[0].count == 1);
    REQUIRE(graph.counts[1].count == 2);
    REQUIRE(graph.counts[2].count == 3);
    REQUIRE(graph.bombs.size() == 0);
}

TEST_CASE("fromBoard: neighbor linking") {
    Board board;
    board.width = 3;
    board.height = 3;
    board.field = {
        "X.X",
        ".3.",
        "..."
    };

    Graph graph = fromBoard(board);
    REQUIRE(graph.counts.size() == 1);
    Count& count = graph.counts[0];
    REQUIRE(count.x == 1);
    REQUIRE(count.y == 1);
    REQUIRE(count.count == 3);
    // Verify all 8 neighbors are connected
    int neighborCount = 0;
    int armedCount = 0;
    for (int i = 0; i < 8; i++) {
        if (count.neighbors[i] != nullptr) {
            neighborCount++;
            if(count.neighbors[i]->armed) {
                armedCount++;
            }
        }
    }
    REQUIRE(neighborCount == 8);
    REQUIRE(armedCount == 2); // two bombs around
}

TEST_CASE("dumpGraph: prints correctly") {
    Board board;
    board.width = 2;
    board.height = 2;
    board.field = {
        "X1",
        "1."
    };

    Graph graph = fromBoard(board);
    ostringstream oss;
    streambuf* old_cout = cout.rdbuf(oss.rdbuf());
    dumpGraph(graph);
    cout.rdbuf(old_cout);
    REQUIRE(oss.str() == "X1\n1.\n");
}

TEST_CASE("fromBoard: empty board") {
    Board board;
    board.width = 2;
    board.height = 2;
    board.field = {
        "..",
        ".."
    };

    Graph graph = fromBoard(board);
    REQUIRE(graph.bombs.empty());
    REQUIRE(graph.counts.empty());
}
