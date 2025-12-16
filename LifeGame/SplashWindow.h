#pragma once
#include <windows.h>
#include <vector>
#include <string>

/**
 * @brief 启动/关闭动画窗口类 (Splash Window)
 *
 * 负责显示应用程序启动时的欢迎动画和关闭时的告别动画。
 * 实现了基于 GDI 的星空粒子效果，增强程序的视觉体验和科技感。
 *
 * 主要功能：
 * 1. 渲染动态星空背景。
 * 2. 显示带有淡入淡出效果的标题文字。
 * 3. 管理动画生命周期（自动关闭）。
 */
class SplashWindow {
public:
    /**
     * @brief 构造函数
     * 初始化成员变量。
     */
    SplashWindow();

    /**
     * @brief 析构函数
     * 清理 GDI 资源。
     */
    ~SplashWindow();

    /**
     * @brief 显示动画窗口
     *
     * 创建并显示全屏或居中的无边框窗口，开始播放动画。
     *
     * @param hInstance 应用程序实例句柄
     * @param mode 动画模式：0 = 启动 (Startup), 1 = 关闭 (Shutdown)
     */
    void Show(HINSTANCE hInstance, int mode);

    /**
     * @brief 运行消息循环
     *
     * 进入独立的模态消息循环，直到动画播放完毕。
     * 这确保了动画播放期间主程序逻辑暂停（或尚未开始）。
     */
    void RunLoop();

private:
    /**
     * @brief 静态窗口过程函数
     * 转发消息到成员函数 WndProc。
     */
    static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief 成员窗口过程函数
     * 处理窗口消息，如定时器、绘图等。
     */
    LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief 处理绘图消息
     * 使用双缓冲技术绘制星空和文字。
     */
    void OnPaint(HWND hWnd);

    /**
     * @brief 处理定时器消息
     * 更新动画状态（星星位置、透明度等）并触发重绘。
     */
    void OnTimer(HWND hWnd);

    // ==========================================
    // 星空效果 (Starfield Effect)
    // ==========================================

    /**
     * @brief 初始化星空
     * 生成随机分布的星星数据。
     * @param width 窗口宽度
     * @param height 窗口高度
     */
    void InitStars(int width, int height);

    /**
     * @brief 更新星星位置
     * 计算星星的 3D 投影运动，模拟飞行效果。
     */
    void UpdateStars();

    /**
     * @brief 绘制星空
     * @param hdc 设备上下文
     * @param width 窗口宽度
     * @param height 窗口高度
     */
    void DrawStars(HDC hdc, int width, int height);

    HWND m_hWnd; ///< 窗口句柄
    int m_mode; ///< 当前模式：0: Startup, 1: Shutdown
    int m_width; ///< 窗口宽度
    int m_height; ///< 窗口高度

    // 动画状态
    int m_frame; ///< 当前帧数
    int m_maxFrames; ///< 总帧数
    float m_alpha; ///< 全局透明度 (0.0 - 1.0)，用于淡入淡出

    /**
     * @brief 星星结构体
     * 定义单个星星的属性。
     */
    struct Star {
        float x, y, z; ///< 3D 坐标
        float speed; ///< 移动速度
        int size; ///< 绘制大小
        COLORREF color; ///< 颜色
    };

    std::vector<Star> m_stars; ///< 星星集合

    HFONT m_hTitleFont; ///< 标题字体
    HFONT m_hSubFont; ///< 副标题字体
};