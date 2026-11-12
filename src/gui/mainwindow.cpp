#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "AutoPlayer.h"
#include "GameRecordFile.h"
#include "RandomState.h"

#include <QAction>
#include <QButtonGroup>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QInputDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QRandomGenerator>
#include <QSettings>
#include <QStandardPaths>
#include <QTimer>

namespace {
constexpr int kPitch = 48;         // distance between neighbouring cells
constexpr int kSquareButton = 44;  // English board buttons
constexpr int kHexButton = 40;     // hexagonal board buttons (round)
constexpr double kHexRowPitch = kPitch * 0.8660254;  // sqrt(3)/2 keeps all 6 neighbours equidistant
constexpr int kPlayIntervalMs = 700;
constexpr int kAutoIntervalMs = 250;

QString outcomeText(Outcome o) {
    switch (o) {
    case Outcome::PerfectWin: return QObject::tr("perfect win");
    case Outcome::Won:        return QObject::tr("win");
    case Outcome::Lost:       return QObject::tr("loss");
    }
    return {};
}
}  // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_game(BoardLayout::create(BoardType::English, BoardSize::Standard)) {
    ui->setupUi(this);

    // Two independent radio groups: board size (existing) and board type (new).
    m_sizeGroup = new QButtonGroup(this);
    m_sizeGroup->addButton(ui->radioStandard, static_cast<int>(BoardSize::Standard));
    m_sizeGroup->addButton(ui->radioLarge, static_cast<int>(BoardSize::Large));
    m_typeGroup = new QButtonGroup(this);
    m_typeGroup->addButton(ui->radioEnglish, static_cast<int>(BoardType::English));
    m_typeGroup->addButton(ui->radioHex, static_cast<int>(BoardType::Hexagonal));
    // Radio buttons sharing a parent are auto-exclusive, so the .ui's two "checked" defaults
    // cancel each other out until each pair lives in its own QButtonGroup. Set them again now.
    ui->radioStandard->setChecked(true);
    ui->radioEnglish->setChecked(true);

    ui->boardLayout->setAlignment(Qt::AlignCenter);

    m_playTimer = new QTimer(this);
    m_playTimer->setInterval(kPlayIntervalMs);
    m_autoTimer = new QTimer(this);
    m_autoTimer->setInterval(kAutoIntervalMs);

    // File menu
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    QAction* openAct = fileMenu->addAction(tr("&Open Recording..."));
    openAct->setShortcut(QKeySequence::Open);
    QAction* fileAct = fileMenu->addAction(tr("Set &Recording File..."));
    fileMenu->addSeparator();
    QAction* quitAct = fileMenu->addAction(tr("&Quit"));
    quitAct->setShortcut(QKeySequence::Quit);
    connect(openAct, &QAction::triggered, this, &MainWindow::onOpenRecording);
    connect(fileAct, &QAction::triggered, this, &MainWindow::onChooseRecordingFile);
    connect(quitAct, &QAction::triggered, this, &QWidget::close);

    connect(m_sizeGroup, &QButtonGroup::idClicked, this, [this](int) { onBoardSelectionChanged(); });
    connect(m_typeGroup, &QButtonGroup::idClicked, this, [this](int) { onBoardSelectionChanged(); });
    connect(ui->resetButton, &QPushButton::clicked, this, &MainWindow::onResetClicked);
    connect(ui->checkShowMoves, &QCheckBox::toggled, this, &MainWindow::onShowMovesToggled);

    connect(ui->replayStartButton, &QPushButton::clicked, this, &MainWindow::onReplayStart);
    connect(ui->replayBackButton, &QPushButton::clicked, this, &MainWindow::onReplayBack);
    connect(ui->replayForwardButton, &QPushButton::clicked, this, &MainWindow::onReplayForward);
    connect(ui->replayPlayButton, &QPushButton::clicked, this, &MainWindow::onReplayPlayPause);
    connect(ui->replayExitButton, &QPushButton::clicked, this, &MainWindow::onReplayExit);
    connect(m_playTimer, &QTimer::timeout, this, &MainWindow::onReplayTick);

    connect(ui->randomStartButton, &QPushButton::clicked, this, &MainWindow::onRandomStart);
    connect(ui->autoPlayButton, &QPushButton::clicked, this, &MainWindow::onAutoPlayToggle);
    connect(m_autoTimer, &QTimer::timeout, this, &MainWindow::onAutoTick);

    rebuildBoard();
    refresh();
}

MainWindow::~MainWindow() = default;

BoardLayout MainWindow::selectedLayout() const {
    return BoardLayout::create(static_cast<BoardType>(m_typeGroup->checkedId()),
                               static_cast<BoardSize>(m_sizeGroup->checkedId()));
}

const Game& MainWindow::viewedGame() const {
    return m_replay ? m_replay->game() : m_game;
}

// playing

void MainWindow::startNewGame(const BoardLayout& layout) {
    m_selected.reset();
    m_game.newGame(layout);
    rebuildBoard();
    refresh();
}

void MainWindow::onBoardSelectionChanged() {
    if (inReplay() || autoPlaying()) return;
    startNewGame(selectedLayout());
}

void MainWindow::onResetClicked() {
    if (inReplay() || autoPlaying()) return;
    startNewGame(m_game.layout());
}

void MainWindow::onShowMovesToggled(bool) {
    refresh();
}

void MainWindow::finishGameIfOver() {
    if (m_game.isGameOver()) showGameOverMessage(saveFinishedGame());
}

// ---------------------------------------------------------------- random start / auto play

bool MainWindow::autoPlaying() const {
    return m_autoTimer->isActive();
}

void MainWindow::onRandomStart() {
    if (inReplay() || autoPlaying()) return;
    // Seed from Qt's generator so every click gives a different position on every platform.
    std::mt19937 rng(QRandomGenerator::global()->generate());
    const BoardLayout layout = selectedLayout();
    m_selected.reset();
    m_game = RandomState::generate(layout, rng, RandomState::randomMoveCount(layout, rng));
    rebuildBoard();
    refresh();
}

void MainWindow::onAutoPlayToggle() {
    if (inReplay()) return;
    if (autoPlaying()) stopAutoPlay();
    else startAutoPlay();
}

void MainWindow::startAutoPlay() {
    if (m_game.isGameOver()) startNewGame(m_game.layout());  // finished game: start a fresh one
    m_selected.reset();
    m_autoStrategy = makeStrategy(StrategyType::Random, QRandomGenerator::global()->generate());
    m_autoTimer->start();
    updateControlStates();
    refresh();
}

void MainWindow::stopAutoPlay() {
    m_autoTimer->stop();
    m_autoStrategy.reset();
    updateControlStates();
}

void MainWindow::onAutoTick() {
    if (!m_autoStrategy || !AutoPlayer::step(m_game, *m_autoStrategy)) {
        stopAutoPlay();
        refresh();
        finishGameIfOver();
        return;
    }
    refresh();
    if (m_game.isGameOver()) {
        stopAutoPlay();
        finishGameIfOver();
    }
}

void MainWindow::rebuildBoard() {
    delete m_canvas;  // also deletes the buttons it owns
    m_canvas = nullptr;
    m_buttons.clear();

    const BoardLayout& layout = viewedGame().layout();
    const bool hex = layout.geometry() == Geometry::Hexagonal;
    const int btn = hex ? kHexButton : kSquareButton;

    auto cellOrigin = [&](int r, int c) {
        return hex ? QPoint(qRound((c + 0.5 * r) * kPitch), qRound(r * kHexRowPitch))
                   : QPoint(c * kPitch, r * kPitch);
    };

    // Bounding box of all holes, so the board can be centred.
    int minX = 1 << 30, maxX = -(1 << 30), maxY = 0;
    for (int r = 0; r < layout.rows(); ++r)
        for (int c = 0; c < layout.cols(); ++c)
            if (layout.isHole({r, c})) {
                const QPoint p = cellOrigin(r, c);
                minX = qMin(minX, p.x());
                maxX = qMax(maxX, p.x());
                maxY = qMax(maxY, p.y());
            }

    m_canvas = new QWidget(ui->boardContainer);
    m_canvas->setFixedSize(maxX - minX + btn, maxY + btn);
    m_buttons.assign(static_cast<size_t>(layout.rows()),
                     std::vector<QPushButton*>(static_cast<size_t>(layout.cols()), nullptr));

    for (int r = 0; r < layout.rows(); ++r) {
        for (int c = 0; c < layout.cols(); ++c) {
            if (!layout.isHole({r, c})) continue;
            auto* b = new QPushButton(m_canvas);
            const QPoint p = cellOrigin(r, c);
            b->setGeometry(p.x() - minX, p.y(), btn, btn);
            b->setFocusPolicy(Qt::NoFocus);
            connect(b, &QPushButton::clicked, this, [this, r, c] { onCellClicked(r, c); });
            m_buttons[static_cast<size_t>(r)][static_cast<size_t>(c)] = b;
        }
    }
    ui->boardLayout->addWidget(m_canvas, 0, 0, Qt::AlignCenter);
    m_canvas->show();
}

void MainWindow::onCellClicked(int row, int col) {
    if (inReplay() || autoPlaying() || m_game.isGameOver()) return;
    const Position p{row, col};

    // Second click on a legal destination: perform the move.
    if (m_selected && m_game.makeMove(Move{*m_selected, p})) {
        m_selected.reset();
        refresh();
        finishGameIfOver();
        return;
    }

    // Otherwise (de)select a peg that can actually move.
    if (m_selected && *m_selected == p)
        m_selected.reset();
    else if (!m_game.validMovesFrom(p).empty())
        m_selected = p;
    else
        m_selected.reset();
    refresh();
}

void MainWindow::refresh() {
    const Game& game = viewedGame();
    const bool hex = game.layout().geometry() == Geometry::Hexagonal;
    const int radius = hex ? kHexButton / 2 : 0;
    const bool showMoves = ui->checkShowMoves->isChecked() && !inReplay();

    std::vector<Position> targets;
    if (m_selected)
        for (const Move& m : game.validMovesFrom(*m_selected)) targets.push_back(m.to);

    std::optional<Move> last = inReplay() ? m_replay->lastMove() : std::nullopt;

    for (int r = 0; r < game.rows(); ++r) {
        for (int c = 0; c < game.cols(); ++c) {
            QPushButton* b = m_buttons[static_cast<size_t>(r)][static_cast<size_t>(c)];
            if (!b) continue;
            const Position p{r, c};
            const CellState s = game.cellAt(p);
            b->setText(s == CellState::Peg ? QStringLiteral("●") : QStringLiteral("○"));

            QString bg = QStringLiteral("#ffffff");
            if (last) {
                if (p == last->to) bg = QStringLiteral("#a5d6a7");            // where the peg landed
                else if (p == last->from) bg = QStringLiteral("#ffe0b2");     // where it started
                else if (p == last->jumped()) bg = QStringLiteral("#ef9a9a"); // the captured peg
            } else if (m_selected && *m_selected == p) {
                bg = QStringLiteral("#ffd54f");  // selected peg
            } else if (showMoves) {
                bool isTarget = false;
                for (const Position& t : targets) isTarget = isTarget || (t == p);
                if (isTarget)
                    bg = QStringLiteral("#a5d6a7");  // destination of selected peg
                else if (s == CellState::Peg && !game.validMovesFrom(p).empty())
                    bg = QStringLiteral("#e3f2fd");  // peg that can move
            }
            b->setStyleSheet(QStringLiteral(
                "QPushButton { background:%1; border:1px solid #bbb; font-size:18px; border-radius:%2px; }")
                                 .arg(bg).arg(radius));
        }
    }

    ui->statusLabel->setText(tr("Pegs remaining: %1    Moves: %2")
                                 .arg(game.pegCount())
                                 .arg(game.moveCount()));
}

void MainWindow::showGameOverMessage(const QString& recordNote) {
    QString text;
    if (m_game.isPerfectWin())
        text = tr("Perfect! One peg left, in the starting hole.");
    else if (m_game.isWon())
        text = tr("You win! Only one peg remains.");
    else
        text = tr("Game over. No moves left; %1 pegs remain.").arg(m_game.pegCount());
    if (!recordNote.isEmpty()) text += QStringLiteral("\n\n") + recordNote;
    QMessageBox::information(this, tr("Game Over"), text);
}

// ---------------------------------------------------------------- recording

QString MainWindow::recordFilePath() const {
    const QString def = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +
                        QStringLiteral("/PegSolitaireGames.txt");
    return QSettings().value(QStringLiteral("recordFile"), def).toString();
}

QString MainWindow::saveFinishedGame() {
    try {
        const GameRecord rec = GameRecord::fromGame(m_game, GameRecord::currentTimestamp());
        const QString path = recordFilePath();
        QDir().mkpath(QFileInfo(path).absolutePath());
        RecordFile::append(path.toStdString(), rec);
        return tr("Game recorded to:\n%1").arg(QDir::toNativeSeparators(path));
    } catch (const std::exception& e) {
        return tr("Could not record this game: %1").arg(QString::fromUtf8(e.what()));
    }
}

void MainWindow::onChooseRecordingFile() {
    const QString path = QFileDialog::getSaveFileName(
        this, tr("Choose Recording File"), recordFilePath(),
        tr("Text files (*.txt);;All files (*)"), nullptr, QFileDialog::DontConfirmOverwrite);
    if (!path.isEmpty()) QSettings().setValue(QStringLiteral("recordFile"), path);
}

// ---------------------------------------------------------------- replay

void MainWindow::onOpenRecording() {
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Open Recording"), QFileInfo(recordFilePath()).absolutePath(),
        tr("Text files (*.txt);;All files (*)"));
    if (path.isEmpty()) return;

    try {
        std::vector<GameRecord> records = RecordFile::load(path.toStdString());
        if (records.empty()) {
            QMessageBox::warning(this, tr("Open Recording"), tr("That file contains no recorded games."));
            return;
        }
        size_t index = 0;
        if (records.size() > 1) {
            QStringList items;
            for (size_t i = 0; i < records.size(); ++i) {
                const GameRecord& r = records[i];
                items << tr("%1. %2 - %3 (%4 moves) %5")
                             .arg(i + 1)
                             .arg(QString::fromStdString(r.boardKey))
                             .arg(outcomeText(r.outcome))
                             .arg(r.moves.size())
                             .arg(QString::fromStdString(r.timestamp));
            }
            bool ok = false;
            const QString chosen = QInputDialog::getItem(this, tr("Open Recording"),
                                                         tr("Choose a game to replay:"), items, 0, false, &ok);
            if (!ok) return;
            index = static_cast<size_t>(items.indexOf(chosen));
        }
        enterReplay(records[index]);
    } catch (const std::exception& e) {
        QMessageBox::warning(this, tr("Open Recording"),
                             tr("Could not open this recording:\n%1").arg(QString::fromUtf8(e.what())));
    }
}

void MainWindow::enterReplay(GameRecord record) {
    auto replay = std::make_unique<Replay>(std::move(record));  // throws before any state changes
    stopAutoPlay();
    stopPlayback();
    m_selected.reset();
    m_replay = std::move(replay);
    updateControlStates();
    ui->replayPanel->show();
    rebuildBoard();
    refresh();
    updateReplayControls();
}

void MainWindow::exitReplay() {
    stopPlayback();
    m_replay.reset();
    ui->replayPanel->hide();
    updateControlStates();
    rebuildBoard();  // back to the live game, which was never touched
    refresh();
}

void MainWindow::updateControlStates() {
    const bool replay = inReplay();
    const bool idle = !replay && !autoPlaying();  // no replay and no automated game running
    for (QWidget* w : {static_cast<QWidget*>(ui->radioStandard), static_cast<QWidget*>(ui->radioLarge),
                       static_cast<QWidget*>(ui->radioEnglish), static_cast<QWidget*>(ui->radioHex),
                       static_cast<QWidget*>(ui->resetButton), static_cast<QWidget*>(ui->randomStartButton)})
        w->setEnabled(idle);
    ui->checkShowMoves->setEnabled(!replay);
    ui->autoPlayButton->setEnabled(!replay);  // stays enabled while running so it can act as "Stop"
    ui->autoPlayButton->setText(autoPlaying() ? tr("Stop Auto Play") : tr("Auto Play"));
}

void MainWindow::stopPlayback() {
    m_playTimer->stop();
}

void MainWindow::updateReplayControls() {
    if (!m_replay) return;
    const GameRecord& r = m_replay->record();
    ui->replayInfoLabel->setText(
        tr("Replay: move %1 of %2\n%3 - %4 (%5 pegs left) - %6")
            .arg(m_replay->step())
            .arg(m_replay->totalSteps())
            .arg(QString::fromStdString(m_replay->game().layout().name()))
            .arg(outcomeText(r.outcome))
            .arg(r.pegsRemaining)
            .arg(QString::fromStdString(r.timestamp)));
    ui->replayStartButton->setEnabled(!m_replay->atStart());
    ui->replayBackButton->setEnabled(!m_replay->atStart());
    ui->replayForwardButton->setEnabled(!m_replay->atEnd());
    ui->replayPlayButton->setText(m_playTimer->isActive() ? tr("Pause") : tr("Play"));
}

void MainWindow::onReplayStart() {
    if (!m_replay) return;
    stopPlayback();
    m_replay->toStart();
    refresh();
    updateReplayControls();
}

void MainWindow::onReplayBack() {
    if (!m_replay) return;
    stopPlayback();
    m_replay->stepBack();
    refresh();
    updateReplayControls();
}

void MainWindow::onReplayForward() {
    if (!m_replay) return;
    stopPlayback();
    m_replay->stepForward();
    refresh();
    updateReplayControls();
}

void MainWindow::onReplayPlayPause() {
    if (!m_replay) return;
    if (m_playTimer->isActive()) {
        stopPlayback();
    } else {
        if (m_replay->atEnd()) m_replay->toStart();
        m_playTimer->start();
        m_replay->stepForward();
        refresh();
    }
    updateReplayControls();
}

void MainWindow::onReplayTick() {
    if (!m_replay || !m_replay->stepForward()) {
        stopPlayback();
    } else {
        refresh();
        if (m_replay->atEnd()) stopPlayback();
    }
    updateReplayControls();
}

void MainWindow::onReplayExit() {
    exitReplay();
}
