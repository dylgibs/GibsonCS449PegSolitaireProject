#include <QtTest>

#include <algorithm>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "AutoPlayer.h"
#include "BoardLayout.h"
#include "Game.h"
#include "GameRecord.h"
#include "MoveStrategy.h"
#include "RandomState.h"
#include "Replay.h"
#include "Solver.h"

namespace {

// Test doubles for the MoveStrategy interface.
class FixedMoveStrategy : public MoveStrategy {
public:
    explicit FixedMoveStrategy(Move m) : m_move(m) {}
    std::optional<Move> chooseMove(const Game&) override { return m_move; }
    std::string name() const override { return "fixed"; }
private:
    Move m_move;
};
class NoMoveStrategy : public MoveStrategy {
public:
    std::optional<Move> chooseMove(const Game&) override { return std::nullopt; }
    std::string name() const override { return "none"; }
};

const char* const kBoards[] = {"english-standard", "english-large", "hex-standard", "hex-large"};

// Applies a move list to a copy of the game; true if every move was legal.
bool applyAll(Game g, const std::vector<Move>& moves, int* pegsLeft = nullptr) {
    for (const Move& m : moves)
        if (!g.makeMove(m)) return false;
    if (pegsLeft) *pegsLeft = g.pegCount();
    return true;
}

}  // namespace

class TestAutoPlay : public QObject {
    Q_OBJECT

private slots:
    // ---- simple strategies ----
    void firstMoveStrategyReturnsALegalMove() {
        Game g;
        FirstMoveStrategy s;
        auto m = s.chooseMove(g);
        QVERIFY(m.has_value());
        QVERIFY(g.isValidMove(*m));
    }
    void firstMoveStrategyIsDeterministic() {
        Game g;
        FirstMoveStrategy a, b;
        QVERIFY(*a.chooseMove(g) == *b.chooseMove(g));
    }
    void strategiesReturnNothingWhenNoMovesRemain() {
        Game g(BoardLayout("mini", {"..."}, {0, 2}));
        g.makeMove({{0, 0}, {0, 2}});
        QVERIFY(!FirstMoveStrategy().chooseMove(g).has_value());
        QVERIFY(!RandomStrategy(1).chooseMove(g).has_value());
        QVERIFY(!SolverStrategy().chooseMove(g).has_value());
    }
    void randomStrategyOnlyChoosesLegalMoves() {
        Game g;
        RandomStrategy s(5);
        for (int i = 0; i < 25 && !g.isGameOver(); ++i) {
            auto m = s.chooseMove(g);
            QVERIFY(m.has_value());
            QVERIFY(g.isValidMove(*m));
            g.makeMove(*m);
        }
    }
    void randomStrategyIsReproducibleForASeed() {
        Game a, b;
        RandomStrategy sa(42), sb(42);
        AutoPlayer::playToEnd(a, sa);
        AutoPlayer::playToEnd(b, sb);
        QCOMPARE(int(a.history().size()), int(b.history().size()));
        for (size_t i = 0; i < a.history().size(); ++i) QVERIFY(a.history()[i] == b.history()[i]);
    }
    void randomStrategyVariesWithSeed() {
        std::vector<Move> first;
        bool differs = false;
        for (unsigned seed = 1; seed <= 6; ++seed) {
            Game g;
            RandomStrategy s(seed);
            AutoPlayer::playToEnd(g, s);
            if (seed == 1) first = g.history();
            else if (g.history().size() != first.size() ||
                     !std::equal(first.begin(), first.end(), g.history().begin())) differs = true;
        }
        QVERIFY(differs);
    }
    void factoryBuildsRequestedStrategy() {
        QCOMPARE(makeStrategy(StrategyType::Solver, 1)->name(), std::string("Solver"));
        QCOMPARE(makeStrategy(StrategyType::Random, 1)->name(), std::string("Random moves"));
    }

    // ---- AutoPlayer ----
    void stepPlaysExactlyOneMove() {
        Game g;
        FirstMoveStrategy s;
        QVERIFY(AutoPlayer::step(g, s));
        QCOMPARE(g.moveCount(), 1);
        QCOMPARE(g.pegCount(), 31);
        QCOMPARE(int(g.history().size()), 1);
    }
    void stepFailsWhenGameIsOver() {
        Game g(BoardLayout("mini", {"..."}, {0, 2}));
        g.makeMove({{0, 0}, {0, 2}});
        FirstMoveStrategy s;
        QVERIFY(!AutoPlayer::step(g, s));
        QCOMPARE(g.moveCount(), 1);
    }
    void stepFailsWhenStrategyHasNoMove() {
        Game g;
        NoMoveStrategy s;
        QVERIFY(!AutoPlayer::step(g, s));
        QCOMPARE(g.moveCount(), 0);
    }
    void stepRejectsIllegalMoveFromStrategy() {
        Game g;
        FixedMoveStrategy s(Move{{0, 3}, {2, 3}});  // destination occupied
        QVERIFY(!AutoPlayer::step(g, s));
        QCOMPARE(g.moveCount(), 0);
        QCOMPARE(g.pegCount(), 32);
    }
    void playToEndFinishesEveryBoardWithEveryStrategy() {
        for (const char* key : kBoards) {
            for (int kind = 0; kind < 2; ++kind) {
                Game g(BoardLayout::fromKey(key));
                const int holes = g.layout().holeCount();
                std::unique_ptr<MoveStrategy> s = kind == 0 ? std::unique_ptr<MoveStrategy>(new FirstMoveStrategy)
                                                            : std::unique_ptr<MoveStrategy>(new RandomStrategy(3));
                const int played = AutoPlayer::playToEnd(g, *s);
                QVERIFY2(g.isGameOver(), key);
                QCOMPARE(played, g.moveCount());
                QCOMPARE(g.pegCount(), holes - 1 - played);  // each move removes exactly one peg
                QVERIFY(g.pegCount() >= 1);
            }
        }
    }
    void playToEndRespectsMaxMoves() {
        Game g;
        FirstMoveStrategy s;
        QCOMPARE(AutoPlayer::playToEnd(g, s, 3), 3);
        QCOMPARE(g.moveCount(), 3);
        QVERIFY(!g.isGameOver());
    }
    void playToEndOnFinishedGameDoesNothing() {
        Game g(BoardLayout("mini", {"..."}, {0, 2}));
        g.makeMove({{0, 0}, {0, 2}});
        FirstMoveStrategy s;
        QCOMPARE(AutoPlayer::playToEnd(g, s), 0);
    }
    void autoPlayedGameCanBeRecordedAndReplayed() {
        Game g(BoardLayout::hexagonal(BoardSize::Standard));
        RandomStrategy s(9);
        AutoPlayer::playToEnd(g, s);
        GameRecord rec = GameRecord::fromGame(g, "2026-01-01 00:00:00");
        Replay rp(rec);
        rp.toEnd();
        QCOMPARE(rp.game().pegCount(), g.pegCount());
        QVERIFY(rp.game().isGameOver());
    }

    // ---- Solver ----
    void solverSolvesTheStandardEnglishBoard() {
        Game g;
        Solver::Result r = Solver::solve(g);
        QVERIFY(r.solved);
        QCOMPARE(r.pegsRemaining, 1);
        QCOMPARE(int(r.moves.size()), 31);
        int left = 0;
        QVERIFY(applyAll(g, r.moves, &left));
        QCOMPARE(left, 1);
    }
    void solverPlansAreLegalOnEveryBoard() {
        for (const char* key : kBoards) {
            Game g(BoardLayout::fromKey(key));
            Solver::Result r = Solver::solve(g, 20000);
            int left = 0;
            QVERIFY2(applyAll(g, r.moves, &left), key);
            QCOMPARE(left, r.pegsRemaining);
            QVERIFY(r.nodesExplored <= 20001);
        }
    }
    void solverWorksFromAMidGamePosition() {
        std::mt19937 rng(7);
        Game g = RandomState::generate(BoardLayout::standard(), rng, 6);
        Solver::Result r = Solver::solve(g, 50000);
        int left = 0;
        QVERIFY(applyAll(g, r.moves, &left));
        QCOMPARE(left, r.pegsRemaining);
        QVERIFY(left < g.pegCount() || g.isGameOver());
    }
    void solverReportsBestEffortWhenBudgetIsTiny() {
        Game g;
        Solver::Result r = Solver::solve(g, 3);
        QVERIFY(!r.solved);
        QVERIFY(applyAll(g, r.moves));
    }
    void solverReportsUnsolvablePosition() {
        // pegs at 1,2,3 and a hole at 0: one jump is possible, then two pegs are stuck
        Game g(BoardLayout("mini", {"...."}, {0, 0}));
        Solver::Result r = Solver::solve(g);
        QVERIFY(!r.solved);
        QCOMPARE(r.pegsRemaining, 2);
        QCOMPARE(int(r.moves.size()), 1);
    }
    void solverRecognisesAnAlreadySolvedPosition() {
        Game g(BoardLayout("one", {".."}, {0, 1}));  // single peg
        QCOMPARE(g.pegCount(), 1);
        Solver::Result r = Solver::solve(g);
        QVERIFY(r.solved);
        QVERIFY(r.moves.empty());
    }
    void solverRejectsBoardsWithMoreThan64Holes() {
        Game huge(BoardLayout("huge", {std::string(65, '.')}, {0, 0}));
        QVERIFY_EXCEPTION_THROWN(Solver::solve(huge), std::invalid_argument);
    }

    // ---- SolverStrategy ----
    void solverStrategyPlaysAWinningGame() {
        Game g;
        SolverStrategy s;
        const int played = AutoPlayer::playToEnd(g, s);
        QVERIFY(g.isGameOver());
        QVERIFY(g.isWon());
        QCOMPARE(played, 31);
    }
    void solverStrategyReplansIfGameLeavesThePlan() {
        Game g;
        SolverStrategy s;
        Move planned = *s.chooseMove(g);  // planned but NOT played
        Move other = planned;
        for (const Move& m : g.allValidMoves()) if (!(m == planned)) { other = m; break; }
        QVERIFY(!(other == planned));
        QVERIFY(g.makeMove(other));       // game deviates from the plan
        auto next = s.chooseMove(g);
        QVERIFY(next.has_value());
        QVERIFY(g.isValidMove(*next));
    }
    void solverStrategyStillPlaysLegalMovesWhenSolverCannotHelp() {
        Game huge(BoardLayout("huge", {std::string(65, '.')}, {0, 0}));
        SolverStrategy s;
        QVERIFY(AutoPlayer::step(huge, s));  // falls back to the first legal move
        QCOMPARE(huge.moveCount(), 1);
    }
    void solverStrategyFinishesLossesToo() {
        Game g(BoardLayout("mini", {"...."}, {0, 0}));
        SolverStrategy s;
        AutoPlayer::playToEnd(g, s);
        QVERIFY(g.isGameOver());
        QVERIFY(!g.isWon());
        QCOMPARE(g.pegCount(), 2);
    }
    void solverStrategyAutoPlaysAHexBoardToTheEnd() {
        Game g(BoardLayout::hexagonal(BoardSize::Standard));
        SolverStrategy s(20000);
        AutoPlayer::playToEnd(g, s);
        QVERIFY(g.isGameOver());
    }
};

QTEST_APPLESS_MAIN(TestAutoPlay)
#include "tst_auto.moc"
