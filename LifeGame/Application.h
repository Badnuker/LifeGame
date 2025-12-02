#pragma once
#include <windows.h>
#include <memory>
#include "Game.h"
#include "Renderer.h"
#include "UI.h"

/**
 * @brief 应用程序主类
 * 
 * 负责管理应用程序的生命周期、主窗口创建、消息循环以及各个子系统的协调。
 * 它是整个程序的入口点封装。
 */
class Application
{
public:
	/**
	 * @brief 构造函数
	 */
	Application();

	/**
	 * @brief 析构函数
	 */
	~Application();

	/**
	 * @brief 运行应用程序
	 * 
	 * @param hInstance 应用程序实例句柄
	 * @param nCmdShow 显示命令
	 * @param lpCmdLine 命令行参数
	 * @return int 退出代码
	 */
	int Run(HINSTANCE hInstance, int nCmdShow, LPSTR lpCmdLine);

private:
	/**
	 * @brief 静态窗口过程函数
	 * 
	 * 用于将 Win32 消息转发给类的成员函数 WndProc。
	 */
	static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	/**
	 * @brief 成员窗口过程函数
	 * 
	 * 处理主窗口的所有消息。
	 */
	LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// ==========================================
	// 消息处理函数 (Message Handlers)
	// ==========================================

	void OnPaint(HWND hWnd);
	void OnTimer(HWND hWnd, WPARAM timerId);
	void OnMouse(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnKeyDown(HWND hWnd, WPARAM key);
	void OnCommand(HWND hWnd, int id, int code);
	void OnDrawItem(LPARAM lParam);
	void OnSize(HWND hWnd, int width, int height);
	void OnDestroy(HWND hWnd);

	// ==========================================
	// 辅助函数 (Helper Functions)
	// ==========================================

	void ParseCommandLine(LPSTR lpCmdLine, int& w, int& h);
	RECT CalcInitialWindowRect();
	void RestartTimer(HWND hWnd);

	// ==========================================
	// 成员变量 (Member Variables)
	// ==========================================

	std::unique_ptr<LifeGame> m_game; ///< 游戏核心逻辑
	std::unique_ptr<Renderer> m_renderer; ///< 渲染器
	std::unique_ptr<UI> m_ui; ///< 用户界面控制器

	bool m_showResetTip; ///< 是否显示重置提示
	UINT_PTR m_timerId; ///< 游戏循环定时器ID
	UINT_PTR m_tipTimerId; ///< 提示信息定时器ID
	int m_clientWidth; ///< 客户区宽度
	int m_clientHeight; ///< 客户区高度
};
