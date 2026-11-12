#pragma once

#include <random>

#include "BoardLayout.h"
#include "Game.h"

// Creates games that start from a random, *reachable* position: it plays random legal moves
// from the normal starting position, so every generated position can really occur in play.
namespace RandomState {

// How many moves to pre-play for a "typical" random start on this board
// (between 2 and about half the pegs).
int randomMoveCount(const BoardLayout& layout, std::mt19937& rng);

// Plays up to `moveCount` random moves from the initial position. It never makes a move
// that would end the game, so the returned game always has at least one legal move
// (it may therefore pre-play fewer moves than requested on very small boards).
// The pre-played moves are counted in setupMoveCount().
Game generate(const BoardLayout& layout, std::mt19937& rng, int moveCount);

}  // namespace RandomState
