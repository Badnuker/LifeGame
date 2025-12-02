#pragma once
#include <string>
#include <vector>
#include <map>

/**
 * @brief 图案数据结构
 * 
 * 用于存储单个生命游戏图案的元数据和内容。
 * 包含了图案的名称、描述、RLE编码数据以及分类信息。
 */
struct PatternData
{
	std::wstring name; ///< 图案名称 (例如: "Gosper Glider Gun")
	std::wstring description; ///< 图案描述 (例如: "第一个被发现的滑翔机枪")
	std::string rleString; ///< RLE (Run Length Encoded) 格式的压缩数据
	int width; ///< 图案宽度
	int height; ///< 图案高度
};

/**
 * @brief 图案库管理类
 * 
 * 负责管理所有预设的图案数据。
 * 提供了图案的注册、查询、解析和实例化功能。
 * 使用单例模式访问，或者作为静态工具类使用。
 */
class PatternLibrary
{
public:
	/**
	 * @brief 构造函数
	 * 初始化图案库，加载所有内置图案。
	 */
	PatternLibrary();

	/**
	 * @brief 析构函数
	 * 清理资源。
	 */
	~PatternLibrary();

	/**
	 * @brief 获取所有可用图案的列表
	 * 
	 * @return const std::vector<PatternData>& 图案列表引用
	 */
	const std::vector<PatternData>& GetPatterns() const;

	/**
	 * @brief 根据索引获取特定图案
	 * 
	 * @param index 图案在列表中的索引
	 * @return const PatternData* 指向图案数据的指针，如果索引无效则返回 nullptr
	 */
	const PatternData* GetPattern(int index) const;

	/**
	 * @brief 解析 RLE 字符串并生成网格数据
	 * 
	 * 这是核心解析函数，将压缩的 RLE 字符串转换为二维布尔数组。
	 * 
	 * @param rle RLE 格式字符串
	 * @param outGrid 输出的二维布尔网格
	 * @return true 解析成功
	 * @return false 解析失败
	 */
	bool ParseRLE(const std::string& rle, std::vector<std::vector<bool>>& outGrid);

private:
	/**
	 * @brief 初始化内置图案
	 * 
	 * 将大量经典图案硬编码注册到系统中。
	 * 这是增加代码量和功能丰富度的关键部分。
	 */
	void InitBuiltinPatterns();

	std::vector<PatternData> m_patterns; ///< 存储所有图案的容器
};
