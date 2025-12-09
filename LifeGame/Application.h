#pragma once
#include <windows.h>
#include <memory>
#include "Game.h"
#include "Renderer.h"
#include "UI.h"
#include "SplashWindow.h"

/**
 * @brief 应用程序主类 (Application Main Class)
 * 
 * 负责管理应用程序的生命周期、主窗口创建、消息循环以及各个子系统的协调。
 * 它是整个程序的入口点封装，采用了面向对象的设计来封装 Win32 API 的复杂性。
 */
class Application {
public:
    /**
     * @brief 构造函数
     * 初始化成员变量，设置默认状态。
     */
    Application();

    /**
     * @brief 析构函数
     * 清理资源。
     */
    ~Application();

    /**
     * @brief 运行应用程序 (Run Application)
     * 
     * 这是程序的主入口点逻辑。它负责初始化所有子系统，创建窗口，并进入主消息循环。
     * 
     * @param hInstance 应用程序实例句柄 (由 WinMain 传入)
     * @param nCmdShow 显示命令 (如 SW_SHOW)
     * @param lpCmdLine 命令行参数 (用于自定义初始网格大小等)
     * @return int 退出代码 (返回给操作系统)
     */
    int Run(HINSTANCE hInstance, int nCmdShow, LPSTR lpCmdLine);

private:
    /**
     * @brief 静态窗口过程函数 (Static Window Procedure)
     * 
     * Win32 API 需要一个全局或静态函数作为窗口过程。
     * 此函数的作用是拦截消息，提取窗口关联的 Application 实例指针 (存储在 GWLP_USERDATA)，
     * 然后将消息转发给成员函数 WndProc 进行实际处理。
     * 
     * @param hWnd 窗口句柄
     * @param uMsg 消息标识符
     * @param wParam 消息参数 1
     * @param lParam 消息参数 2
     * @return LRESULT 消息处理结果
     */
    static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief 成员窗口过程函数 (Member Window Procedure)
     * 
     * 处理主窗口的所有消息。这是实际的业务逻辑处理中心。
     * 根据消息类型 (uMsg) 分发到具体的处理函数 (如 OnPaint, OnTimer 等)。
     * 
     * @param hWnd 窗口句柄
     * @param uMsg 消息标识符
     * @param wParam 消息参数 1
     * @param lParam 消息参数 2
     * @return LRESULT 消息处理结果
     */
    LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // ==========================================
    // 消息处理函数 (Message Handlers)
    // ==========================================

    /**
     * @brief 处理 WM_PAINT 消息
     * 负责窗口的绘制。使用双缓冲技术防止闪烁。
     */
    void OnPaint(HWND hWnd);

    /**
     * @brief 处理 WM_TIMER 消息
     * 处理定时器事件，如游戏逻辑更新、提示信息消失等。
     */
    void OnTimer(HWND hWnd, WPARAM timerId);

    /**
     * @brief 处理鼠标消息 (WM_LBUTTONDOWN, WM_MOUSEMOVE 等)
     * 响应用户的点击和拖拽操作，用于绘制/擦除细胞。
     */
    void OnMouse(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief 处理鼠标滚轮消息 (WM_MOUSEWHEEL)
     * 用于缩放视图。
     */
    void OnMouseWheel(HWND hWnd, WPARAM wParam, LPARAM lParam);

    /**
     * @brief 处理键盘按键消息 (WM_KEYDOWN)
     * 响应快捷键，如空格(暂停/开始), R(重置), G(随机生成) 等。
     */
    void OnKeyDown(HWND hWnd, WPARAM key);

    /**
     * @brief 处理 WM_COMMAND 消息
     * 响应 UI 控件的交互，如按钮点击、下拉框选择等。
     */
    void OnCommand(HWND hWnd, int id, int code);

    /**
     * @brief 处理 WM_DRAWITEM 消息
     * 用于自绘控件 (Owner-drawn controls)，如自定义样式的按钮。
     */
    void OnDrawItem(LPARAM lParam);

    /**
     * @brief 处理 WM_SIZE 消息
     * 当窗口大小改变时调用，用于重新布局 UI 控件。
     */
    void OnSize(HWND hWnd, int width, int height);

    /**
     * @brief 处理 WM_DESTROY 消息
     * 窗口销毁时的清理工作，如停止定时器、发送退出消息。
     */
    void OnDestroy(HWND hWnd);

    // ==========================================
    // 辅助函数 (Helper Functions)
    // ==========================================

    /**
     * @brief 解析命令行参数
     * 允许用户通过命令行指定初始网格大小 (例如: -w 200 -h 150)。
     */
    void ParseCommandLine(LPSTR lpCmdLine, int &w, int &h);

    /**
     * @brief 计算初始窗口大小
     * 根据期望的客户区大小计算实际的窗口边框大小。
     */
    RECT CalcInitialWindowRect();

    /**
     * @brief 重启游戏定时器
     * 当速度改变或暂停/恢复时调用，以更新定时器频率。
     */
    void RestartTimer(HWND hWnd);

    // ==========================================
    // 成员变量 (Member Variables)
    // ==========================================

    std::unique_ptr<LifeGame> m_game; ///< 游戏核心逻辑对象 (Model)
    std::unique_ptr<Renderer> m_renderer; ///< 渲染器对象 (View)
    std::unique_ptr<UI> m_ui; ///< 用户界面控制器对象 (Controller)

    bool m_showResetTip; ///< 标志位：是否正在显示"已重置"的提示信息
    UINT_PTR m_timerId; ///< 游戏主循环定时器 ID (用于控制演化速度)
    UINT_PTR m_tipTimerId; ///< 提示信息自动消失定时器 ID
    int m_clientWidth; ///< 当前窗口客户区的宽度
    int m_clientHeight; ///< 当前窗口客户区的高度
};
