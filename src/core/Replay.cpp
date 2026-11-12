#include "Replay.h"

#include <stdexcept>
#include <string>
#include <utility>

Replay::Replay(GameRecord record)
    : m_record(std::move(record)),
      m_game(BoardLayout::fromKey(m_record.boardKey)) {
    // Validate the whole record up front so a corrupt file is reported immediately.
    for (size_t i = 0; i < m_record.moves.size(); ++i)
        if (!m_game.makeMove(m_record.moves[i]))
            throw std::runtime_error("Recorded move " + std::to_string(i + 1) +
                                     " is not legal on board '" + m_record.boardKey + "'");
    m_game.newGame();
}

std::optional<Move> Replay::lastMove() const {
    if (m_step == 0) return std::nullopt;
    return m_record.moves[static_cast<size_t>(m_step - 1)];
}

bool Replay::stepForward() {
    if (atEnd()) return false;
    m_game.makeMove(m_record.moves[static_cast<size_t>(m_step)]);
    ++m_step;
    return true;
}

bool Replay::stepBack() {
    if (atStart()) return false;
    rebuildTo(m_step - 1);
    return true;
}

void Replay::toStart() { rebuildTo(0); }
void Replay::toEnd() { rebuildTo(totalSteps()); }

void Replay::rebuildTo(int step) {
    m_game.newGame();
    for (int i = 0; i < step; ++i) m_game.makeMove(m_record.moves[static_cast<size_t>(i)]);
    m_step = step;
}
