#include "Solver.h"

#include <algorithm>
#include <cstdint>
#include <vector>
#include <queue>
#include <stdexcept>
#include <unordered_set>

namespace {

struct Jump {
    int from, over, to;  // bit indices
    Move move;
};

class Search {
public:
    Search(const Game& game, long budget) : m_budget(budget) {
        const BoardLayout& layout = game.layout();
        std::vector<int> index(static_cast<size_t>(game.rows() * game.cols()), -1);
        int holes = 0;
        for (int r = 0; r < game.rows(); ++r)
            for (int c = 0; c < game.cols(); ++c)
                if (layout.isHole({r, c})) index[static_cast<size_t>(r * game.cols() + c)] = holes++;
        if (holes > Solver::kMaxHoles)
            throw std::invalid_argument("Solver: boards with more than 64 holes are not supported");

        auto idx = [&](Position p) {
            return layout.isHole(p) ? index[static_cast<size_t>(p.row * game.cols() + p.col)] : -1;
        };
        for (int r = 0; r < game.rows(); ++r)
            for (int c = 0; c < game.cols(); ++c) {
                const Position from{r, c};
                if (!layout.isHole(from)) continue;
                for (const Position& d : layout.directions()) {
                    const Position over{r + d.row, c + d.col};
                    const Position to{r + 2 * d.row, c + 2 * d.col};
                    if (layout.isHole(over) && layout.isHole(to))
                        m_jumps.push_back({idx(from), idx(over), idx(to), Move{from, to}});
                }
            }

        // Heuristic weight of each hole = its step distance from the starting hole.
        // Moves that pull pegs towards that hole are tried first (see dfs()).
        m_weight.assign(static_cast<size_t>(holes), 1000);
        {
            std::queue<Position> q;
            q.push(layout.startEmpty());
            m_weight[static_cast<size_t>(idx(layout.startEmpty()))] = 0;
            while (!q.empty()) {
                const Position p = q.front();
                q.pop();
                for (const Position& d : layout.directions()) {
                    const Position n{p.row + d.row, p.col + d.col};
                    if (!layout.isHole(n)) continue;
                    int& w = m_weight[static_cast<size_t>(idx(n))];
                    if (w == 1000) {
                        w = m_weight[static_cast<size_t>(idx(p))] + 1;
                        q.push(n);
                    }
                }
            }
        }

        m_start = 0;
        m_startPegs = 0;
        m_startScore = 0;
        for (int r = 0; r < game.rows(); ++r)
            for (int c = 0; c < game.cols(); ++c)
                if (game.cellAt({r, c}) == CellState::Peg) {
                    m_start |= (1ULL << idx({r, c}));
                    m_startScore += m_weight[static_cast<size_t>(idx({r, c}))];
                    ++m_startPegs;
                }
        m_bestPegs = m_startPegs;
    }

    Solver::Result run() {
        Solver::Result res;
        const bool solved = dfs(m_start, m_startPegs, m_startScore);
        res.solved = solved;
        res.moves = solved ? m_path : m_best;
        res.pegsRemaining = solved ? 1 : m_bestPegs;
        res.nodesExplored = m_nodes;
        return res;
    }

private:
    struct Candidate {
        int score;
        const Jump* jump;
        std::uint64_t state;
    };

    bool dfs(std::uint64_t state, int pegs, int score) {
        if (pegs < m_bestPegs) {
            m_bestPegs = pegs;
            m_best = m_path;
        }
        if (pegs == 1) return true;
        if (!m_visited.insert(state).second) return false;  // already explored: dead end
        if (++m_nodes > m_budget) {
            m_exhausted = true;
            return false;
        }

        // Collect legal jumps, best-looking (lowest total peg distance) first.
        std::vector<Candidate> next;
        for (const Jump& j : m_jumps) {
            if (!((state >> j.from) & 1) || !((state >> j.over) & 1) || ((state >> j.to) & 1)) continue;
            const std::uint64_t s =
                (state & ~(1ULL << j.from) & ~(1ULL << j.over)) | (1ULL << j.to);
            const int sc = score - m_weight[static_cast<size_t>(j.from)] -
                           m_weight[static_cast<size_t>(j.over)] + m_weight[static_cast<size_t>(j.to)];
            next.push_back({sc, &j, s});
        }
        std::stable_sort(next.begin(), next.end(),
                         [](const Candidate& a, const Candidate& b) { return a.score < b.score; });

        for (const Candidate& c : next) {
            if (m_exhausted) return false;
            m_path.push_back(c.jump->move);
            if (dfs(c.state, pegs - 1, c.score)) return true;
            m_path.pop_back();
        }
        return false;
    }

    long m_budget;
    long m_nodes = 0;
    bool m_exhausted = false;
    std::vector<Jump> m_jumps;
    std::uint64_t m_start = 0;
    int m_startPegs = 0;
    int m_startScore = 0;
    std::vector<int> m_weight;
    std::unordered_set<std::uint64_t> m_visited;
    std::vector<Move> m_path, m_best;
    int m_bestPegs = 0;
};

}  // namespace

namespace Solver {

Result solve(const Game& game, long nodeBudget) {
    Search search(game, nodeBudget);
    return search.run();
}

}  // namespace Solver
