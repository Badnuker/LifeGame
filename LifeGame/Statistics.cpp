#include "Statistics.h"
#include <algorithm>

/**
 * @brief 构造函数
 */
Statistics::Statistics(int width, int height)
	: m_maxPopulation(0), m_totalPopulation(0), m_frameCount(0), m_maxHeat(0),
	  m_width(width), m_height(height)
{
	Reset(width, height);
}

/**
 * @brief 析构函数
 */
Statistics::~Statistics()
{
}

/**
 * @brief 重置统计数据
 */
void Statistics::Reset(int width, int height)
{
	m_width = width;
	m_height = height;

	m_populationHistory.clear();
	m_maxPopulation = 0;
	m_totalPopulation = 0;
	m_frameCount = 0;

	// 初始化热力图
	m_heatMap.assign(height, std::vector<unsigned int>(width, 0));
	m_maxHeat = 0;
}

/**
 * @brief 记录一帧数据
 */
void Statistics::RecordFrame(int population, const std::vector<std::vector<bool>>& grid)
{
	// 1. 更新种群历史
	m_populationHistory.push_back(population);
	if (m_populationHistory.size() > MAX_HISTORY_SIZE)
	{
		m_populationHistory.pop_front();
	}

	// 更新最大值
	if (population > m_maxPopulation)
	{
		m_maxPopulation = population;
	}
	else if (m_populationHistory.size() == MAX_HISTORY_SIZE)
	{
		// 如果队列满了，且刚才弹出的可能是最大值，我们需要重新扫描最大值
		// 这是一个 O(N) 操作，但 N 很小 (200)，所以没问题
		m_maxPopulation = 0;
		for (int p : m_populationHistory)
		{
			if (p > m_maxPopulation) m_maxPopulation = p;
		}
	}

	m_totalPopulation += population;
	m_frameCount++;

	// 2. 更新热力图
	// 确保网格大小匹配
	if (grid.size() != static_cast<size_t>(m_height) || (m_height > 0 && grid[0].size() != static_cast<size_t>(
		m_width)))
	{
		// 如果网格大小变了，重置热力图
		Reset(static_cast<int>(grid[0].size()), static_cast<int>(grid.size()));
	}

	for (int y = 0; y < m_height; ++y)
	{
		for (int x = 0; x < m_width; ++x)
		{
			if (grid[y][x])
			{
				m_heatMap[y][x]++;
				if (m_heatMap[y][x] > m_maxHeat)
				{
					m_maxHeat = m_heatMap[y][x];
				}
			}
		}
	}
}

/**
 * @brief 获取平均种群
 */
double Statistics::GetAveragePopulation() const
{
	if (m_frameCount == 0) return 0.0;
	return static_cast<double>(m_totalPopulation) / m_frameCount;
}

/**
 * @brief 获取热力值
 */
unsigned int Statistics::GetHeatValue(int x, int y) const
{
	if (x >= 0 && x < m_width && y >= 0 && y < m_height)
	{
		return m_heatMap[y][x];
	}
	return 0;
}
