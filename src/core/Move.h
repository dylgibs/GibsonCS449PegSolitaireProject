#pragma once

// Plain value types shared by the game logic and the UI
// No Qt dependency: the core library is pure standard C++

struct Position {
    int row = 0;
    int col = 0;

    bool operator==(const Position& o) const { return row == o.row && col == o.col; }
    bool operator!=(const Position& o) const { return !(*this == o); }
};

// A move is a peg jumping from `from` over an adjacent peg into `to`
// (exactly two cells away along one of the layout's directions)
struct Move {
    Position from;
    Position to;

    // The cell that is jumped over and removed
    Position jumped() const {
        return Position{(from.row + to.row) / 2, (from.col + to.col) / 2};
    }
    bool operator==(const Move& o) const { return from == o.from && to == o.to; }
};
