#pragma once

#include <vector>
#include <windows.h>

class LifeGame {
public:
    // 构造函数和初始化
    LifeGame(int width = 80, int height = 60);
    ~LifeGame();

    // 网格操作
    void InitGrid();
    void UpdateGrid();
    void ResetGrid();
    void ResizeGrid(int newWidth, int newHeight);
    void SetCell(int x, int y, bool state);
    bool GetCell(int x, int y) const;

    // 游戏状态控制
    void Start();
    void Pause();
    void ToggleRunning();
    void SetSpeed(int interval);
    void IncreaseSpeed();
    void DecreaseSpeed();

    // 访问器
    int GetWidth() const { return m_gridWidth; }
    int GetHeight() const { return m_gridHeight; }
    bool IsRunning() const { return m_isRunning; }
    int GetSpeed() const { return m_updateInterval; }

private:
    int CountNeighbors(int x, int y);

    // 网格数据
    std::vector<std::vector<bool>> m_grid;
    std::vector<std::vector<bool>> m_nextGrid;
    int m_gridWidth;
    int m_gridHeight;

    // 游戏状态
    bool m_isRunning;
    int m_updateInterval;

    // 常量
    static constexpr int MIN_INTERVAL = 50;
    static constexpr int MAX_INTERVAL = 500;
    static constexpr int SPEED_STEP = 50;
};