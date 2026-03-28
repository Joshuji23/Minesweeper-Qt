#ifndef MINEWINDOW_H
#define MINEWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QSettings>
#include "minefield.h"

namespace Ui {
class MineWindow;
}

class MineWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MineWindow(QWidget *parent = nullptr);
    ~MineWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onNewGame();
    void onPrimaryLevel();
    void onSecondaryLevel();
    void onAdvancedLevel();
    void onCustomLevel();
    void onTimerTick();
    void onCellLeftClicked();
    void onCellRightClicked();
    void onCellUpdated(int row, int col);
    void onGameStateChanged(MineField::GameState state);
    void onRemainingMinesChanged(int count);

private:
    Ui::MineWindow *ui;
    MineField *m_mineField;
    QTimer *m_timer;
    int m_elapsedTime;
    QGridLayout *m_fieldLayout;
    QVector<QVector<QPushButton*>> m_cellButtons;
    QLabel *m_mineCountLabel;
    QLabel *m_timeLabel;
    QLabel *m_bestTimeLabel;
    QPushButton *m_resetButton;

    QSettings m_settings;

    void setupUI();
    void connectSignals();
    void createCellButtons();
    void updateCellButton(int row, int col);
    void updateUI();
    void resetGame();
    void showBestTimes();
    void updateBestTimeDisplay();
    int getBestTimeForDifficulty() const;
    void saveBestTime(int time);
};

#endif // MINEWINDOW_H