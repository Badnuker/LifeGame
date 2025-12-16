#include "PatternPreview.h"
#include "PatternLibrary.h" // Ensure PatternLibrary is included for ParseRLE

// 全局指针，用于在静态 WndProc 中访问实例
// 注意：这种方式不支持多个预览窗口实例，但在本应用中只有一个预览窗口，所以是可以接受的简化
PatternPreview *g_pPreview = nullptr;

PatternPreview::PatternPreview()
    : m_hWnd(nullptr), m_pCurrentPattern(nullptr),
      m_hBgBrush(nullptr), m_hCellBrush(nullptr) {
}

PatternPreview::~PatternPreview() {
    // 释放 GDI 资源
    if (m_hBgBrush) DeleteObject(m_hBgBrush);
    if (m_hCellBrush) DeleteObject(m_hCellBrush);
}

// 初始化控件：注册窗口类并创建窗口
bool PatternPreview::Initialize(HINSTANCE hInstance, HWND hParent, int x, int y, int w, int h) {
    g_pPreview = this;

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = TEXT("LifeGamePatternPreview");
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    // 注册自定义窗口类
    RegisterClass(&wc);

    // 创建子窗口 (WS_CHILD)
    m_hWnd = CreateWindowEx(WS_EX_CLIENTEDGE, // 带边框样式
                            TEXT("LifeGamePatternPreview"), nullptr,
                            WS_CHILD | WS_VISIBLE,
                            x, y, w, h,
                            hParent, nullptr, hInstance, this);

    // 创建绘图资源
    m_hBgBrush = CreateSolidBrush(RGB(30, 34, 40)); // 深灰背景
    m_hCellBrush = CreateSolidBrush(RGB(0, 255, 255)); // 青色细胞

    return (m_hWnd != nullptr);
}

// 设置当前图案并解析 RLE 数据
void PatternPreview::SetPattern(const PatternData *p) {
    m_pCurrentPattern = p;
    m_previewGrid.clear();

    if (p) {
        // 解析 RLE
        PatternLibrary lib; // 创建临时实例来调用解析函数
        lib.ParseRLE(p->rleString, m_previewGrid);
    }

    Update(); // 触发重绘
}

// 调整窗口位置和大小
void PatternPreview::Move(int x, int y, int w, int h) {
    if (m_hWnd) {
        MoveWindow(m_hWnd, x, y, w, h, TRUE);
    }
}

// 触发重绘
void PatternPreview::Update() {
    if (m_hWnd) InvalidateRect(m_hWnd, nullptr, TRUE);
}

// 静态窗口过程
LRESULT CALLBACK PatternPreview::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
    PatternPreview *pThis = g_pPreview; // 简单单例，不支持多个预览窗口

    if (msg == WM_PAINT && pThis) {
        pThis->OnPaint(hWnd);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wp, lp);
}

// 绘图逻辑
void PatternPreview::OnPaint(HWND hWnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);

    RECT rc;
    GetClientRect(hWnd, &rc);
    FillRect(hdc, &rc, m_hBgBrush); // 填充背景

    if (!m_previewGrid.empty()) {
        int rows = static_cast<int>(m_previewGrid.size());
        int cols = static_cast<int>(m_previewGrid[0].size());

        if (rows > 0 && cols > 0) {
            // 计算自适应缩放比例
            int w = rc.right - rc.left;
            int h = rc.bottom - rc.top;

            int cellW = w / cols;
            int cellH = h / rows;
            // 取较小值以保持纵横比
            int cellSize = (cellW < cellH) ? cellW : cellH;

            // 对于超大图案，如果 cellSize 为 0，则使用浮点数计算
            if (cellSize < 1) {
                // 超大图案：绘制缩略图
                float scaleX = static_cast<float>(w) / cols;
                float scaleY = static_cast<float>(h) / rows;
                float scale = (scaleX < scaleY) ? scaleX : scaleY;

                int gridW = static_cast<int>(cols * scale);
                int gridH = static_cast<int>(rows * scale);
                int offX = (w - gridW) / 2;
                int offY = (h - gridH) / 2;

                // 绘制缩略图：每个像素代表多个细胞
                for (int y = 0; y < rows; ++y) {
                    for (int x = 0; x < cols; ++x) {
                        if (m_previewGrid[y][x]) {
                            int px = offX + static_cast<int>(x * scale);
                            int py = offY + static_cast<int>(y * scale);
                            // 至少绘制1个像素
                            RECT cell = {px, py, px + 1, py + 1};
                            if (scale >= 0.5f) {
                                cell.right = px + static_cast<int>(scale);
                                cell.bottom = py + static_cast<int>(scale);
                                if (cell.right <= cell.left) cell.right = cell.left + 1;
                                if (cell.bottom <= cell.top) cell.bottom = cell.top + 1;
                            }
                            FillRect(hdc, &cell, m_hCellBrush);
                        }
                    }
                }
            } else {
                // 正常大小图案
                if (cellSize > 20) cellSize = 20; // 最大限制

                // 计算居中偏移量
                int gridW = cols * cellSize;
                int gridH = rows * cellSize;
                int offX = (w - gridW) / 2;
                int offY = (h - gridH) / 2;

                // 绘制网格
                for (int y = 0; y < rows; ++y) {
                    for (int x = 0; x < cols; ++x) {
                        if (m_previewGrid[y][x]) {
                            RECT cell = {
                                offX + x * cellSize,
                                offY + y * cellSize,
                                offX + (x + 1) * cellSize,
                                offY + (y + 1) * cellSize
                            };
                            // 稍微缩小一点，留出间隔 (Grid Gap)
                            if (cellSize > 2) {
                                cell.right--;
                                cell.bottom--;
                            }
                            FillRect(hdc, &cell, m_hCellBrush);
                        }
                    }
                }
            }
        }
    } else if (m_pCurrentPattern) {
        // 如果没有网格数据但有名字 (比如"单点绘制"这种特殊模式)
        // 显示文字提示
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(150, 150, 150));
        DrawText(hdc, m_pCurrentPattern->name.c_str(), -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    EndPaint(hWnd, &ps);
}
