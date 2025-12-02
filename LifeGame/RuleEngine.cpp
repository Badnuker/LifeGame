#include "RuleEngine.h"
#include <sstream>
#include <algorithm>

/**
 * @brief 构造函数
 */
RuleEngine::RuleEngine()
{
	InitBuiltinRules();
}

/**
 * @brief 析构函数
 */
RuleEngine::~RuleEngine()
{
	m_rules.clear();
}

/**
 * @brief 获取所有规则
 */
const std::vector<RuleData>& RuleEngine::GetRules() const
{
	return m_rules;
}

/**
 * @brief 获取指定规则
 */
const RuleData* RuleEngine::GetRule(int index) const
{
	if (index >= 0 && index < static_cast<int>(m_rules.size()))
	{
		return &m_rules[index];
	}
	return nullptr;
}

/**
 * @brief 解析规则字符串
 * 
 * 支持格式: "B3/S23", "23/3", "B36S23" 等常见变体。
 * 
 * @param ruleStr 规则字符串
 * @param outBirth 输出出生集合
 * @param outSurvival 输出存活集合
 * @return true 成功
 */
bool RuleEngine::ParseRule(const std::string& ruleStr, std::set<int>& outBirth, std::set<int>& outSurvival)
{
	outBirth.clear();
	outSurvival.clear();

	std::string upperStr = ruleStr;
	std::transform(upperStr.begin(), upperStr.end(), upperStr.begin(), toupper);

	bool parsingBirth = false;
	bool parsingSurvival = false;

	// 简单状态机解析
	// 如果以 'B' 开头，则进入 B 模式，直到遇到 'S' 或 '/'
	// 如果以数字开头，通常假设是 S/B 格式 (Golly 格式) 或者 B/S 格式
	// 这里我们主要支持 B.../S... 格式

	size_t slashPos = upperStr.find('/');
	if (slashPos != std::string::npos)
	{
		// 存在斜杠，分割处理
		std::string part1 = upperStr.substr(0, slashPos);
		std::string part2 = upperStr.substr(slashPos + 1);

		// 处理第一部分
		if (part1.find('B') != std::string::npos)
		{
			for (char c : part1) if (isdigit(c)) outBirth.insert(c - '0');
		}
		else if (part1.find('S') != std::string::npos)
		{
			for (char c : part1) if (isdigit(c)) outSurvival.insert(c - '0');
		}
		else
		{
			// 默认假设第一部分是 Survival (如果不带字母) - 或者是 Birth? 
			// 标准 Life 是 23/3 -> S23/B3
			// 但有些地方是 B3/S23
			// 为了保险，我们要求必须带 B 或 S，或者使用标准 B3/S23 顺序
			// 这里简化处理：如果没字母，假设是 B/S
			for (char c : part1) if (isdigit(c)) outBirth.insert(c - '0');
		}

		// 处理第二部分
		if (part2.find('B') != std::string::npos)
		{
			for (char c : part2) if (isdigit(c)) outBirth.insert(c - '0');
		}
		else if (part2.find('S') != std::string::npos)
		{
			for (char c : part2) if (isdigit(c)) outSurvival.insert(c - '0');
		}
		else
		{
			for (char c : part2) if (isdigit(c)) outSurvival.insert(c - '0');
		}
	}
	else
	{
		// 无斜杠，可能是 "B3S23"
		bool isB = false;
		for (char c : upperStr)
		{
			if (c == 'B')
			{
				isB = true;
				continue;
			}
			if (c == 'S')
			{
				isB = false;
				continue;
			}
			if (isdigit(c))
			{
				if (isB) outBirth.insert(c - '0');
				else outSurvival.insert(c - '0');
			}
		}
	}

	return true;
}

/**
 * @brief 计算下一代状态
 */
bool RuleEngine::CalculateNextState(bool currentState, int neighbors, int ruleIndex) const
{
	const RuleData* rule = GetRule(ruleIndex);
	if (!rule) return currentState; // 默认保持不变

	if (currentState)
	{
		// 存活检查
		return rule->survival.count(neighbors) > 0;
	}
	// 出生检查
	return rule->birth.count(neighbors) > 0;
}

/**
 * @brief 初始化内置规则
 * 
 * 填充大量有趣的规则。
 */
void RuleEngine::InitBuiltinRules()
{
	// 辅助 lambda
	auto add = [&](const std::wstring& name, const std::wstring& desc, const std::string& rule)
	{
		RuleData d;
		d.name = name;
		d.description = desc;
		d.ruleString = rule;
		ParseRule(rule, d.birth, d.survival);
		m_rules.push_back(d);
	};

	// 1. Conway's Life
	add(L"Conway (经典)", L"最经典的生命游戏规则 (B3/S23)。", "B3/S23");

	// 2. HighLife
	add(L"HighLife (复制)", L"类似经典规则，但 6 个邻居也会重生，导致复制子出现 (B36/S23)。", "B36/S23");

	// 3. Day & Night
	add(L"Day & Night", L"死活细胞对称，非常复杂的规则 (B3678/S34678)。", "B3678/S34678");

	// 4. Seeds
	add(L"Seeds (种子)", L"所有活细胞都会死，只有2个邻居时出生，爆发性极强 (B2/S)。", "B2/S");

	// 5. Life without Death
	add(L"Life without Death", L"细胞一旦活了就永远不死，形成迷宫 (B3/S012345678)。", "B3/S012345678");

	// 6. 34 Life
	add(L"34 Life", L"3或4个邻居出生或存活 (B34/S34)。", "B34/S34");

	// 7. Diamoeba
	add(L"Diamoeba", L"形成类似变形虫的动态图案 (B35678/S5678)。", "B35678/S5678");

	// 8. 2x2
	add(L"2x2", L"包含大量周期振荡器 (B36/S125)。", "B36/S125");

	// 9. Morley
	add(L"Morley", L"支持很多飞船 (B368/S245)。", "B368/S245");

	// 10. Anneal
	add(L"Anneal", L"退火算法，形成块状结构 (B4678/S35678)。", "B4678/S35678");

	// 11. Maze
	add(L"Maze (迷宫)", L"自动生成迷宫结构 (B3/S12345)。", "B3/S12345");

	// 12. Maze 2
	add(L"Maze 2", L"另一种迷宫生成规则 (B37/S12345)。", "B37/S12345");

	// 13. Coral
	add(L"Coral (珊瑚)", L"生长缓慢，像珊瑚一样 (B3/S45678)。", "B3/S45678");

	// 14. Gnarl
	add(L"Gnarl", L"简单的规则，复杂的行为 (B1/S1)。", "B1/S1");

	// 15. Replicator
	add(L"Replicator", L"每个图案都会复制自身 (B1357/S1357)。", "B1357/S1357");

	// 16. Fredkin
	add(L"Fredkin", L"每个细胞根据奇偶性复制 (B1357/S02468)。", "B1357/S02468");

	// 17. Move
	add(L"Move", L"细胞倾向于移动 (B268/S245)。", "B268/S245");

	// 18. Assimilation
	add(L"Assimilation", L"同化规则 (B345/S4567)。", "B345/S4567");

	// 19. Coagulations
	add(L"Coagulations", L"凝结 (B378/S235678)。", "B378/S235678");

	// 20. Walled Cities
	add(L"Walled Cities", L"形成围墙城市 (B45678/S2345)。", "B45678/S2345");
}
