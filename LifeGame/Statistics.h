#pragma once
#include <vector>
#include <deque>

/**
 * @brief 统计数据管理器
 * 
 * 负责收集、分析和存储游戏的运行时统计数据。
 * 包括种群数量历史、帧率历史、以及细胞活跃度热力图。
 * 这些数据用于在界面上绘制图表，增强科技感。
 */
class Statistics
{
public:
	/**
	 * @brief 构造函数
	 * @param width 网格宽度
	 * @param height 网格高度
	 */
	Statistics(int width, int height);
	~Statistics();

	/**
	 * @brief 重置统计数据
	 * @param width 新网格宽度
	 * @param height 新网格高度
	 */
	void Reset(int width, int height);

	/**
	 * @brief 记录一帧的数据
	 * 
	 * @param population 当前活细胞数量
	 * @param grid 当前网格数据 (用于更新热力图)
	 */
	void RecordFrame(int population, const std::vector<std::vector<bool>>& grid);

	/**
	 * @brief 获取种群历史数据
	 * @return const std::deque<int>& 种群数量队列
	 */
	const std::deque<int>& GetPopulationHistory() const { return m_populationHistory; }

	/**
	 * @brief 获取最大种群数量 (用于图表归一化)
	 */
	int GetMaxPopulation() const { return m_maxPopulation; }

	/**
	 * @brief 获取平均种群数量
	 */
	double GetAveragePopulation() const;

	/**
	 * @brief 获取指定位置的热力值
	 * 
	 * 热力值表示该细胞在历史上存活的累积次数。
	 * 
	 * @param x X坐标
	 * @param y Y坐标
	 * @return unsigned int 累积存活次数
	 */
	unsigned int GetHeatValue(int x, int y) const;

	/**
	 * @brief 获取最大热力值 (用于颜色映射)
	 */
	unsigned int GetMaxHeat() const { return m_maxHeat; }

private:
	// 历史数据配置
	static constexpr int MAX_HISTORY_SIZE = 200; ///< 保留最近 200 帧的数据

	std::deque<int> m_populationHistory; ///< 种群历史队列
	int m_maxPopulation; ///< 历史最大种群数
	long long m_totalPopulation; ///< 历史总种群数 (用于计算平均值)
	long long m_frameCount; ///< 总帧数

	// 热力图数据
	std::vector<std::vector<unsigned int>> m_heatMap; ///< 热力图网格
	unsigned int m_maxHeat; ///< 全局最大热力值
	int m_width;
	int m_height;
};
