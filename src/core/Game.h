#pragma once

#include <vector>

#include "BoardLayout.h"
#include "Move.h"

enum class CellState {
    Invalid,  // not part of the board (or off the board)
    Empty,    // hole without a peg
    Peg       // hole with a peg
};

// Pure game logic: owns the board state and enforces the rules.
// Knows nothing about the user interface, so it can be unit-tested
// and reused with any front end.
class Game {
public:
    explicit Game(BoardLayout layout = BoardLayout::standard());

    //starting games
    void newGame();                      // restart with the current layout
    void newGame(const BoardLayout& l);  // switch layout and restart

    // --- queries ---
    const BoardLayout& layout() const { return m_layout; }
    int rows() const { return m_layout.rows(); }
    int cols() const { return m_layout.cols(); }
    CellState cellAt(Position p) const;
    int pegCount() const { return m_pegCount; }
    int moveCount() const { return m_moveCount; }
    const std::vector<Move>& history() const { return m_history; }  // moves played so far

    // Moves that were pre-played to create a random starting position (see RandomState).
    // They are part of history()/moveCount(), so recordings and replays stay consistent.
    int setupMoveCount() const { return m_setupMoves; }
    void markSetupComplete() { m_setupMoves = m_moveCount; }

    // moves
    bool isValidMove(const Move& m) const;
    bool makeMove(const Move& m);  // returns false if illegal
    std::vector<Move> validMovesFrom(Position p) const;
    std::vector<Move> allValidMoves() const;

    // game status
    bool isGameOver() const;  // no legal move remains
    bool isWon() const;       // game over with exactly one peg left
    bool isPerfectWin() const;  // single peg left in the starting-empty cell

private:
    BoardLayout m_layout;
    std::vector<std::vector<CellState>> m_cells;
    int m_pegCount = 0;
    int m_moveCount = 0;
    int m_setupMoves = 0;
    std::vector<Move> m_history;

    void setCell(Position p, CellState s);
};
