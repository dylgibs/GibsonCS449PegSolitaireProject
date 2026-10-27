#pragma once

#include <string>
#include <vector>

#include "Move.h"

// Identifies the built-in boards
enum class BoardType {
    Standard,  // classic 33-hole English cross (7x7 grid)
    Large      // 57-hole board on a 9x9 grid
    //TODO More boards
};

// Describes the shape and starting position of a board. Doesn't contain game state.

// Rows are strings: '.' = a hole that belongs to the board, '#' = not part of the board.
// The game starts with every hole filled except `startEmpty`.
// `directions` are the jump directions allowed (default: the 4 orthogonal ones)
class BoardLayout {
public:
    BoardLayout(std::string name,
                std::vector<std::string> rows,
                Position startEmpty,
                std::vector<Position> directions = defaultDirections());

    static BoardLayout create(BoardType type);
    static BoardLayout standard();
    static BoardLayout large();
    static std::vector<Position> defaultDirections();

    const std::string& name() const { return m_name; }
    int rows() const { return static_cast<int>(m_rows.size()); }
    int cols() const { return m_rows.empty() ? 0 : static_cast<int>(m_rows[0].size()); }
    bool isHole(Position p) const;
    int holeCount() const;
    Position startEmpty() const { return m_startEmpty; }
    const std::vector<Position>& directions() const { return m_directions; }

private:
    std::string m_name;
    std::vector<std::string> m_rows;
    Position m_startEmpty;
    std::vector<Position> m_directions;
};
