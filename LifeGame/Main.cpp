#include <windows.h>
#include <time.h>

// 全局常量定义
const int CELL_SIZE = 10;         // 细胞像素大小
const int GRID_WIDTH = 80;        // 网格列数
const int GRID_HEIGHT = 60;       // 网格行数
const int WINDOW_WIDTH = CELL_SIZE * GRID_WIDTH;  // 窗口宽度
const int WINDOW_HEIGHT = CELL_SIZE * GRID_HEIGHT;// 窗口高度
const int UPDATE_INTERVAL = 100;  // 游戏更新间隔（毫秒）

// 全局变量定义
bool g_grid[GRID_HEIGHT][GRID_WIDTH] = { false };  // 当前细胞状态（false=死亡，true=存活）
bool g_nextGrid[GRID_HEIGHT][GRID_WIDTH] = { false };// 下一代细胞状态
bool g_isRunning = false;  // 游戏运行状态（false=暂停，true=运行）
UINT_PTR g_timerId = 0;    // 定时器ID
bool g_showResetTip = false; // 重置提示显示标记
ULONGLONG g_resetTipTime = 0;   // 重置提示显示时间

// 函数声明
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitGrid();                // 初始化随机细胞网格
void DrawGrid(HDC hdc);         // 绘制细胞网格
int CountNeighbors(int x, int y);// 计算细胞周围存活邻居数
void UpdateGrid();              // 计算下一代细胞状态
void ResetGrid();               // 重置网格（全细胞死亡）

// 程序入口
int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow
) {
    // 注册窗口类
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = TEXT("LifeGameWindow");
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)WHITE_BRUSH;

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, TEXT("窗口类注册失败！"), TEXT("错误"), MB_ICONERROR);
        return 1;
    }

    // 计算窗口实际大小（适配边框和标题栏）
    RECT windowRect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, FALSE);

    // 创建窗口
    HWND hWnd = CreateWindow(
        TEXT("LifeGameWindow"),
        TEXT("生命游戏 - 空格开始/暂停 | R重置 | G随机 | ESC退出"),
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        NULL, NULL, hInstance, NULL
    );

    if (!hWnd) {
        MessageBox(NULL, TEXT("窗口创建失败！"), TEXT("错误"), MB_ICONERROR);
        return 1;
    }

    // 初始化细胞网格并显示窗口
    InitGrid();
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // 消息循环
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

// 窗口过程函数（消息处理核心）
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        // 绘图消息
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        DrawGrid(hdc);
        EndPaint(hWnd, &ps);
        break;
    }

                 // 定时器消息（控制游戏刷新）
    case WM_TIMER:
        if (g_isRunning) {
            UpdateGrid();
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;

        // 鼠标左键点击（切换细胞状态）
    case WM_LBUTTONDOWN: {
        int x = LOWORD(lParam) / CELL_SIZE;
        int y = HIWORD(lParam) / CELL_SIZE;
        if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT) {
            g_grid[y][x] = !g_grid[y][x];
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;
    }

                       // 键盘按键消息
    case WM_KEYDOWN:
        switch (wParam) {
        case VK_SPACE:  // 开始/暂停
            g_isRunning = !g_isRunning;
            if (g_isRunning) {
                g_timerId = SetTimer(hWnd, 1, UPDATE_INTERVAL, NULL);
            }
            else {
                KillTimer(hWnd, g_timerId);
            }
            break;
        case 'R':  // 重置网格
            g_isRunning = false;
            KillTimer(hWnd, g_timerId);
            ResetGrid();
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        case 'G':  // 生成随机初始状态
            g_isRunning = false;
            KillTimer(hWnd, g_timerId);
            InitGrid();
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        case VK_ESCAPE:  // 退出程序
            DestroyWindow(hWnd);
            break;
        }
        break;

        // 窗口关闭消息
    case WM_CLOSE:
        if (MessageBox(hWnd, TEXT("确定退出吗？"), TEXT("提示"), MB_YESNO) == IDYES) {
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
void InitGrid() {
    srand((unsigned int)time(NULL));
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            g_grid[y][x] = (rand() % 10 < 4) ? true : false;
        }
    }

    // 清空下一代网格
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            g_nextGrid[y][x] = false;
        }
    }
}

// 绘制细胞网格
void DrawGrid(HDC hdc) {
    SetBkMode(hdc, TRANSPARENT);

    // 创建绘图工具
    HBRUSH hBlackBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
    HBRUSH hWhiteBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
    HPEN hGrayPen = CreatePen(PS_SOLID, 0, RGB(200, 200, 200));
    SelectObject(hdc, hGrayPen);

    // 绘制所有细胞
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            int left = x * CELL_SIZE;
            int top = y * CELL_SIZE;
            int right = left + CELL_SIZE;
            int bottom = top + CELL_SIZE;

            // 根据细胞状态选择笔刷
            SelectObject(hdc, g_grid[y][x] ? hBlackBrush : hWhiteBrush);
            Rectangle(hdc, left, top, right, bottom);
        }
    }

    // 绘制重置提示文字（持续2秒）
    if (g_showResetTip) {
        if (GetTickCount64() - g_resetTipTime > 2000) {
            g_showResetTip = false;
        }
        else {
            SetTextColor(hdc, RGB(255, 0, 0));
            TextOut(hdc, WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2,
                TEXT("网格已重置！按 G 生成随机细胞"),
                lstrlen(TEXT("网格已重置！按 G 生成随机细胞")));
        }
    }

    // 释放资源
    DeleteObject(hGrayPen);
}

// 计算细胞（x列，y行）周围存活邻居数
int CountNeighbors(int x, int y) {
    int count = 0;
    // 遍历8个邻居方向
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;  // 跳过自身

            // 循环边界处理
            int nx = (x + dx + GRID_WIDTH) % GRID_WIDTH;
            int ny = (y + dy + GRID_HEIGHT) % GRID_HEIGHT;

            if (g_grid[ny][nx]) count++;
        }
    }
    return count;
}

// 计算下一代细胞状态（生命游戏核心规则）
void UpdateGrid() {
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            int neighbors = CountNeighbors(x, y);
            bool currentState = g_grid[y][x];

            // 存活细胞规则：邻居数2-3个保持存活，否则死亡
            // 死亡细胞规则：邻居数3个复活，否则保持死亡
            g_nextGrid[y][x] = currentState ?
                (neighbors == 2 || neighbors == 3) :
                (neighbors == 3);
        }
    }

    // 更新当前网格状态
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            g_grid[y][x] = g_nextGrid[y][x];
        }
    }
}

// 重置网格（所有细胞设为死亡）
void ResetGrid() {
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            g_grid[y][x] = false;
            g_nextGrid[y][x] = false;
        }
    }

    // 启用重置提示
    g_showResetTip = true;
    g_resetTipTime = GetTickCount64();
}
