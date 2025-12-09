#include "HelpWindow.h"
#include <commctrl.h>

HelpWindow *g_pHelpWindow = nullptr;

HelpWindow::HelpWindow()
    : m_hWnd(nullptr), m_hList(nullptr), m_hZoomInBtn(nullptr), m_hZoomOutBtn(nullptr),
      m_currentPage(0), m_scrollY(0), m_contentHeight(0), m_fontScale(1.3f),
      m_hTitleFont(nullptr), m_hBodyFont(nullptr) {
    // 初始化帮助内容
    m_pages.push_back({
        L"新手指南",
        L"欢迎来到生命游戏！如果你是第一次接触，请尝试以下步骤：\n\n"
        L"1. 随机开始：\n"
        L"   按键盘上的 'G' 键，画布会随机生成细胞。按 'SPACE' (空格) 键开始演化，观察混乱如何变成有序。\n\n"
        L"2. 放置图案：\n"
        L"   在左侧面板的 'Pattern' 下拉框中选择 '滑翔机枪'。\n"
        L"   在画布空白处点击左键，放置它。按空格开始，你会看到它不断发射小滑翔机。\n\n"
        L"3. 绘制细胞：\n"
        L"   按 'R' 键清空画布。用鼠标左键在画布上随意画一些点，然后按空格观察它们如何变化。\n\n"
        L"4. 探索规则：\n"
        L"   在左侧 'Rule' 下拉框中切换到 'HighLife' 或 'Day & Night'，同样的图案会有完全不同的演化结果！"
    });

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
        L"演化规则详解",
        L"本程序内置了多种有趣的演化规则，以下是详细介绍：\n\n"
        L"1. Conway (经典): B3/S23\n   最经典的规则，平衡性极佳，拥有丰富的稳定态和飞船。\n\n"
        L"2. HighLife: B36/S23\n   类似经典规则，但6个邻居也能重生。特点是存在'复制子'，可以自我复制。\n\n"
        L"3. Day & Night: B3678/S34678\n   死活细胞性质对称。非常复杂，拥有类似经典规则的丰富结构。\n\n"
        L"4. Seeds (种子): B2/S\n   活细胞必死，只有2个邻居时出生。爆发性极强，瞬间填满屏幕。\n\n"
        L"5. Life without Death: B3/S012345678\n   细胞一旦活了就永远不死。用于生成类似迷宫或珊瑚的静态结构。\n\n"
        L"6. 34 Life: B34/S34\n   3或4个邻居出生或存活。早期被认为可能像经典规则一样有趣，但实际上较混乱。\n\n"
        L"7. Diamoeba: B35678/S5678\n   形成类似变形虫的动态图案，边界不断波动。\n\n"
        L"8. Maze (迷宫): B3/S12345\n   能够自动生成迷宫般的路径结构。\n\n"
        L"9. Coral (珊瑚): B3/S45678\n   生长缓慢，形成类似珊瑚的有机纹理。\n\n"
        L"10. Replicator: B1357/S1357\n   每个图案都会在演化过程中复制自身，形成分形结构。"
    });

    m_pages.push_back({
        L"操作指南",
        L"鼠标操作：\n"
        L"左键点击/拖动：绘制细胞 (使用当前选中的笔刷)\n"
        L"右键拖动：平移画布\n"
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
        L"- 振荡器 (Oscillators)：信号灯、脉冲星等，按周期变化。\n"
        L"- 飞船 (Spaceships)：滑翔机、太空船等，会移动。\n"
        L"- 枪 (Guns)：滑翔机枪、繁殖者，能不断发射滑翔机。\n\n"
        L"在左侧面板选择笔刷模式即可使用。"
    });

    m_pages.push_back({
        L"高级功能",
        L"1. 规则引擎：支持多种变体规则，如 HighLife (B36/S23), Day & Night (B3678/S34678) 等。\n"
        L"2. 统计图表：右下角实时显示种群数量变化曲线。\n"
        L"3. 文件系统：支持保存 (.life) 和加载存档，以及导出 RLE 格式。\n"
        L"4. 视觉设置：可自定义颜色、网格线、HUD 等外观。\n"
        L"5. 无限画布：支持向任意方向无限平移，探索广阔的演化空间。"
    });

    m_pages.push_back({
        L"关于",
        L"LifeGame v3.3 (Win32 GDI)\n\n"
        L"开发：Zhong yi, Liu qingxin, Dong kehong\n"
        L"技术栈：C++, Windows API (GDI)\n\n"
        L"本项目旨在展示高性能的细胞自动机模拟与现代化的 GDI 绘图技术。\n"
    });
}

HelpWindow::~HelpWindow() {
    if (m_hTitleFont) DeleteObject(m_hTitleFont);
    if (m_hBodyFont) DeleteObject(m_hBodyFont);
}

void HelpWindow::Show(HWND hParent) {
    if (m_hWnd) {
        SetForegroundWindow(m_hWnd);
        return;
    }

    g_pHelpWindow = this;

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = TEXT("LifeGameHelpWnd");
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(nullptr, IDI_INFORMATION);
    RegisterClass(&wc);

    int w = 1200; // 再次增大窗口宽度
    int h = 900; // 再次增大窗口高度
    int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

    m_hWnd = CreateWindow(TEXT("LifeGameHelpWnd"), TEXT("使用手册"),
                          WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, // 不可最大化
                          x, y, w, h,
                          hParent, nullptr, wc.hInstance, this);

    if (m_hWnd) {
        ShowWindow(m_hWnd, SW_SHOW);
        UpdateWindow(m_hWnd);
    }
}

LRESULT CALLBACK HelpWindow::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
    HelpWindow *pThis = g_pHelpWindow;
    if (msg == WM_DESTROY) {
        if (pThis) pThis->m_hWnd = nullptr;
        return 0;
    }

    if (pThis) {
        switch (msg) {
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
            case WM_MOUSEWHEEL:
                pThis->OnMouseWheel(hWnd, GET_WHEEL_DELTA_WPARAM(wp));
                return 0;
        }
    }
    return DefWindowProc(hWnd, msg, wp, lp);
}

void HelpWindow::OnCreate(HWND hWnd) {
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
    for (const auto &page: m_pages) {
        SendMessage(m_hList, LB_ADDSTRING, 0, (LPARAM) page.title.c_str());
    }
    SendMessage(m_hList, LB_SETCURSEL, 0, 0);

    UpdateFonts();
}

void HelpWindow::UpdateFonts() {
    if (m_hTitleFont) DeleteObject(m_hTitleFont);
    if (m_hBodyFont) DeleteObject(m_hBodyFont);

    int titleSize = static_cast<int>(36 * m_fontScale);
    int bodySize = static_cast<int>(24 * m_fontScale);

    // 创建字体 (加大字号)
    m_hTitleFont = CreateFont(titleSize, 0, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE,
                              DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                              VARIABLE_PITCH | FF_SWISS, TEXT("Microsoft YaHei UI"));

    m_hBodyFont = CreateFont(bodySize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                             VARIABLE_PITCH | FF_SWISS, TEXT("Microsoft YaHei UI"));

    // 设置列表框字体
    if (m_hList)
        SendMessage(m_hList, WM_SETFONT, (WPARAM) m_hBodyFont, TRUE);
    if (m_hZoomInBtn)
        SendMessage(m_hZoomInBtn, WM_SETFONT, (WPARAM) m_hBodyFont, TRUE);
    if (m_hZoomOutBtn)
        SendMessage(m_hZoomOutBtn, WM_SETFONT, (WPARAM) m_hBodyFont, TRUE);
}

void HelpWindow::OnSize(HWND hWnd) {
    RECT rc;
    GetClientRect(hWnd, &rc);
    if (m_hList) {
        // 列表框占据左侧，底部留出 40 像素给按钮
        MoveWindow(m_hList, 0, 0, 220, rc.bottom - 40, TRUE);
    }
    if (m_hZoomInBtn && m_hZoomOutBtn) {
        // 按钮位于左侧底部
        MoveWindow(m_hZoomInBtn, 5, rc.bottom - 35, 100, 30, TRUE);
        MoveWindow(m_hZoomOutBtn, 115, rc.bottom - 35, 100, 30, TRUE);
    }
    InvalidateRect(hWnd, nullptr, TRUE);
}

void HelpWindow::OnCommand(HWND hWnd, int id, int code) {
    if (id == 100) // ListBox
    {
        if (code == LBN_SELCHANGE || code == LBN_DBLCLK) {
            int sel = static_cast<int>(SendMessage(m_hList, LB_GETCURSEL, 0, 0));
            if (sel >= 0 && sel < static_cast<int>(m_pages.size())) {
                m_currentPage = sel;
                m_scrollY = 0; // 切换页面时重置滚动位置
                // 重绘整个窗口
                InvalidateRect(hWnd, nullptr, TRUE);
            }
        }
    } else if (id == 101) // Zoom In
    {
        if (m_fontScale < 3.0f) {
            m_fontScale += 0.1f;
            UpdateFonts();
            InvalidateRect(hWnd, nullptr, TRUE);
        }
    } else if (id == 102) // Zoom Out
    {
        if (m_fontScale > 0.5f) {
            m_fontScale -= 0.1f;
            UpdateFonts();
            InvalidateRect(hWnd, nullptr, TRUE);
        }
    }
}

void HelpWindow::OnPaint(HWND hWnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);

    RECT rc;
    GetClientRect(hWnd, &rc);

    // 创建双缓冲
    HDC hdcMem = CreateCompatibleDC(hdc);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);

    // 填充背景
    HBRUSH hBgBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
    FillRect(hdcMem, &rc, hBgBrush);
    DeleteObject(hBgBrush);

    // 计算内容区域
    RECT contentRect = rc;
    contentRect.left = 220 + 30;
    contentRect.top += 30;
    contentRect.right -= 30;
    contentRect.bottom -= 30;

    if (m_currentPage >= 0 && m_currentPage < static_cast<int>(m_pages.size())) {
        DrawPage(hdcMem, contentRect);
    }

    // 将缓冲区复制到屏幕
    BitBlt(hdc, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);

    // 清理
    SelectObject(hdcMem, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);

    EndPaint(hWnd, &ps);
}

void HelpWindow::DrawPage(HDC hdc, const RECT &rect) {
    const auto &page = m_pages[m_currentPage];

    SetBkMode(hdc, TRANSPARENT);

    // 计算内容总高度
    int titleH = static_cast<int>(60 * m_fontScale);

    // 先计算正文高度
    RECT calcRect = rect;
    calcRect.top += titleH;
    SelectObject(hdc, m_hBodyFont);
    int bodyHeight = DrawText(hdc, page.content.c_str(), -1, &calcRect, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_CALCRECT);
    m_contentHeight = titleH + bodyHeight;

    // 限制滚动范围
    int viewHeight = rect.bottom - rect.top;
    int maxScroll = m_contentHeight - viewHeight;
    if (maxScroll < 0) maxScroll = 0;
    if (m_scrollY > maxScroll) m_scrollY = maxScroll;
    if (m_scrollY < 0) m_scrollY = 0;

    // 设置裁剪区域
    HRGN hClipRgn = CreateRectRgn(rect.left, rect.top, rect.right, rect.bottom);
    SelectClipRgn(hdc, hClipRgn);

    // 绘制标题 (应用滚动偏移)
    auto hOld = static_cast<HFONT>(SelectObject(hdc, m_hTitleFont));
    SetTextColor(hdc, RGB(0, 50, 100));

    RECT titleRect = rect;
    titleRect.top -= m_scrollY;
    DrawText(hdc, page.title.c_str(), -1, &titleRect, DT_LEFT | DT_TOP | DT_SINGLELINE);

    // 绘制分割线 (应用滚动偏移)
    int lineY = rect.top + static_cast<int>(50 * m_fontScale) - m_scrollY;
    if (lineY >= rect.top && lineY <= rect.bottom) {
        HPEN hPen = CreatePen(PS_SOLID, 2, RGB(200, 200, 200));
        auto hOldPen = static_cast<HPEN>(SelectObject(hdc, hPen));
        MoveToEx(hdc, rect.left, lineY, nullptr);
        LineTo(hdc, rect.right, lineY);
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
    }

    // 绘制正文 (应用滚动偏移)
    SelectObject(hdc, m_hBodyFont);
    SetTextColor(hdc, RGB(50, 50, 50));

    RECT bodyRect = rect;
    bodyRect.top += titleH - m_scrollY;

    DrawText(hdc, page.content.c_str(), -1, &bodyRect, DT_LEFT | DT_TOP | DT_WORDBREAK);

    SelectObject(hdc, hOld);

    // 清除裁剪区域
    SelectClipRgn(hdc, nullptr);
    DeleteObject(hClipRgn);
}

void HelpWindow::OnMouseWheel(HWND hWnd, int delta) {
    // delta > 0 向上滚动，delta < 0 向下滚动
    int scrollAmount = 40; // 每次滚动的像素数

    if (delta > 0) {
        m_scrollY -= scrollAmount;
        if (m_scrollY < 0) m_scrollY = 0;
    } else {
        m_scrollY += scrollAmount;
        // 上限在 DrawPage 中检查
    }

    // 只重绘右侧内容区域，不擦除背景（由双缓冲处理）
    RECT rc;
    GetClientRect(hWnd, &rc);
    rc.left = 220;
    InvalidateRect(hWnd, &rc, FALSE);
}
