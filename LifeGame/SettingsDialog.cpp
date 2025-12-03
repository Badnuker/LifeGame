#include "SettingsDialog.h"
#include <commdlg.h> // For ChooseColor

// 全局指针，用于在静态回调函数中访问实例成员
SettingsDialog* g_pSettingsDialog = nullptr;

SettingsDialog::SettingsDialog() : m_hDialog(nullptr)
{
}

SettingsDialog::~SettingsDialog()
{
}

/**
 * @brief 显示模态设置对话框
 * 
 * 创建并显示一个包含设置选项的窗口。
 * 
 * @param hParent 父窗口句柄
 * @return true 用户点击了"确定"并保存了设置
 * @return false 用户点击了"取消"或关闭了窗口
 */
bool SettingsDialog::Show(HWND hParent)
{
	g_pSettingsDialog = this;

	// 获取当前设置的副本，用于临时修改
	m_tempSettings = SettingsManager::GetInstance().GetSettings();

	// 注册窗口类 (如果是一个自定义窗口模拟对话框)
	// 为了简单，我们使用 CreateDialogIndirect 或者直接创建一个弹出窗口
	// 这里我们创建一个模态弹出窗口

	WNDCLASS wc = {0};
	wc.lpfnWndProc = DialogProc;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.lpszClassName = TEXT("LifeGameSettingsDlg");
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	RegisterClass(&wc);

	// 计算窗口居中位置
	int width = 400;
	int height = 350;
	int x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
	int y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

	HWND hWnd = CreateWindowEx(WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
	                           TEXT("LifeGameSettingsDlg"), TEXT("外观设置"),
	                           WS_VISIBLE | WS_POPUP | WS_CAPTION | WS_SYSMENU,
	                           x, y, width, height,
	                           hParent, nullptr, wc.hInstance, this);

	if (!hWnd) return false;

	m_hDialog = hWnd;
	EnableWindow(hParent, FALSE); // 禁用父窗口，模拟模态对话框行为

	// 消息循环
	MSG msg;
	bool result = false;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (msg.message == WM_NULL) // 自定义退出消息
		{
			result = (msg.wParam == ID_OK);
			break;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// 恢复父窗口
	EnableWindow(hParent, TRUE);
	SetFocus(hParent);
	DestroyWindow(hWnd);

	// 如果用户点击确定，保存设置
	if (result)
	{
		SettingsManager::GetInstance().GetSettings() = m_tempSettings;
	}

	return result;
}

/**
 * @brief 窗口过程回调函数
 * 
 * 处理对话框的消息。
 */
LRESULT CALLBACK SettingsDialog::DialogProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	SettingsDialog* pThis = g_pSettingsDialog;

	switch (msg)
	{
	case WM_CREATE:
		if (pThis) pThis->InitializeControls(hWnd);
		return 0;
	case WM_COMMAND:
		if (pThis) pThis->OnCommand(hWnd, LOWORD(wp), HIWORD(wp));
		return 0;
	case WM_PAINT:
		if (pThis) pThis->OnPaint(hWnd);
		return 0;
	case WM_CLOSE:
		PostMessage(nullptr, WM_NULL, ID_CANCEL, 0); // 发送退出消息
		return 0;
	}
	return DefWindowProc(hWnd, msg, wp, lp);
}

/**
 * @brief 初始化控件
 * 
 * 创建标签、按钮和复选框。
 */
void SettingsDialog::InitializeControls(HWND hWnd)
{
	int x = 20, y = 20;
	int h = 25;
	int w = 150;

	// 颜色设置区域
	CreateWindow(TEXT("STATIC"), TEXT("活细胞颜色:"), WS_CHILD | WS_VISIBLE, x, y, w, h, hWnd, nullptr, nullptr, nullptr);
	CreateWindow(TEXT("BUTTON"), TEXT("选择..."), WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, x + 160, y, 80, h, hWnd,
	             (HMENU)ID_COLOR_CELL, nullptr, nullptr);
	y += 40;

	CreateWindow(TEXT("STATIC"), TEXT("背景颜色:"), WS_CHILD | WS_VISIBLE, x, y, w, h, hWnd, nullptr, nullptr, nullptr);
	CreateWindow(TEXT("BUTTON"), TEXT("选择..."), WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, x + 160, y, 80, h, hWnd,
	             (HMENU)ID_COLOR_BG, nullptr, nullptr);
	y += 40;

	CreateWindow(TEXT("STATIC"), TEXT("网格线颜色:"), WS_CHILD | WS_VISIBLE, x, y, w, h, hWnd, nullptr, nullptr, nullptr);
	CreateWindow(TEXT("BUTTON"), TEXT("选择..."), WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, x + 160, y, 80, h, hWnd,
	             (HMENU)ID_COLOR_GRID, nullptr, nullptr);
	y += 40;

	// 开关设置区域
	CreateWindow(TEXT("BUTTON"), TEXT("显示网格线"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, x, y, 200, h, hWnd,
	             (HMENU)ID_CHECK_GRID, nullptr, nullptr);
	if (m_tempSettings.showGrid) CheckDlgButton(hWnd, ID_CHECK_GRID, BST_CHECKED);
	y += 30;

	CreateWindow(TEXT("BUTTON"), TEXT("显示 HUD 信息"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, x, y, 200, h, hWnd,
	             (HMENU)ID_CHECK_HUD, nullptr, nullptr);
	if (m_tempSettings.showHUD) CheckDlgButton(hWnd, ID_CHECK_HUD, BST_CHECKED);
	y += 30;

	CreateWindow(TEXT("BUTTON"), TEXT("显示历史统计图"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, x, y, 200, h, hWnd,
	             (HMENU)ID_CHECK_HISTORY, nullptr, nullptr);
	if (m_tempSettings.showHistory) CheckDlgButton(hWnd, ID_CHECK_HISTORY, BST_CHECKED);
	y += 50;

	// 底部按钮区域
	CreateWindow(TEXT("BUTTON"), TEXT("确定"), WS_CHILD | WS_VISIBLE, 80, y, 100, 30, hWnd, (HMENU)ID_OK, nullptr,
	             nullptr);
	CreateWindow(TEXT("BUTTON"), TEXT("取消"), WS_CHILD | WS_VISIBLE, 200, y, 100, 30, hWnd, (HMENU)ID_CANCEL, nullptr,
	             nullptr);
}

/**
 * @brief 处理命令消息
 * 
 * 处理按钮点击和复选框状态改变。
 */
void SettingsDialog::OnCommand(HWND hWnd, int id, int code)
{
	switch (id)
	{
	case ID_OK:
		// 读取 Checkbox 状态并更新临时设置
		m_tempSettings.showGrid = (IsDlgButtonChecked(hWnd, ID_CHECK_GRID) == BST_CHECKED);
		m_tempSettings.showHUD = (IsDlgButtonChecked(hWnd, ID_CHECK_HUD) == BST_CHECKED);
		m_tempSettings.showHistory = (IsDlgButtonChecked(hWnd, ID_CHECK_HISTORY) == BST_CHECKED);
		PostMessage(nullptr, WM_NULL, ID_OK, 0);
		break;
	case ID_CANCEL:
		PostMessage(nullptr, WM_NULL, ID_CANCEL, 0);
		break;
	case ID_COLOR_CELL:
	case ID_COLOR_BG:
	case ID_COLOR_GRID:
		OnColorButton(hWnd, id);
		break;
	}
}

/**
 * @brief 处理颜色选择按钮点击
 * 
 * 弹出系统颜色选择对话框。
 */
void SettingsDialog::OnColorButton(HWND hWnd, int id)
{
	CHOOSECOLOR cc;
	static COLORREF acrCustClr[16]; // 自定义颜色数组
	ZeroMemory(&cc, sizeof(cc));
	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = hWnd;
	cc.lpCustColors = static_cast<LPDWORD>(acrCustClr);
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;

	// 设置初始颜色
	if (id == ID_COLOR_CELL) cc.rgbResult = m_tempSettings.cellColor;
	else if (id == ID_COLOR_BG) cc.rgbResult = m_tempSettings.bgColor;
	else if (id == ID_COLOR_GRID) cc.rgbResult = m_tempSettings.gridColor;

	if (ChooseColor(&cc))
	{
		// 保存选择的颜色
		if (id == ID_COLOR_CELL) m_tempSettings.cellColor = cc.rgbResult;
		else if (id == ID_COLOR_BG) m_tempSettings.bgColor = cc.rgbResult;
		else if (id == ID_COLOR_GRID) m_tempSettings.gridColor = cc.rgbResult;

		InvalidateRect(hWnd, nullptr, TRUE); // 重绘窗口以更新颜色预览
	}
}

/**
 * @brief 绘制窗口
 * 
 * 主要用于绘制颜色选择按钮旁边的颜色预览块。
 */
void SettingsDialog::OnPaint(HWND hWnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);

	// 绘制颜色预览块的 Lambda 函数
	auto DrawPreview = [&](int id, COLORREF color)
	{
		HWND hBtn = GetDlgItem(hWnd, id);
		if (hBtn)
		{
			RECT rc;
			GetWindowRect(hBtn, &rc);
			POINT pt = {rc.left, rc.top};
			ScreenToClient(hWnd, &pt);

			// 在按钮旁边画一个色块
			RECT rcColor = {pt.x + 90, pt.y + 2, pt.x + 120, pt.y + 23};
			HBRUSH hBr = CreateSolidBrush(color);
			FillRect(hdc, &rcColor, hBr);
			FrameRect(hdc, &rcColor, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
			DeleteObject(hBr);
		}
	};

	DrawPreview(ID_COLOR_CELL, m_tempSettings.cellColor);
	DrawPreview(ID_COLOR_BG, m_tempSettings.bgColor);
	DrawPreview(ID_COLOR_GRID, m_tempSettings.gridColor);

	EndPaint(hWnd, &ps);
}
