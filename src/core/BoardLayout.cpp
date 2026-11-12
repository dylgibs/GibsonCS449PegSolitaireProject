#include "BoardLayout.h"

#include <cstdlib>
#include <stdexcept>
#include <utility>

namespace {

// Hexagon of the given radius in axial coordinates (q = col - R, r = row - R).
// A cell belongs to the hexagon when |q| <= R, |r| <= R and |q + r| <= R.
std::vector<std::string> hexRows(int radius) {
    const int n = 2 * radius + 1;
    std::vector<std::string> rows(static_cast<size_t>(n), std::string(static_cast<size_t>(n), '#'));
    for (int r = -radius; r <= radius; ++r)
        for (int q = -radius; q <= radius; ++q)
            if (std::abs(q + r) <= radius)
                rows[static_cast<size_t>(r + radius)][static_cast<size_t>(q + radius)] = '.';
    return rows;
}

}  // namespace

BoardLayout::BoardLayout(std::string name,
                         std::vector<std::string> rows,
                         Position startEmpty,
                         std::vector<Position> directions,
                         Geometry geometry,
                         std::string key)
    : m_name(std::move(name)),
      m_rows(std::move(rows)),
      m_startEmpty(startEmpty),
      m_directions(std::move(directions)),
      m_geometry(geometry),
      m_key(std::move(key)) {
    if (m_rows.empty() || m_rows[0].empty())
        throw std::invalid_argument("BoardLayout: board must not be empty");
    for (const auto& r : m_rows)
        if (r.size() != m_rows[0].size())
            throw std::invalid_argument("BoardLayout: all rows must have the same length");
    if (!isHole(m_startEmpty))
        throw std::invalid_argument("BoardLayout: start-empty cell must be a hole");
    if (m_directions.empty())
        throw std::invalid_argument("BoardLayout: at least one direction is required");
}

std::vector<Position> BoardLayout::defaultDirections() {
    return {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
}

std::vector<Position> BoardLayout::hexDirections() {
    // axial neighbours (dq, dr) -> (row, col) = (dr, dq)
    return {{0, 1}, {0, -1}, {1, 0}, {-1, 0}, {-1, 1}, {1, -1}};
}

bool BoardLayout::isHole(Position p) const {
    if (p.row < 0 || p.row >= rows() || p.col < 0 || p.col >= cols()) return false;
    return m_rows[static_cast<size_t>(p.row)][static_cast<size_t>(p.col)] == '.';
}

int BoardLayout::holeCount() const {
    int n = 0;
    for (int r = 0; r < rows(); ++r)
        for (int c = 0; c < cols(); ++c)
            if (isHole({r, c})) ++n;
    return n;
}

BoardLayout BoardLayout::create(BoardType type, BoardSize size) {
    switch (type) {
    case BoardType::English:   return english(size);
    case BoardType::Hexagonal: return hexagonal(size);
    }
    throw std::invalid_argument("BoardLayout::create: unknown board type");
}

BoardLayout BoardLayout::create(BoardSize size) {
    return english(size);
}

BoardLayout BoardLayout::english(BoardSize size) {
    switch (size) {
    case BoardSize::Standard:
        return BoardLayout("Standard Board",
                           {"##...##",
                            "##...##",
                            ".......",
                            ".......",
                            ".......",
                            "##...##",
                            "##...##"},
                           {3, 3}, defaultDirections(), Geometry::Square, "english-standard");
    case BoardSize::Large:
        return BoardLayout("9x9 Board",
                           {"###...###",
                            "##.....##",
                            "#.......#",
                            ".........",
                            ".........",
                            ".........",
                            "#.......#",
                            "##.....##",
                            "###...###"},
                           {4, 4}, defaultDirections(), Geometry::Square, "english-large");
    }
    throw std::invalid_argument("BoardLayout::english: unknown board size");
}

BoardLayout BoardLayout::hexagonal(BoardSize size) {
    // Radius 3 -> 37 holes on a 7x7 grid; radius 4 -> 61 holes on a 9x9 grid.
    const bool standard = (size == BoardSize::Standard);
    const int radius = standard ? 3 : 4;
    return BoardLayout(standard ? "Hexagonal Board (7x7)" : "Hexagonal Board (9x9)",
                       hexRows(radius), {radius, radius}, hexDirections(), Geometry::Hexagonal,
                       standard ? "hex-standard" : "hex-large");
}

BoardLayout BoardLayout::standard() { return english(BoardSize::Standard); }
BoardLayout BoardLayout::large() { return english(BoardSize::Large); }

BoardLayout BoardLayout::fromKey(const std::string& key) {
    for (BoardType t : {BoardType::English, BoardType::Hexagonal})
        for (BoardSize s : {BoardSize::Standard, BoardSize::Large}) {
            BoardLayout l = create(t, s);
            if (l.key() == key) return l;
        }
    throw std::invalid_argument("Unknown board key: " + key);
}
