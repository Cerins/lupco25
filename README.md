# Minesweeper Consistency Problem Optimization

## What is this?

This repository contains the code for optimizing the solution to the Minesweeper Consistency Problem.

The Minesweeper Consistency Problem is where given a Minesweeper board with numbers filled out, output a board with mines placed such that the numbers are correct. If that is not possible, then output the fact.

Since that problem is NP-complete, this code attempts to find solution that is "close enough".
This is done by trying to minimize the board's error, using the Late Acceptance Hill Climbing algorithm.



## How to compile?

### Linux

`make`

### Windows

Either use WSL, or manually compile and link with the system's C++ toolset.

## How to run?

### Linux

`./bin/sweeper`

The program accepts the board from stdin, and outputs the result to stdout.

The expected input format is:
```
...4..
.....1
..2...
......
```

You can either pipe in the input in, or input it manually.
When you are done inputting the board, press Ctrl+D to signal EOF.

## Structure of the code

The main algorithm is in `src/optimization.cpp`.

The way the standard input gets parsed into the graph representation and back is in `src/representation.cpp`.

## Tests?

### Performance

### Correctness

### Unit tests

`make test`