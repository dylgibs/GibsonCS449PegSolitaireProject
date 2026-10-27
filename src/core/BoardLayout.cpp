#include "BoardLayout.h"

#include <stdexcept>
#include <utility>

BoardLayout::BoardLayout(std::string name,
                         std::vector<std::string> rows,
                         Position startEmpty,
                         std::vector<Position> directions)
    : m_name(std::move(name)),
      m_rows(std::move(rows)),
      m_startEmpty(startEmpty),
      m_directions(std::move(directions)) {
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

BoardLayout BoardLayout::create(BoardType type) {
    switch (type) {
    case BoardType::Standard: return standard();
    case BoardType::Large:    return large();
    }
    throw std::invalid_argument("BoardLayout::create: unknown board type");
}

BoardLayout BoardLayout::standard() {
    return BoardLayout("Standard Board",
                       {"##...##",
                        "##...##",
                        ".......",
                        ".......",
                        ".......",
                        "##...##",
                        "##...##"},
                       {3, 3});
}

BoardLayout BoardLayout::large() {
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
                       {4, 4});
}
