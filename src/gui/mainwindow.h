#pragma once

#include <QMainWindow>
#include <QString>
#include <memory>
#include <optional>
#include <vector>

#include "Game.h"
#include "GameRecord.h"
#include "MoveStrategy.h"
#include "Replay.h"

class QButtonGroup;
class QPushButton;
class QTimer;
class QWidget;

namespace Ui {
class MainWindow;
}

// User-interface layer only. All rules, state, recording and replay logic live in the core
// library (Game, Replay, RecordFile); this class renders them and translates user actions.
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    // playing
    void onCellClicked(int row, int col);
    void onBoardSelectionChanged();
    void onResetClicked();
    void onShowMovesToggled(bool checked);
    // recording / replay
    void onOpenRecording();
    void onChooseRecordingFile();
    void onReplayStart();
    void onReplayBack();
    void onReplayForward();
    void onReplayPlayPause();
    void onReplayTick();
    void onReplayExit();
    // random start / automated play
    void onRandomStart();
    void onAutoPlayToggle();
    void onAutoTick();

private:
    BoardLayout selectedLayout() const;  // from the size + type radio buttons
    const Game& viewedGame() const;      // the replay's game while replaying, else the live game
    bool inReplay() const { return m_replay != nullptr; }

    void startNewGame(const BoardLayout& layout);
    void rebuildBoard();  // recreate the buttons (layout changed)
    void refresh();       // update text/colours from the viewed game
    void showGameOverMessage(const QString& recordNote);

    QString recordFilePath() const;
    QString saveFinishedGame();  // appends the finished game to the record file
    void enterReplay(GameRecord record);
    void exitReplay();
    void stopPlayback();
    void updateReplayControls();
    void updateControlStates();  // enable/disable widgets for replay / auto-play modes
    void startAutoPlay();
    void stopAutoPlay();
    bool autoPlaying() const;
    void finishGameIfOver();     // shows the result and records the game once it has ended

    std::unique_ptr<Ui::MainWindow> ui;
    QButtonGroup* m_sizeGroup = nullptr;
    QButtonGroup* m_typeGroup = nullptr;
    QTimer* m_playTimer = nullptr;
    QTimer* m_autoTimer = nullptr;
    std::unique_ptr<MoveStrategy> m_autoStrategy;
    QWidget* m_canvas = nullptr;  // holds the board buttons
    Game m_game;
    std::unique_ptr<Replay> m_replay;
    std::vector<std::vector<QPushButton*>> m_buttons;  // nullptr where there is no hole
    std::optional<Position> m_selected;
};
