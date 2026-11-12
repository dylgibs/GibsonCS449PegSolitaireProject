#include "GameRecord.h"

#include <cstdio>
#include <ctime>
#include <stdexcept>
#include <utility>

std::string toString(Outcome o) {
    switch (o) {
    case Outcome::Lost:       return "lost";
    case Outcome::Won:        return "won";
    case Outcome::PerfectWin: return "perfect";
    }
    return "lost";
}

Outcome outcomeFromString(const std::string& s) {
    if (s == "lost") return Outcome::Lost;
    if (s == "won") return Outcome::Won;
    if (s == "perfect") return Outcome::PerfectWin;
    throw std::invalid_argument("Unknown outcome: " + s);
}

GameRecord GameRecord::fromGame(const Game& game, std::string timestamp) {
    if (!game.isGameOver())
        throw std::logic_error("GameRecord: only finished games can be recorded");
    GameRecord r;
    r.boardKey = game.layout().key();
    r.timestamp = std::move(timestamp);
    r.pegsRemaining = game.pegCount();
    r.moves = game.history();
    r.outcome = game.isPerfectWin() ? Outcome::PerfectWin
              : game.isWon()        ? Outcome::Won
                                    : Outcome::Lost;
    return r;
}

std::string GameRecord::currentTimestamp() {
    std::time_t t = std::time(nullptr);
    const std::tm* tmp = std::localtime(&t);  // portable across MSVC, MinGW and GCC/Clang
    if (!tmp) return "unknown";
    const std::tm tm = *tmp;
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    return buf;
}
