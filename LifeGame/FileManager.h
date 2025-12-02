#pragma once
#include <string>
#include <vector>
#include "Game.h"

/**
 * @brief 文件管理器类
 * 
 * 负责处理游戏数据的持久化存储。
 * 支持保存和加载游戏存档，包括网格状态、当前规则、统计数据等。
 * 使用自定义的文本格式 (.life) 或标准 RLE 格式。
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
	 * @param filePath 文件路径
	 * @param game 游戏实例引用
	 * @return true 保存成功
	 * @return false 保存失败
	 */
	bool SaveGame(const std::wstring& filePath, const LifeGame& game);

	/**
	 * @brief 从文件加载游戏状态
	 * 
	 * @param filePath 文件路径
	 * @param game 游戏实例引用 (将被修改)
	 * @return true 加载成功
	 * @return false 加载失败
	 */
	bool LoadGame(const std::wstring& filePath, LifeGame& game);

	/**
	 * @brief 导出为 RLE 格式
	 * 
	 * 仅导出当前网格图案，不包含游戏设置。
	 * 
	 * @param filePath 文件路径
	 * @param game 游戏实例
	 * @return true 成功
	 */
	bool ExportRLE(const std::wstring& filePath, const LifeGame& game);

	/**
	 * @brief 获取最后一次错误信息
	 */
	std::wstring GetLastError() const { return m_lastError; }

private:
	std::wstring m_lastError;

	// 辅助函数
	void WriteHeader(std::ofstream& file);
	bool ReadHeader(std::ifstream& file);
};
