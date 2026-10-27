#include "Game.h"

#include <utility>

Game::Game(BoardLayout layout) : m_layout(std::move(layout)) {
    newGame();
}

void Game::newGame(const BoardLayout& l) {
    m_layout = l;
    newGame();
}

void Game::newGame() {
    m_cells.assign(static_cast<size_t>(rows()),
                   std::vector<CellState>(static_cast<size_t>(cols()), CellState::Invalid));
    m_pegCount = 0;
    m_moveCount = 0;
    for (int r = 0; r < rows(); ++r) {
        for (int c = 0; c < cols(); ++c) {
            Position p{r, c};
            if (!m_layout.isHole(p)) continue;
            bool empty = (p == m_layout.startEmpty());
            m_cells[static_cast<size_t>(r)][static_cast<size_t>(c)] =
                empty ? CellState::Empty : CellState::Peg;
            if (!empty) ++m_pegCount;
        }
    }
}

CellState Game::cellAt(Position p) const {
    if (p.row < 0 || p.row >= rows() || p.col < 0 || p.col >= cols())
        return CellState::Invalid;
    return m_cells[static_cast<size_t>(p.row)][static_cast<size_t>(p.col)];
}

void Game::setCell(Position p, CellState s) {
    m_cells[static_cast<size_t>(p.row)][static_cast<size_t>(p.col)] = s;
}

bool Game::isValidMove(const Move& m) const {
    const int dr = m.to.row - m.from.row;
    const int dc = m.to.col - m.from.col;

    bool directionOk = false;
    for (const Position& d : m_layout.directions())
        if (dr == 2 * d.row && dc == 2 * d.col) { directionOk = true; break; }
    if (!directionOk) return false;

    // cellAt() returns Invalid for off-board or non-hole cells, so those fail below
    return cellAt(m.from) == CellState::Peg &&
           cellAt(m.jumped()) == CellState::Peg &&
           cellAt(m.to) == CellState::Empty;
}

bool Game::makeMove(const Move& m) {
    if (!isValidMove(m)) return false;
    setCell(m.from, CellState::Empty);
    setCell(m.jumped(), CellState::Empty);
    setCell(m.to, CellState::Peg);
    --m_pegCount;
    ++m_moveCount;
    return true;
}

std::vector<Move> Game::validMovesFrom(Position p) const {
    std::vector<Move> result;
    if (cellAt(p) != CellState::Peg) return result;
    for (const Position& d : m_layout.directions()) {
        Move m{p, {p.row + 2 * d.row, p.col + 2 * d.col}};
        if (isValidMove(m)) result.push_back(m);
    }
    return result;
}

std::vector<Move> Game::allValidMoves() const {
    std::vector<Move> result;
    for (int r = 0; r < rows(); ++r)
        for (int c = 0; c < cols(); ++c) {
            auto moves = validMovesFrom({r, c});
            result.insert(result.end(), moves.begin(), moves.end());
        }
    return result;
}

bool Game::isGameOver() const {
    for (int r = 0; r < rows(); ++r)
        for (int c = 0; c < cols(); ++c)
            if (!validMovesFrom({r, c}).empty()) return false;
    return true;
}

bool Game::isWon() const {
    return m_pegCount == 1 && isGameOver();
}

bool Game::isPerfectWin() const {
    return isWon() && cellAt(m_layout.startEmpty()) == CellState::Peg;
}
