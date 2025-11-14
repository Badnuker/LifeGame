#include <windows.h>
#include <time.h>

// 全局常量定义
const int CELL_SIZE = 10; // 细胞像素大小
const int GRID_WIDTH = 80; // 网格列数
const int GRID_HEIGHT = 60; // 网格行数
const int WINDOW_WIDTH = CELL_SIZE * GRID_WIDTH; // 窗口宽度
const int WINDOW_HEIGHT = CELL_SIZE * GRID_HEIGHT; // 窗口高度
const int MIN_INTERVAL = 50; // 最小更新间隔（最快速度，50毫秒/帧）
const int MAX_INTERVAL = 500; // 最大更新间隔（最慢速度，500毫秒/帧）
const int SPEED_STEP = 50; // 速度调整步长（每次加减50毫秒）
int g_updateInterval = 100; // 游戏更新间隔（默认100毫秒，可动态调整）

// 全局变量定义
bool g_grid[GRID_HEIGHT][GRID_WIDTH] = { false }; // 当前细胞状态（false=死亡，true=存活）
bool g_nextGrid[GRID_HEIGHT][GRID_WIDTH] = { false }; // 下一代细胞状态
bool g_isRunning = false; // 游戏运行状态（false=暂停，true=运行）
UINT_PTR g_timerId = 0; // 定时器ID
bool g_showResetTip = false; // 重置提示显示标记
ULONGLONG g_resetTipTime = 0; // 重置提示显示时间

// 全局 GDI 对象
HBRUSH g_hBlackBrush = NULL;
HBRUSH g_hWhiteBrush = NULL;
HPEN g_hGrayPen = NULL;

// 函数声明
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitGrid(); // 初始化随机细胞网格
void DrawGrid(HDC hdc, const RECT* pDirty); // 绘制细胞网格（含重置提示）
int CountNeighbors(int x, int y); // 计算细胞周围存活邻居数
void UpdateGrid(); // 计算下一代细胞状态
void ResetGrid(); // 重置网格（全细胞死亡）
void RestartTimer(HWND hWnd); // 重启定时器（适配速度调整）
void UpdateWindowTitle(HWND hWnd); // 更新标题栏（含速度提示）

// 程序入口
int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nCmdShow
)
{
	// 注册窗口类
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = TEXT("LifeGameWindow");
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;

	// 初始化 GDI 对象（只创建一次，程序退出时销毁）
	g_hBlackBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
	g_hWhiteBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
	g_hGrayPen = CreatePen(PS_SOLID, 0, RGB(200, 200, 200));

	if (!RegisterClass(&wc))
	{
		MessageBox(NULL, TEXT("窗口类注册失败！"), TEXT("错误"), MB_ICONERROR);
		return 1;
	}

	// 计算窗口实际大小（适配边框和标题栏）
	RECT windowRect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
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
		MessageBox(NULL, TEXT("窗口创建失败！"), TEXT("错误"), MB_ICONERROR);
		return 1;
	}

	// 初始化细胞网格、更新标题栏并显示窗口
	InitGrid();
	UpdateWindowTitle(hWnd); // 首次设置标题栏（含初始速度）
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// 消息循环
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// 程序退出时销毁 GDI 对象
	DeleteObject(g_hGrayPen);

	return (int)msg.wParam;
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

		// 双缓冲
		HDC memDC = CreateCompatibleDC(hdc);
		HBITMAP hbm = CreateCompatibleBitmap(hdc, WINDOW_WIDTH, WINDOW_HEIGHT);
		HBITMAP hOldBmp = (HBITMAP)SelectObject(memDC, hbm);

		// 用白色填充背景
		HBRUSH hWhite = g_hWhiteBrush;
		RECT rc = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
		FillRect(memDC, &rc, hWhite);

		//只重绘脏矩形区域
		DrawGrid(memDC, &ps.rcPaint);

		//只将脏区域 blit 到窗口
		BitBlt(hdc,
			ps.rcPaint.left, ps.rcPaint.top,
			ps.rcPaint.right - ps.rcPaint.left,
			ps.rcPaint.bottom - ps.rcPaint.top,
			memDC,
			ps.rcPaint.left, ps.rcPaint.top,
			SRCCOPY);

		SelectObject(memDC, hOldBmp);
		DeleteObject(hbm);
		DeleteDC(memDC);
		EndPaint(hWnd, &ps);
		break;
	}

	// 防止默认擦除背景，配合双缓冲可以减少闪烁
	case WM_ERASEBKGND:
		return 1;

	// 定时器消息（控制游戏刷新）
	case WM_TIMER:
		if (g_isRunning)
		{
			UpdateGrid();
			InvalidateRect(hWnd, NULL, FALSE); // 不擦除背景，已由双缓冲处理
		}
		break;

	// 鼠标左键点击（切换细胞状态）
	case WM_LBUTTONDOWN:
	{
		int x = LOWORD(lParam) / CELL_SIZE;
		int y = HIWORD(lParam) / CELL_SIZE;
		if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT)
		{
			g_grid[y][x] = !g_grid[y][x];
			InvalidateRect(hWnd, NULL, FALSE);
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
			ResetGrid();
			InvalidateRect(hWnd, NULL, FALSE);
			UpdateWindowTitle(hWnd); // 更新标题栏（暂停状态）
			break;
		case 'G': // 生成随机初始状态
			g_isRunning = false;
			KillTimer(hWnd, g_timerId);
			InitGrid();
			InvalidateRect(hWnd, NULL, FALSE);
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
	srand((unsigned int)time(NULL));
	for (int y = 0; y < GRID_HEIGHT; y++)
	{
		for (int x = 0; x < GRID_WIDTH; x++)
		{
			g_grid[y][x] = (rand() % 10 < 4) ? true : false;
		}
	}

	// 清空下一代网格
	for (int y = 0; y < GRID_HEIGHT; y++)
	{
		for (int x = 0; x < GRID_WIDTH; x++)
		{
			g_nextGrid[y][x] = false;
		}
	}
}

// 绘制细胞网格（含重置提示，移除原窗口内速度提示）
void DrawGrid(HDC hdc, const RECT* pDirty)
{
	SetBkMode(hdc, TRANSPARENT);
	SelectObject(hdc, g_hGrayPen);

	//计算需要重绘的细胞范围
	int xStart = pDirty ? pDirty->left / CELL_SIZE : 0;
	int xEnd = pDirty ? (pDirty->right + CELL_SIZE - 1) / CELL_SIZE : GRID_WIDTH;
	int yStart = pDirty ? pDirty->top / CELL_SIZE : 0;
	int yEnd = pDirty ? (pDirty->bottom + CELL_SIZE - 1) / CELL_SIZE : GRID_HEIGHT;

	for (int y = yStart; y < yEnd && y < GRID_HEIGHT; y++)
	{
		for (int x = xStart; x < xEnd && x < GRID_WIDTH; x++)
		{
			int left = x * CELL_SIZE;
			int top = y * CELL_SIZE;
			int right = left + CELL_SIZE;
			int bottom = top + CELL_SIZE;
			SelectObject(hdc, g_grid[y][x] ? g_hBlackBrush : g_hWhiteBrush);
			Rectangle(hdc, left, top, right, bottom);
		}
	}

	// 绘制重置提示文字（持续2秒）
	if (g_showResetTip)
	{
		if (GetTickCount64() - g_resetTipTime > 2000)
		{
			g_showResetTip = false;
		}
		else
		{
			SetTextColor(hdc, RGB(255, 0, 0));
			TextOut(hdc, WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2,
				TEXT("网格已重置！按 G 生成随机细胞"),
				lstrlen(TEXT("网格已重置！按 G 生成随机细胞")));
		}
	}
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
			int nx = (x + dx + GRID_WIDTH) % GRID_WIDTH;
			int ny = (y + dy + GRID_HEIGHT) % GRID_HEIGHT;

			if (g_grid[ny][nx]) count++;
		}
	}
	return count;
}

// 计算下一代细胞状态（生命游戏核心规则）
void UpdateGrid()
{
	for (int y = 0; y < GRID_HEIGHT; y++)
	{
		for (int x = 0; x < GRID_WIDTH; x++)
		{
			int neighbors = CountNeighbors(x, y);
			bool currentState = g_grid[y][x];

			// 存活细胞规则：邻居数2-3个保持存活，否则死亡
			// 死亡细胞规则：邻居数3个复活，否则保持死亡
			g_nextGrid[y][x] = currentState ? (neighbors == 2 || neighbors == 3) : (neighbors == 3);
		}
	}

	// 更新当前网格状态
	for (int y = 0; y < GRID_HEIGHT; y++)
	{
		for (int x = 0; x < GRID_WIDTH; x++)
		{
			g_grid[y][x] = g_nextGrid[y][x];
		}
	}
}

// 重置网格（所有细胞设为死亡）
void ResetGrid()
{
	for (int y = 0; y < GRID_HEIGHT; y++)
	{
		for (int x = 0; x < GRID_WIDTH; x++)
		{
			g_grid[y][x] = false;
			g_nextGrid[y][x] = false;
		}
	}

	// 启用重置提示
	g_showResetTip = true;
	g_resetTipTime = GetTickCount64();
}

// 重启定时器（适配速度调整，避免多个定时器冲突）
void RestartTimer(HWND hWnd)
{
	if (g_timerId != 0)
	{
		KillTimer(hWnd, g_timerId); // 先停止旧定时器
	}
	g_timerId = SetTimer(hWnd, 1, g_updateInterval, NULL); // 启动新定时器（使用当前速度）
}

// 更新标题栏（含运行状态、当前速度、所有操作快捷键）
void UpdateWindowTitle(HWND hWnd)
{
	TCHAR title[200];
	// 拼接标题内容：运行状态 + 当前速度 + 操作提示
	wsprintf(title,
		TEXT("生命游戏 - %s | 速度：%d ms/帧 | 空格开始/暂停 | R重置 | G随机 | +/-加速减速 | ESC退出"),
		g_isRunning ? TEXT("运行中") : TEXT("已暂停"), // 显示当前运行状态
		g_updateInterval); // 显示当前速度
	SetWindowText(hWnd, title); // 设置窗口标题
}
