#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QButtonGroup>
#include <QGridLayout>
#include <QMessageBox>
#include <QPushButton>

namespace {
constexpr int kCellSize = 44;
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_game(BoardLayout::create(BoardType::Standard)) {
    ui->setupUi(this);

    // Map each radio button to a BoardType so new boards only need a new radio + enum value.
    m_boardGroup = new QButtonGroup(this);
    m_boardGroup->addButton(ui->radioStandard, static_cast<int>(BoardType::Standard));
    m_boardGroup->addButton(ui->radioLarge, static_cast<int>(BoardType::Large));

    ui->boardLayout->setAlignment(Qt::AlignCenter);

    connect(m_boardGroup, &QButtonGroup::idClicked, this, &MainWindow::onBoardTypeChanged);
    connect(ui->resetButton, &QPushButton::clicked, this, &MainWindow::onResetClicked);
    connect(ui->checkShowMoves, &QCheckBox::toggled, this, &MainWindow::onShowMovesToggled);

    rebuildBoard();
    refresh();
}

MainWindow::~MainWindow() = default;

void MainWindow::startNewGame(const BoardLayout& layout) {
    m_selected.reset();
    m_game.newGame(layout);
    rebuildBoard();
    refresh();
}

void MainWindow::onBoardTypeChanged(int id) {
    startNewGame(BoardLayout::create(static_cast<BoardType>(id)));
}

void MainWindow::onResetClicked() {
    startNewGame(m_game.layout());
}

void MainWindow::onShowMovesToggled(bool) {
    refresh();
}

void MainWindow::rebuildBoard() {
    for (auto& row : m_buttons)
        for (QPushButton* b : row)
            delete b;  // also removes it from the layout
    m_buttons.clear();

    // Reset stale minimum sizes left over from a previously larger board.
    for (int i = 0; i < 16; ++i) {
        ui->boardLayout->setColumnMinimumWidth(i, 0);
        ui->boardLayout->setRowMinimumHeight(i, 0);
    }

    m_buttons.assign(static_cast<size_t>(m_game.rows()),
                     std::vector<QPushButton*>(static_cast<size_t>(m_game.cols()), nullptr));

    for (int r = 0; r < m_game.rows(); ++r) {
        ui->boardLayout->setRowMinimumHeight(r, kCellSize);
        for (int c = 0; c < m_game.cols(); ++c) {
            ui->boardLayout->setColumnMinimumWidth(c, kCellSize);
            if (!m_game.layout().isHole({r, c})) continue;

            auto* b = new QPushButton(ui->boardContainer);
            b->setFixedSize(kCellSize, kCellSize);
            b->setFocusPolicy(Qt::NoFocus);
            connect(b, &QPushButton::clicked, this, [this, r, c] { onCellClicked(r, c); });
            ui->boardLayout->addWidget(b, r, c);
            m_buttons[static_cast<size_t>(r)][static_cast<size_t>(c)] = b;
        }
    }
}

void MainWindow::onCellClicked(int row, int col) {
    if (m_game.isGameOver()) return;
    const Position p{row, col};

    // Second click on a legal destination: perform the move.
    if (m_selected && m_game.makeMove(Move{*m_selected, p})) {
        m_selected.reset();
        refresh();
        if (m_game.isGameOver()) showGameOverMessage();
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
    std::vector<Position> targets;
    if (m_selected)
        for (const Move& m : m_game.validMovesFrom(*m_selected)) targets.push_back(m.to);

    const bool showMoves = ui->checkShowMoves->isChecked();

    for (int r = 0; r < m_game.rows(); ++r) {
        for (int c = 0; c < m_game.cols(); ++c) {
            QPushButton* b = m_buttons[static_cast<size_t>(r)][static_cast<size_t>(c)];
            if (!b) continue;
            const Position p{r, c};
            const CellState s = m_game.cellAt(p);
            b->setText(s == CellState::Peg ? QStringLiteral("●") : QStringLiteral("○"));

            QString bg = QStringLiteral("#ffffff");
            if (m_selected && *m_selected == p) {
                bg = QStringLiteral("#ffd54f");  // selected peg
            } else if (showMoves) {
                bool isTarget = false;
                for (const Position& t : targets) isTarget = isTarget || (t == p);
                if (isTarget)
                    bg = QStringLiteral("#a5d6a7");  // destination of selected peg
                else if (s == CellState::Peg && !m_game.validMovesFrom(p).empty())
                    bg = QStringLiteral("#e3f2fd");  // peg that can move
            }
            b->setStyleSheet(QStringLiteral(
                "QPushButton { background:%1; border:1px solid #bbb; font-size:18px; }").arg(bg));
        }
    }

    ui->statusLabel->setText(tr("Pegs remaining: %1    Moves: %2")
                                 .arg(m_game.pegCount())
                                 .arg(m_game.moveCount()));
}

void MainWindow::showGameOverMessage() {
    QString text;
    if (m_game.isPerfectWin())
        text = tr("Perfect! One peg left, in the starting hole.");
    else if (m_game.isWon())
        text = tr("You win! Only one peg remains.");
    else
        text = tr("Game over. No moves left; %1 pegs remain.").arg(m_game.pegCount());
    QMessageBox::information(this, tr("Game Over"), text);
}
