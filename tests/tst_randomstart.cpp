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

namespace {
const char* const kBoards[] = {"english-standard", "english-large", "hex-standard", "hex-large"};
}

class TestRandomStart : public QObject {
    Q_OBJECT

private slots:
    void zeroMovesGivesTheNormalStart() {
        std::mt19937 rng(1);
        Game g = RandomState::generate(BoardLayout::standard(), rng, 0);
        QCOMPARE(g.pegCount(), 32);
        QCOMPARE(g.moveCount(), 0);
        QCOMPARE(g.setupMoveCount(), 0);
    }
    void preplaysTheRequestedNumberOfMoves() {
        std::mt19937 rng(1);
        Game g = RandomState::generate(BoardLayout::standard(), rng, 5);
        QCOMPARE(g.setupMoveCount(), 5);
        QCOMPARE(g.moveCount(), 5);
        QCOMPARE(g.pegCount(), 27);
        QCOMPARE(int(g.history().size()), 5);
    }
    void generatedGamesAreNeverOver() {
        for (const char* key : kBoards)
            for (unsigned seed = 1; seed <= 20; ++seed) {
                std::mt19937 rng(seed);
                BoardLayout l = BoardLayout::fromKey(key);
                Game g = RandomState::generate(l, rng, RandomState::randomMoveCount(l, rng));
                QVERIFY2(!g.isGameOver(), key);
                QVERIFY(!g.allValidMoves().empty());
            }
    }
    void generatedPositionsAreReachable() {
        for (const char* key : kBoards) {
            std::mt19937 rng(11);
            BoardLayout l = BoardLayout::fromKey(key);
            Game g = RandomState::generate(l, rng, 8);
            Game again(l);  // replaying the history from the normal start gives the same position
            for (const Move& m : g.history()) QVERIFY(again.makeMove(m));
            for (int r = 0; r < g.rows(); ++r)
                for (int c = 0; c < g.cols(); ++c)
                    QCOMPARE(int(again.cellAt({r, c})), int(g.cellAt({r, c})));
        }
    }
    void sameSeedGivesSameState() {
        std::mt19937 a(77), b(77);
        Game ga = RandomState::generate(BoardLayout::standard(), a, 7);
        Game gb = RandomState::generate(BoardLayout::standard(), b, 7);
        QCOMPARE(int(ga.history().size()), int(gb.history().size()));
        for (size_t i = 0; i < ga.history().size(); ++i) QVERIFY(ga.history()[i] == gb.history()[i]);
    }
    void differentSeedsGiveDifferentStates() {
        std::vector<std::vector<Move>> seen;
        for (unsigned seed = 1; seed <= 8; ++seed) {
            std::mt19937 rng(seed);
            Game g = RandomState::generate(BoardLayout::standard(), rng, 8);
            bool dup = false;
            for (const auto& h : seen)
                dup = dup || (h.size() == g.history().size() && std::equal(h.begin(), h.end(), g.history().begin()));
            if (!dup) seen.push_back(g.history());
        }
        QVERIFY(seen.size() >= 2);
    }
    void neverPlaysIntoAFinishedGame() {
        // Only one legal first move exists and it would end the game, so nothing is pre-played.
        std::mt19937 rng(3);
        Game g = RandomState::generate(BoardLayout("mini", {"...."}, {0, 0}), rng, 10);
        QCOMPARE(g.setupMoveCount(), 0);
        QVERIFY(!g.isGameOver());
    }
    void stopsEarlyWhenFurtherMovesWouldEndTheGame() {
        std::mt19937 rng(3);
        Game g = RandomState::generate(BoardLayout::standard(), rng, 500);
        QVERIFY(g.setupMoveCount() < 31);
        QVERIFY(!g.isGameOver());
    }
    void randomMoveCountStaysInBounds() {
        std::mt19937 rng(5);
        for (const char* key : kBoards) {
            BoardLayout l = BoardLayout::fromKey(key);
            const int hi = std::max(2, (l.holeCount() - 1) / 2);
            for (int i = 0; i < 200; ++i) {
                const int n = RandomState::randomMoveCount(l, rng);
                QVERIFY(n >= 2 && n <= hi);
            }
        }
    }
    void newGameClearsTheSetupCount() {
        std::mt19937 rng(1);
        Game g = RandomState::generate(BoardLayout::standard(), rng, 5);
        g.newGame();
        QCOMPARE(g.setupMoveCount(), 0);
        QCOMPARE(g.pegCount(), 32);
    }
    void movesAfterTheRandomStartAreNotSetupMoves() {
        std::mt19937 rng(1);
        Game g = RandomState::generate(BoardLayout::standard(), rng, 4);
        QVERIFY(g.makeMove(g.allValidMoves().front()));
        QCOMPARE(g.setupMoveCount(), 4);
        QCOMPARE(g.moveCount(), 5);
    }
    void randomStartGamesCanBeAutoPlayedRecordedAndReplayed() {
        for (const char* key : kBoards) {
            std::mt19937 rng(21);
            BoardLayout l = BoardLayout::fromKey(key);
            Game g = RandomState::generate(l, rng, 6);
            RandomStrategy s(4);
            AutoPlayer::playToEnd(g, s);
            QVERIFY2(g.isGameOver(), key);
            Replay rp(GameRecord::fromGame(g, "t"));  // history includes the pre-played moves
            rp.toEnd();
            QCOMPARE(rp.game().pegCount(), g.pegCount());
        }
    }
};

QTEST_APPLESS_MAIN(TestRandomStart)
#include "tst_randomstart.moc"
