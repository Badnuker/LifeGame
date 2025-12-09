#pragma once

class LifeGame;

/**
 * @brief 命令接口 (Command Interface)
 * 
 * 定义了命令模式的抽象基类。
 * 所有具体的游戏操作（如放置图案、修改细胞）都必须继承此类。
 * 通过封装操作为对象，支持撤销 (Undo) 和重做 (Redo) 功能。
 */
class Command {
public:
    // 虚析构函数，确保派生类对象被正确销毁
    virtual ~Command() {
    }

    /**
     * @brief 执行命令
     * 
     * 执行具体的业务逻辑。
     * @param game 游戏核心逻辑实例，命令将作用于此实例。
     */
    virtual void Execute(LifeGame &game) = 0;

    /**
     * @brief 撤销命令
     * 
     * 回滚 Execute 所做的操作，将游戏状态恢复到执行前。
     * @param game 游戏核心逻辑实例。
     */
    virtual void Undo(LifeGame &game) = 0;
};
