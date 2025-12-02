#include "CommandHistory.h"

CommandHistory::CommandHistory()
{
}

CommandHistory::~CommandHistory()
{
	Clear();
}

void CommandHistory::ExecuteCommand(std::unique_ptr<Command> cmd, LifeGame& game)
{
	cmd->Execute(game);
	m_undoStack.push_back(std::move(cmd));

	// 清空重做栈
	m_redoStack.clear();

	// 限制历史记录大小
	if (m_undoStack.size() > MAX_HISTORY)
	{
		m_undoStack.erase(m_undoStack.begin());
	}
}

void CommandHistory::Undo(LifeGame& game)
{
	if (m_undoStack.empty()) return;

	auto cmd = std::move(m_undoStack.back());
	m_undoStack.pop_back();

	cmd->Undo(game);
	m_redoStack.push_back(std::move(cmd));
}

void CommandHistory::Redo(LifeGame& game)
{
	if (m_redoStack.empty()) return;

	auto cmd = std::move(m_redoStack.back());
	m_redoStack.pop_back();

	cmd->Execute(game);
	m_undoStack.push_back(std::move(cmd));
}

void CommandHistory::Clear()
{
	m_undoStack.clear();
	m_redoStack.clear();
}

bool CommandHistory::CanUndo() const
{
	return !m_undoStack.empty();
}

bool CommandHistory::CanRedo() const
{
	return !m_redoStack.empty();
}
