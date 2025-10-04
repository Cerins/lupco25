# Minesweeper Constraint Satisfaction Problem

## What is this?

This repository contains the code for optimizing the solution to the Minesweeper Constraint Satisfaction Problem.

The Minesweeper Constraint Satisfaction Problem is where given a Minesweeper board with numbers filled out, output a board with mines placed such that the numbers are correct. If that is not possible, then output the fact.

[Since that problem is NP-complete](https://ocw.mit.edu/courses/es-268-the-mathematics-in-toys-and-games-spring-2010/50a061a2f76a503d8473a072965bc8ff_MITES_268S10_ses11_slides.pdf), this code attempts to find solution that is "close enough".
This is done by trying to minimize the board's error, using the Late Acceptance Hill Climbing algorithm.

## How to compile?

### Linux

`make`

### Windows

If g++ is in your PATH, then you can run `build.bat` while being in this directory.


Alternatively, you can download prebuilt binaries from the releases page.

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

### Windows

`bin\sweeper.exe`

Instead of Ctrl+D, press Ctrl+Z + Enter to signal EOF.

## Structure of the code

The main algorithm is in `src/optimization.cpp`.

The way the standard input gets parsed into the graph representation and back is in `src/representation.cpp`.

## Tests?

### Performance

Can be tested by running `python3 ./tests/speed.py`.

### Correctness

Can be tested by running `python3 ./tests/correctness.py`.

### Unit tests

`make test`