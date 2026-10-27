#pragma once

#include <QMainWindow>
#include <memory>
#include <optional>
#include <vector>

#include "Game.h"

class QButtonGroup;
class QPushButton;

namespace Ui {
class MainWindow;
}

// User-interface layer only. All rules and state live in `Game`
// this class just renders the game and translates clicks into Game calls.
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onCellClicked(int row, int col);
    void onBoardTypeChanged(int id);
    void onResetClicked();
    void onShowMovesToggled(bool checked);

private:
    void startNewGame(const BoardLayout& layout);
    void rebuildBoard();  // recreate the buttons (board size changed)
    void refresh();       // update text/colours from the Game state
    void showGameOverMessage();

    std::unique_ptr<Ui::MainWindow> ui;
    QButtonGroup* m_boardGroup = nullptr;
    Game m_game;
    std::vector<std::vector<QPushButton*>> m_buttons;  // nullptr where there is no hole
    std::optional<Position> m_selected;
};
