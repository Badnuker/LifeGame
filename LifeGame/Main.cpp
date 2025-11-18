#pragma comment(lib, "comctl32.lib")

#include <windows.h>
#include <time.h>
#include <vector>
#include <string>
#include <sstream>
#include <tchar.h>
#include <commctrl.h>

// 前向声明子类过程
LRESULT CALLBACK RowsEditProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK ColsEditProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK ApplyBtnProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

// 全局常量定义
constexpr int CELL_SIZE = 10; // 细胞像素大小
// 网格列行数可配置（默认 80x60）
int g_gridWidth = 80; // 列数（默认）
int g_gridHeight = 60; // 行数（默认）
constexpr int WINDOW_WIDTH = CELL_SIZE * 80; // 默认窗口宽度（用于初始窗口大小）
constexpr int WINDOW_HEIGHT = CELL_SIZE * 60; // 默认窗口高度（用于初始窗口大小）
// 动态客户区尺寸（支持缩放）
int g_clientWidth = WINDOW_WIDTH;
int g_clientHeight = WINDOW_HEIGHT;
constexpr int STATUS_BAR_HEIGHT = 28; // 底部状态栏高度
constexpr int LEFT_PANEL_WIDTH = 150; // 左侧固定面板宽度（显示快捷键）
constexpr int MIN_INTERVAL = 50; // 最小更新间隔（最快速度，50毫秒/帧）
constexpr int MAX_INTERVAL = 500; // 最大更新间隔（最慢速度，500毫秒/帧）
constexpr int SPEED_STEP = 50; // 速度调整步长（每次加减50毫秒）
int g_updateInterval = 100; // 游戏更新间隔（默认100毫秒，可动态调整）

// 全局变量定义（动态分配，支持可配置行列）
std::vector<std::vector<bool>> g_grid; // 当前细胞状态（false=死亡，true=存活）
std::vector<std::vector<bool>> g_nextGrid; // 下一代细胞状态
bool g_isRunning = false; // 游戏运行状态（false=暂停，true=运行）
UINT_PTR g_timerId = 0; // 定时器ID
UINT_PTR g_tipTimerId = 0; // 提示定时器ID
bool g_showResetTip = false; // 重置提示显示标记
ULONGLONG g_resetTipTime = 0; // 重置提示显示时间
// 鼠标拖拽绘制状态
bool g_isDragging = false; // 鼠标左键正在拖拽
bool g_dragValue = true; // 拖拽时要设置的细胞状态（按下第一格后确定）
// 右键拖拽用于擦除
bool g_isRightDragging = false; // 鼠标右键正在拖拽

// UI 控件矩形（两个按钮：一用于行 Rows，一用于列 Cols）
RECT g_rowsBtnRect = {0};
RECT g_colsBtnRect = {0};

// 全局 GDI 对象
HBRUSH g_hBlackBrush = nullptr;
HBRUSH g_hWhiteBrush = nullptr;
HPEN g_hGrayPen = nullptr;
// 额外的视觉资源
HBRUSH g_hAliveBrush = nullptr;
HBRUSH g_hDeadBrush = nullptr;
HBRUSH g_hBackgroundBrush = nullptr;
HPEN g_hGridPen = nullptr;
HFONT g_hTitleFont = nullptr;
HFONT g_hTipFont = nullptr;
HBRUSH g_hTipBrush = nullptr;
HFONT g_hBtnFont = nullptr;
HBRUSH g_hLeftPanelBrush = nullptr;
HFONT g_hLeftKeyFont = nullptr;
HFONT g_hLeftDescFont = nullptr;

// UI 控件句柄
HWND g_hRowsEdit = nullptr;
HWND g_hColsEdit = nullptr;
HWND g_hApplyBtn = nullptr;
HWND g_hRowsLabel = nullptr; // 行标签
HWND g_hColsLabel = nullptr; // 列标签
WNDPROC g_oldRowsProc = nullptr;
WNDPROC g_oldColsProc = nullptr;
WNDPROC g_oldApplyBtnProc = nullptr;
bool g_applyHover = false;

// tooltip
HWND g_hToolTip = nullptr;

// 控件 ID
#define ID_ROWS_EDIT 1001
#define ID_COLS_EDIT 1002
#define ID_APPLY_BTN 1003

// 函数声明
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitGrid(); // 初始化随机细胞网格
void DrawGrid(HDC hdc, const RECT* pDirty); // 绘制细胞网格（含重置提示）
int CountNeighbors(int x, int y); // 计算细胞周围存活邻居数
void UpdateGrid(); // 计算下一代细胞状态
UINT_PTR ResetGrid(HWND hWnd); // 重置网格（全细胞死亡）
void RestartTimer(HWND hWnd); // 重启定时器（适配速度调整）
void UpdateWindowTitle(HWND hWnd); // 更新标题栏（含速度提示）

// 根据当前客户区计算每个单元的像素大小与偏移
void CalcLayout(int& outCellSize, int& outOffsetX, int& outOffsetY, int& outGridWidthPx, int& outGridHeightPx)
{
	// 网格区域可用宽高
	int availW = g_clientWidth - LEFT_PANEL_WIDTH;
	int availH = g_clientHeight - STATUS_BAR_HEIGHT;
	if (availW < 1) availW = 1;
	if (availH < 1) availH = 1;
	int cellSize = CELL_SIZE; // 始终使用固定像素大小
	int gridW = cellSize * g_gridWidth;
	int gridH = cellSize * g_gridHeight;
	// 网格左上角从 LEFT_PANEL_WIDTH,0 开始，居中于可用区域
	int offX = LEFT_PANEL_WIDTH + (availW - gridW) / 2;
	int offY = (availH - gridH) / 2;
	if (offX < LEFT_PANEL_WIDTH) offX = LEFT_PANEL_WIDTH;
	if (offY < 0) offY = 0;
	outCellSize = cellSize;
	outOffsetX = offX;
	outOffsetY = offY;
	outGridWidthPx = gridW;
	outGridHeightPx = gridH;
}

// 布局并移动控件（在 WM_SIZE 和创建后调用）
void LayoutControls(HWND hWnd)
{
	if (!g_hRowsEdit || !g_hColsEdit || !g_hApplyBtn || !g_hRowsLabel || !g_hColsLabel) return;
	int shortcutCount = 5;
	int panelPaddingY = 12;
	int lineH = 24;
	int leftX = 16;
	int leftY = panelPaddingY + shortcutCount * lineH + 24; // 快捷键区下方
	int labelW = 32, editW = 72, editH = 22, gapY = 12;
	SetWindowPos(g_hColsLabel, nullptr, leftX, leftY, labelW, editH, SWP_NOZORDER);
	SetWindowPos(g_hColsEdit, nullptr, leftX + labelW + 8, leftY, editW, editH, SWP_NOZORDER);
	leftY += editH + gapY;
	SetWindowPos(g_hRowsLabel, nullptr, leftX, leftY, labelW, editH, SWP_NOZORDER);
	SetWindowPos(g_hRowsEdit, nullptr, leftX + labelW + 8, leftY, editW, editH, SWP_NOZORDER);
	leftY += editH + gapY;
	SetWindowPos(g_hApplyBtn, nullptr, leftX, leftY, labelW + editW + 8, editH, SWP_NOZORDER);

	// 更新编辑框内容为当前值
	TCHAR buf[32];
	wsprintf(buf, TEXT("%d"), g_gridHeight);
	SetWindowText(g_hRowsEdit, buf);
	wsprintf(buf, TEXT("%d"), g_gridWidth);
	SetWindowText(g_hColsEdit, buf);
}

// 调整网格尺寸并尽量保留原有数据（新增区域为 0）
void ResizeGrid(int newWidth, int newHeight)
{
	if (newWidth < 4) newWidth = 4;
	if (newHeight < 4) newHeight = 4;
	if (newWidth > 120) newWidth = 120; // 列上限（最大120）
	if (newHeight > 80) newHeight = 80; // 行上限（最大80）

	if (newWidth == g_gridWidth && newHeight == g_gridHeight) return;

	std::vector<std::vector<bool>> newGrid;
	std::vector<std::vector<bool>> newNext;
	newGrid.assign(newHeight, std::vector<bool>(newWidth, false));
	newNext.assign(newHeight, std::vector<bool>(newWidth, false));

	// 复制已有数据到新网格（取重叠区域）
	int minH = (newHeight < g_gridHeight) ? newHeight : g_gridHeight;
	int minW = (newWidth < g_gridWidth) ? newWidth : g_gridWidth;
	for (int y = 0; y < minH; y++)
	{
		for (int x = 0; x < minW; x++)
		{
			newGrid[y][x] = g_grid[y][x];
			newNext[y][x] = (y < static_cast<int>(g_nextGrid.size()) && x < static_cast<int>(g_nextGrid[y].size()))
				                ? g_nextGrid[y][x]
				                : false;
		}
	}

	g_gridWidth = newWidth;
	g_gridHeight = newHeight;
	g_grid.swap(newGrid);
	g_nextGrid.swap(newNext);
}

// 程序入口
int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nCmdShow
)
{
	// 解析命令行（支持 -w <cols> -h <rows> 或 w=<cols> h=<rows>）
	if (lpCmdLine && lpCmdLine[0] != '\0')
	{
		std::string cmd(lpCmdLine);
		std::istringstream iss(cmd);
		std::string tok;
		while (iss >> tok)
		{
			if ((tok == "-w" || tok == "-cols" || tok == "--width") && (iss >> tok))
			{
				int v = atoi(tok.c_str());
				if (v > 0) g_gridWidth = v;
			}
			else if ((tok == "-h" || tok == "-rows" || tok == "--height") && (iss >> tok))
			{
				int v = atoi(tok.c_str());
				if (v > 0) g_gridHeight = v;
			}
			else
			{
				// 支持 w=NN h=NN 格式
				auto pos = tok.find('=');
				if (pos != std::string::npos)
				{
					std::string key = tok.substr(0, pos);
					std::string val = tok.substr(pos + 1);
					if ((key == "w" || key == "cols" || key == "width") && !val.empty())
					{
						int v = atoi(val.c_str());
						if (v > 0) g_gridWidth = v;
					}
					else if ((key == "h" || key == "rows" || key == "height") && !val.empty())
					{
						int v = atoi(val.c_str());
						if (v > 0) g_gridHeight = v;
					}
				}
			}
		}
	}

	// 限制合理范围
	if (g_gridWidth < 4) g_gridWidth = 4;
	if (g_gridHeight < 4) g_gridHeight = 4;
	if (g_gridWidth > 120) g_gridWidth = 120;
	if (g_gridHeight > 80) g_gridHeight = 80;

	// 注册窗口类
	WNDCLASS wc = {0};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = TEXT("LifeGameWindow");
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = nullptr;

	// 初始化 GDI 对象（只创建一次，程序退出时销毁）
	g_hBlackBrush = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
	g_hWhiteBrush = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
	g_hGrayPen = CreatePen(PS_SOLID, 0, RGB(200, 200, 200));

	// 视觉优化：背景色、存活/死亡细胞颜色、网格线、标题字体与提示框
	g_hBackgroundBrush = CreateSolidBrush(RGB(245, 247, 250));
	g_hAliveBrush = CreateSolidBrush(RGB(34, 40, 49)); // 深色活细胞
	g_hDeadBrush = CreateSolidBrush(RGB(255, 255, 255)); // 空白单元
	g_hGridPen = CreatePen(PS_SOLID, 1, RGB(210, 213, 220));
	g_hTipBrush = CreateSolidBrush(RGB(255, 255, 255));
	// 左侧面板背景
	// 左侧面板背景（更柔和的蓝灰）
	g_hLeftPanelBrush = CreateSolidBrush(RGB(225, 235, 245));

	// 字体（标题与小提示）
	g_hTitleFont = CreateFontW(-18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
	                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
	                           VARIABLE_PITCH | FF_SWISS, L"Segoe UI");
	g_hTipFont = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
	                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
	                         VARIABLE_PITCH | FF_SWISS, L"Segoe UI");

	// 按钮字体（粗体）
	g_hBtnFont = CreateFontW(-13, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
	                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
	                         VARIABLE_PITCH | FF_SWISS, L"Segoe UI");

	// 左侧面板字体：key(加粗)、desc(常规)
	g_hLeftKeyFont = CreateFontW(-14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
	                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
	                             VARIABLE_PITCH | FF_SWISS, L"Segoe UI");
	g_hLeftDescFont = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
	                              DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
	                              VARIABLE_PITCH | FF_SWISS, L"Segoe UI");

	if (!RegisterClass(&wc))
	{
		MessageBox(nullptr, TEXT("窗口类注册失败！"), TEXT("错误"), MB_ICONERROR);
		return 1;
	}

	// 计算窗口实际大小（适配边框和标题栏）
	RECT windowRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT + STATUS_BAR_HEIGHT};
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, FALSE);

	// 创建窗口
	HWND hWnd = CreateWindow(
		TEXT("LifeGameWindow"),
		TEXT("生命游戏"), // 初始标题，后续会动态更新
		WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL, NULL, hInstance, NULL
	);

	if (!hWnd)
	{
		MessageBox(nullptr, TEXT("窗口创建失败！"), TEXT("错误"), MB_ICONERROR);
		return 1;
	}

	// 创建左侧栏控件
	int leftX = 16, leftY = 32, labelW = 32, editW = 72, editH = 22, gapY = 12;
	g_hColsLabel = CreateWindowEx(0, TEXT("STATIC"), TEXT("列"), WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE,
	                              leftX, leftY, labelW, editH, hWnd, nullptr, hInstance, nullptr);
	g_hColsEdit = CreateWindowEx(0, TEXT("EDIT"), nullptr,
	                             WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOVSCROLL | WS_TABSTOP,
	                             leftX + labelW + 8, leftY, editW, editH, hWnd, (HMENU)ID_COLS_EDIT, hInstance,
	                             nullptr);
	leftY += editH + gapY;
	g_hRowsLabel = CreateWindowEx(0, TEXT("STATIC"), TEXT("行"), WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE,
	                              leftX, leftY, labelW, editH, hWnd, nullptr, hInstance, nullptr);
	g_hRowsEdit = CreateWindowEx(0, TEXT("EDIT"), nullptr,
	                             WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOVSCROLL | WS_TABSTOP,
	                             leftX + labelW + 8, leftY, editW, editH, hWnd, (HMENU)ID_ROWS_EDIT, hInstance,
	                             nullptr);
	leftY += editH + gapY;
	g_hApplyBtn = CreateWindowEx(0, TEXT("BUTTON"), TEXT("Apply"), WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | WS_TABSTOP,
	                             leftX, leftY, labelW + editW + 8, editH, hWnd, (HMENU)ID_APPLY_BTN, hInstance,
	                             nullptr);

	// 设置初始编辑框文本
	TCHAR tmpbuf[32];
	wsprintf(tmpbuf, TEXT("%d"), g_gridHeight);
	SetWindowText(g_hRowsEdit, tmpbuf);
	wsprintf(tmpbuf, TEXT("%d"), g_gridWidth);
	SetWindowText(g_hColsEdit, tmpbuf);

	// 子类化 Edit 控件以捕获回车键（Enter => Apply）
	g_oldRowsProc = (WNDPROC)SetWindowLongPtr(g_hRowsEdit, GWLP_WNDPROC, (LONG_PTR)RowsEditProc);
	g_oldColsProc = (WNDPROC)SetWindowLongPtr(g_hColsEdit, GWLP_WNDPROC, (LONG_PTR)ColsEditProc);
	g_oldApplyBtnProc = (WNDPROC)SetWindowLongPtr(g_hApplyBtn, GWLP_WNDPROC, (LONG_PTR)ApplyBtnProc);

	// 创建 tooltip 并关联到 Apply 按钮
	INITCOMMONCONTROLSEX icex = {sizeof(icex), ICC_WIN95_CLASSES};
	InitCommonControlsEx(&icex);
	g_hToolTip = CreateWindowEx(0, TOOLTIPS_CLASS, nullptr, WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX,
	                            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
	                            hWnd, nullptr, hInstance, nullptr);
	if (g_hToolTip)
	{
		TOOLINFO ti = {0};
		ti.cbSize = sizeof(ti);
		ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
		ti.hwnd = hWnd;
		ti.uId = (UINT_PTR)g_hApplyBtn;
		ti.lpszText = (LPTSTR)TEXT("应用行/列设置 (Enter) ");
		SendMessage(g_hToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
	}


	// 初始化细胞网格、更新标题栏并显示窗口
	InitGrid();
	UpdateWindowTitle(hWnd); // 首次设置标题栏（含初始速度）
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// 初始化客户区尺寸
	RECT client;
	GetClientRect(hWnd, &client);
	g_clientWidth = client.right - client.left;
	g_clientHeight = client.bottom - client.top;

	// 布局控件到正确位置
	LayoutControls(hWnd);
	// 将键盘焦点设回主窗口，确保空格用于开始/暂停
	SetFocus(hWnd);

	// 消息循环
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// 程序退出时销毁 GDI 对象
	DeleteObject(g_hGrayPen);
	if (g_hGridPen) DeleteObject(g_hGridPen);
	if (g_hBackgroundBrush) DeleteObject(g_hBackgroundBrush);
	if (g_hAliveBrush) DeleteObject(g_hAliveBrush);
	if (g_hDeadBrush) DeleteObject(g_hDeadBrush);
	if (g_hTipBrush) DeleteObject(g_hTipBrush);
	if (g_hLeftPanelBrush) DeleteObject(g_hLeftPanelBrush);
	if (g_hTitleFont) DeleteObject(g_hTitleFont);
	if (g_hTipFont) DeleteObject(g_hTipFont);
	if (g_hBtnFont) DeleteObject(g_hBtnFont);
	// 恢复并移除子类
	if (g_hRowsEdit && g_oldRowsProc)
		SetWindowLongPtr(g_hRowsEdit, GWLP_WNDPROC, (LONG_PTR)g_oldRowsProc);
	if (g_hColsEdit && g_oldColsProc)
		SetWindowLongPtr(g_hColsEdit, GWLP_WNDPROC, (LONG_PTR)g_oldColsProc);
	if (g_hApplyBtn && g_oldApplyBtnProc)
		SetWindowLongPtr(g_hApplyBtn, GWLP_WNDPROC, (LONG_PTR)g_oldApplyBtnProc);
	// 销毁 tooltip
	if (g_hToolTip) DestroyWindow(g_hToolTip);
	if (g_hLeftKeyFont) DeleteObject(g_hLeftKeyFont);
	if (g_hLeftDescFont) DeleteObject(g_hLeftDescFont);

	return static_cast<int>(msg.wParam);
}

// 窗口过程函数（消息处理核心）
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	// 绘图消息
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);

			// 获取当前客户区并更新全局尺寸
			RECT clientRect;
			GetClientRect(hWnd, &clientRect);
			g_clientWidth = clientRect.right - clientRect.left;
			g_clientHeight = clientRect.bottom - clientRect.top;

			// 双缓冲：根据当前客户区大小创建位图
			HDC memDC = CreateCompatibleDC(hdc);
			HBITMAP hbm = CreateCompatibleBitmap(hdc, g_clientWidth, g_clientHeight);
			auto hOldBmp = static_cast<HBITMAP>(SelectObject(memDC, hbm));

			// 用柔和背景色填充整个客户区
			FillRect(memDC, &clientRect, g_hBackgroundBrush);

			// 只重绘脏矩形区域（DrawGrid 内会绘制细胞、状态栏与提示）
			DrawGrid(memDC, &ps.rcPaint);

			// 将缓冲区全部 blit 到窗口（更简单且稳定）
			BitBlt(hdc, 0, 0, g_clientWidth, g_clientHeight, memDC, 0, 0, SRCCOPY);

			SelectObject(memDC, hOldBmp);
			DeleteObject(hbm);
			DeleteDC(memDC);
			EndPaint(hWnd, &ps);
			break;
		}

	// 防止默认擦除背景，配合双缓冲可以减少闪烁
	case WM_ERASEBKGND:
		return 1;

	// 窗口尺寸变化，更新客户区并重绘
	case WM_SIZE:
		g_clientWidth = LOWORD(lParam);
		g_clientHeight = HIWORD(lParam);
		// 重新布局控件并重绘
		LayoutControls(hWnd);
		InvalidateRect(hWnd, nullptr, TRUE);
		break;

	// 定时器消息（控制游戏刷新）
	case WM_TIMER:
		if (wParam == 1) // 游戏更新定时器
		{
			if (g_isRunning)
			{
				UpdateGrid();
				InvalidateRect(hWnd, nullptr, FALSE);
			}
		}
		else if (wParam == 2) // 重置提示隐藏定时器
		{
			g_showResetTip = false;
			g_tipTimerId = 0; // 重置定时器ID
			InvalidateRect(hWnd, nullptr, FALSE);
			KillTimer(hWnd, 2); // 销毁定时器
		}
		break;

	// 鼠标左键点击（切换细胞状态）
	case WM_LBUTTONDOWN:
		{
			// 隐藏重置提示并取消定时器
			if (g_showResetTip)
			{
				g_showResetTip = false;
				if (g_tipTimerId != 0)
				{
					KillTimer(hWnd, 2);
					g_tipTimerId = 0;
				}
				InvalidateRect(hWnd, nullptr, FALSE); // 立即重绘隐藏提示
			}

			int px = LOWORD(lParam);
			int py = HIWORD(lParam);
			int cellSize, offX, offY, gridWpx, gridHpx;
			CalcLayout(cellSize, offX, offY, gridWpx, gridHpx);
			if (px >= offX && px < offX + gridWpx && py >= offY && py < offY + gridHpx)
			{
				int x = (px - offX) / cellSize;
				int y = (py - offY) / cellSize;
				if (x >= 0 && x < g_gridWidth && y >= 0 && y < g_gridHeight)
				{
					// 左键始终染色（设为true）
					g_grid[y][x] = true;
					g_isDragging = true;
					g_dragValue = true;
					SetCapture(hWnd);
					RECT r = {
						offX + x * cellSize, offY + y * cellSize, offX + (x + 1) * cellSize, offY + (y + 1) * cellSize
					};
					InflateRect(&r, 1, 1);
					InvalidateRect(hWnd, &r, FALSE);
				}
			}
			break;
		}

	// 鼠标移动：如果左键按下处于拖拽状态，则绘制路径上的格子
	case WM_MOUSEMOVE:
		{
			// 隐藏重置提示并取消定时器
			if (g_showResetTip && (g_isDragging || g_isRightDragging))
			{
				g_showResetTip = false;
				if (g_tipTimerId != 0)
				{
					KillTimer(hWnd, 2);
					g_tipTimerId = 0;
				}
				InvalidateRect(hWnd, nullptr, FALSE); // 立即重绘隐藏提示
			}

			if (g_isDragging || g_isRightDragging)
			{
				int px = LOWORD(lParam);
				int py = HIWORD(lParam);
				int cellSize, offX, offY, gridWpx, gridHpx;
				CalcLayout(cellSize, offX, offY, gridWpx, gridHpx);
				if (px >= offX && px < offX + gridWpx && py >= offY && py < offY + gridHpx)
				{
					int x = (px - offX) / cellSize;
					int y = (py - offY) / cellSize;
					if (x >= 0 && x < g_gridWidth && y >= 0 && y < g_gridHeight)
					{
						bool target = g_isDragging; // 左键拖拽始终染色，右键拖拽始终擦除
						if (g_grid[y][x] != target)
						{
							g_grid[y][x] = target;
							RECT r = {
								offX + x * cellSize, offY + y * cellSize, offX + (x + 1) * cellSize,
								offY + (y + 1) * cellSize
							};
							InflateRect(&r, 1, 1);
							InvalidateRect(hWnd, &r, FALSE);
						}
					}
				}
			}
			break;
		}

	// 鼠标左键抬起：结束拖拽，释放捕获
	case WM_LBUTTONUP:
		{
			if (g_isDragging)
			{
				g_isDragging = false;
				if (!g_isRightDragging) ReleaseCapture();
			}
			break;
		}

	// 鼠标右键按下：进入擦除拖拽模式
	case WM_RBUTTONDOWN:
		{
			int px = LOWORD(lParam);
			int py = HIWORD(lParam);
			int cellSize, offX, offY, gridWpx, gridHpx;
			CalcLayout(cellSize, offX, offY, gridWpx, gridHpx);
			// 先检查是否右键点击了按钮（右键：减少）
			if (PtInRect(&g_rowsBtnRect, POINT{px, py}))
			{
				ResizeGrid(g_gridWidth, g_gridHeight - 1);
				InvalidateRect(hWnd, nullptr, TRUE);
				UpdateWindowTitle(hWnd);
				break;
			}
			if (PtInRect(&g_colsBtnRect, POINT{px, py}))
			{
				ResizeGrid(g_gridWidth - 1, g_gridHeight);
				InvalidateRect(hWnd, nullptr, TRUE);
				UpdateWindowTitle(hWnd);
				break;
			}
			if (px >= offX && px < offX + gridWpx && py >= offY && py < offY + gridHpx)
			{
				int x = (px - offX) / cellSize;
				int y = (py - offY) / cellSize;
				if (x >= 0 && x < g_gridWidth && y >= 0 && y < g_gridHeight)
				{
					// 右键按下将该格设为死，并进入右键拖拽模式
					g_grid[y][x] = false;
					g_isRightDragging = true;
					SetCapture(hWnd);
					RECT r = {
						offX + x * cellSize, offY + y * cellSize, offX + (x + 1) * cellSize, offY + (y + 1) * cellSize
					};
					InflateRect(&r, 1, 1);
					InvalidateRect(hWnd, &r, FALSE);
				}
			}
			break;
		}

	// 鼠标右键抬起：结束右键拖拽，释放捕获（若左键未拖拽）
	case WM_RBUTTONUP:
		{
			if (g_isRightDragging)
			{
				g_isRightDragging = false;
				if (!g_isDragging) ReleaseCapture();
			}
			break;
		}

	// 键盘按键消息
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_SPACE: // 开始/暂停
			g_isRunning = !g_isRunning;
			if (g_isRunning)
			{
				RestartTimer(hWnd);
			}
			else
			{
				KillTimer(hWnd, g_timerId);
			}
			UpdateWindowTitle(hWnd); // 更新标题栏（显示运行/暂停状态）
			break;
		case 'R': // 重置网格
			g_isRunning = false;
			KillTimer(hWnd, g_timerId);
			// 如果已有提示定时器，先取消
			if (g_tipTimerId != 0)
			{
				KillTimer(hWnd, 2);
			}
			g_tipTimerId = ResetGrid(hWnd); // 保存新的定时器ID
			InvalidateRect(hWnd, nullptr, FALSE);
			UpdateWindowTitle(hWnd);
			break;
		case 'G': // 生成随机初始状态
			g_isRunning = false;
			KillTimer(hWnd, g_timerId);
			InitGrid();
			InvalidateRect(hWnd, nullptr, FALSE);
			UpdateWindowTitle(hWnd); // 更新标题栏（暂停状态）
			break;
		case VK_ADD: // + 键（小键盘）：加速
		case 0xBB: // + 键（主键盘）：加速
			if (g_updateInterval > MIN_INTERVAL)
			{
				g_updateInterval -= SPEED_STEP;
				if (g_isRunning) RestartTimer(hWnd);
				UpdateWindowTitle(hWnd); // 实时更新速度提示
			}
			break;


		case VK_SUBTRACT: // - 键（小键盘）：减速
		case 0xBD: // - 键（主键盘）：减速
			if (g_updateInterval < MAX_INTERVAL)
			{
				g_updateInterval += SPEED_STEP;
				if (g_isRunning) RestartTimer(hWnd);
				UpdateWindowTitle(hWnd); // 实时更新速度提示
			}
			break;
		case VK_ESCAPE: // 退出程序
			DestroyWindow(hWnd);
			break;
		}
		break;

	// 命令消息（处理 Apply 按钮）
	case WM_COMMAND:
		{
			int id = LOWORD(wParam);
			int code = HIWORD(wParam);
			if (id == ID_APPLY_BTN && code == BN_CLICKED)
			{
				// 读取编辑框内容
				TCHAR buf[64];
				int newRows = g_gridHeight;
				int newCols = g_gridWidth;
				if (g_hRowsEdit)
				{
					GetWindowText(g_hRowsEdit, buf, _countof(buf));
					int v = _ttoi(buf);
					if (v > 0) newRows = v;
				}
				if (g_hColsEdit)
				{
					GetWindowText(g_hColsEdit, buf, _countof(buf));
					int v = _ttoi(buf);
					if (v > 0) newCols = v;
				}

				// 限制范围
				if (newCols < 4) newCols = 4;
				if (newRows < 4) newRows = 4;
				if (newCols > 120) newCols = 120;
				if (newRows > 80) newRows = 80;

				// 调整网格并更新窗口大小以匹配每单元像素大小不变
				ResizeGrid(newCols, newRows);

				// 计算所需客户区大小：网格像素 + 状态栏 + 顶部控件高度
				int gridWpx = CELL_SIZE * g_gridWidth;
				int gridHpx = CELL_SIZE * g_gridHeight;
				int topControlsH = 44; // 为 Edit/Button 留出空间
				// 包含左侧固定面板宽度
				int desiredClientW = LEFT_PANEL_WIDTH + gridWpx + 20; // 左侧面板 + 网格 + 两侧边距
				int desiredClientH = gridHpx + STATUS_BAR_HEIGHT + topControlsH + 10;

				// 保证窗口在列数较小时不会过窄：当列数小于40时，使用40列对应的最小客户端宽度
				constexpr int MIN_VISIBLE_COLS = 40;
				int minGridWpx = CELL_SIZE * MIN_VISIBLE_COLS;
				int minClientW = LEFT_PANEL_WIDTH + minGridWpx + 20; // 包含左侧面板宽度
				if (desiredClientW < minClientW) desiredClientW = minClientW;

				// 保证窗口在行数较小时不会过矮：当行数小于40时，使用40行对应的最小客户端高度
				constexpr int MIN_VISIBLE_ROWS = 40;
				int minGridHpx = CELL_SIZE * MIN_VISIBLE_ROWS;
				int minClientH = minGridHpx + STATUS_BAR_HEIGHT + topControlsH + 10;
				if (desiredClientH < minClientH) desiredClientH = minClientH;

				// 计算窗口外框尺寸
				RECT wr = {0, 0, desiredClientW, desiredClientH};
				AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, FALSE);
				int winW = wr.right - wr.left;
				int winH = wr.bottom - wr.top;

				// 调整窗口大小（保持位置不变）
				SetWindowPos(hWnd, nullptr, 0, 0, winW, winH, SWP_NOMOVE | SWP_NOZORDER);

				// 更新客户端尺寸缓存并重新布局控件
				RECT client;
				GetClientRect(hWnd, &client);
				g_clientWidth = client.right - client.left;
				g_clientHeight = client.bottom - client.top;
				LayoutControls(hWnd);
				InvalidateRect(hWnd, nullptr, TRUE);
				UpdateWindowTitle(hWnd);
				// 把焦点回到主窗口，这样空格键控制开始/暂停
				SetFocus(hWnd);
			}
			break;
		}

	// 绘制自绘按钮等
	case WM_DRAWITEM:
		{
			auto pdis = (LPDRAWITEMSTRUCT)lParam;
			if (pdis && pdis->CtlID == ID_APPLY_BTN)
			{
				HDC hdc = pdis->hDC;
				RECT rc = pdis->rcItem;
				// 背景色，hover 更亮，pressed 更暗
				COLORREF base = RGB(40, 120, 200);
				COLORREF hoverCol = RGB(70, 150, 230);
				COLORREF pressCol = RGB(20, 90, 170);
				COLORREF fill = base;
				if (pdis->itemState & ODS_SELECTED) fill = pressCol;
				else if (g_applyHover) fill = hoverCol;
				HBRUSH hbr = CreateSolidBrush(fill);
				FillRect(hdc, &rc, hbr);
				DeleteObject(hbr);
				// 边框
				HPEN hPen = CreatePen(PS_SOLID, 1, RGB(15, 60, 120));
				auto hOld = static_cast<HPEN>(SelectObject(hdc, hPen));
				MoveToEx(hdc, rc.left, rc.top, nullptr);
				LineTo(hdc, rc.right - 1, rc.top);
				LineTo(hdc, rc.right - 1, rc.bottom - 1);
				LineTo(hdc, rc.left, rc.bottom - 1);
				LineTo(hdc, rc.left, rc.top);
				SelectObject(hdc, hOld);
				DeleteObject(hPen);
				// 文本（居中，中文“应用”）
				auto hOldF = static_cast<HFONT>(SelectObject(hdc, g_hBtnFont ? g_hBtnFont : g_hTipFont));
				SetTextColor(hdc, RGB(255, 255, 255));
				SetBkMode(hdc, TRANSPARENT);
				RECT txt = rc;
				DrawText(hdc, TEXT("应用"), -1, &txt, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				SelectObject(hdc, hOldF);
				return TRUE;
			}
			break;
		}

	// 窗口关闭消息
	case WM_CLOSE:
		if (MessageBox(hWnd, TEXT("确定退出吗？"), TEXT("提示"), MB_YESNO) == IDYES)
		{
			DestroyWindow(hWnd);
		}
		break;

	// 窗口销毁消息
	case WM_DESTROY:
		KillTimer(hWnd, g_timerId);
		if (g_tipTimerId != 0)
		{
			KillTimer(hWnd, 2);
		}
		PostQuitMessage(0);
		break;

	// 其他消息默认处理
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

// 初始化随机细胞网格（40%存活率）
void InitGrid()
{
	srand(static_cast<unsigned int>(time(nullptr)));
	// 根据运行时配置尺寸分配网格
	g_grid.assign(g_gridHeight, std::vector<bool>(g_gridWidth, false));
	g_nextGrid.assign(g_gridHeight, std::vector<bool>(g_gridWidth, false));

	for (int y = 0; y < g_gridHeight; y++)
	{
		for (int x = 0; x < g_gridWidth; x++)
		{
			g_grid[y][x] = (rand() % 10 < 4);
			g_nextGrid[y][x] = false;
		}
	}
}

// 绘制细胞网格（含重置提示，移除原窗口内速度提示）
void DrawGrid(HDC hdc, const RECT* pDirty)
{
	SetBkMode(hdc, TRANSPARENT);

	// 计算布局（单元大小与偏移）
	int cellSize, offX, offY, gridWpx, gridHpx;
	CalcLayout(cellSize, offX, offY, gridWpx, gridHpx);

	// 计算需要重绘的细胞范围（基于像素脏矩形映射到单元坐标）
	int xStart = 0, xEnd = g_gridWidth, yStart = 0, yEnd = g_gridHeight;
	if (pDirty)
	{
		xStart = (pDirty->left - offX) / cellSize;
		xEnd = (pDirty->right - offX + cellSize - 1) / cellSize;
		yStart = (pDirty->top - offY) / cellSize;
		yEnd = (pDirty->bottom - offY + cellSize - 1) / cellSize;
		if (xStart < 0) xStart = 0;
		if (yStart < 0) yStart = 0;
		if (xEnd > g_gridWidth) xEnd = g_gridWidth;
		if (yEnd > g_gridHeight) yEnd = g_gridHeight;
	}

	// 填充每个单元（先填充颜色，不画边框），坐标基于偏移
	for (int y = yStart; y < yEnd && y < g_gridHeight; y++)
	{
		for (int x = xStart; x < xEnd && x < g_gridWidth; x++)
		{
			int left = offX + x * cellSize;
			int top = offY + y * cellSize;
			int right = left + cellSize;
			int bottom = top + cellSize;
			RECT cell = {left, top, right, bottom};
			FillRect(hdc, &cell, g_grid[y][x] ? g_hAliveBrush : g_hDeadBrush);
		}
	}

	// 绘制细线网格（限于网格区域）
	auto hOldPen = static_cast<HPEN>(SelectObject(hdc, g_hGridPen));
	for (int xi = 0; xi <= g_gridWidth; xi++)
	{
		int xpos = offX + xi * cellSize;
		MoveToEx(hdc, xpos, offY, nullptr);
		LineTo(hdc, xpos, offY + gridHpx);
	}
	for (int yi = 0; yi <= g_gridHeight; yi++)
	{
		int ypos = offY + yi * cellSize;
		MoveToEx(hdc, offX, ypos, nullptr);
		LineTo(hdc, offX + gridWpx, ypos);
	}
	SelectObject(hdc, hOldPen);

	// 左侧固定面板：绘制背景并显示快捷键说明（固定在窗口左侧）
	RECT leftPanel = {0, 0, LEFT_PANEL_WIDTH, g_clientHeight};
	FillRect(hdc, &leftPanel, g_hLeftPanelBrush ? g_hLeftPanelBrush : g_hTipBrush);
	// 绘制左侧面板的快捷键：左侧为 key(深蓝色, 加粗)，右侧为描述(深灰色)
	SetBkMode(hdc, TRANSPARENT);
	int panelPaddingX = 10;
	int panelPaddingY = 12;
	int lineH = 24;
	struct Shortcut
	{
		const TCHAR* key;
		const TCHAR* desc;
	};
	Shortcut sc[] = {
		{TEXT("空格"), TEXT("开始/暂停")},
		{TEXT("R"), TEXT("重置")},
		{TEXT("G"), TEXT("随机")},
		{TEXT("+ / -"), TEXT("加速/减速")},
		{TEXT("ESC"), TEXT("退出")}
	};
	int keyColW = 56; // key 列宽
	for (int i = 0; i < static_cast<int>(sizeof(sc) / sizeof(sc[0])); ++i)
	{
		int y = panelPaddingY + i * lineH;
		RECT keyR = {panelPaddingX, y, panelPaddingX + keyColW, y + lineH};
		RECT descR = {panelPaddingX + keyColW + 8, y, LEFT_PANEL_WIDTH - 12, y + lineH};

		auto hOld = static_cast<HFONT>(SelectObject(hdc, g_hLeftKeyFont ? g_hLeftKeyFont : g_hTipFont));
		SetTextColor(hdc, RGB(0, 102, 204)); // 深蓝色 key
		DrawText(hdc, sc[i].key, -1, &keyR, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		SelectObject(hdc, g_hLeftDescFont ? g_hLeftDescFont : g_hTipFont);
		SetTextColor(hdc, RGB(70, 80, 90)); // 深灰色描述
		DrawText(hdc, sc[i].desc, -1, &descR, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_WORDBREAK);
		SelectObject(hdc, hOld);
	}

	// 将标题放置到左侧面板靠底部的位置（在状态栏上方）
	int titleH = 36;
	int titleMargin = 12;
	RECT titleRect = {
		0, g_clientHeight - STATUS_BAR_HEIGHT - titleH - titleMargin, LEFT_PANEL_WIDTH,
		g_clientHeight - STATUS_BAR_HEIGHT - titleMargin
	};
	FillRect(hdc, &titleRect, g_hTipBrush);
	auto hOldFont = static_cast<HFONT>(SelectObject(hdc, g_hTitleFont));
	SetTextColor(hdc, RGB(34, 40, 49));
	DrawText(hdc, TEXT("生命游戏"), -1, &titleRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	SelectObject(hdc, hOldFont);

	// 绘制重置提示（持续2秒），居中显示带边框的提示框（相对于网格区域）
	if (g_showResetTip)
	{
		auto tip = TEXT("网格已重置！按 G 生成随机细胞");
		auto hOld = static_cast<HFONT>(SelectObject(hdc, g_hTipFont));
		RECT measure = {0, 0, 0, 0};
		DrawText(hdc, tip, -1, &measure, DT_CALCRECT | DT_SINGLELINE);
		int tw = measure.right - measure.left + 20;
		int th = measure.bottom - measure.top + 10;
		RECT box = {
			offX + (gridWpx - tw) / 2, offY + (gridHpx - th) / 2, offX + (gridWpx + tw) / 2,
			offY + (gridHpx + th) / 2
		};
		// 背景与边框
		FillRect(hdc, &box, g_hTipBrush);
		HPEN hPenTip = CreatePen(PS_SOLID, 1, RGB(200, 50, 50));
		auto hOld2 = static_cast<HPEN>(SelectObject(hdc, hPenTip));
		Rectangle(hdc, box.left, box.top, box.right, box.bottom);
		SetTextColor(hdc, RGB(200, 30, 30));
		DrawText(hdc, tip, -1, &box, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		SelectObject(hdc, hOld2);
		DeleteObject(hPenTip);
		SelectObject(hdc, hOld);
	}

	// 底部状态栏：背景 + 文字（位于整个客户区底部）
	RECT statusRect = {0, g_clientHeight - STATUS_BAR_HEIGHT, g_clientWidth, g_clientHeight};
	HBRUSH statusBg = CreateSolidBrush(RGB(230, 236, 240));
	FillRect(hdc, &statusRect, statusBg);
	SelectObject(hdc, g_hTipFont);
	SetTextColor(hdc, RGB(50, 60, 70));
	// 左侧状态信息
	TCHAR leftStatus[128];
	wsprintf(leftStatus, TEXT("状态: %s    速度: %d ms/帧"), g_isRunning ? TEXT("运行中") : TEXT("已暂停"), g_updateInterval);
	RECT leftRect = {8, g_clientHeight - STATUS_BAR_HEIGHT, g_clientWidth / 2, g_clientHeight};
	DrawText(hdc, leftStatus, -1, &leftRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	// 右侧显示网格尺寸和快捷键
	TCHAR rightStatus[128];
	wsprintf(rightStatus, TEXT("网格: %d x %d    空格: 开始/暂停"), g_gridWidth, g_gridHeight);
	RECT rightRect = {g_clientWidth / 2, g_clientHeight - STATUS_BAR_HEIGHT, g_clientWidth - 8, g_clientHeight};
	DrawText(hdc, rightStatus, -1, &rightRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	DeleteObject(statusBg);
}

// 计算细胞（x列，y行）周围存活邻居数
int CountNeighbors(int x, int y)
{
	int count = 0;
	// 遍历8个邻居方向
	for (int dy = -1; dy <= 1; dy++)
	{
		for (int dx = -1; dx <= 1; dx++)
		{
			if (dx == 0 && dy == 0) continue; // 跳过自身

			// 循环边界处理
			int nx = (x + dx + g_gridWidth) % g_gridWidth;
			int ny = (y + dy + g_gridHeight) % g_gridHeight;

			if (g_grid[ny][nx]) count++;
		}
	}
	return count;
}

// 子类过程：Rows Edit
LRESULT CALLBACK RowsEditProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_KEYDOWN && wp == VK_RETURN)
	{
		// 触发 Apply 按钮点击
		if (g_hApplyBtn)
			SendMessage(g_hApplyBtn, BM_CLICK, 0, 0);
		return 0;
	}
	return CallWindowProc(g_oldRowsProc, hwnd, msg, wp, lp);
}

// 子类过程：Cols Edit
LRESULT CALLBACK ColsEditProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_KEYDOWN && wp == VK_RETURN)
	{
		if (g_hApplyBtn)
			SendMessage(g_hApplyBtn, BM_CLICK, 0, 0);
		return 0;
	}
	return CallWindowProc(g_oldColsProc, hwnd, msg, wp, lp);
}

// 子类过程：Apply 按钮（追踪鼠标悬停）
LRESULT CALLBACK ApplyBtnProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_MOUSEMOVE)
	{
		if (!g_applyHover)
		{
			g_applyHover = true;
			InvalidateRect(GetParent(hwnd), nullptr, TRUE);
		}
		TRACKMOUSEEVENT tme = {sizeof(tme), TME_LEAVE, hwnd, 0};
		TrackMouseEvent(&tme);
	}
	else if (msg == WM_MOUSELEAVE)
	{
		if (g_applyHover)
		{
			g_applyHover = false;
			InvalidateRect(GetParent(hwnd), nullptr, TRUE);
		}
	}
	return CallWindowProc(g_oldApplyBtnProc, hwnd, msg, wp, lp);
}

// 计算下一代细胞状态（生命游戏核心规则）
void UpdateGrid()
{
	for (int y = 0; y < g_gridHeight; y++)
	{
		for (int x = 0; x < g_gridWidth; x++)
		{
			int neighbors = CountNeighbors(x, y);
			bool currentState = g_grid[y][x];
			g_nextGrid[y][x] = currentState ? (neighbors == 2 || neighbors == 3) : (neighbors == 3);
		}
	}

	// 更新当前网格状态
	for (int y = 0; y < g_gridHeight; y++)
	{
		for (int x = 0; x < g_gridWidth; x++)
		{
			g_grid[y][x] = g_nextGrid[y][x];
		}
	}
}

// 重置网格（所有细胞设为死亡）
UINT_PTR ResetGrid(HWND hWnd) // 返回定时器ID
{
	// 若网格尚未分配则分配
	if (g_grid.size() != static_cast<size_t>(g_gridHeight) || g_grid.empty() || g_grid[0].size() != static_cast<size_t>(
		g_gridWidth))
	{
		g_grid.assign(g_gridHeight, std::vector<bool>(g_gridWidth, false));
		g_nextGrid.assign(g_gridHeight, std::vector<bool>(g_gridWidth, false));
	}
	for (int y = 0; y < g_gridHeight; y++)
	{
		for (int x = 0; x < g_gridWidth; x++)
		{
			g_grid[y][x] = false;
			g_nextGrid[y][x] = false;
		}
	}

	// 启用重置提示
	g_showResetTip = true;
	g_resetTipTime = GetTickCount64();

	// 设置一个2秒后隐藏提示的定时器
	return SetTimer(hWnd, 2, 2000, nullptr); // 使用不同的定时器ID
}

// 重启定时器（适配速度调整，避免多个定时器冲突）
void RestartTimer(HWND hWnd)
{
	if (g_timerId != 0)
	{
		KillTimer(hWnd, g_timerId); // 先停止旧定时器
	}
	g_timerId = SetTimer(hWnd, 1, g_updateInterval, nullptr); // 启动新定时器（使用当前速度）
}

// 更新标题栏（含运行状态、当前速度、所有操作快捷键）
void UpdateWindowTitle(HWND hWnd)
{
	TCHAR title[200];
	// 拼接标题内容：运行状态 + 当前速度 + 操作提示
	wsprintf(title,
	         TEXT("生命游戏 - %s | 速度：%d ms/帧"),
	         g_isRunning ? TEXT("运行中") : TEXT("已暂停"), // 显示当前运行状态
	         g_updateInterval); // 显示当前速度
	SetWindowText(hWnd, title); // 设置窗口标题
}
