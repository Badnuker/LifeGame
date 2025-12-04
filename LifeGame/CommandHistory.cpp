/**
 * @file CommandHistory.cpp
 * @brief 命令历史管理器实现
 * 
 * 实现了撤销/重做栈的具体逻辑，包括命令的执行、回滚、重做以及历史记录的容量管理。
 */

#include "CommandHistory.h"

// 构造函数
CommandHistory::CommandHistory()
{
}

// 析构函数：清理所有存储的命令
CommandHistory::~CommandHistory()
{
	Clear();
}

// 执行新命令并加入历史记录
void CommandHistory::ExecuteCommand(std::unique_ptr<Command> cmd, LifeGame& game)
{
	// 1. 执行命令的具体逻辑
	cmd->Execute(game);

	// 2. 将命令压入撤销栈 (Undo Stack)
	// 使用 std::move 转移所有权，因为 unique_ptr 不能复制
	m_undoStack.push_back(std::move(cmd));

	// 3. 清空重做栈 (Redo Stack)
	// 一旦执行了新操作，之前的重做历史就失效了（历史分支改变）
	m_redoStack.clear();

	// 4. 限制历史记录大小
	// 如果超过最大限制，移除最早的记录，防止内存无限增长
	if (m_undoStack.size() > MAX_HISTORY)
	{
		m_undoStack.erase(m_undoStack.begin());
	}
}

// 撤销操作
void CommandHistory::Undo(LifeGame& game)
{
	// 如果没有可撤销的操作，直接返回
	if (m_undoStack.empty()) return;

	// 1. 从撤销栈顶取出最近的一个命令
	auto cmd = std::move(m_undoStack.back());
	m_undoStack.pop_back();

	// 2. 执行该命令的撤销逻辑
	cmd->Undo(game);

	// 3. 将该命令压入重做栈，以便后续可以重做
	m_redoStack.push_back(std::move(cmd));
}

// 重做操作
void CommandHistory::Redo(LifeGame& game)
{
	// 如果没有可重做的操作，直接返回
	if (m_redoStack.empty()) return;

	// 1. 从重做栈顶取出最近撤销的一个命令
	auto cmd = std::move(m_redoStack.back());
	m_redoStack.pop_back();

	// 2. 再次执行该命令
	cmd->Execute(game);

	// 3. 将该命令放回撤销栈
	m_undoStack.push_back(std::move(cmd));
}

// 清空所有历史记录
void CommandHistory::Clear()
{
	m_undoStack.clear();
	m_redoStack.clear();
}

// 检查是否可以撤销
bool CommandHistory::CanUndo() const
{
	return !m_undoStack.empty();
}

// 检查是否可以重做
bool CommandHistory::CanRedo() const
{
	return !m_redoStack.empty();
}
