#pragma once

#include <string>
#include <vector>

#include "Move.h"

// Board size
enum class BoardSize {
    Standard,  // 7x7 grid
    Large      // 9x9 grid
};

// Board type: the shape/rules family of the board.
enum class BoardType {
    English,    // classic cross-shaped board, 4 orthogonal jump directions
    Hexagonal   // hexagon-shaped board, 6 jump directions
};

// How the grid is drawn by a front end
enum class Geometry {
    Square,     // cell (row, col) is drawn at (col, row)
    Hexagonal   // axial coordinates: rows are shifted by half a cell
};

// Describes the *shape* and starting position of a board. Contains no game state.

// Rows are strings: '.' = a hole that belongs to the board, '#' = not part of the board.
// The game starts with every hole filled except `startEmpty`.
// `directions` are the jump directions allowed (row/col deltas of one step).
// `key` is a stable identifier used when recording games to disk.
class BoardLayout {
public:
    BoardLayout(std::string name,
                std::vector<std::string> rows,
                Position startEmpty,
                std::vector<Position> directions = defaultDirections(),
                Geometry geometry = Geometry::Square,
                std::string key = "custom");

    // Factories
    static BoardLayout create(BoardType type, BoardSize size);
    static BoardLayout create(BoardSize size);  // English board of the given size
    static BoardLayout english(BoardSize size);
    static BoardLayout hexagonal(BoardSize size);
    static BoardLayout standard();  // English, Standard size
    static BoardLayout large();     // English, Large (9x9) size
    static BoardLayout fromKey(const std::string& key);  // throws std::invalid_argument

    static std::vector<Position> defaultDirections();  // 4 orthogonal directions
    static std::vector<Position> hexDirections();      // 6 axial directions

    const std::string& name() const { return m_name; }
    const std::string& key() const { return m_key; }
    Geometry geometry() const { return m_geometry; }
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
    Geometry m_geometry;
    std::string m_key;
};
