#pragma once
#include <string>
#include <vector>
#include "Game.h"

/**
 * @brief 文件管理器类 (File Manager)
 * 
 * 负责处理游戏数据的持久化存储。
 * 支持保存和加载游戏存档，包括网格状态、当前规则、统计数据等。
 * 使用自定义的文本格式 (.life) 或标准 RLE 格式。
 * 
 * 功能包括：
 * 1. 保存当前游戏状态（网格、速度、规则等）到文件。
 * 2. 从文件加载游戏状态，恢复之前的进度。
 * 3. 导出当前图案为通用的 RLE (Run Length Encoded) 格式，以便与其他生命游戏软件交换数据。
 */
class FileManager
{
public:
	/**
	 * @brief 构造函数
	 */
	FileManager();
	~FileManager();

	/**
	 * @brief 保存游戏状态到文件
	 * 
	 * 将当前的游戏配置（宽度、高度、速度）和网格数据写入指定文件。
	 * 文件格式为自定义的文本格式，包含头部信息和数据块。
	 * 
	 * @param filePath 目标文件路径（宽字符，支持中文路径）
	 * @param game 游戏实例引用，用于获取当前状态
	 * @return true 保存成功
	 * @return false 保存失败（如权限不足、磁盘已满等）
	 */
	bool SaveGame(const std::wstring& filePath, const LifeGame& game);

	/**
	 * @brief 从文件加载游戏状态
	 * 
	 * 读取指定文件的内容，解析配置和网格数据，并应用到游戏实例中。
	 * 加载前会重置当前游戏状态。
	 * 
	 * @param filePath 源文件路径
	 * @param game 游戏实例引用 (将被修改以反映加载的状态)
	 * @return true 加载成功
	 * @return false 加载失败（如文件不存在、格式错误等）
	 */
	bool LoadGame(const std::wstring& filePath, LifeGame& game);

	/**
	 * @brief 导出为 RLE 格式
	 * 
	 * 仅导出当前网格图案，不包含游戏设置（如速度）。
	 * RLE (Run Length Encoded) 是生命游戏社区通用的图案交换格式。
	 * 
	 * @param filePath 目标文件路径
	 * @param game 游戏实例
	 * @return true 导出成功
	 */
	bool ExportRLE(const std::wstring& filePath, const LifeGame& game);

	/**
	 * @brief 获取最后一次错误信息
	 * 
	 * 当 SaveGame 或 LoadGame 返回 false 时，调用此函数获取具体的错误描述。
	 * @return 错误信息字符串
	 */
	std::wstring GetLastError() const { return m_lastError; }

private:
	// 存储最后一次操作的错误信息
	std::wstring m_lastError;

	// 辅助函数 (目前未使用，保留接口)
	void WriteHeader(std::ofstream& file);
	bool ReadHeader(std::ifstream& file);
};
