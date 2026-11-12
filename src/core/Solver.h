#pragma once

#include <vector>

#include "Game.h"

// Depth-first search (with memoisation of dead positions) for a sequence of moves that
// leaves a single peg. Works from any position of any board with at most 64 holes.
namespace Solver {

constexpr int kMaxHoles = 64;

struct Result {
    std::vector<Move> moves;   // legal sequence from the given position
    bool solved = false;       // true if the sequence leaves exactly one peg
    int pegsRemaining = 0;     // pegs left after playing `moves`
    long nodesExplored = 0;
};

// If no solution is found within `nodeBudget` positions, returns the best sequence seen
// (fewest pegs left) with solved == false.
// Throws std::invalid_argument if the board has more than kMaxHoles holes.
Result solve(const Game& game, long nodeBudget = 1000000);

}  // namespace Solver
