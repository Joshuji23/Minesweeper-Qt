#include "minefield.h"
#include <QRandomGenerator>

MineField::MineField(QObject *parent) : QObject(parent)
{
    m_width = 0;
    m_height = 0;
    m_mineCount = 0;
    m_remainingMines = 0;
    m_gameState = GS_Wait;
}

MineField::~MineField()
{
}

void MineField::initialize(int width, int height, int mineCount)
{
    m_width = width;
    m_height = height;
    m_mineCount = mineCount;
    m_remainingMines = mineCount;
    m_gameState = GS_Wait;

    m_cells.resize(height);
    for (int i = 0; i < height; ++i) {
        m_cells[i].resize(width);
        for (int j = 0; j < width; ++j) {
            m_cells[i][j].row = i;
            m_cells[i][j].col = j;
            m_cells[i][j].state = State_Normal;
            m_cells[i][j].attrib = Attrib_Empty;
            m_cells[i][j].oldState = State_Normal;
        }
    }

    emit gameStateChanged(m_gameState);
    emit remainingMinesChanged(m_remainingMines);
}

void MineField::reset()
{
    for (int i = 0; i < m_height; ++i) {
        for (int j = 0; j < m_width; ++j) {
            m_cells[i][j].state = State_Normal;
            m_cells[i][j].attrib = Attrib_Empty;
            m_cells[i][j].oldState = State_Normal;
            emit cellUpdated(i, j);
        }
    }

    m_remainingMines = m_mineCount;
    m_gameState = GS_Wait;
    emit gameStateChanged(m_gameState);
    emit remainingMinesChanged(m_remainingMines);
}

void MineField::layMines(int clickedRow, int clickedCol)
{
    int minesPlaced = 0;
    while (minesPlaced < m_mineCount) {
        int row = QRandomGenerator::global()->bounded(m_height);
        int col = QRandomGenerator::global()->bounded(m_width);

        if ((row == clickedRow && col == clickedCol) || m_cells[row][col].attrib == Attrib_Mine) {
            continue;
        }

        m_cells[row][col].attrib = Attrib_Mine;
        minesPlaced++;
    }
}

bool MineField::revealCell(int row, int col)
{
    if (!isInField(row, col) || m_cells[row][col].state != State_Normal) {
        return false;
    }

    if (m_gameState == GS_Wait) {
        m_gameState = GS_Run;
        layMines(row, col);
        emit gameStateChanged(m_gameState);
    }

    if (m_cells[row][col].attrib == Attrib_Mine) {
        // Hit a mine
        for (int i = 0; i < m_height; ++i) {
            for (int j = 0; j < m_width; ++j) {
                if (m_cells[i][j].attrib == Attrib_Mine) {
                    m_cells[i][j].state = State_Mine;
                    emit cellUpdated(i, j);
                }
            }
        }
        m_cells[row][col].state = State_Blast;
        emit cellUpdated(row, col);
        m_gameState = GS_Dead;
        emit gameStateChanged(m_gameState);
        return false;
    } else {
        // Reveal the cell
        expandEmptyCells(row, col);
        if (checkVictory()) {
            m_gameState = GS_Victory;
            emit gameStateChanged(m_gameState);
            // Mark all mines with flags
            for (int i = 0; i < m_height; ++i) {
                for (int j = 0; j < m_width; ++j) {
                    if (m_cells[i][j].attrib == Attrib_Mine) {
                        m_cells[i][j].state = State_Flag;
                        emit cellUpdated(i, j);
                    }
                }
            }
            m_remainingMines = 0;
            emit remainingMinesChanged(m_remainingMines);
        }
        return true;
    }
}

void MineField::toggleFlag(int row, int col)
{
    if (!isInField(row, col) || m_gameState != GS_Run) {
        return;
    }

    switch (m_cells[row][col].state) {
    case State_Normal:
        m_cells[row][col].state = State_Flag;
        m_remainingMines--;
        break;
    case State_Flag:
        m_cells[row][col].state = State_Dicey;
        m_remainingMines++;
        break;
    case State_Dicey:
        m_cells[row][col].state = State_Normal;
        break;
    default:
        return;
    }

    emit cellUpdated(row, col);
    emit remainingMinesChanged(m_remainingMines);
}

void MineField::openAround(int row, int col)
{
    if (!isInField(row, col) || m_gameState != GS_Run) {
        return;
    }

    int flagCount = getAroundFlagCount(row, col);
    int mineCount = getAroundMineCount(row, col);

    if (flagCount != mineCount) {
        return;
    }

    for (int i = row - 1; i <= row + 1; ++i) {
        for (int j = col - 1; j <= col + 1; ++j) {
            if (isInField(i, j) && m_cells[i][j].state == State_Normal) {
                revealCell(i, j);
            }
        }
    }
}

bool MineField::checkVictory()
{
    for (int i = 0; i < m_height; ++i) {
        for (int j = 0; j < m_width; ++j) {
            if (m_cells[i][j].state == State_Normal || m_cells[i][j].state == State_Dicey) {
                if (m_cells[i][j].attrib != Attrib_Mine) {
                    return false;
                }
            }
        }
    }
    return true;
}

const MineField::Cell &MineField::getCell(int row, int col) const
{
    static Cell emptyCell = { -1, -1, State_Normal, Attrib_Empty, State_Normal };
    if (isInField(row, col)) {
        return m_cells[row][col];
    }
    return emptyCell;
}

int MineField::getAroundMineCount(int row, int col) const
{
    int count = 0;
    for (int i = row - 1; i <= row + 1; ++i) {
        for (int j = col - 1; j <= col + 1; ++j) {
            if (isInField(i, j) && m_cells[i][j].attrib == Attrib_Mine) {
                count++;
            }
        }
    }
    return count;
}

int MineField::getAroundFlagCount(int row, int col) const
{
    int count = 0;
    for (int i = row - 1; i <= row + 1; ++i) {
        for (int j = col - 1; j <= col + 1; ++j) {
            if (isInField(i, j) && m_cells[i][j].state == State_Flag) {
                count++;
            }
        }
    }
    return count;
}

bool MineField::isInField(int row, int col) const
{
    return row >= 0 && row < m_height && col >= 0 && col < m_width;
}

void MineField::expandEmptyCells(int row, int col)
{
    if (!isInField(row, col) || m_cells[row][col].state != State_Normal) {
        return;
    }

    int mineCount = getAroundMineCount(row, col);
    if (mineCount > 0) {
        // 周围有雷，显示数字（数字 1~8 直接作为状态值）
        m_cells[row][col].state = static_cast<CellState>(mineCount);
        emit cellUpdated(row, col);
        return;
    }

    // 周围无雷，设置为空状态
    m_cells[row][col].state = State_Empty;
    emit cellUpdated(row, col);

    // 递归展开相邻格子
    for (int i = row - 1; i <= row + 1; ++i) {
        for (int j = col - 1; j <= col + 1; ++j) {
            if (isInField(i, j) && m_cells[i][j].state == State_Normal) {
                expandEmptyCells(i, j);
            }
        }
    }
}