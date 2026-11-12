#include "AutoPlayer.h"

namespace AutoPlayer {

bool step(Game& game, MoveStrategy& strategy) {
    if (game.isGameOver()) return false;
    const std::optional<Move> move = strategy.chooseMove(game);
    if (!move) return false;
    return game.makeMove(*move);  // makeMove rejects illegal moves without changing the game
}

int playToEnd(Game& game, MoveStrategy& strategy, int maxMoves) {
    int played = 0;
    while (played < maxMoves && step(game, strategy)) ++played;
    return played;
}

}  // namespace AutoPlayer
