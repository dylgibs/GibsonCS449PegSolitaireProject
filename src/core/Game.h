#pragma once

#include <vector>

#include "BoardLayout.h"
#include "Move.h"

enum class CellState {
    Invalid,  // not part of the board
    Empty,    // hole without a peg
    Peg       // hole with a peg
};

// Game logic: owns the board state and enforces rules
// Knows nothing about the user interface
class Game {
public:
    explicit Game(BoardLayout layout = BoardLayout::standard());

    // starting games
    void newGame();                      // restart with the current layout
    void newGame(const BoardLayout& l);  // switch layout and restart

    //  queries
    const BoardLayout& layout() const { return m_layout; }
    int rows() const { return m_layout.rows(); }
    int cols() const { return m_layout.cols(); }
    CellState cellAt(Position p) const;
    int pegCount() const { return m_pegCount; }
    int moveCount() const { return m_moveCount; }

    //  moves
    bool isValidMove(const Move& m) const;
    bool makeMove(const Move& m);  // returns false (state unchanged) if illegal
    std::vector<Move> validMovesFrom(Position p) const;
    std::vector<Move> allValidMoves() const;

    //  game status
    bool isGameOver() const;  // no legal move remains
    bool isWon() const;       // game over with exactly one peg left
    bool isPerfectWin() const;  // single peg left in the starting-empty cell

private:
    BoardLayout m_layout;
    std::vector<std::vector<CellState>> m_cells;
    int m_pegCount = 0;
    int m_moveCount = 0;

    void setCell(Position p, CellState s);
};
