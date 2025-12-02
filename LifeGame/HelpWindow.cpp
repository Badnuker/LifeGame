#include "HelpWindow.h"
#include <commctrl.h>

HelpWindow* g_pHelpWindow = nullptr;

HelpWindow::HelpWindow()
	: m_hWnd(nullptr), m_hList(nullptr), m_hZoomInBtn(nullptr), m_hZoomOutBtn(nullptr),
	  m_currentPage(0), m_fontScale(1.0f), m_hTitleFont(nullptr), m_hBodyFont(nullptr)
{
	// 初始化帮助内容
	m_pages.push_back({
		L"简介",
		L"康威生命游戏 (Conway's Game of Life) 是由英国数学家约翰·何顿·康威在1970年发明的细胞自动机。\n\n"
		L"它是一个零玩家游戏，意味着其演化完全取决于初始状态，不需要进一步的输入。\n"
		L"在一个二维网格上，每个细胞有两种状态：存活或死亡。每一代的状态由上一代根据简单的规则决定。"
	});

	m_pages.push_back({
		L"基本规则",
		L"标准的生命游戏规则 (B3/S23) 如下：\n\n"
		L"1. 人口过少：任何活细胞如果活邻居少于2个，则死亡。\n"
		L"2. 正常生存：任何活细胞如果活邻居为2个或3个，则继续存活。\n"
		L"3. 人口过多：任何活细胞如果活邻居多于3个，则死亡。\n"
		L"4. 繁殖：任何死细胞如果活邻居正好是3个，则变为活细胞。\n\n"
		L"本程序支持自定义规则，格式为 Bx/Sy，其中 B 代表出生 (Birth) 所需邻居数，S 代表生存 (Survival) 所需邻居数。"
	});

	m_pages.push_back({
		L"操作指南",
		L"鼠标操作：\n"
		L"- 左键点击/拖动：绘制细胞 (使用当前选中的笔刷)\n"
		L"- 右键点击/拖动：擦除细胞\n\n"
		L"键盘快捷键：\n"
		L"- SPACE：开始 / 暂停演化\n"
		L"- R：重置画布 (清空)\n"
		L"- G：随机生成初始状态\n"
		L"- + / -：调节演化速度\n"
		L"- ESC：退出程序"
	});

	m_pages.push_back({
		L"图案库",
		L"程序内置了丰富的图案库，包括：\n\n"
		L"- 静态物体 (Still Lifes)：方块、蜂巢等，永远不变。\n"
		L"- 振荡器 (Oscillators)：信号灯、脉冲星等，按周期变化。\n"
		L"- 飞船 (Spaceships)：滑翔机、轻型飞船等，会移动。\n"
		L"- 枪 (Guns)：高斯帕滑翔机枪，能不断发射滑翔机。\n\n"
		L"在左侧面板选择笔刷模式即可使用。"
	});

	m_pages.push_back({
		L"高级功能",
		L"1. 规则引擎：支持多种变体规则，如 HighLife (B36/S23), Day & Night (B3678/S34678) 等。\n"
		L"2. 统计图表：右下角实时显示种群数量变化曲线。\n"
		L"3. 文件系统：支持保存 (.life) 和加载存档，以及导出 RLE 格式。\n"
		L"4. 视觉设置：可自定义颜色、网格线、HUD 等外观。"
	});

	m_pages.push_back({
		L"关于",
		L"LifeGame v2.0 (Win32 GDI)\n\n"
		L"开发：Zhong yi, Liu qingxin, Dong kehong\n"
		L"技术栈：C++17, Windows API (GDI), MinGW-w64\n\n"
		L"本项目旨在展示高性能的细胞自动机模拟与现代化的 GDI 绘图技术。\n"
	});
}

HelpWindow::~HelpWindow()
{
	if (m_hTitleFont) DeleteObject(m_hTitleFont);
	if (m_hBodyFont) DeleteObject(m_hBodyFont);
}

void HelpWindow::Show(HWND hParent)
{
	if (m_hWnd)
	{
		SetForegroundWindow(m_hWnd);
		return;
	}

	g_pHelpWindow = this;

	WNDCLASS wc = {0};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.lpszClassName = TEXT("LifeGameHelpWnd");
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hIcon = LoadIcon(nullptr, IDI_INFORMATION);
	RegisterClass(&wc);

	int w = 900; // 增大窗口宽度
	int h = 700; // 增大窗口高度
	int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
	int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

	m_hWnd = CreateWindow(TEXT("LifeGameHelpWnd"), TEXT("使用手册"),
	                      WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, // 不可最大化
	                      x, y, w, h,
	                      hParent, nullptr, wc.hInstance, this);

	if (m_hWnd)
	{
		ShowWindow(m_hWnd, SW_SHOW);
		UpdateWindow(m_hWnd);
	}
}

LRESULT CALLBACK HelpWindow::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	HelpWindow* pThis = g_pHelpWindow;
	if (msg == WM_DESTROY)
	{
		if (pThis) pThis->m_hWnd = nullptr;
		return 0;
	}

	if (pThis)
	{
		switch (msg)
		{
		case WM_CREATE:
			pThis->OnCreate(hWnd);
			return 0;
		case WM_SIZE:
			pThis->OnSize(hWnd);
			return 0;
		case WM_PAINT:
			pThis->OnPaint(hWnd);
			return 0;
		case WM_COMMAND:
			pThis->OnCommand(hWnd, LOWORD(wp), HIWORD(wp));
			return 0;
		}
	}
	return DefWindowProc(hWnd, msg, wp, lp);
}

void HelpWindow::OnCreate(HWND hWnd)
{
	// 创建左侧列表框 (加宽)
	// 留出底部空间给缩放按钮
	m_hList = CreateWindow(TEXT("LISTBOX"), nullptr,
	                       WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY | WS_VSCROLL,
	                       0, 0, 220, 400,
	                       hWnd, (HMENU)100, GetModuleHandle(nullptr), nullptr);

	// 创建缩放按钮
	m_hZoomInBtn = CreateWindow(TEXT("BUTTON"), TEXT("A+"),
	                            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
	                            10, 0, 95, 30,
	                            hWnd, (HMENU)101, GetModuleHandle(nullptr), nullptr);

	m_hZoomOutBtn = CreateWindow(TEXT("BUTTON"), TEXT("A-"),
	                             WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
	                             115, 0, 95, 30,
	                             hWnd, (HMENU)102, GetModuleHandle(nullptr), nullptr);

	// 添加条目
	for (const auto& page : m_pages)
	{
		SendMessage(m_hList, LB_ADDSTRING, 0, (LPARAM)page.title.c_str());
	}
	SendMessage(m_hList, LB_SETCURSEL, 0, 0);

	UpdateFonts();
}

void HelpWindow::UpdateFonts()
{
	if (m_hTitleFont) DeleteObject(m_hTitleFont);
	if (m_hBodyFont) DeleteObject(m_hBodyFont);

	int titleSize = static_cast<int>(36 * m_fontScale);
	int bodySize = static_cast<int>(24 * m_fontScale);

	// 创建字体 (加大字号)
	m_hTitleFont = CreateFont(titleSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
	                          DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
	                          VARIABLE_PITCH | FF_SWISS, TEXT("Microsoft YaHei UI"));

	m_hBodyFont = CreateFont(bodySize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
	                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
	                         VARIABLE_PITCH | FF_SWISS, TEXT("Microsoft YaHei UI"));

	// 设置列表框字体
	if (m_hList)
		SendMessage(m_hList, WM_SETFONT, (WPARAM)m_hBodyFont, TRUE);
	if (m_hZoomInBtn)
		SendMessage(m_hZoomInBtn, WM_SETFONT, (WPARAM)m_hBodyFont, TRUE);
	if (m_hZoomOutBtn)
		SendMessage(m_hZoomOutBtn, WM_SETFONT, (WPARAM)m_hBodyFont, TRUE);
}

void HelpWindow::OnSize(HWND hWnd)
{
	RECT rc;
	GetClientRect(hWnd, &rc);
	if (m_hList)
	{
		// 列表框占据左侧，底部留出 40 像素给按钮
		MoveWindow(m_hList, 0, 0, 220, rc.bottom - 40, TRUE);
	}
	if (m_hZoomInBtn && m_hZoomOutBtn)
	{
		// 按钮位于左侧底部
		MoveWindow(m_hZoomInBtn, 5, rc.bottom - 35, 100, 30, TRUE);
		MoveWindow(m_hZoomOutBtn, 115, rc.bottom - 35, 100, 30, TRUE);
	}
	InvalidateRect(hWnd, nullptr, TRUE);
}

void HelpWindow::OnCommand(HWND hWnd, int id, int code)
{
	if (id == 100) // ListBox
	{
		if (code == LBN_SELCHANGE)
		{
			int sel = static_cast<int>(SendMessage(m_hList, LB_GETCURSEL, 0, 0));
			if (sel >= 0 && sel < static_cast<int>(m_pages.size()))
			{
				m_currentPage = sel;
				// 重绘右侧区域
				RECT rc;
				GetClientRect(hWnd, &rc);
				rc.left = 220;
				InvalidateRect(hWnd, &rc, TRUE);
			}
		}
	}
	else if (id == 101) // Zoom In
	{
		if (m_fontScale < 3.0f)
		{
			m_fontScale += 0.1f;
			UpdateFonts();
			InvalidateRect(hWnd, nullptr, TRUE);
		}
	}
	else if (id == 102) // Zoom Out
	{
		if (m_fontScale > 0.5f)
		{
			m_fontScale -= 0.1f;
			UpdateFonts();
			InvalidateRect(hWnd, nullptr, TRUE);
		}
	}
}

void HelpWindow::OnPaint(HWND hWnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);

	RECT rc;
	GetClientRect(hWnd, &rc);
	rc.left = 220 + 30; // 右侧内容区域，留出更多边距
	rc.top += 30;
	rc.right -= 30;
	rc.bottom -= 30;

	if (m_currentPage >= 0 && m_currentPage < static_cast<int>(m_pages.size()))
	{
		DrawPage(hdc, rc);
	}

	EndPaint(hWnd, &ps);
}

void HelpWindow::DrawPage(HDC hdc, const RECT& rect)
{
	const auto& page = m_pages[m_currentPage];

	SetBkMode(hdc, TRANSPARENT);

	// 绘制标题
	auto hOld = static_cast<HFONT>(SelectObject(hdc, m_hTitleFont));
	SetTextColor(hdc, RGB(0, 50, 100));

	RECT titleRect = rect;
	DrawText(hdc, page.title.c_str(), -1, &titleRect, DT_LEFT | DT_TOP | DT_SINGLELINE);

	// 绘制分割线
	int titleH = static_cast<int>(60 * m_fontScale); // 增加标题高度
	HPEN hPen = CreatePen(PS_SOLID, 2, RGB(200, 200, 200));
	auto hOldPen = static_cast<HPEN>(SelectObject(hdc, hPen));
	MoveToEx(hdc, rect.left, rect.top + static_cast<int>(50 * m_fontScale), nullptr);
	LineTo(hdc, rect.right, rect.top + static_cast<int>(50 * m_fontScale));
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);

	// 绘制正文
	SelectObject(hdc, m_hBodyFont);
	SetTextColor(hdc, RGB(50, 50, 50));

	RECT bodyRect = rect;
	bodyRect.top += titleH;

	DrawText(hdc, page.content.c_str(), -1, &bodyRect, DT_LEFT | DT_TOP | DT_WORDBREAK);

	SelectObject(hdc, hOld);
}
