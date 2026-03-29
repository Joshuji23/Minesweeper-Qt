#include "minefield.h"
#include <QRandomGenerator>
#include <queue>
#include <utility>

MineField::MineField(QObject *parent) : QObject(parent)
{
    m_width = 0;
    m_height = 0;
    m_mineCount = 0;
    m_remainingMines = 0;
    m_gameState = GameState::Wait;
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
    m_gameState = GameState::Wait;

    m_cells.resize(height);
    for (int i = 0; i < height; ++i) {
        m_cells[i].resize(width);
        for (int j = 0; j < width; ++j) {
            m_cells[i][j].row = i;
            m_cells[i][j].col = j;
            m_cells[i][j].state = CellState::Normal;
            m_cells[i][j].attrib = CellAttribute::Empty;
        }
    }

    emit gameStateChanged(m_gameState);
    emit remainingMinesChanged(m_remainingMines);
}

void MineField::reset()
{
    for (int i = 0; i < m_height; ++i) {
        for (int j = 0; j < m_width; ++j) {
            m_cells[i][j].state = CellState::Normal;
            m_cells[i][j].attrib = CellAttribute::Empty;
            emit cellUpdated(i, j);
        }
    }

    m_remainingMines = m_mineCount;
    m_gameState = GameState::Wait;
    emit gameStateChanged(m_gameState);
    emit remainingMinesChanged(m_remainingMines);
}

void MineField::layMines(int clickedRow, int clickedCol)
{
    int minesPlaced = 0;
    while (minesPlaced < m_mineCount) {
        int row = QRandomGenerator::global()->bounded(m_height);
        int col = QRandomGenerator::global()->bounded(m_width);

        if ((row == clickedRow && col == clickedCol) || m_cells[row][col].attrib == CellAttribute::Mine) {
            continue;
        }

        m_cells[row][col].attrib = CellAttribute::Mine;
        minesPlaced++;
    }
}

bool MineField::revealCell(int row, int col)
{
    if (!isInField(row, col) || m_cells[row][col].state != CellState::Normal) {
        return false;
    }

    if (m_gameState == GameState::Wait) {
        m_gameState = GameState::Run;
        layMines(row, col);
        emit gameStateChanged(m_gameState);
    }

    if (m_cells[row][col].attrib == CellAttribute::Mine) {
        // Hit a mine
        for (int i = 0; i < m_height; ++i) {
            for (int j = 0; j < m_width; ++j) {
                if (m_cells[i][j].attrib == CellAttribute::Mine) {
                    m_cells[i][j].state = CellState::Mine;
                    emit cellUpdated(i, j);
                }
            }
        }
        m_cells[row][col].state = CellState::Blast;
        emit cellUpdated(row, col);
        m_gameState = GameState::Dead;
        emit gameStateChanged(m_gameState);
        return false;
    } else {
        // Reveal the cell
        expandEmptyCells(row, col);
        if (checkVictory()) {
            m_gameState = GameState::Victory;
            emit gameStateChanged(m_gameState);
            // Mark all mines with flags
            for (int i = 0; i < m_height; ++i) {
                for (int j = 0; j < m_width; ++j) {
                    if (m_cells[i][j].attrib == CellAttribute::Mine) {
                        m_cells[i][j].state = CellState::Flag;
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
    if (!isInField(row, col) || m_gameState != GameState::Run) {
        return;
    }

    switch (m_cells[row][col].state) {
    case CellState::Normal:
        m_cells[row][col].state = CellState::Flag;
        m_remainingMines--;
        break;
    case CellState::Flag:
        m_cells[row][col].state = CellState::Dicey;
        m_remainingMines++;
        break;
    case CellState::Dicey:
        m_cells[row][col].state = CellState::Normal;
        break;
    default:
        return;
    }

    emit cellUpdated(row, col);
    emit remainingMinesChanged(m_remainingMines);
}

void MineField::openAround(int row, int col)
{
    if (!isInField(row, col) || m_gameState != GameState::Run) {
        return;
    }

    int flagCount = getAroundFlagCount(row, col);
    int mineCount = getAroundMineCount(row, col);

    if (flagCount != mineCount) {
        return;
    }

    for (int i = row - 1; i <= row + 1; ++i) {
        for (int j = col - 1; j <= col + 1; ++j) {
            if (isInField(i, j) && m_cells[i][j].state == CellState::Normal) {
                revealCell(i, j);
            }
        }
    }
}

bool MineField::checkVictory()
{
    for (int i = 0; i < m_height; ++i) {
        for (int j = 0; j < m_width; ++j) {
            if (m_cells[i][j].state == CellState::Normal || m_cells[i][j].state == CellState::Dicey) {
                if (m_cells[i][j].attrib != CellAttribute::Mine) {
                    return false;
                }
            }
        }
    }
    return true;
}

const MineField::Cell& MineField::getCell(int row, int col) const
{
    static Cell emptyCell = { -1, -1, CellState::Normal, CellAttribute::Empty };
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
            if (isInField(i, j) && m_cells[i][j].attrib == CellAttribute::Mine) {
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
            if (isInField(i, j) && m_cells[i][j].state == CellState::Flag) {
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
    std::queue<std::pair<int, int>> q;
    q.push({row, col});

    while (!q.empty()) {
        auto [r, c] = q.front();
        q.pop();

        if (!isInField(r, c) || m_cells[r][c].state != CellState::Normal)
            continue;

        int mineCount = getAroundMineCount(r, c);
        if (mineCount > 0) {
            m_cells[r][c].state = static_cast<CellState>(mineCount);
            emit cellUpdated(r, c);
            continue;
        }

        m_cells[r][c].state = CellState::Empty;
        emit cellUpdated(r, c);

        for (int i = r - 1; i <= r + 1; ++i) {
            for (int j = c - 1; j <= c + 1; ++j) {
                if (isInField(i, j) && m_cells[i][j].state == CellState::Normal) {
                    q.push({i, j});
                }
            }
        }
    }
}