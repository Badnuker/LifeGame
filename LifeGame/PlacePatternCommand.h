#pragma once
#include "Command.h"
#include <vector>

/**
 * @brief 细胞状态结构体
 * 
 * 用于记录单个细胞在操作前的状态，以便撤销。
 */
struct CellState {
    int x; ///< X坐标
    int y; ///< Y坐标
    bool oldState; ///< 旧状态 (true=活, false=死)
};

/**
 * @brief 放置图案命令 (Place Pattern Command)
 * 
 * 实现了 Command 接口，用于处理"放置图案"操作。
 * 它在执行前会记录受影响区域的原始细胞状态，从而支持撤销操作。
 * 相比于备份整个网格，这种增量备份方式大大节省了内存。
 */
class PlacePatternCommand : public Command {
public:
    /**
     * @brief 构造函数
     * 
     * 在构造时就会计算并保存受影响区域的旧状态。
     * @param x 放置位置 X
     * @param y 放置位置 Y
     * @param patternIndex 图案索引
     * @param game 游戏实例 (用于读取当前状态)
     */
    PlacePatternCommand(int x, int y, int patternIndex, const LifeGame &game);

    /**
     * @brief 执行命令
     * 调用 Game 的 PlacePattern 方法。
     */
    void Execute(LifeGame &game) override;

    /**
     * @brief 撤销命令
     * 恢复所有受影响细胞的旧状态。
     */
    void Undo(LifeGame &game) override;

private:
    int m_x; ///< 放置位置 X
    int m_y; ///< 放置位置 Y
    int m_patternIndex; ///< 图案索引
    std::vector<CellState> m_affectedCells; ///< 存储受影响细胞的旧状态
};
