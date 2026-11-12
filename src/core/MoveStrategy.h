#pragma once

#include <memory>
#include <optional>
#include <random>
#include <string>
#include <vector>

#include "Game.h"

// Decides which move an automated player makes. Add a new strategy by deriving from this
// class (and, optionally, adding a StrategyType + a line in makeStrategy()).
class MoveStrategy {
public:
    virtual ~MoveStrategy() = default;
    // Returns the next move, or nullopt if there is none to make.
    virtual std::optional<Move> chooseMove(const Game& game) = 0;
    virtual std::string name() const = 0;
};

// Always plays the first legal move found (deterministic, no lookahead).
class FirstMoveStrategy : public MoveStrategy {
public:
    std::optional<Move> chooseMove(const Game& game) override;
    std::string name() const override { return "First legal move"; }
};

// Plays a uniformly random legal move. The seed makes a game reproducible.
class RandomStrategy : public MoveStrategy {
public:
    explicit RandomStrategy(unsigned seed = std::random_device{}()) : m_rng(seed) {}
    std::optional<Move> chooseMove(const Game& game) override;
    std::string name() const override { return "Random moves"; }

private:
    std::mt19937 m_rng;
};

// Searches for a winning sequence (see Solver) and plays it. If no win is found within
// the node budget it plays the best sequence found, and keeps re-planning as needed.
class SolverStrategy : public MoveStrategy {
public:
    explicit SolverStrategy(long nodeBudget = 300000) : m_budget(nodeBudget) {}
    std::optional<Move> chooseMove(const Game& game) override;
    std::string name() const override { return "Solver"; }

private:
    void plan(const Game& game);

    long m_budget;
    std::vector<Move> m_plan;
    size_t m_planIndex = 0;
    int m_planBase = 0;  // game.moveCount() when the plan was made
    bool m_planned = false;
};

enum class StrategyType { Solver, Random };
std::unique_ptr<MoveStrategy> makeStrategy(StrategyType type, unsigned seed);
