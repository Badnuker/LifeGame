#pragma once
#include "Command.h"

/**
 * @brief 设置细胞状态命令
 * 
 * 实现了 Command 接口，用于修改单个细胞的状态（生/死）。
 * 支持撤销操作，是命令模式的具体实现之一。
 */
class SetCellCommand : public Command
{
public:
	/**
	 * @brief 构造函数
	 * @param x 细胞 X 坐标
	 * @param y 细胞 Y 坐标
	 * @param newState 目标状态
	 * @param oldState 原始状态 (用于撤销)
	 */
	SetCellCommand(int x, int y, bool newState, bool oldState);

	/**
	 * @brief 执行命令：设置细胞为新状态
	 */
	void Execute(LifeGame& game) override;

	/**
	 * @brief 撤销命令：恢复细胞为旧状态
	 */
	void Undo(LifeGame& game) override;

private:
	int m_x;            ///< 细胞 X 坐标
	int m_y;            ///< 细胞 Y 坐标
	bool m_newState;    ///< 新状态
	bool m_oldState;    ///< 旧状态
};
