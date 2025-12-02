#pragma once
#include <windows.h>
#include "Settings.h"

/**
 * @brief 设置对话框类
 * 
 * 提供一个模态对话框，允许用户修改游戏的外观设置。
 * 包含颜色选择器和复选框。
 */
class SettingsDialog
{
public:
	SettingsDialog();
	~SettingsDialog();

	/**
	 * @brief 显示设置对话框
	 * 
	 * @param hParent 父窗口句柄
	 * @return true 如果用户点击了"确定"
	 * @return false 如果用户点击了"取消"
	 */
	bool Show(HWND hParent);

private:
	static LRESULT CALLBACK DialogProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

	void InitializeControls(HWND hWnd);
	void OnCommand(HWND hWnd, int id, int code);
	void OnPaint(HWND hWnd);
	void OnColorButton(HWND hWnd, int id);

	// 临时存储设置，直到用户点击确定
	GameSettings m_tempSettings;

	// 控件ID
	static constexpr int ID_OK = 1;
	static constexpr int ID_CANCEL = 2;
	static constexpr int ID_COLOR_CELL = 101;
	static constexpr int ID_COLOR_BG = 102;
	static constexpr int ID_COLOR_GRID = 103;
	static constexpr int ID_CHECK_GRID = 104;
	static constexpr int ID_CHECK_HUD = 105;
	static constexpr int ID_CHECK_HISTORY = 106;

	HWND m_hDialog;
};
