#ifndef MINEFIELD_H
#define MINEFIELD_H

#include <QObject>
#include <QVector>

class MineField : public QObject
{
    Q_OBJECT

public:
    enum class CellState {
        Normal = 0,
        // 数字 1~8 直接作为值，其他状态避开这些值
        Empty = 20,
        Flag = 21,
        Dicey = 22,
        Blast = 24,
        Mine = 25,
        Error = 26
    };

    enum class CellAttribute {
        Empty,
        Mine
    };

    struct Cell {
        int row;
        int col;
        CellState state;
        CellAttribute attrib;
    };

    enum class GameState {
        Wait,
        Run,
        Dead,
        Victory
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
    const Cell& getCell(int row, int col) const;

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
    void expandEmptyCells(int row, int col);   // BFS 实现
};

#endif // MINEFIELD_H