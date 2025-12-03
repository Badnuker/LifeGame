#pragma once
#include <windows.h>
#include "Settings.h"

/**
 * @brief 设置对话框类
 * 
 * 提供一个模态对话框，允许用户修改游戏的外观设置。
 * 包含颜色选择器和复选框。
 * 
 * 实现细节：
 * - 使用 Win32 API 创建一个模态窗口。
 * - 使用自定义消息循环来处理模态行为。
 * - 使用 ChooseColor 对话框选择颜色。
 */
class SettingsDialog
{
public:
	SettingsDialog();
	~SettingsDialog();

	/**
	 * @brief 显示设置对话框
	 * 
	 * 这是一个阻塞调用，直到用户关闭对话框。
	 * 
	 * @param hParent 父窗口句柄
	 * @return true 如果用户点击了"确定"
	 * @return false 如果用户点击了"取消"
	 */
	bool Show(HWND hParent);

private:
	/**
	 * @brief 静态窗口过程函数
	 * 
	 * 将消息转发给实例的成员函数处理。
	 */
	static LRESULT CALLBACK DialogProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

	/**
	 * @brief 初始化控件
	 * 
	 * 创建对话框中的所有子控件。
	 */
	void InitializeControls(HWND hWnd);

	/**
	 * @brief 处理 WM_COMMAND 消息
	 */
	void OnCommand(HWND hWnd, int id, int code);

	/**
	 * @brief 处理 WM_PAINT 消息
	 * 
	 * 绘制颜色预览块。
	 */
	void OnPaint(HWND hWnd);

	/**
	 * @brief 处理颜色选择按钮点击
	 */
	void OnColorButton(HWND hWnd, int id);

	// 临时存储设置，直到用户点击确定
	GameSettings m_tempSettings;

	// 控件ID常量
	static constexpr int ID_OK = 1;
	static constexpr int ID_CANCEL = 2;
	static constexpr int ID_COLOR_CELL = 101;
	static constexpr int ID_COLOR_BG = 102;
	static constexpr int ID_COLOR_GRID = 103;
	static constexpr int ID_CHECK_GRID = 104;
	static constexpr int ID_CHECK_HUD = 105;
	static constexpr int ID_CHECK_HISTORY = 106;

	HWND m_hDialog; ///< 对话框窗口句柄
};
