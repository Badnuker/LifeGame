#pragma once

class LifeGame;

/**
 * @brief 命令接口
 * 
 * 实现命令模式，支持撤销和重做操作。
 */
class Command
{
public:
	virtual ~Command()
	{
	}

	/**
	 * @brief 执行命令
	 * @param game 游戏实例
	 */
	virtual void Execute(LifeGame& game) = 0;

	/**
	 * @brief 撤销命令
	 * @param game 游戏实例
	 */
	virtual void Undo(LifeGame& game) = 0;
};
