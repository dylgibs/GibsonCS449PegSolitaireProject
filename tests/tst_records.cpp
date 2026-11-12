#include <QtTest>

#include <sstream>

#include "BoardLayout.h"
#include "Game.h"
#include "GameRecord.h"
#include "GameRecordFile.h"
#include "Replay.h"

namespace {
// Deterministic "player": always take the first legal move until none remain.
void playToEnd(Game& g) {
    for (;;) {
        auto moves = g.allValidMoves();
        if (moves.empty()) return;
        g.makeMove(moves.front());
    }
}
GameRecord finishedRecord(const BoardLayout& layout) {
    Game g(layout);
    playToEnd(g);
    return GameRecord::fromGame(g, "2026-01-02 03:04:05");
}
}  // namespace

class TestRecords : public QObject {
    Q_OBJECT

private slots:
    // ---- history ----
    void historyStartsEmptyAndTracksMoves() {
        Game g;
        QVERIFY(g.history().empty());
        QVERIFY(g.makeMove({{3, 1}, {3, 3}}));
        QCOMPARE(int(g.history().size()), 1);
        QVERIFY(g.history()[0] == (Move{{3, 1}, {3, 3}}));
    }
    void rejectedMovesAreNotRecorded() {
        Game g;
        QVERIFY(!g.makeMove({{0, 3}, {2, 3}}));
        QVERIFY(g.history().empty());
    }
    void newGameClearsHistory() {
        Game g;
        g.makeMove({{3, 1}, {3, 3}});
        g.newGame();
        QVERIFY(g.history().empty());
    }

    // ---- GameRecord ----
    void onlyFinishedGamesCanBeRecorded() {
        Game g;
        QVERIFY_EXCEPTION_THROWN(GameRecord::fromGame(g, "t"), std::logic_error);
    }
    void recordCapturesBoardMovesAndOutcome() {
        Game g;
        playToEnd(g);
        GameRecord r = GameRecord::fromGame(g, "stamp");
        QCOMPARE(r.boardKey, std::string("english-standard"));
        QCOMPARE(r.timestamp, std::string("stamp"));
        QCOMPARE(r.pegsRemaining, g.pegCount());
        QCOMPARE(int(r.moves.size()), g.moveCount());
        QCOMPARE(int(r.outcome), int(g.isPerfectWin() ? Outcome::PerfectWin
                                     : g.isWon() ? Outcome::Won : Outcome::Lost));
    }
    void winAndPerfectOutcomesAreDetected() {
        Game win(BoardLayout("mini", {"..."}, {0, 2}));
        win.makeMove({{0, 0}, {0, 2}});
        QCOMPARE(int(GameRecord::fromGame(win, "t").outcome), int(Outcome::PerfectWin));
        Game lose(BoardLayout("mini", {"...."}, {0, 0}));
        lose.makeMove({{0, 2}, {0, 0}});
        QCOMPARE(int(GameRecord::fromGame(lose, "t").outcome), int(Outcome::Lost));
    }
    void timestampHasExpectedFormat() {
        const std::string t = GameRecord::currentTimestamp();
        QCOMPARE(int(t.size()), 19);  // YYYY-MM-DD HH:MM:SS
        QCOMPARE(t[4], '-');
        QCOMPARE(t[13], ':');
    }
    void outcomeStringsRoundTrip() {
        for (Outcome o : {Outcome::Lost, Outcome::Won, Outcome::PerfectWin})
            QCOMPARE(int(outcomeFromString(toString(o))), int(o));
        QVERIFY_EXCEPTION_THROWN(outcomeFromString("draw"), std::invalid_argument);
    }

    // ---- text format ----
    void serializeProducesReadableText() {
        GameRecord r;
        r.boardKey = "english-standard";
        r.timestamp = "2026-11-10 14:32:05";
        r.outcome = Outcome::Lost;
        r.pegsRemaining = 5;
        r.moves = {{{3, 1}, {3, 3}}, {{1, 2}, {3, 2}}};
        const std::string expected =
            "PEG-SOLITAIRE-RECORD v1\n"
            "board: english-standard\n"
            "date: 2026-11-10 14:32:05\n"
            "result: lost\n"
            "pegs: 5\n"
            "moves: 2\n"
            "3,1 3,3\n"
            "1,2 3,2\n";
        QCOMPARE(RecordFile::serialize(r), expected);
    }
    void roundTripPreservesEverything() {
        for (const char* key : {"english-standard", "english-large", "hex-standard", "hex-large"}) {
            GameRecord a = finishedRecord(BoardLayout::fromKey(key));
            std::istringstream in(RecordFile::serialize(a));
            auto parsed = RecordFile::parse(in);
            QCOMPARE(int(parsed.size()), 1);
            const GameRecord& b = parsed[0];
            QCOMPARE(b.boardKey, a.boardKey);
            QCOMPARE(b.timestamp, a.timestamp);
            QCOMPARE(int(b.outcome), int(a.outcome));
            QCOMPARE(b.pegsRemaining, a.pegsRemaining);
            QCOMPARE(int(b.moves.size()), int(a.moves.size()));
            for (size_t i = 0; i < a.moves.size(); ++i) QVERIFY(a.moves[i] == b.moves[i]);
        }
    }
    void oneFileCanHoldSeveralGames() {
        std::string text = RecordFile::serialize(finishedRecord(BoardLayout::standard())) + "\n" +
                           RecordFile::serialize(finishedRecord(BoardLayout::hexagonal(BoardSize::Large)));
        std::istringstream in(text);
        auto parsed = RecordFile::parse(in);
        QCOMPARE(int(parsed.size()), 2);
        QCOMPARE(parsed[0].boardKey, std::string("english-standard"));
        QCOMPARE(parsed[1].boardKey, std::string("hex-large"));
    }
    void emptyInputGivesNoRecords() {
        std::istringstream in("\n\n");
        QVERIFY(RecordFile::parse(in).empty());
    }
    void malformedFilesAreRejected() {
        const char* bad[] = {
            "not a record\n",
            "PEG-SOLITAIRE-RECORD v1\nboard: english-standard\n",                              // truncated
            "PEG-SOLITAIRE-RECORD v1\nresult: lost\nmoves: 0\n",                               // no board
            "PEG-SOLITAIRE-RECORD v1\nboard: english-standard\nmoves: 0\n",                    // no result
            "PEG-SOLITAIRE-RECORD v1\nboard: x\nresult: draw\nmoves: 0\n",                     // bad result
            "PEG-SOLITAIRE-RECORD v1\nboard: x\nresult: lost\nflavour: odd\nmoves: 0\n",       // unknown field
            "PEG-SOLITAIRE-RECORD v1\nboard: x\nresult: lost\nmoves: 2\n1,1 1,3\n",           // too few moves
            "PEG-SOLITAIRE-RECORD v1\nboard: x\nresult: lost\nmoves: 1\nhello\n",             // bad move line
            "PEG-SOLITAIRE-RECORD v1\nboard: x\nresult: lost\nmoves: abc\n",                  // bad number
        };
        for (const char* text : bad) {
            std::istringstream in(text);
            { bool threw = false; try { RecordFile::parse(in); } catch (const std::runtime_error&) { threw = true; } QVERIFY2(threw, text); }
        }
    }
    void fileAppendAndLoad() {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const std::string path = dir.filePath("games.txt").toStdString();
        GameRecord a = finishedRecord(BoardLayout::standard());
        GameRecord b = finishedRecord(BoardLayout::hexagonal(BoardSize::Standard));
        RecordFile::append(path, a);
        RecordFile::append(path, b);  // appends, does not overwrite
        auto loaded = RecordFile::load(path);
        QCOMPARE(int(loaded.size()), 2);
        QCOMPARE(loaded[0].boardKey, a.boardKey);
        QCOMPARE(loaded[1].boardKey, b.boardKey);
        QCOMPARE(int(loaded[1].moves.size()), int(b.moves.size()));
    }
    void loadingMissingFileThrows() {
        QVERIFY_EXCEPTION_THROWN(RecordFile::load("/no/such/dir/none.txt"), std::runtime_error);
    }

    // ---- Replay ----
    void replayStartsAtInitialPosition() {
        Replay rp(finishedRecord(BoardLayout::standard()));
        QCOMPARE(rp.step(), 0);
        QVERIFY(rp.atStart());
        QVERIFY(!rp.atEnd());
        QCOMPARE(rp.game().pegCount(), 32);
        QVERIFY(!rp.lastMove().has_value());
    }
    void replayStepsForwardAndReachesRecordedEnd() {
        GameRecord rec = finishedRecord(BoardLayout::hexagonal(BoardSize::Large));
        Replay rp(rec);
        int n = 0;
        while (rp.stepForward()) ++n;
        QCOMPARE(n, int(rec.moves.size()));
        QVERIFY(rp.atEnd());
        QCOMPARE(rp.game().pegCount(), rec.pegsRemaining);
        QVERIFY(rp.game().isGameOver());
        QVERIFY(!rp.stepForward());
    }
    void replayStepBackUndoesMoves() {
        GameRecord rec = finishedRecord(BoardLayout::standard());
        Replay rp(rec);
        rp.stepForward();
        rp.stepForward();
        rp.stepForward();
        QCOMPARE(rp.game().pegCount(), 29);
        QVERIFY(rp.stepBack());
        QCOMPARE(rp.step(), 2);
        QCOMPARE(rp.game().pegCount(), 30);
        QVERIFY(rp.lastMove().value() == rec.moves[1]);
        rp.toStart();
        QCOMPARE(rp.game().pegCount(), 32);
        QVERIFY(!rp.stepBack());
    }
    void replayToEndJumpsToFinalPosition() {
        GameRecord rec = finishedRecord(BoardLayout::standard());
        Replay rp(rec);
        rp.toEnd();
        QVERIFY(rp.atEnd());
        QCOMPARE(rp.game().pegCount(), rec.pegsRemaining);
    }
    void replayDoesNotAffectOtherGames() {
        Game live;
        live.makeMove({{3, 1}, {3, 3}});
        Replay rp(finishedRecord(BoardLayout::standard()));
        rp.toEnd();
        QCOMPARE(live.pegCount(), 31);  // untouched
    }
    void replayRejectsUnknownBoard() {
        GameRecord rec = finishedRecord(BoardLayout::standard());
        rec.boardKey = "pentagon";
        QVERIFY_EXCEPTION_THROWN(Replay{rec}, std::invalid_argument);
    }
    void replayRejectsIllegalRecordedMove() {
        GameRecord rec = finishedRecord(BoardLayout::standard());
        rec.moves[0] = Move{{0, 3}, {2, 3}};  // destination occupied at game start
        QVERIFY_EXCEPTION_THROWN(Replay{rec}, std::runtime_error);
    }
};

QTEST_APPLESS_MAIN(TestRecords)
#include "tst_records.moc"
