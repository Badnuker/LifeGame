#pragma once
#include "Command.h"

/**
 * @brief 设置细胞状态命令
 */
class SetCellCommand : public Command
{
public:
	SetCellCommand(int x, int y, bool newState, bool oldState);

	void Execute(LifeGame& game) override;
	void Undo(LifeGame& game) override;

private:
	int m_x;
	int m_y;
	bool m_newState;
	bool m_oldState;
};
