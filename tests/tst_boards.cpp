#include <QtTest>

#include "BoardLayout.h"
#include "Game.h"

// Board size (Standard / 9x9) x board type (English / Hexagonal).
class TestBoards : public QObject {
    Q_OBJECT

private slots:
    // ---- layouts ----
    void hexStandardHas37HolesOn7x7() {
        BoardLayout l = BoardLayout::hexagonal(BoardSize::Standard);
        QCOMPARE(l.holeCount(), 37);
        QCOMPARE(l.rows(), 7);
        QCOMPARE(l.cols(), 7);
        QCOMPARE(int(l.geometry()), int(Geometry::Hexagonal));
    }
    void hexLargeHas61HolesOn9x9() {
        BoardLayout l = BoardLayout::hexagonal(BoardSize::Large);
        QCOMPARE(l.holeCount(), 61);
        QCOMPARE(l.rows(), 9);
        QCOMPARE(l.cols(), 9);
    }
    void hexBoardIsSymmetricHexagon() {
        BoardLayout l = BoardLayout::hexagonal(BoardSize::Standard);
        // corners of the 7x7 grid that lie outside the hexagon
        QVERIFY(!l.isHole({0, 0}));
        QVERIFY(!l.isHole({6, 6}));
        // the six corners of the hexagon itself
        QVERIFY(l.isHole({0, 3}));
        QVERIFY(l.isHole({0, 6}));
        QVERIFY(l.isHole({3, 0}));
        QVERIFY(l.isHole({3, 6}));
        QVERIFY(l.isHole({6, 0}));
        QVERIFY(l.isHole({6, 3}));
    }
    void englishBoardsKeepSquareGeometry() {
        QCOMPARE(int(BoardLayout::english(BoardSize::Standard).geometry()), int(Geometry::Square));
        QCOMPARE(int(BoardLayout::english(BoardSize::Large).geometry()), int(Geometry::Square));
    }
    void createCombinesTypeAndSize() {
        QCOMPARE(BoardLayout::create(BoardType::English, BoardSize::Standard).holeCount(), 33);
        QCOMPARE(BoardLayout::create(BoardType::English, BoardSize::Large).holeCount(), 57);
        QCOMPARE(BoardLayout::create(BoardType::Hexagonal, BoardSize::Standard).holeCount(), 37);
        QCOMPARE(BoardLayout::create(BoardType::Hexagonal, BoardSize::Large).holeCount(), 61);
    }

    // ---- keys (used by the recording file) ----
    void keysRoundTripForEveryBoard() {
        for (BoardType t : {BoardType::English, BoardType::Hexagonal})
            for (BoardSize s : {BoardSize::Standard, BoardSize::Large}) {
                BoardLayout l = BoardLayout::create(t, s);
                QCOMPARE(BoardLayout::fromKey(l.key()).key(), l.key());
                QCOMPARE(BoardLayout::fromKey(l.key()).holeCount(), l.holeCount());
            }
    }
    void keysAreDistinct() {
        QCOMPARE(BoardLayout::english(BoardSize::Standard).key(), std::string("english-standard"));
        QCOMPARE(BoardLayout::english(BoardSize::Large).key(), std::string("english-large"));
        QCOMPARE(BoardLayout::hexagonal(BoardSize::Standard).key(), std::string("hex-standard"));
        QCOMPARE(BoardLayout::hexagonal(BoardSize::Large).key(), std::string("hex-large"));
    }
    void unknownKeyThrows() {
        QVERIFY_EXCEPTION_THROWN(BoardLayout::fromKey("triangle-tiny"), std::invalid_argument);
    }

    // ---- hexagonal rules ----
    void hexNewGameState() {
        Game g(BoardLayout::hexagonal(BoardSize::Standard));
        QCOMPARE(g.pegCount(), 36);
        QCOMPARE(g.cellAt({3, 3}), CellState::Empty);
        QCOMPARE(g.cellAt({0, 0}), CellState::Invalid);
        Game big(BoardLayout::hexagonal(BoardSize::Large));
        QCOMPARE(big.pegCount(), 60);
        QCOMPARE(big.cellAt({4, 4}), CellState::Empty);
    }
    void hexStartHasSixMoves() {
        Game g(BoardLayout::hexagonal(BoardSize::Standard));
        QCOMPARE(int(g.allValidMoves().size()), 6);
        Game big(BoardLayout::hexagonal(BoardSize::Large));
        QCOMPARE(int(big.allValidMoves().size()), 6);
    }
    void hexDiagonalJumpWorks() {
        Game g(BoardLayout::hexagonal(BoardSize::Standard));
        // (5,1) jumps over (4,2) into the centre (3,3): direction (-1,+1)
        QVERIFY(g.isValidMove({{5, 1}, {3, 3}}));
        QVERIFY(g.makeMove({{5, 1}, {3, 3}}));
        QCOMPARE(g.cellAt({4, 2}), CellState::Empty);
        QCOMPARE(g.cellAt({3, 3}), CellState::Peg);
        QCOMPARE(g.pegCount(), 35);
    }
    void hexRejectsNonHexDirection() {
        Game g(BoardLayout::hexagonal(BoardSize::Standard));
        // (1,1)-style diagonal is not a neighbour direction on a hex grid
        QVERIFY(!g.isValidMove({{1, 1}, {3, 3}}));
        QVERIFY(!g.isValidMove({{5, 5}, {3, 3}}));
    }
    void englishStillRejectsDiagonalJumps() {
        Game g;
        QVERIFY(!g.isValidMove({{5, 1}, {3, 3}}));
    }
    void hexGameCanBePlayedToTheEnd() {
        Game g(BoardLayout::hexagonal(BoardSize::Large));
        int guard = 0;
        while (!g.isGameOver() && guard++ < 200) QVERIFY(g.makeMove(g.allValidMoves().front()));
        QVERIFY(g.isGameOver());
        QVERIFY(g.pegCount() >= 1);
        QCOMPARE(g.moveCount(), 60 - g.pegCount());
    }
};

QTEST_APPLESS_MAIN(TestBoards)
#include "tst_boards.moc"
