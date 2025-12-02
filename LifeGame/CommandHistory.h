#pragma once
#include <vector>
#include <memory>
#include "Command.h"

/**
 * @brief 命令历史管理器
 * 
 * 维护撤销和重做栈。
 */
class CommandHistory
{
public:
	CommandHistory();
	~CommandHistory();

	void ExecuteCommand(std::unique_ptr<Command> cmd, LifeGame& game);
	void Undo(LifeGame& game);
	void Redo(LifeGame& game);
	void Clear();

	bool CanUndo() const;
	bool CanRedo() const;

private:
	std::vector<std::unique_ptr<Command>> m_undoStack;
	std::vector<std::unique_ptr<Command>> m_redoStack;

	static constexpr size_t MAX_HISTORY = 1000;
};
