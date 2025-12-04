#pragma once
#include <windows.h>
#include <vector>
#include <string>

/**
 * @brief 帮助页面结构体
 * 
 * 存储单个帮助页面的标题和内容。
 */
struct HelpPage
{
	std::wstring title; ///< 页面标题 (显示在左侧列表)
	std::wstring content; ///< 页面正文内容 (显示在右侧区域)
};

/**
 * @brief 帮助窗口类
 * 
 * 显示应用程序的使用说明文档。
 * 包含左侧的导航列表和右侧的内容显示区域。
 * 支持字体缩放功能，方便阅读。
 */
class HelpWindow
{
public:
	HelpWindow();
	~HelpWindow();

	/**
	 * @brief 显示帮助窗口
	 * 
	 * 创建并显示模态或非模态的帮助窗口。
	 * @param hParent 父窗口句柄
	 */
	void Show(HWND hParent);

private:
	/**
	 * @brief 窗口过程函数
	 * 
	 * 处理帮助窗口的消息循环。
	 */
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

	// 消息处理函数
	void OnCreate(HWND hWnd); ///< 处理窗口创建消息 (WM_CREATE)
	void OnPaint(HWND hWnd); ///< 处理绘图消息 (WM_PAINT)
	void OnSize(HWND hWnd); ///< 处理窗口大小改变消息 (WM_SIZE)
	void OnCommand(HWND hWnd, int id, int code); ///< 处理命令消息 (WM_COMMAND)

	/**
	 * @brief 绘制当前页面内容
	 * 
	 * @param hdc 设备上下文句柄
	 * @param rect 绘制区域
	 */
	void DrawPage(HDC hdc, const RECT& rect);

	/**
	 * @brief 更新字体
	 * 
	 * 根据当前的缩放比例 (m_fontScale) 重新创建标题和正文字体。
	 */
	void UpdateFonts(); // 更新字体

	HWND m_hWnd; ///< 帮助窗口句柄
	HWND m_hList; ///< 左侧导航列表框句柄
	HWND m_hZoomInBtn; ///< 放大按钮句柄
	HWND m_hZoomOutBtn; ///< 缩小按钮句柄

	int m_currentPage; ///< 当前选中的页面索引
	float m_fontScale; ///< 字体缩放比例 (默认 1.0)

	std::vector<HelpPage> m_pages; ///< 帮助页面数据集合

	HFONT m_hTitleFont; ///< 标题字体句柄
	HFONT m_hBodyFont; ///< 正文字体句柄
};
