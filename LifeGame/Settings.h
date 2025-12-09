#pragma once
#include <windows.h>

/**
 * @brief 游戏设置结构体
 * 
 * 存储所有可视化的配置选项，包括颜色方案和显示开关。
 * 这些设置可以在运行时通过设置对话框进行修改。
 */
struct GameSettings {
    // 颜色设置
    COLORREF gridColor; ///< 网格线颜色
    COLORREF cellColor; ///< 活细胞颜色
    COLORREF bgColor; ///< 背景颜色
    COLORREF textColor; ///< 文本颜色

    // 显示设置
    bool showGrid; ///< 是否显示网格线
    bool showHUD; ///< 是否显示HUD信息 (如装饰线)
    bool showHistory; ///< 是否显示历史统计图表
    int gridLineWidth; ///< 网格线宽度 (像素)

    /**
     * @brief 默认构造函数
     * 
     * 初始化为默认的深色主题配色方案。
     */
    GameSettings() {
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
 * 
 * 使用单例模式 (Singleton Pattern) 管理全局唯一的游戏设置实例。
 * 确保在程序的任何地方都能访问和修改同一份配置。
 */
class SettingsManager {
public:
    /**
     * @brief 获取单例实例
     * 
     * @return SettingsManager& 全局唯一的管理器引用
     */
    static SettingsManager &GetInstance() {
        static SettingsManager instance;
        return instance;
    }

    /**
     * @brief 获取可修改的设置对象
     */
    GameSettings &GetSettings() { return m_settings; }

    /**
     * @brief 获取只读的设置对象
     */
    const GameSettings &GetSettings() const { return m_settings; }

private:
    /**
     * @brief 私有构造函数
     * 
     * 防止外部直接创建实例，确保单例性。
     */
    SettingsManager() {
    }

    GameSettings m_settings; ///< 内部存储的设置数据
};
