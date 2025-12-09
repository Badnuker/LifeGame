#pragma once
#include <vector>
#include <memory>
#include "Command.h"

/**
 * @brief 命令历史管理器 (Command History Manager)
 * 
 * 负责维护应用程序的撤销 (Undo) 和重做 (Redo) 栈。
 * 它持有所有已执行命令的所有权，并负责在撤销/重做时调用命令的相应方法。
 * 实现了标准的命令模式历史记录管理。
 */
class CommandHistory {
public:
    CommandHistory();

    ~CommandHistory();

    /**
     * @brief 执行新命令并加入历史记录
     * 
     * 执行命令的 Execute 方法，并将其压入撤销栈。
     * 同时会清空重做栈，因为产生了新的历史分支。
     * 
     * @param cmd 要执行的命令对象（智能指针，所有权将被转移给历史管理器）
     * @param game 游戏核心逻辑实例
     */
    void ExecuteCommand(std::unique_ptr<Command> cmd, LifeGame &game);

    /**
     * @brief 撤销上一步操作
     * 
     * 从撤销栈弹出最近的命令，执行其 Undo 方法，并将其压入重做栈。
     * @param game 游戏核心逻辑实例
     */
    void Undo(LifeGame &game);

    /**
     * @brief 重做上一步撤销的操作
     * 
     * 从重做栈弹出最近的命令，再次执行其 Execute 方法，并将其压回撤销栈。
     * @param game 游戏核心逻辑实例
     */
    void Redo(LifeGame &game);

    /**
     * @brief 清空所有历史记录
     * 
     * 释放所有存储的命令对象，重置撤销和重做栈。
     */
    void Clear();

    /**
     * @brief 检查是否可以撤销
     * @return true 如果撤销栈不为空
     */
    bool CanUndo() const;

    /**
     * @brief 检查是否可以重做
     * @return true 如果重做栈不为空
     */
    bool CanRedo() const;

private:
    // 撤销栈：存储已执行的命令，用于回滚操作
    std::vector<std::unique_ptr<Command> > m_undoStack;

    // 重做栈：存储已撤销的命令，用于恢复操作
    std::vector<std::unique_ptr<Command> > m_redoStack;

    // 最大历史记录条数，防止无限增长导致内存溢出
    static constexpr size_t MAX_HISTORY = 1000;
};
