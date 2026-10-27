#include <QtTest>

#include "BoardLayout.h"
#include "Game.h"

class TestGame : public QObject {
    Q_OBJECT

private slots:
    //  layouts
    // AC 4.3 - Allow additional moves
    void additionalMovesAllowedAfterValidMove() {
        Game g;
        QVERIFY(g.makeMove({{3, 1}, {3, 3}}));        // first jump
        QVERIFY(!g.isGameOver());                     // game continues
        QVERIFY(!g.allValidMoves().empty());          // more moves are available

        QVERIFY(g.makeMove({{1, 2}, {3, 2}}));        // second jump is accepted
        QCOMPARE(g.cellAt({1, 2}), CellState::Empty);
        QCOMPARE(g.cellAt({2, 2}), CellState::Empty); // jumped peg removed
        QCOMPARE(g.cellAt({3, 2}), CellState::Peg);
        QCOMPARE(g.pegCount(), 30);
        QCOMPARE(g.moveCount(), 2);
    }

    // AC 5.3 - Restart the game after a win/fail state
    void restartAfterGameOverKeepsConfiguration() {
        // Tiny board: one jump ends the game with a win.
        Game g(BoardLayout("mini", {"..."}, {0, 2}));
        QVERIFY(g.makeMove({{0, 0}, {0, 2}}));
        QVERIFY(g.isGameOver());

        g.newGame();                                  // restart
        QVERIFY(!g.isGameOver());
        QCOMPARE(g.pegCount(), 2);
        QCOMPARE(g.moveCount(), 0);
        QCOMPARE(g.cellAt({0, 2}), CellState::Empty);
        QCOMPARE(g.layout().name(), std::string("mini"));
        QVERIFY(g.makeMove({{0, 0}, {0, 2}}));        // playable again

        // Same restart behaviour on the 9x9 board.
        Game big(BoardLayout::large());
        QVERIFY(big.makeMove({{4, 2}, {4, 4}}));
        big.newGame();
        QCOMPARE(big.rows(), 9);
        QCOMPARE(big.pegCount(), 56);
        QCOMPARE(big.moveCount(), 0);
    }
    void standardLayoutHas33Holes() {
        QCOMPARE(BoardLayout::standard().holeCount(), 33);
        QCOMPARE(BoardLayout::standard().rows(), 7);
    }
    void largeLayoutHas57HolesOn9x9() {
        BoardLayout l = BoardLayout::large();
        QCOMPARE(l.holeCount(), 57);
        QCOMPARE(l.rows(), 9);
        QCOMPARE(l.cols(), 9);
    }
    void createMapsBoardTypes() {
        QCOMPARE(BoardLayout::create(BoardType::Standard).holeCount(), 33);
        QCOMPARE(BoardLayout::create(BoardType::Large).holeCount(), 57);
    }
    void invalidLayoutsThrow() {
        QVERIFY_EXCEPTION_THROWN(BoardLayout("x", {}, {0, 0}), std::invalid_argument);
        QVERIFY_EXCEPTION_THROWN(BoardLayout("x", {"..", "."}, {0, 0}), std::invalid_argument);
        QVERIFY_EXCEPTION_THROWN(BoardLayout("x", {"#."}, {0, 0}), std::invalid_argument);
    }

    // ---- new game ----
    void newGameStandardInitialState() {
        Game g(BoardLayout::standard());
        QCOMPARE(g.pegCount(), 32);
        QCOMPARE(g.moveCount(), 0);
        QCOMPARE(g.cellAt({3, 3}), CellState::Empty);
        QCOMPARE(g.cellAt({0, 3}), CellState::Peg);
        QCOMPARE(g.cellAt({0, 0}), CellState::Invalid);
        QCOMPARE(g.cellAt({-1, 0}), CellState::Invalid);
        QCOMPARE(g.cellAt({7, 7}), CellState::Invalid);
    }
    void newGameLargeInitialState() {
        Game g(BoardLayout::large());
        QCOMPARE(g.pegCount(), 56);
        QCOMPARE(g.cellAt({4, 4}), CellState::Empty);
    }
    void newGameResetsAfterMoves() {
        Game g;
        QVERIFY(g.makeMove({{3, 1}, {3, 3}}));
        g.newGame();
        QCOMPARE(g.pegCount(), 32);
        QCOMPARE(g.moveCount(), 0);
        QCOMPARE(g.cellAt({3, 3}), CellState::Empty);
    }
    void newGameCanSwitchLayout() {
        Game g;
        g.newGame(BoardLayout::large());
        QCOMPARE(g.rows(), 9);
        QCOMPARE(g.pegCount(), 56);
    }

    // ---- moves ----
    void initialStandardHasFourMoves() {
        Game g;
        QCOMPARE(int(g.allValidMoves().size()), 4);
    }
    void initialLargeHasFourMoves() {
        Game g(BoardLayout::large());
        QCOMPARE(int(g.allValidMoves().size()), 4);
    }
    void validMoveUpdatesBoard() {
        Game g;
        QVERIFY(g.makeMove({{3, 1}, {3, 3}}));
        QCOMPARE(g.cellAt({3, 1}), CellState::Empty);
        QCOMPARE(g.cellAt({3, 2}), CellState::Empty);
        QCOMPARE(g.cellAt({3, 3}), CellState::Peg);
        QCOMPARE(g.pegCount(), 31);
        QCOMPARE(g.moveCount(), 1);
    }
    void moveWorksInAllFourDirections() {
        Game g;
        QVERIFY(g.isValidMove({{3, 1}, {3, 3}}));  // right
        QVERIFY(g.isValidMove({{3, 5}, {3, 3}}));  // left
        QVERIFY(g.isValidMove({{1, 3}, {3, 3}}));  // down
        QVERIFY(g.isValidMove({{5, 3}, {3, 3}}));  // up
    }
    void invalidMovesAreRejected() {
        Game g;
        QVERIFY(!g.isValidMove({{3, 1}, {3, 2}}));  // only one cell
        QVERIFY(!g.isValidMove({{3, 0}, {3, 3}}));  // three cells
        QVERIFY(!g.isValidMove({{1, 1}, {3, 3}}));  // diagonal
        QVERIFY(!g.isValidMove({{2, 2}, {4, 4}}));  // diagonal
        QVERIFY(!g.isValidMove({{3, 3}, {3, 5}}));  // from an empty cell
        QVERIFY(!g.isValidMove({{1, 2}, {1, 4}}));  // destination occupied
        QVERIFY(!g.isValidMove({{0, 3}, {-2, 3}}));  // off the board
        QVERIFY(!g.isValidMove({{2, 1}, {2, -1}}));  // off the board
    }
    void cannotJumpOverEmptyCell() {
        Game g;
        QVERIFY(g.makeMove({{3, 1}, {3, 3}}));  // (3,2) and (3,1) now empty
        QVERIFY(!g.isValidMove({{3, 0}, {3, 2}}));  // jumps empty (3,1)
    }
    void cannotLandOnInvalidCell() {
        Game g;
        QVERIFY(!g.isValidMove({{2, 2}, {0, 2}}));  // (0,2) is not part of the board
    }
    void illegalMoveLeavesStateUnchanged() {
        Game g;
        QVERIFY(!g.makeMove({{0, 3}, {2, 3}}));
        QCOMPARE(g.pegCount(), 32);
        QCOMPARE(g.moveCount(), 0);
        QCOMPARE(g.cellAt({0, 3}), CellState::Peg);
    }
    void validMovesFromReturnsOnlyLegal() {
        Game g;
        QCOMPARE(int(g.validMovesFrom({3, 1}).size()), 1);
        QCOMPARE(int(g.validMovesFrom({0, 3}).size()), 0);
        QCOMPARE(int(g.validMovesFrom({3, 3}).size()), 0);  // empty cell
    }

    //  game over
    void gameNotOverAtStart() {
        Game g;
        QVERIFY(!g.isGameOver());
        QVERIFY(!g.isWon());
    }
    void winWhenOnePegLeft() {
        // pegs at 0,1; hole at 2
        Game g(BoardLayout("mini", {"..."}, {0, 2}));
        QCOMPARE(g.pegCount(), 2);
        QVERIFY(!g.isGameOver());
        QVERIFY(g.makeMove({{0, 0}, {0, 2}}));
        QVERIFY(g.isGameOver());
        QVERIFY(g.isWon());
        QCOMPARE(g.pegCount(), 1);
    }
    void perfectWinWhenLastPegInStartHole() {
        Game g(BoardLayout("mini", {"..."}, {0, 2}));
        QVERIFY(g.makeMove({{0, 0}, {0, 2}}));
        QVERIFY(g.isPerfectWin());  // last peg sits in the start-empty cell
    }
    void lossWhenStuckWithMultiplePegs() {
        // pegs at 1,2,3; hole at 0 -> after 2 jumps over 1, pegs at 0 and 3 are stuck
        Game g(BoardLayout("mini", {"...."}, {0, 0}));
        QVERIFY(g.makeMove({{0, 2}, {0, 0}}));
        QVERIFY(g.isGameOver());
        QVERIFY(!g.isWon());
        QVERIFY(!g.isPerfectWin());
        QCOMPARE(g.pegCount(), 2);
    }
    void gameOverBlocksFurtherMoves() {
        Game g(BoardLayout("mini", {"..."}, {0, 2}));
        QVERIFY(g.makeMove({{0, 0}, {0, 2}}));
        QVERIFY(g.allValidMoves().empty());
        QVERIFY(!g.makeMove({{0, 2}, {0, 0}}));
    }
};

QTEST_APPLESS_MAIN(TestGame)
#include "tst_game.moc"
