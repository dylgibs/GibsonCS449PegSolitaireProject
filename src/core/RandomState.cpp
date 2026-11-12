#include "RandomState.h"

#include <algorithm>
#include <utility>

namespace RandomState {

int randomMoveCount(const BoardLayout& layout, std::mt19937& rng) {
    const int pegs = layout.holeCount() - 1;
    const int hi = std::max(2, pegs / 2);
    std::uniform_int_distribution<int> pick(2, hi);
    return pick(rng);
}

Game generate(const BoardLayout& layout, std::mt19937& rng, int moveCount) {
    Game game(layout);
    for (int i = 0; i < moveCount; ++i) {
        std::vector<Move> moves = game.allValidMoves();
        std::shuffle(moves.begin(), moves.end(), rng);
        bool moved = false;
        for (const Move& m : moves) {
            Game trial = game;
            trial.makeMove(m);
            if (!trial.isGameOver()) {  // never pre-play into a finished game
                game = std::move(trial);
                moved = true;
                break;
            }
        }
        if (!moved) break;
    }
    game.markSetupComplete();
    return game;
}

}  // namespace RandomState
