#include "UI.h"
#include "Resource.h"
#include "SettingsDialog.h"
#include "SetCellCommand.h"
#include "PlacePatternCommand.h"
#include <tchar.h>
#include <stdio.h>
#include <commdlg.h> // 新增：文件对话框

UI* UI::s_pInstance = nullptr;

UI::UI()
	: m_hRowsEdit(nullptr), m_hColsEdit(nullptr), m_hApplyBtn(nullptr),
	  m_hRowsLabel(nullptr), m_hColsLabel(nullptr), m_hPatternLabel(nullptr), m_hPatternCombo(nullptr),
	  m_hRuleLabel(nullptr), m_hRuleCombo(nullptr), m_hToolTip(nullptr),
	  m_oldRowsProc(nullptr), m_oldColsProc(nullptr), m_oldApplyBtnProc(nullptr),
	  m_isDragging(false), m_isRightDragging(false), m_dragValue(true),
	  m_applyHover(false)
{
	s_pInstance = this;
}

UI::~UI()
{
	Cleanup();
	if (s_pInstance == this) s_pInstance = nullptr;
}

bool UI::Initialize(HINSTANCE hInstance, HWND hParent, LifeGame& game)
{
	int leftX = 16, leftY = 20, labelW = 200, editW = 220, editH = 28, gapY = 16; // 增加控件宽度

	// 1. 笔刷选择
	m_hPatternLabel = CreateWindowEx(0, TEXT("STATIC"), TEXT("笔刷模式"),
	                                 WS_CHILD | WS_VISIBLE | SS_LEFT,
	                                 leftX, leftY, labelW, editH, hParent,
	                                 nullptr, hInstance, nullptr);
	leftY += 28; // 增加间距
	m_hPatternCombo = CreateWindowEx(0, TEXT("COMBOBOX"), nullptr,
	                                 WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
	                                 leftX, leftY, editW, 300, hParent, // 增加下拉高度
	                                 (HMENU)ID_PATTERN_COMBO, hInstance, nullptr);

	// 添加选项
	const auto& patterns = game.GetPatternLibrary().GetPatterns();
	for (const auto& p : patterns)
	{
		SendMessage(m_hPatternCombo, CB_ADDSTRING, 0, (LPARAM)p.name.c_str());
	}
	SendMessage(m_hPatternCombo, CB_SETCURSEL, 0, 0);

	leftY += editH + gapY;

	// 1.5 预览窗口 (新增)
	m_preview.Initialize(hInstance, hParent, leftX, leftY, editW, 140); // 预览窗口也变大
	// 设置初始预览
	const auto* p = game.GetPatternLibrary().GetPattern(0);
	m_preview.SetPattern(p);

	leftY += 140 + gapY;

	// 2. 规则选择
	m_hRuleLabel = CreateWindowEx(0, TEXT("STATIC"), TEXT("演化规则"),
	                              WS_CHILD | WS_VISIBLE | SS_LEFT,
	                              leftX, leftY, labelW, editH, hParent,
	                              nullptr, hInstance, nullptr);
	leftY += 28;
	m_hRuleCombo = CreateWindowEx(0, TEXT("COMBOBOX"), nullptr,
	                              WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
	                              leftX, leftY, editW, 300, hParent,
	                              (HMENU)ID_RULE_COMBO, hInstance, nullptr);

	// 添加规则选项
	const auto& rules = game.GetRuleEngine().GetRules();
	for (const auto& r : rules)
	{
		SendMessage(m_hRuleCombo, CB_ADDSTRING, 0, (LPARAM)r.name.c_str());
	}
	SendMessage(m_hRuleCombo, CB_SETCURSEL, 0, 0);

	leftY += editH + gapY;

	// 2.5 尺寸选择 (新增)
	m_hSizeLabel = CreateWindowEx(0, TEXT("STATIC"), TEXT("网格尺寸"),
	                              WS_CHILD | WS_VISIBLE | SS_LEFT,
	                              leftX, leftY, labelW, editH, hParent,
	                              nullptr, hInstance, nullptr);
	leftY += 28;
	m_hSizeCombo = CreateWindowEx(0, TEXT("COMBOBOX"), nullptr,
	                              WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
	                              leftX, leftY, editW, 300, hParent,
	                              (HMENU)ID_SIZE_COMBO, hInstance, nullptr);

	// 添加尺寸选项
	SendMessage(m_hSizeCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("大 (30x40)"));
	SendMessage(m_hSizeCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("中 (120x160)"));
	SendMessage(m_hSizeCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("小 (300x400)"));
	SendMessage(m_hSizeCombo, CB_SETCURSEL, 1, 0); // 默认选中"中"

	leftY += editH + gapY + 10;

	// 3. 网格设置
	m_hColsLabel = CreateWindowEx(0, TEXT("STATIC"), TEXT("网格列数 (W)"),
	                              WS_CHILD | WS_VISIBLE | SS_LEFT,
	                              leftX, leftY, labelW, editH, hParent,
	                              nullptr, hInstance, nullptr);
	leftY += 28;
	m_hColsEdit = CreateWindowEx(0, TEXT("EDIT"), nullptr,
	                             WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOVSCROLL | WS_TABSTOP,
	                             leftX, leftY, editW, editH, hParent,
	                             (HMENU)ID_COLS_EDIT, hInstance, nullptr);

	leftY += editH + gapY;
	m_hRowsLabel = CreateWindowEx(0, TEXT("STATIC"), TEXT("网格行数 (H)"),
	                              WS_CHILD | WS_VISIBLE | SS_LEFT,
	                              leftX, leftY, labelW, editH, hParent,
	                              nullptr, hInstance, nullptr);
	leftY += 28;
	m_hRowsEdit = CreateWindowEx(0, TEXT("EDIT"), nullptr,
	                             WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOVSCROLL | WS_TABSTOP,
	                             leftX, leftY, editW, editH, hParent,
	                             (HMENU)ID_ROWS_EDIT, hInstance, nullptr);

	leftY += editH + gapY + 10;
	m_hApplyBtn = CreateWindowEx(0, TEXT("BUTTON"), TEXT("应用设置"),
	                             WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | WS_TABSTOP,
	                             leftX, leftY, editW, 40, hParent, // 按钮高度增加
	                             (HMENU)ID_APPLY_BTN, hInstance, nullptr);

	leftY += 40 + gapY;

	// 4. 文件操作 (新增)
	m_hSaveBtn = CreateWindowEx(0, TEXT("BUTTON"), TEXT("保存存档"),
	                            WS_CHILD | WS_VISIBLE | WS_TABSTOP,
	                            leftX, leftY, 105, 30, hParent,
	                            (HMENU)ID_SAVE_BTN, hInstance, nullptr);

	m_hLoadBtn = CreateWindowEx(0, TEXT("BUTTON"), TEXT("加载存档"),
	                            WS_CHILD | WS_VISIBLE | WS_TABSTOP,
	                            leftX + 115, leftY, 105, 30, hParent,
	                            (HMENU)ID_LOAD_BTN, hInstance, nullptr);

	leftY += 30 + 10;
	m_hExportBtn = CreateWindowEx(0, TEXT("BUTTON"), TEXT("导出 RLE"),
	                              WS_CHILD | WS_VISIBLE | WS_TABSTOP,
	                              leftX, leftY, editW, 30, hParent,
	                              (HMENU)ID_EXPORT_BTN, hInstance, nullptr);

	leftY += 30 + gapY;
	m_hSettingsBtn = CreateWindowEx(0, TEXT("BUTTON"), TEXT("外观设置"),
	                                WS_CHILD | WS_VISIBLE | WS_TABSTOP,
	                                leftX, leftY, editW, 30, hParent,
	                                (HMENU)ID_SETTINGS_BTN, hInstance, nullptr);

	leftY += 30 + gapY;
	m_hHelpBtn = CreateWindowEx(0, TEXT("BUTTON"), TEXT("使用手册"),
	                            WS_CHILD | WS_VISIBLE | WS_TABSTOP,
	                            leftX, leftY, editW, 30, hParent,
	                            (HMENU)ID_HELP_BTN, hInstance, nullptr);

	leftY += 30 + gapY;
	m_hUndoBtn = CreateWindowEx(0, TEXT("BUTTON"), TEXT("撤销操作 (Undo)"),
	                            WS_CHILD | WS_VISIBLE | WS_TABSTOP,
	                            leftX, leftY, editW, 30, hParent,
	                            (HMENU)ID_UNDO_BTN, hInstance, nullptr);

	// 设置初始值
	TCHAR tmpbuf[32];
	_stprintf_s(tmpbuf, TEXT("%d"), game.GetHeight());
	SetWindowText(m_hRowsEdit, tmpbuf);
	_stprintf_s(tmpbuf, TEXT("%d"), game.GetWidth());
	SetWindowText(m_hColsEdit, tmpbuf);

	// 子类化控件以处理特殊事件（如回车键）
	m_oldRowsProc = (WNDPROC)SetWindowLongPtr(m_hRowsEdit, GWLP_WNDPROC, (LONG_PTR)RowsEditProc);
	m_oldColsProc = (WNDPROC)SetWindowLongPtr(m_hColsEdit, GWLP_WNDPROC, (LONG_PTR)ColsEditProc);
	m_oldApplyBtnProc = (WNDPROC)SetWindowLongPtr(m_hApplyBtn, GWLP_WNDPROC, (LONG_PTR)ApplyBtnProc);

	// 创建 Tooltip
	INITCOMMONCONTROLSEX icex = {sizeof(icex), ICC_WIN95_CLASSES};
	InitCommonControlsEx(&icex);
	m_hToolTip = CreateWindowEx(0, TOOLTIPS_CLASS, nullptr, WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX,
	                            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
	                            hParent, nullptr, hInstance, nullptr);
	if (m_hToolTip)
	{
		TOOLINFO ti = {0};
		ti.cbSize = sizeof(ti);
		ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
		ti.hwnd = hParent;
		ti.uId = (UINT_PTR)m_hApplyBtn;
		ti.lpszText = (LPTSTR)TEXT("应用行/列设置 (Enter)");
		SendMessage(m_hToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
	}

	return (m_hRowsEdit && m_hColsEdit && m_hApplyBtn);
}

void UI::Cleanup()
{
	if (m_hRowsEdit && m_oldRowsProc)
		SetWindowLongPtr(m_hRowsEdit, GWLP_WNDPROC, (LONG_PTR)m_oldRowsProc);
	if (m_hColsEdit && m_oldColsProc)
		SetWindowLongPtr(m_hColsEdit, GWLP_WNDPROC, (LONG_PTR)m_oldColsProc);
	if (m_hApplyBtn && m_oldApplyBtnProc)
		SetWindowLongPtr(m_hApplyBtn, GWLP_WNDPROC, (LONG_PTR)m_oldApplyBtnProc);
	if (m_hToolTip) DestroyWindow(m_hToolTip);
}

void UI::SetAllFonts(HFONT hFont)
{
	if (m_hPatternLabel)
		SendMessage(m_hPatternLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
	if (m_hPatternCombo)
		SendMessage(m_hPatternCombo, WM_SETFONT, (WPARAM)hFont, TRUE);
	if (m_hRuleLabel)
		SendMessage(m_hRuleLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
	if (m_hRuleCombo)
		SendMessage(m_hRuleCombo, WM_SETFONT, (WPARAM)hFont, TRUE);
	if (m_hSizeLabel)
		SendMessage(m_hSizeLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
	if (m_hSizeCombo)
		SendMessage(m_hSizeCombo, WM_SETFONT, (WPARAM)hFont, TRUE);
	if (m_hColsLabel)
		SendMessage(m_hColsLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
	if (m_hColsEdit)
		SendMessage(m_hColsEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
	if (m_hRowsLabel)
		SendMessage(m_hRowsLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
	if (m_hRowsEdit)
		SendMessage(m_hRowsEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
	if (m_hApplyBtn)
		SendMessage(m_hApplyBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
	if (m_hSaveBtn)
		SendMessage(m_hSaveBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
	if (m_hLoadBtn)
		SendMessage(m_hLoadBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
	if (m_hExportBtn)
		SendMessage(m_hExportBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
	if (m_hSettingsBtn)
		SendMessage(m_hSettingsBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
	if (m_hHelpBtn)
		SendMessage(m_hHelpBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
	if (m_hUndoBtn)
		SendMessage(m_hUndoBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
}

void UI::LayoutControls(int clientWidth, int clientHeight)
{
	if (!m_hRowsEdit || !m_hColsEdit || !m_hApplyBtn) return;

	int leftX = 16;
	int leftY = 20;
	int labelW = 200;
	int editW = 220;
	int editH = 28; // 增加高度
	int gapY = 16;

	// 1. 笔刷
	SetWindowPos(m_hPatternLabel, nullptr, leftX, leftY, labelW, editH, SWP_NOZORDER);
	leftY += 28;
	SetWindowPos(m_hPatternCombo, nullptr, leftX, leftY, editW, editH, SWP_NOZORDER);

	leftY += editH + gapY;

	// 1.5 预览
	m_preview.Move(leftX, leftY, editW, 140);
	leftY += 140 + gapY;

	// 2. 规则
	SetWindowPos(m_hRuleLabel, nullptr, leftX, leftY, labelW, editH, SWP_NOZORDER);
	leftY += 28;
	SetWindowPos(m_hRuleCombo, nullptr, leftX, leftY, editW, editH, SWP_NOZORDER);

	leftY += editH + gapY;

	// 2.5 尺寸
	SetWindowPos(m_hSizeLabel, nullptr, leftX, leftY, labelW, editH, SWP_NOZORDER);
	leftY += 28;
	SetWindowPos(m_hSizeCombo, nullptr, leftX, leftY, editW, editH, SWP_NOZORDER);

	leftY += editH + gapY + 10;

	// 3. 网格
	SetWindowPos(m_hColsLabel, nullptr, leftX, leftY, labelW, editH, SWP_NOZORDER);
	leftY += 28;
	SetWindowPos(m_hColsEdit, nullptr, leftX, leftY, editW, editH, SWP_NOZORDER);

	leftY += editH + gapY;

	SetWindowPos(m_hRowsLabel, nullptr, leftX, leftY, labelW, editH, SWP_NOZORDER);
	leftY += 28;
	SetWindowPos(m_hRowsEdit, nullptr, leftX, leftY, editW, editH, SWP_NOZORDER);

	leftY += editH + gapY + 10;
	SetWindowPos(m_hApplyBtn, nullptr, leftX, leftY, editW, 40, SWP_NOZORDER);

	leftY += 40 + gapY;
	SetWindowPos(m_hSaveBtn, nullptr, leftX, leftY, 105, 30, SWP_NOZORDER);
	SetWindowPos(m_hLoadBtn, nullptr, leftX + 115, leftY, 105, 30, SWP_NOZORDER);

	leftY += 30 + 10;
	SetWindowPos(m_hExportBtn, nullptr, leftX, leftY, editW, 30, SWP_NOZORDER);

	leftY += 30 + gapY;
	SetWindowPos(m_hSettingsBtn, nullptr, leftX, leftY, editW, 30, SWP_NOZORDER);

	leftY += 30 + gapY;
	SetWindowPos(m_hHelpBtn, nullptr, leftX, leftY, editW, 30, SWP_NOZORDER);

	leftY += 30 + gapY;
	SetWindowPos(m_hUndoBtn, nullptr, leftX, leftY, editW, 30, SWP_NOZORDER);
}

void UI::UpdateWindowTitle(HWND hWnd, const LifeGame& game)
{
	TCHAR title[200];
	_stprintf_s(title, TEXT("LifeGame (Win32 GDI) - %s | 速度：%d ms/帧"),
	            game.IsRunning() ? TEXT("运行中") : TEXT("已暂停"),
	            game.GetSpeed());
	SetWindowText(hWnd, title);
}

void UI::HandleCommand(int id, int code, HWND hWnd, LifeGame& game, Renderer* pRenderer)
{
	if (id == ID_PATTERN_COMBO && code == CBN_SELCHANGE)
	{
		int sel = static_cast<int>(SendMessage(m_hPatternCombo, CB_GETCURSEL, 0, 0));
		if (sel >= 0)
		{
			const auto* p = game.GetPatternLibrary().GetPattern(sel);
			m_preview.SetPattern(p);
		}
		SetFocus(hWnd); // 自动聚焦回主窗口
	}
	else if (id == ID_APPLY_BTN && code == BN_CLICKED)
	{
		TCHAR buf[64];
		int newRows = game.GetHeight();
		int newCols = game.GetWidth();

		if (m_hRowsEdit)
		{
			GetWindowText(m_hRowsEdit, buf, _countof(buf));
			int v = _ttoi(buf);
			if (v > 0) newRows = v;
		}
		if (m_hColsEdit)
		{
			GetWindowText(m_hColsEdit, buf, _countof(buf));
			int v = _ttoi(buf);
			if (v > 0) newCols = v;
		}

		// 限制范围
		if (newCols < 4) newCols = 4;
		if (newRows < 4) newRows = 4;
		if (newCols > 400) newCols = 400; // 扩大上限
		if (newRows > 300) newRows = 300;

		game.ResizeGrid(newCols, newRows);
		if (pRenderer) pRenderer->ClearVisuals(); // 清除视觉残留

		// 调整窗口大小以适应新网格 (这里不再强制调整窗口大小，而是让网格适应窗口)
		// 因为用户要求适应 2000x2700 屏幕，窗口大小应该保持较大
		// 仅当窗口太小时才扩大
		/*
		int gridWpx = Renderer::CELL_SIZE * game.GetWidth();
		int gridHpx = Renderer::CELL_SIZE * game.GetHeight();
		int topControlsH = 44;
		int desiredClientW = Renderer::LEFT_PANEL_WIDTH + gridWpx + 20;
		int desiredClientH = gridHpx + Renderer::STATUS_BAR_HEIGHT + topControlsH + 10;

		int minGridWpx = Renderer::CELL_SIZE * 40;
		int minClientW = Renderer::LEFT_PANEL_WIDTH + minGridWpx + 20;
		int minGridHpx = Renderer::CELL_SIZE * 40;
		int minClientH = minGridHpx + Renderer::STATUS_BAR_HEIGHT + topControlsH + 10;

		if (desiredClientW < minClientW) desiredClientW = minClientW;
		if (desiredClientH < minClientH) desiredClientH = minClientH;

		RECT wr = {0, 0, desiredClientW, desiredClientH};
		AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, FALSE);

		SetWindowPos(hWnd, nullptr, 0, 0, wr.right - wr.left, wr.bottom - wr.top, SWP_NOMOVE | SWP_NOZORDER);
		*/

		// 更新输入框显示
		_stprintf_s(buf, TEXT("%d"), game.GetHeight());
		SetWindowText(m_hRowsEdit, buf);
		_stprintf_s(buf, TEXT("%d"), game.GetWidth());
		SetWindowText(m_hColsEdit, buf);

		UpdateWindowTitle(hWnd, game);
		SetFocus(hWnd);
	}
	else if (id == ID_RULE_COMBO && code == CBN_SELCHANGE)
	{
		int sel = static_cast<int>(SendMessage(m_hRuleCombo, CB_GETCURSEL, 0, 0));
		if (sel >= 0)
		{
			game.SetRule(sel);
		}
		SetFocus(hWnd); // 自动聚焦回主窗口
	}
	else if (id == ID_SIZE_COMBO && code == CBN_SELCHANGE)
	{
		int sel = static_cast<int>(SendMessage(m_hSizeCombo, CB_GETCURSEL, 0, 0));
		int w = 160, h = 120;
		if (sel == 0)
		{
			w = 40;
			h = 30;
		} // 大 (30x40)
		else if (sel == 1)
		{
			w = 160;
			h = 120;
		} // 中 (120x160)
		else if (sel == 2)
		{
			w = 400;
			h = 300;
		} // 小 (300x400)

		game.ResizeGrid(w, h);
		if (pRenderer) pRenderer->ClearVisuals(); // 清除视觉残留

		// 更新输入框显示
		TCHAR buf[32];
		_stprintf_s(buf, TEXT("%d"), game.GetHeight());
		SetWindowText(m_hRowsEdit, buf);
		_stprintf_s(buf, TEXT("%d"), game.GetWidth());
		SetWindowText(m_hColsEdit, buf);

		// 触发重绘
		InvalidateRect(hWnd, nullptr, TRUE);
		SetFocus(hWnd);
	}
	else if (id == ID_SAVE_BTN && code == BN_CLICKED)
	{
		OPENFILENAME ofn;
		TCHAR szFile[260] = {0};

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = hWnd;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = TEXT("LifeGame Save (*.life)\0*.life\0All Files (*.*)\0*.*\0");
		ofn.nFilterIndex = 1;
		ofn.lpstrDefExt = TEXT("life");
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

		if (GetSaveFileName(&ofn) == TRUE)
		{
			// 暂停游戏
			bool wasRunning = game.IsRunning();
			game.SetRunning(false);

			if (m_fileManager.SaveGame(szFile, game))
			{
				MessageBox(hWnd, TEXT("保存成功！"), TEXT("提示"), MB_OK | MB_ICONINFORMATION);
			}
			else
			{
				MessageBox(hWnd, TEXT("保存失败！"), TEXT("错误"), MB_OK | MB_ICONERROR);
			}

			if (wasRunning) game.SetRunning(true);
			SetFocus(hWnd);
		}
	}
	else if (id == ID_LOAD_BTN && code == BN_CLICKED)
	{
		OPENFILENAME ofn;
		TCHAR szFile[260] = {0};

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = hWnd;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = TEXT("LifeGame Save (*.life)\0*.life\0All Files (*.*)\0*.*\0");
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		if (GetOpenFileName(&ofn) == TRUE)
		{
			bool wasRunning = game.IsRunning();
			game.SetRunning(false);

			if (m_fileManager.LoadGame(szFile, game))
			{
				// 更新 UI
				TCHAR buf[32];
				_stprintf_s(buf, TEXT("%d"), game.GetHeight());
				SetWindowText(m_hRowsEdit, buf);
				_stprintf_s(buf, TEXT("%d"), game.GetWidth());
				SetWindowText(m_hColsEdit, buf);

				InvalidateRect(hWnd, nullptr, TRUE);
				MessageBox(hWnd, TEXT("加载成功！"), TEXT("提示"), MB_OK | MB_ICONINFORMATION);
			}
			else
			{
				MessageBox(hWnd, TEXT("加载失败！"), TEXT("错误"), MB_OK | MB_ICONERROR);
			}

			// 加载后通常保持暂停，让用户看一眼
			// if (wasRunning) game.SetRunning(true);
			SetFocus(hWnd);
		}
	}
	else if (id == ID_EXPORT_BTN && code == BN_CLICKED)
	{
		OPENFILENAME ofn;
		TCHAR szFile[260] = {0};

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = hWnd;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = TEXT("RLE Pattern (*.rle)\0*.rle\0All Files (*.*)\0*.*\0");
		ofn.nFilterIndex = 1;
		ofn.lpstrDefExt = TEXT("rle");
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

		if (GetSaveFileName(&ofn) == TRUE)
		{
			if (m_fileManager.ExportRLE(szFile, game))
			{
				MessageBox(hWnd, TEXT("导出成功！"), TEXT("提示"), MB_OK | MB_ICONINFORMATION);
			}
			else
			{
				MessageBox(hWnd, TEXT("导出失败！"), TEXT("错误"), MB_OK | MB_ICONERROR);
			}
			SetFocus(hWnd);
		}
	}
	else if (id == ID_SETTINGS_BTN && code == BN_CLICKED)
	{
		SettingsDialog dlg;
		if (dlg.Show(hWnd))
		{
			// 设置已更新，通知渲染器
			// 由于 Renderer 是 Main.cpp 管理的，我们这里无法直接访问 Renderer 实例
			// 但我们可以通过 InvalidateRect 触发重绘，Renderer::Draw 会读取新设置
			// 不过 Renderer 需要 UpdateSettings() 来重建画刷
			// 这是一个架构问题。UI 应该能访问 Renderer 或者通过回调。
			// 简单起见，我们在 Main.cpp 中处理，或者让 UI 持有 Renderer 指针?
			// UI 只有 LifeGame 引用。

			// 解决方案：发送自定义消息给主窗口，让主窗口调用 Renderer::UpdateSettings
			SendMessage(hWnd, WM_USER + 1, 0, 0);
		}
		SetFocus(hWnd);
	}
	else if (id == ID_HELP_BTN && code == BN_CLICKED)
	{
		m_helpWindow.Show(hWnd);
		// 不需要 SetFocus，因为 HelpWindow 可能会获取焦点
	}
	else if (id == ID_UNDO_BTN && code == BN_CLICKED)
	{
		game.GetCommandHistory().Undo(game);
		InvalidateRect(hWnd, nullptr, TRUE);
		SetFocus(hWnd);
	}
}

bool UI::HandleMouseClick(int x, int y, bool leftButton, LifeGame& game,
                          int clientWidth, int clientHeight)
{
	int cellSize, offX, offY, gridW, gridH;
	Renderer r; // 仅用于计算布局
	r.CalcLayout(game, cellSize, offX, offY, gridW, gridH, clientWidth, clientHeight);

	if (x >= offX && x < offX + gridW && y >= offY && y < offY + gridH)
	{
		int cellX = (x - offX) / cellSize;
		int cellY = (y - offY) / cellSize;

		if (cellX >= 0 && cellX < game.GetWidth() && cellY >= 0 && cellY < game.GetHeight())
		{
			if (leftButton)
			{
				// 获取当前选中的笔刷
				int sel = static_cast<int>(SendMessage(m_hPatternCombo, CB_GETCURSEL, 0, 0));
				if (sel < 0) sel = 0;

				const PatternData* p = game.GetPatternLibrary().GetPattern(sel);

				// 检查是否是单点绘制 (索引0 或 名字匹配)
				if (sel == 0 || (p && p->name == L"单点绘制"))
				{
					bool oldState = game.GetCell(cellX, cellY);
					if (oldState != true)
					{
						std::unique_ptr<Command> cmd(new SetCellCommand(cellX, cellY, true, oldState));
						game.GetCommandHistory().ExecuteCommand(std::move(cmd), game);
					}
					m_isDragging = true;
					m_dragValue = true;
				}
				else if (p && p->name == L"随机填充 (Random)")
				{
					bool oldState = game.GetCell(cellX, cellY);
					std::unique_ptr<Command> cmd(new SetCellCommand(cellX, cellY, true, oldState));
					game.GetCommandHistory().ExecuteCommand(std::move(cmd), game);
				}
				else
				{
					std::unique_ptr<Command> cmd(new PlacePatternCommand(cellX, cellY, sel, game));
					game.GetCommandHistory().ExecuteCommand(std::move(cmd), game);

					m_isDragging = false;
				}
				return true;
			}
			// 右键擦除
			bool oldState = game.GetCell(cellX, cellY);
			if (oldState != false)
			{
				std::unique_ptr<Command> cmd(new SetCellCommand(cellX, cellY, false, oldState));
				game.GetCommandHistory().ExecuteCommand(std::move(cmd), game);
			}
			m_isRightDragging = true;
			return true;
		}
	}
	return false;
}

bool UI::HandleMouseMove(int x, int y, LifeGame& game,
                         int clientWidth, int clientHeight)
{
	if (!m_isDragging && !m_isRightDragging) return false;

	int cellSize, offX, offY, gridW, gridH;
	Renderer r;
	r.CalcLayout(game, cellSize, offX, offY, gridW, gridH, clientWidth, clientHeight);

	if (x >= offX && x < offX + gridW && y >= offY && y < offY + gridH)
	{
		int cellX = (x - offX) / cellSize;
		int cellY = (y - offY) / cellSize;

		if (cellX >= 0 && cellX < game.GetWidth() && cellY >= 0 && cellY < game.GetHeight())
		{
			bool target = m_isDragging;
			bool oldState = game.GetCell(cellX, cellY);
			if (oldState != target)
			{
				std::unique_ptr<Command> cmd(new SetCellCommand(cellX, cellY, target, oldState));
				game.GetCommandHistory().ExecuteCommand(std::move(cmd), game);
				return true;
			}
		}
	}
	return false;
}

void UI::HandleMouseUp(bool leftButton)
{
	if (leftButton) m_isDragging = false;
	else m_isRightDragging = false;
}

LRESULT CALLBACK UI::RowsEditProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_KEYDOWN && wp == VK_RETURN)
	{
		if (s_pInstance && s_pInstance->m_hApplyBtn)
			SendMessage(s_pInstance->m_hApplyBtn, BM_CLICK, 0, 0);
		return 0;
	}
	if (s_pInstance && s_pInstance->m_oldRowsProc)
		return CallWindowProc(s_pInstance->m_oldRowsProc, hwnd, msg, wp, lp);
	return DefWindowProc(hwnd, msg, wp, lp);
}

LRESULT CALLBACK UI::ColsEditProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_KEYDOWN && wp == VK_RETURN)
	{
		if (s_pInstance && s_pInstance->m_hApplyBtn)
			SendMessage(s_pInstance->m_hApplyBtn, BM_CLICK, 0, 0);
		return 0;
	}
	if (s_pInstance && s_pInstance->m_oldColsProc)
		return CallWindowProc(s_pInstance->m_oldColsProc, hwnd, msg, wp, lp);
	return DefWindowProc(hwnd, msg, wp, lp);
}

LRESULT CALLBACK UI::ApplyBtnProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (!s_pInstance) return DefWindowProc(hwnd, msg, wp, lp);

	if (msg == WM_MOUSEMOVE)
	{
		if (!s_pInstance->m_applyHover)
		{
			s_pInstance->m_applyHover = true;
			InvalidateRect(GetParent(hwnd), nullptr, TRUE);
		}
		TRACKMOUSEEVENT tme = {sizeof(tme), TME_LEAVE, hwnd, 0};
		TrackMouseEvent(&tme);
	}
	else if (msg == WM_MOUSELEAVE)
	{
		if (s_pInstance->m_applyHover)
		{
			s_pInstance->m_applyHover = false;
			InvalidateRect(GetParent(hwnd), nullptr, TRUE);
		}
	}

	if (s_pInstance->m_oldApplyBtnProc)
		return CallWindowProc(s_pInstance->m_oldApplyBtnProc, hwnd, msg, wp, lp);
	return DefWindowProc(hwnd, msg, wp, lp);
}
