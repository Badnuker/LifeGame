#pragma once
#include <string>
#include <vector>
#include <set>

/**
 * @brief 规则定义结构
 * 
 * 存储一个具体的细胞自动机规则。
 * 包括规则名称、描述以及出生/存活条件。
 */
struct RuleData {
    std::wstring name; ///< 规则名称
    std::wstring description; ///< 规则描述
    std::string ruleString; ///< 规则字符串 (例如 "B3/S23")
    std::set<int> birth; ///< 出生所需的邻居数量集合
    std::set<int> survival; ///< 存活所需的邻居数量集合
};

/**
 * @brief 规则引擎类
 * 
 * 负责解析规则字符串，管理预设规则，并执行状态转换逻辑。
 * 支持标准的 B/S 记法。
 */
class RuleEngine {
public:
    /**
     * @brief 构造函数
     * 初始化规则引擎并加载内置规则。
     */
    RuleEngine();

    /**
     * @brief 析构函数
     */
    ~RuleEngine();

    /**
     * @brief 获取所有可用规则
     * @return const std::vector<RuleData>& 规则列表
     */
    const std::vector<RuleData> &GetRules() const;

    /**
     * @brief 根据索引获取规则
     * @param index 索引
     * @return const RuleData* 规则指针
     */
    const RuleData *GetRule(int index) const;

    /**
     * @brief 解析规则字符串
     * 
     * 解析如 "B3/S23" 格式的字符串。
     * 
     * @param ruleStr 规则字符串
     * @param outBirth 输出的出生条件集合
     * @param outSurvival 输出的存活条件集合
     * @return true 解析成功
     * @return false 解析失败
     */
    bool ParseRule(const std::string &ruleStr, std::set<int> &outBirth, std::set<int> &outSurvival);

    /**
     * @brief 计算下一个状态
     * 
     * 根据当前状态、邻居数量和指定规则计算细胞的下一代状态。
     * 
     * @param currentState 当前细胞状态 (true=活, false=死)
     * @param neighbors 活邻居数量
     * @param ruleIndex 使用的规则索引
     * @return true 下一代存活
     * @return false 下一代死亡
     */
    bool CalculateNextState(bool currentState, int neighbors, int ruleIndex) const;

private:
    /**
     * @brief 初始化内置规则
     */
    void InitBuiltinRules();

    std::vector<RuleData> m_rules; ///< 存储所有规则
};
