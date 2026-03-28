#ifndef MINEFIELD_H
#define MINEFIELD_H

#include <QObject>
#include <QVector>

class MineField : public QObject
{
    Q_OBJECT

public:
    enum CellState {
        State_Normal = 0,
        // 数字状态将使用 1~8 直接表示，因此其他状态必须避开这些值
        State_Empty = 20,   // 已翻开且周围无雷
        State_Flag = 21,
        State_Dicey = 22,
        State_DiceyDown = 23,
        State_Blast = 24,
        State_Mine = 25,
        State_Error = 26
    };

    enum CellAttribute {
        Attrib_Empty,
        Attrib_Mine
    };

    struct Cell {
        int row;
        int col;
        CellState state;
        CellAttribute attrib;
        CellState oldState;
    };

    enum GameState {
        GS_Wait,
        GS_Run,
        GS_Dead,
        GS_Victory
    };

    enum Difficulty {
        Level_Primary,
        Level_Secondary,
        Level_Advance,
        Level_Custom
    };

    explicit MineField(QObject *parent = nullptr);
    ~MineField();

    void initialize(int width, int height, int mineCount);
    void reset();
    void layMines(int clickedRow, int clickedCol);
    bool revealCell(int row, int col);
    void toggleFlag(int row, int col);
    void openAround(int row, int col);
    bool checkVictory();

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    int getMineCount() const { return m_mineCount; }
    int getRemainingMines() const { return m_remainingMines; }
    GameState getGameState() const { return m_gameState; }
    const Cell &getCell(int row, int col) const;

signals:
    void cellUpdated(int row, int col);
    void gameStateChanged(GameState state);
    void remainingMinesChanged(int count);

private:
    int m_width;
    int m_height;
    int m_mineCount;
    int m_remainingMines;
    GameState m_gameState;
    QVector<QVector<Cell>> m_cells;

    int getAroundMineCount(int row, int col) const;
    int getAroundFlagCount(int row, int col) const;
    bool isInField(int row, int col) const;
    void expandEmptyCells(int row, int col);
};

#endif // MINEFIELD_H