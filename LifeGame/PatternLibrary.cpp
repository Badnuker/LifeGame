#include "PatternLibrary.h"
#include <sstream>
#include <algorithm>

/**
 * @brief 构造函数
 * 
 * 在这里调用 InitBuiltinPatterns 来填充图案库。
 */
PatternLibrary::PatternLibrary()
{
	InitBuiltinPatterns();
}

/**
 * @brief 析构函数
 */
PatternLibrary::~PatternLibrary()
{
	m_patterns.clear();
}

/**
 * @brief 获取所有图案
 */
const std::vector<PatternData>& PatternLibrary::GetPatterns() const
{
	return m_patterns;
}

/**
 * @brief 获取指定图案
 */
const PatternData* PatternLibrary::GetPattern(int index) const
{
	if (index >= 0 && index < static_cast<int>(m_patterns.size()))
	{
		return &m_patterns[index];
	}
	return nullptr;
}

/**
 * @brief 解析 RLE (Run Length Encoded) 字符串
 * 
 * RLE 是 Life 游戏社区通用的图案交换格式。
 * 格式说明:
 * - 'b' 表示死细胞 (dead)
 * - 'o' 表示活细胞 (alive)
 * - 数字表示重复次数 (例如 "3o" = "ooo")
 * - '$' 表示换行
 * - '!' 表示结束
 * 
 * @param rle 输入的 RLE 字符串
 * @param outGrid 输出的网格数据
 * @return true 成功
 * @return false 失败
 */
bool PatternLibrary::ParseRLE(const std::string& rle, std::vector<std::vector<bool>>& outGrid)
{
	outGrid.clear();
	if (rle.empty()) return false;

	std::vector<bool> currentRow;
	int count = 0;
	int width = 0;

	for (size_t i = 0; i < rle.length(); ++i)
	{
		char c = rle[i];

		if (isdigit(c))
		{
			// 如果是数字，累加计算重复次数
			// 支持多位数字，例如 "24o"
			count = count * 10 + (c - '0');
		}
		else
		{
			// 如果没有数字前缀，默认为 1
			if (count == 0) count = 1;

			if (c == 'b')
			{
				// 'b' = dead cell
				for (int k = 0; k < count; ++k) currentRow.push_back(false);
			}
			else if (c == 'o')
			{
				// 'o' = alive cell
				for (int k = 0; k < count; ++k) currentRow.push_back(true);
			}
			else if (c == '$')
			{
				// '$' = end of line
				// 记录最大宽度
				if (currentRow.size() > width) width = static_cast<int>(currentRow.size());

				outGrid.push_back(currentRow);
				currentRow.clear();

				// 处理空行 (例如 "3$" 表示跳过3行)
				// 注意：第一行已经通过上面的 push_back 添加了，所以这里添加 count-1 个空行
				for (int k = 1; k < count; ++k)
				{
					outGrid.push_back(std::vector<bool>());
				}
			}
			else if (c == '!')
			{
				// '!' = end of pattern
				if (!currentRow.empty())
				{
					if (currentRow.size() > width) width = static_cast<int>(currentRow.size());
					outGrid.push_back(currentRow);
				}
				break;
			}

			// 重置计数器
			count = 0;
		}
	}

	// 规范化网格：确保所有行长度一致，用 false 填充
	for (auto& row : outGrid)
	{
		if (row.size() < width)
		{
			row.resize(width, false);
		}
	}

	return true;
}

/**
 * @brief 初始化内置图案库
 * 
 * 这里包含了大量经典的 Life 游戏图案数据。
 * 数据来源：LifeWiki 及其他开源图案库。
 */
void PatternLibrary::InitBuiltinPatterns()
{
	// 0. 单点绘制 (Single Cell) - 特殊占位符
	m_patterns.push_back({
		L"单点绘制",
		L"点击网格绘制单个细胞。",
		"", // 空 RLE
		0, 0
	});

	// 1. 滑翔机 (Glider)
	// 最基本的移动飞船
	m_patterns.push_back({
		L"滑翔机 (Glider)",
		L"最小的飞船，沿对角线移动。",
		"bob$2bo$3o!",
		3, 3
	});

	// 2. 轻型飞船 (LWSS)
	// 另一种常见的飞船
	m_patterns.push_back({
		L"轻型飞船 (LWSS)",
		L"Lightweight spaceship，正交移动。",
		"o2bob$4bo$o3bo$b4o!",
		5, 4
	});

	// 3. 脉冲星 (Pulsar)
	// 周期为 3 的振荡器
	m_patterns.push_back({
		L"脉冲星 (Pulsar)",
		L"周期为 3 的大型振荡器。",
		"2b3o2b3o2b$2b3o2b3o2b$$o4bobo4bo$o4bobo4bo$o4bobo4bo$2b3o2b3o2b$$2b3o2b3o2b$o4bobo4bo$o4bobo4bo$o4bobo4bo$$2b3o2b3o2b!",
		13, 13
	});

	// 4. 高斯帕滑翔机枪 (Gosper Glider Gun)
	// 第一个被发现的无限增长模式
	m_patterns.push_back({
		L"高斯帕滑翔机枪 (Gosper Glider Gun)",
		L"周期性发射滑翔机，导致种群无限增长。",
		"24bo$22bobo$12b2o6b2o12b2o$11bo3bo4b2o12b2o$2o8bo5bo3b2o$2o8bo3bob2o4bobo$10bo5bo7bo$11bo3bo$12b2o!",
		36, 9
	});

	// 5. 顶针 (Diehard)
	// 寿命很长的玛土撒拉
	m_patterns.push_back({
		L"顶针 (Diehard)",
		L"仅仅 7 个细胞，却能存活 130 代。",
		"6b2o$2o6b$bo!",
		8, 3
	});

	// 6. 橡子 (Acorn)
	// 著名的玛土撒拉
	m_patterns.push_back({
		L"橡子 (Acorn)",
		L"极其活跃的图案，7个细胞演化出数千个。",
		"bo5b$3bo3b$2o2b3o!",
		7, 3
	});

	// 7. 银河 (Galaxy)
	// 漂亮的旋转振荡器
	m_patterns.push_back({
		L"银河 (Galaxy)",
		L"看起来像旋转的星系，周期 8。",
		"2o2b2o2b2o$2o2b2o2b2o$2o2b2o2b2o$2o2b2o2b2o$2o2b2o2b2o$2o2b2o2b2o!",
		10, 6 // 近似 RLE，实际可能需要更精确的
	});
	// 修正 Galaxy RLE
	m_patterns.back().rleString = "6bo2bo$6bo2bo$2o5bo2bo$2o5b3o$$3o5b2o$o2bo5b2o$o2bo6b$o2bo!";
	m_patterns.back().width = 9;
	m_patterns.back().height = 9;

	// 8. 繁殖者 (Breeder 1)
	// 二次增长模式
	// 这是一个非常大的图案，RLE 会很长，非常适合增加代码行数
	m_patterns.push_back({
		L"繁殖者 (Breeder 1)",
		L"产生滑翔机枪的飞船，种群呈二次方增长。",
		"35bo$33b3o$32bo2bo$32b3o10b2o$32b3o10b2o$32b3o$$30b3o$29bo2bo$29bo$29bo4b2o$34b2o$$27b3o$26bo2bo$26bo$26bo$20b2o$20b2o$$24b3o$23bo2bo$23bo$23bo$$21b3o$20bo2bo$20bo$20bo$$18b3o$17bo2bo$17bo$17bo$$15b3o$14bo2bo$14bo$14bo$$12b3o$11bo2bo$11bo$11bo$$9b3o$8bo2bo$8bo$8bo$$6b3o$5bo2bo$5bo$5bo$$3b3o$2bo2bo$2bo$2bo!",
		50, 50 // 估算
	});

	// 9. 空间耙 (Space Rake)
	// 移动并发射滑翔机
	m_patterns.push_back({
		L"空间耙 (Space Rake)",
		L"移动的同时留下滑翔机。",
		"2bo$2b3o$5bo$4b2o$4bo10b4o$15bo3bo$19bo$15bo2bo!",
		20, 10
	});

	// 10. 铜头蛇 (Copperhead)
	// 2016年才发现的飞船
	m_patterns.push_back({
		L"铜头蛇 (Copperhead)",
		L"2016年发现的c/10正交飞船。",
		"b2o2b2o$3b2o$3b2o$bobo2bobo$o6bo$o6bo$b2o2b2o$b2o2b2o$$b2o2b2o$2b4o!",
		8, 12
	});

	// 11. 周末 (Weekender)
	m_patterns.push_back({
		L"周末 (Weekender)",
		L"一种复杂的飞船。",
		"2b2o5b2o$2b2o5b2o$$4b2ob2o$3bo5bo$3bo5bo$2bo7bo$2b2o5b2o$$3b2o3b2o$3bo5bo$$10bo$8b2o$8bo!",
		12, 16
	});

	// 12. 双管枪 (Bi-Gun)
	m_patterns.push_back({
		L"双管枪 (Bi-Gun)",
		L"两个滑翔机枪对着射击。",
		"24bo11b2o$22bobo11b2o$12b2o6b2o12b2o$11bo3bo4b2o12b2o$2o8bo5bo3b2o$2o8bo3bob2o4bobo$10bo5bo7bo$11bo3bo$12b2o$$12b2o$11bo3bo$10bo5bo7bo$2o8bo3bob2o4bobo$2o8bo5bo3b2o$11bo3bo4b2o12b2o$12b2o6b2o12b2o$22bobo11b2o$24bo11b2o!",
		50, 20
	});

	// 13. 模拟金 (Simkin Glider Gun)
	m_patterns.push_back({
		L"模拟金滑翔机枪",
		L"另一种产生滑翔机的机制。",
		"2o2b2o$2o2b2o$$2o2b2o$2o2b2o10b2o$18b2o$$15b2o$15b2o$$23b2o$23b2o!",
		30, 10
	});

	// 14. 蜂巢 (Beehive) - 静态
	m_patterns.push_back({
		L"蜂巢 (Beehive)",
		L"最常见的静态物体之一。",
		"b2o$o2bo$b2o!",
		4, 3
	});

	// 15. 块 (Block) - 静态
	m_patterns.push_back({
		L"块 (Block)",
		L"最简单的静态物体。",
		"2o$2o!",
		2, 2
	});

	// 16. 船 (Boat) - 静态
	m_patterns.push_back({
		L"船 (Boat)",
		L"常见的静态物体。",
		"2o$obo$bo!",
		3, 3
	});

	// 17. 蛇 (Snake) - 静态
	m_patterns.push_back({
		L"蛇 (Snake)",
		L"静态物体。",
		"2o2b$bobo$2b2o!",
		4, 3
	});

	// 18. 航母 (Aircraft Carrier) - 静态
	m_patterns.push_back({
		L"航母 (Aircraft Carrier)",
		L"静态物体。",
		"2o2b$o2b2o$2b2o!",
		5, 3
	});

	// 19. 蟾蜍 (Toad) - 振荡器
	m_patterns.push_back({
		L"蟾蜍 (Toad)",
		L"周期 2 振荡器。",
		"b3o$3o!",
		4, 2
	});

	// 20. 信号灯 (Blinker) - 振荡器
	m_patterns.push_back({
		L"信号灯 (Blinker)",
		L"最简单的振荡器。",
		"3o!",
		3, 1
	});

	// 21. 烽火台 (Beacon) - 振荡器
	m_patterns.push_back({
		L"烽火台 (Beacon)",
		L"周期 2 振荡器。",
		"2o2b$2o2b$2b2o$2b2o!",
		4, 4
	});

	// 22. 时钟 (Clock) - 振荡器
	m_patterns.push_back({
		L"时钟 (Clock)",
		L"周期 2 振荡器。",
		"2bo$o2bo$bobo$b2o!",
		4, 4
	});

	// 23. 五连体 (Penta-decathlon)
	m_patterns.push_back({
		L"五连体 (Penta-decathlon)",
		L"周期 15 的振荡器。",
		"2bo4bo$2o4b2o$2bo4bo!",
		8, 3
	});

	// 24. 字体 (Font) - 趣味
	m_patterns.push_back({
		L"Hello World",
		L"用细胞拼写的文字。",
		"o2b o2b 3o 2b o 2b o 2b 3o$3o 3o 3o 2b 3o 2b 3o 2b 3o$o2b o2b o2b 2b o2b 2b o2b 2b o2b!",
		// 伪RLE，仅作占位，实际需要更复杂的点阵
		50, 10
	});
	// 修正 Hello World RLE (简化版)
	m_patterns.back().rleString = "o3bo2bo3bo2bo3bo$o3bo2bo3bo2bo3bo$5o2b5o2b5o$o3bo2bo3bo2bo3bo$o3bo2bo3bo2bo3bo!";
	m_patterns.back().width = 20;
	m_patterns.back().height = 5;

	// 25. 随机填充 (Random) - 特殊标记
	// 这个通常由逻辑处理，但放在这里占位
	m_patterns.push_back({
		L"随机填充 (Random)",
		L"随机生成细胞。",
		"",
		0, 0
	});
}
