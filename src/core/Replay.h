#pragma once

#include <optional>

#include "Game.h"
#include "GameRecord.h"

// Steps through a recorded game. Owns its own Game, so a game in progress is never disturbed.
class Replay {
public:
    // Throws if the board key is unknown or any recorded move is illegal on that board.
    explicit Replay(GameRecord record);

    const GameRecord& record() const { return m_record; }
    const Game& game() const { return m_game; }

    int step() const { return m_step; }  // moves applied so far
    int totalSteps() const { return static_cast<int>(m_record.moves.size()); }
    bool atStart() const { return m_step == 0; }
    bool atEnd() const { return m_step == totalSteps(); }
    std::optional<Move> lastMove() const;  // most recently applied move, if any

    bool stepForward();  // false if already at the end
    bool stepBack();     // false if already at the start
    void toStart();
    void toEnd();

private:
    GameRecord m_record;
    Game m_game;
    int m_step = 0;

    void rebuildTo(int step);
};
