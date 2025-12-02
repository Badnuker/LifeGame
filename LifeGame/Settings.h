#pragma once
#include <windows.h>

/**
 * @brief 游戏设置结构体
 * 
 * 存储所有可视化的配置选项。
 */
struct GameSettings
{
	// 颜色设置
	COLORREF gridColor; ///< 网格线颜色
	COLORREF cellColor; ///< 活细胞颜色
	COLORREF bgColor; ///< 背景颜色
	COLORREF textColor; ///< 文本颜色

	// 显示设置
	bool showGrid; ///< 是否显示网格线
	bool showHUD; ///< 是否显示HUD信息
	bool showHistory; ///< 是否显示历史统计图
	int gridLineWidth; ///< 网格线宽度

	// 默认构造函数
	GameSettings()
	{
		gridColor = RGB(40, 44, 52);
		cellColor = RGB(0, 255, 0); // 默认绿色
		bgColor = RGB(20, 24, 28);
		textColor = RGB(200, 200, 200);

		showGrid = true;
		showHUD = true;
		showHistory = true;
		gridLineWidth = 1;
	}
};

/**
 * @brief 全局设置管理器
 */
class SettingsManager
{
public:
	static SettingsManager& GetInstance()
	{
		static SettingsManager instance;
		return instance;
	}

	GameSettings& GetSettings() { return m_settings; }
	const GameSettings& GetSettings() const { return m_settings; }

private:
	SettingsManager()
	{
	}

	GameSettings m_settings;
};
