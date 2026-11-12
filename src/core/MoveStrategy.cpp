#include "MoveStrategy.h"

#include <stdexcept>

#include "Solver.h"

std::optional<Move> FirstMoveStrategy::chooseMove(const Game& game) {
    const auto moves = game.allValidMoves();
    if (moves.empty()) return std::nullopt;
    return moves.front();
}

std::optional<Move> RandomStrategy::chooseMove(const Game& game) {
    const auto moves = game.allValidMoves();
    if (moves.empty()) return std::nullopt;
    std::uniform_int_distribution<size_t> pick(0, moves.size() - 1);
    return moves[pick(m_rng)];
}

void SolverStrategy::plan(const Game& game) {
    m_planned = true;
    m_planIndex = 0;
    m_planBase = game.moveCount();
    try {
        m_plan = Solver::solve(game, m_budget).moves;
    } catch (const std::invalid_argument&) {
        m_plan.clear();  // board too big for the solver: fall back below
    }
}

std::optional<Move> SolverStrategy::chooseMove(const Game& game) {
    if (game.isGameOver()) return std::nullopt;

    const bool usable = m_planned && m_planIndex < m_plan.size() &&
                        game.moveCount() == m_planBase + static_cast<int>(m_planIndex) &&
                        game.isValidMove(m_plan[m_planIndex]);
    if (!usable) plan(game);  // first call, plan used up, or the game went off-plan

    if (m_planIndex < m_plan.size() && game.isValidMove(m_plan[m_planIndex]))
        return m_plan[m_planIndex++];

    const auto moves = game.allValidMoves();  // fallback so the game always progresses
    if (moves.empty()) return std::nullopt;
    return moves.front();
}

std::unique_ptr<MoveStrategy> makeStrategy(StrategyType type, unsigned seed) {
    switch (type) {
    case StrategyType::Solver: return std::make_unique<SolverStrategy>();
    case StrategyType::Random: return std::make_unique<RandomStrategy>(seed);
    }
    throw std::invalid_argument("makeStrategy: unknown strategy type");
}
