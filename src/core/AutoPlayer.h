#pragma once

#include "Game.h"
#include "MoveStrategy.h"

// Runs a game automatically using a MoveStrategy. Stateless helpers, so a UI can call
// step() from a timer (to animate) while tests can call playToEnd().
namespace AutoPlayer {

// Plays one move chosen by the strategy. Returns false if the game is
// over, the strategy has no move, or the strategy proposes an illegal move.
bool step(Game& game, MoveStrategy& strategy);

// Plays until the game is over, the strategy stops, or maxMoves have been played.
// Returns the number of moves played.
int playToEnd(Game& game, MoveStrategy& strategy, int maxMoves = 10000);

}  // namespace AutoPlayer
