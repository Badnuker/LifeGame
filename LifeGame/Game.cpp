#include "Game.h"
#include <time.h>
#include <stdlib.h>

LifeGame::LifeGame(int width, int height)
    : m_gridWidth(width), m_gridHeight(height), m_isRunning(false), m_updateInterval(100) {

    // 限制合理范围
    if (m_gridWidth < 4) m_gridWidth = 4;
    if (m_gridHeight < 4) m_gridHeight = 4;
    if (m_gridWidth > 120) m_gridWidth = 120;
    if (m_gridHeight > 80) m_gridHeight = 80;

    InitGrid();
}

LifeGame::~LifeGame() {
    // 清理资源（如果需要）
}

void LifeGame::InitGrid() {
    srand(static_cast<unsigned int>(time(nullptr)));
    m_grid.assign(m_gridHeight, std::vector<bool>(m_gridWidth, false));
    m_nextGrid.assign(m_gridHeight, std::vector<bool>(m_gridWidth, false));

    for (int y = 0; y < m_gridHeight; y++) {
        for (int x = 0; x < m_gridWidth; x++) {
            m_grid[y][x] = (rand() % 10 < 4);
        }
    }
}

void LifeGame::UpdateGrid() {
    for (int y = 0; y < m_gridHeight; y++) {
        for (int x = 0; x < m_gridWidth; x++) {
            int neighbors = CountNeighbors(x, y);
            bool currentState = m_grid[y][x];
            m_nextGrid[y][x] = currentState ? (neighbors == 2 || neighbors == 3) : (neighbors == 3);
        }
    }

    // 更新当前网格状态
    for (int y = 0; y < m_gridHeight; y++) {
        for (int x = 0; x < m_gridWidth; x++) {
            m_grid[y][x] = m_nextGrid[y][x];
        }
    }
}

int LifeGame::CountNeighbors(int x, int y) {
    int count = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;

            // 循环边界处理
            int nx = (x + dx + m_gridWidth) % m_gridWidth;
            int ny = (y + dy + m_gridHeight) % m_gridHeight;

            if (m_grid[ny][nx]) count++;
        }
    }
    return count;
}

void LifeGame::ResetGrid() {
    if (m_grid.size() != static_cast<size_t>(m_gridHeight) || m_grid.empty() ||
        m_grid[0].size() != static_cast<size_t>(m_gridWidth)) {
        m_grid.assign(m_gridHeight, std::vector<bool>(m_gridWidth, false));
        m_nextGrid.assign(m_gridHeight, std::vector<bool>(m_gridWidth, false));
    }

    for (int y = 0; y < m_gridHeight; y++) {
        for (int x = 0; x < m_gridWidth; x++) {
            m_grid[y][x] = false;
            m_nextGrid[y][x] = false;
        }
    }
}

void LifeGame::ResizeGrid(int newWidth, int newHeight) {
    if (newWidth < 4) newWidth = 4;
    if (newHeight < 4) newHeight = 4;
    if (newWidth > 120) newWidth = 120;
    if (newHeight > 80) newHeight = 80;

    if (newWidth == m_gridWidth && newHeight == m_gridHeight) return;

    std::vector<std::vector<bool>> newGrid;
    std::vector<std::vector<bool>> newNext;
    newGrid.assign(newHeight, std::vector<bool>(newWidth, false));
    newNext.assign(newHeight, std::vector<bool>(newWidth, false));

    // 复制已有数据到新网格
    int minH = (newHeight < m_gridHeight) ? newHeight : m_gridHeight;
    int minW = (newWidth < m_gridWidth) ? newWidth : m_gridWidth;
    for (int y = 0; y < minH; y++) {
        for (int x = 0; x < minW; x++) {
            newGrid[y][x] = m_grid[y][x];
            newNext[y][x] = (y < static_cast<int>(m_nextGrid.size()) &&
                x < static_cast<int>(m_nextGrid[y].size())) ? m_nextGrid[y][x] : false;
        }
    }

    m_gridWidth = newWidth;
    m_gridHeight = newHeight;
    m_grid.swap(newGrid);
    m_nextGrid.swap(newNext);
}

void LifeGame::SetCell(int x, int y, bool state) {
    if (x >= 0 && x < m_gridWidth && y >= 0 && y < m_gridHeight) {
        m_grid[y][x] = state;
    }
}

bool LifeGame::GetCell(int x, int y) const {
    if (x >= 0 && x < m_gridWidth && y >= 0 && y < m_gridHeight) {
        return m_grid[y][x];
    }
    return false;
}

void LifeGame::Start() {
    m_isRunning = true;
}

void LifeGame::Pause() {
    m_isRunning = false;
}

void LifeGame::ToggleRunning() {
    m_isRunning = !m_isRunning;
}

void LifeGame::SetSpeed(int interval) {
    if (interval >= MIN_INTERVAL && interval <= MAX_INTERVAL) {
        m_updateInterval = interval;
    }
}

void LifeGame::IncreaseSpeed() {
    if (m_updateInterval > MIN_INTERVAL) {
        m_updateInterval -= SPEED_STEP;
    }
}

void LifeGame::DecreaseSpeed() {
    if (m_updateInterval < MAX_INTERVAL) {
        m_updateInterval += SPEED_STEP;
    }
}