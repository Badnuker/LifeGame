#pragma once

#include <vector>
#include "RuleEngine.h"
#include "PatternLibrary.h"
#include "Statistics.h"
#include "CommandHistory.h"

/**
 * @brief 游戏核心逻辑类
 * 
 * 负责维护游戏状态、网格数据、演化逻辑以及与规则和图案库的交互。
 * 它是 Model-View-Controller (MVC) 架构中的 Model 部分。
 */
class LifeGame
{
public:
	/**
	 * @brief 构造函数
	 * 
	 * @param width 初始网格宽度
	 * @param height 初始网格高度
	 */
	LifeGame(int width = 80, int height = 60);

	/**
	 * @brief 析构函数
	 */
	~LifeGame();

	// ==========================================
	// 网格操作 (Grid Operations)
	// ==========================================

	/**
	 * @brief 随机初始化网格
	 * 
	 * 使用随机数生成器填充网格，通常用于演示。
	 */
	void InitGrid();

	/**
	 * @brief 计算下一代状态
	 * 
	 * 这是游戏的核心循环函数。它遍历所有细胞，
	 * 计算邻居数量，并根据当前规则更新状态。
	 */
	void UpdateGrid();

	/**
	 * @brief 清空网格
	 * 
	 * 将所有细胞设置为死亡状态。
	 */
	void ResetGrid();

	/**
	 * @brief 反转整个网格
	 * 
	 * 将所有活细胞变死，死细胞变活。
	 */
	void InvertGrid();

	/**
	 * @brief 清空指定区域
	 */
	void ClearArea(int x, int y, int w, int h);

	/**
	 * @brief 随机填充指定区域
	 */
	void RandomizeArea(int x, int y, int w, int h, float density = 0.5f);

	/**
	 * @brief 调整网格大小
	 * 
	 * 动态调整游戏区域的大小。会尝试保留左上角的现有图案。
	 * 
	 * @param newWidth 新宽度
	 * @param newHeight 新高度
	 */
	void ResizeGrid(int newWidth, int newHeight);

	/**
	 * @brief 设置单个细胞状态
	 * 
	 * @param x X坐标
	 * @param y Y坐标
	 * @param state true为活，false为死
	 */
	void SetCell(int x, int y, bool state);

	/**
	 * @brief 获取单个细胞状态
	 * 
	 * @param x X坐标
	 * @param y Y坐标
	 * @return true 活
	 * @return false 死
	 */
	bool GetCell(int x, int y) const;

	// ==========================================
	// 游戏控制 (Game Control)
	// ==========================================

	void Start(); ///< 开始演化
	void Pause(); ///< 暂停演化
	void ToggleRunning(); ///< 切换运行状态
	void SetRunning(bool running); ///< 设置运行状态

	// ==========================================
	// 图案与规则 (Patterns & Rules)
	// ==========================================

	/**
	 * @brief 在指定位置放置图案
	 * 
	 * @param x 左上角X坐标
	 * @param y 左上角Y坐标
	 * @param patternIndex 图案库中的索引
	 */
	void PlacePattern(int x, int y, int patternIndex);

	/**
	 * @brief 设置当前演化规则
	 * 
	 * @param ruleIndex 规则引擎中的索引
	 */
	void SetRule(int ruleIndex);

	/**
	 * @brief 获取规则引擎引用
	 */
	const RuleEngine& GetRuleEngine() const { return m_ruleEngine; }

	/**
	 * @brief 获取图案库引用
	 */
	const PatternLibrary& GetPatternLibrary() const { return m_patternLibrary; }

	/**
	 * @brief 获取统计模块引用
	 */
	const Statistics& GetStatistics() const { return m_stats; }

	/**
	 * @brief 获取命令历史记录引用 (用于撤销/重做)
	 */
	CommandHistory& GetCommandHistory() { return m_commandHistory; }

	// ==========================================
	// 速度控制 (Speed Control)
	// ==========================================

	void SetSpeed(int interval); ///< 设置更新间隔(ms)
	void IncreaseSpeed(); ///< 增加速度 (减少间隔)
	void DecreaseSpeed(); ///< 减少速度 (增加间隔)

	// ==========================================
	// 状态查询 (State Query)
	// ==========================================

	int GetWidth() const { return m_gridWidth; }
	int GetHeight() const { return m_gridHeight; }
	bool IsRunning() const { return m_isRunning; }
	int GetSpeed() const { return m_updateInterval; }
	int GetPopulation() const; ///< 获取当前活细胞总数

private:
	/**
	 * @brief 计算邻居数量
	 * 
	 * 计算指定坐标周围8个邻居中活细胞的数量。
	 * 支持环绕世界 (Toroidal) 拓扑结构。
	 * 
	 * @param x X坐标
	 * @param y Y坐标
	 * @return int 活邻居数量 (0-8)
	 */
	int CountNeighbors(int x, int y);

	// 数据成员
	std::vector<std::vector<bool>> m_grid; ///< 当前代网格数据
	std::vector<std::vector<bool>> m_nextGrid; ///< 下一代网格缓存 (双缓冲)
	int m_gridWidth; ///< 网格宽度
	int m_gridHeight; ///< 网格高度

	// 运行状态
	bool m_isRunning; ///< 是否正在自动演化
	int m_updateInterval; ///< 帧更新间隔 (毫秒)
	int m_currentRuleIndex; ///< 当前使用的规则索引

	// 子系统
	RuleEngine m_ruleEngine; ///< 规则引擎实例
	PatternLibrary m_patternLibrary; ///< 图案库实例
	Statistics m_stats; ///< 统计模块实例
	CommandHistory m_commandHistory; ///< 命令历史记录

	// 常量定义
	static constexpr int MIN_INTERVAL = 10; ///< 最小间隔 (最快)
	static constexpr int MAX_INTERVAL = 1000; ///< 最大间隔 (最慢)
	static constexpr int SPEED_STEP = 10; ///< 速度调节步长
};
