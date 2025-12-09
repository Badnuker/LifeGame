#include "SetCellCommand.h"
#include "Game.h"

/**
 * @brief 构造函数
 * 
 * 创建一个设置细胞状态的命令。
 * 
 * @param x 细胞的 X 坐标
 * @param y 细胞的 Y 坐标
 * @param newState 想要设置的新状态 (true: 活, false: 死)
 * @param oldState 修改前的旧状态 (用于撤销)
 */
SetCellCommand::SetCellCommand(int x, int y, bool newState, bool oldState)
    : m_x(x), m_y(y), m_newState(newState), m_oldState(oldState) {
}

/**
 * @brief 执行命令
 * 
 * 将指定位置的细胞设置为新状态。
 * 
 * @param game 游戏实例引用
 */
void SetCellCommand::Execute(LifeGame &game) {
    game.SetCell(m_x, m_y, m_newState);
}

/**
 * @brief 撤销命令
 * 
 * 将指定位置的细胞恢复为旧状态。
 * 
 * @param game 游戏实例引用
 */
void SetCellCommand::Undo(LifeGame &game) {
    game.SetCell(m_x, m_y, m_oldState);
}
