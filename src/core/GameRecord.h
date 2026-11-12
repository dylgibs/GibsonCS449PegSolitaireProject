#pragma once

#include <string>
#include <vector>

#include "Game.h"
#include "Move.h"

enum class Outcome { Lost, Won, PerfectWin };

std::string toString(Outcome o);                    // "lost" / "won" / "perfect"
Outcome outcomeFromString(const std::string& s);    // throws std::invalid_argument

// Everything needed to replay a finished game: which board, and the moves in order.
struct GameRecord {
    std::string boardKey;        // BoardLayout::key()
    std::string timestamp;       // e.g. "2026-11-10 14:32:05"
    Outcome outcome = Outcome::Lost;
    int pegsRemaining = 0;
    std::vector<Move> moves;

    // Build a record from a finished game. Throws std::logic_error if the game is not over.
    static GameRecord fromGame(const Game& game, std::string timestamp);
    static std::string currentTimestamp();
};
