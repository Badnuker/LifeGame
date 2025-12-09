#include "Game.h"
#include <time.h>
#include <stdlib.h>
#include <algorithm> // for std::min

/**
 * @brief 构造函数
 * 初始化游戏状态，设置默认网格大小和规则。
 */
LifeGame::LifeGame(int width, int height)
    : m_gridWidth(width), m_gridHeight(height), m_isRunning(false),
      m_updateInterval(100), m_currentRuleIndex(0),
      m_stats(width, height) {
    // 限制网格大小范围，防止内存溢出或性能过低
    // 比赛要求：支持大网格 (最大 2000x2000)
    if (m_gridWidth < 4) m_gridWidth = 4;
    if (m_gridHeight < 4) m_gridHeight = 4;
    if (m_gridWidth > 2000) m_gridWidth = 2000;
    if (m_gridHeight > 2000) m_gridHeight = 2000;

    InitGrid();
}

LifeGame::~LifeGame() {
}

/**
 * @brief 初始化网格
 * 随机生成初始状态，用于演示。
 */
void LifeGame::InitGrid() {
    srand(static_cast<unsigned int>(time(nullptr)));
    // 初始化两个网格缓冲区
    m_grid.assign(m_gridHeight, std::vector<bool>(m_gridWidth, false));
    m_nextGrid.assign(m_gridHeight, std::vector<bool>(m_gridWidth, false));

    // 随机生成初始状态
    // 密度约为 40% (rand() % 10 < 4)
    for (int y = 0; y < m_gridHeight; y++) {
        for (int x = 0; x < m_gridWidth; x++) {
            m_grid[y][x] = (rand() % 10 < 4);
        }
    }
}

/**
 * @brief 设置当前规则
 */
void LifeGame::SetRule(int ruleIndex) {
    if (m_ruleEngine.GetRule(ruleIndex) != nullptr) {
        m_currentRuleIndex = ruleIndex;
    }
}

/**
 * @brief 更新网格状态
 * 核心演化算法。
 */
void LifeGame::UpdateGrid() {
    // 并行化优化潜力：这里可以使用 OpenMP 或 std::execution::par
    // 但为了保持代码简单和兼容性，使用单线程循环
    for (int y = 0; y < m_gridHeight; y++) {
        for (int x = 0; x < m_gridWidth; x++) {
            // 1. 计算邻居数量
            int neighbors = CountNeighbors(x, y);
            bool currentState = m_grid[y][x];

            // 2. 委托给规则引擎计算下一状态
            // 不同的规则（如 Conway, HighLife）会有不同的判定逻辑
            bool nextState = m_ruleEngine.CalculateNextState(currentState, neighbors, m_currentRuleIndex);

            // 3. 写入下一代缓冲区
            m_nextGrid[y][x] = nextState;
        }
    }

    // 4. 交换缓冲区 (Swap Buffers)
    // 直接赋值 vector 会触发移动语义 (Move Semantics)，效率很高，避免了深拷贝
    m_grid = m_nextGrid;

    // 5. 记录统计数据 (用于图表显示)
    m_stats.RecordFrame(GetPopulation(), m_grid);
}

/**
 * @brief 获取活细胞总数
 */
int LifeGame::GetPopulation() const {
    int count = 0;
    for (const auto &row: m_grid) {
        for (bool cell: row) {
            if (cell) count++;
        }
    }
    return count;
}

/**
 * @brief 计算邻居数量
 * 实现了环绕世界 (Toroidal) 逻辑。
 */
int LifeGame::CountNeighbors(int x, int y) {
    int count = 0;
    // 优化：展开循环可以减少分支预测失败，但这里为了可读性保持循环
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue; // 跳过自己

            // 环绕处理：左边出界从右边回来，上边出界从下边回来
            // 使用取模运算实现坐标循环
            int nx = (x + dx + m_gridWidth) % m_gridWidth;
            int ny = (y + dy + m_gridHeight) % m_gridHeight;

            if (m_grid[ny][nx]) count++;
        }
    }
    return count;
}

/**
 * @brief 重置网格
 */
void LifeGame::ResetGrid() {
    // 清空当前网格
    for (auto &row: m_grid) {
        std::fill(row.begin(), row.end(), false);
    }
    // 清空下一代缓冲区
    for (auto &row: m_nextGrid) {
        std::fill(row.begin(), row.end(), false);
    }
    // 重置统计数据
    m_stats.Reset(m_gridWidth, m_gridHeight);
}

// 反转网格状态
void LifeGame::InvertGrid() {
    for (int y = 0; y < m_gridHeight; ++y) {
        for (int x = 0; x < m_gridWidth; ++x) {
            m_grid[y][x] = !m_grid[y][x];
        }
    }
}

// 清空指定区域
void LifeGame::ClearArea(int x, int y, int w, int h) {
    for (int dy = 0; dy < h; ++dy) {
        for (int dx = 0; dx < w; ++dx) {
            SetCell(x + dx, y + dy, false);
        }
    }
}

// 随机填充指定区域
void LifeGame::RandomizeArea(int x, int y, int w, int h, float density) {
    for (int dy = 0; dy < h; ++dy) {
        for (int dx = 0; dx < w; ++dx) {
            bool alive = (rand() % 100) < (density * 100);
            SetCell(x + dx, y + dy, alive);
        }
    }
}

/**
 * @brief 调整网格大小
 */
void LifeGame::ResizeGrid(int newWidth, int newHeight) {
    if (newWidth < 4) newWidth = 4;
    if (newHeight < 4) newHeight = 4;
    if (newWidth > 2000) newWidth = 2000;
    if (newHeight > 2000) newHeight = 2000;

    if (newWidth == m_gridWidth && newHeight == m_gridHeight) return;

    std::vector<std::vector<bool> > newGrid(newHeight, std::vector<bool>(newWidth, false));
    std::vector<std::vector<bool> > newNext(newHeight, std::vector<bool>(newWidth, false));

    // 用户要求：调整大小时清空画布，不保留原有内容
    // 原有的数据迁移逻辑已移除

    m_gridWidth = newWidth;
    m_gridHeight = newHeight;
    m_grid = std::move(newGrid);
    m_nextGrid = std::move(newNext);

    m_stats.Reset(newWidth, newHeight);
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

void LifeGame::Start() { m_isRunning = true; }
void LifeGame::Pause() { m_isRunning = false; }
void LifeGame::ToggleRunning() { m_isRunning = !m_isRunning; }
void LifeGame::SetRunning(bool running) { m_isRunning = running; }

void LifeGame::SetSpeed(int interval) {
    if (interval >= MIN_INTERVAL && interval <= MAX_INTERVAL) {
        m_updateInterval = interval;
    }
}

void LifeGame::IncreaseSpeed() {
    if (m_updateInterval > MIN_INTERVAL) m_updateInterval -= SPEED_STEP;
}

void LifeGame::DecreaseSpeed() {
    if (m_updateInterval < MAX_INTERVAL) m_updateInterval += SPEED_STEP;
}

/**
 * @brief 放置图案
 * 
 * 使用 PatternLibrary 解析并放置图案。
 */
void LifeGame::PlacePattern(int x, int y, int patternIndex) {
    const PatternData *pattern = m_patternLibrary.GetPattern(patternIndex);
    if (!pattern) return;

    std::vector<std::vector<bool> > patternGrid;
    // 解析 RLE 字符串
    if (m_patternLibrary.ParseRLE(pattern->rleString, patternGrid)) {
        int pH = static_cast<int>(patternGrid.size());
        if (pH == 0) return;
        int pW = static_cast<int>(patternGrid[0].size());

        for (int dy = 0; dy < pH; ++dy) {
            for (int dx = 0; dx < pW; ++dx) {
                // 仅当图案中的细胞为活时才设置，或者覆盖？
                // 通常覆盖比较好
                if (patternGrid[dy][dx]) {
                    SetCell(x + dx, y + dy, true);
                }
            }
        }
    }
}
