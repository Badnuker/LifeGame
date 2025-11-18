// Renderer.cpp
#include "Renderer.h"
#include "Game.h"

Renderer::Renderer()
    : m_hBackgroundBrush(nullptr), m_hAliveBrush(nullptr), m_hDeadBrush(nullptr),
    m_hTipBrush(nullptr), m_hLeftPanelBrush(nullptr), m_hGridPen(nullptr),
    m_hTitleFont(nullptr), m_hTipFont(nullptr), m_hBtnFont(nullptr),
    m_hLeftKeyFont(nullptr), m_hLeftDescFont(nullptr) {
}

Renderer::~Renderer() {
    Cleanup();
}

bool Renderer::Initialize(HINSTANCE hInstance) {
    // 创建 GDI 对象
    m_hBackgroundBrush = CreateSolidBrush(RGB(245, 247, 250));
    m_hAliveBrush = CreateSolidBrush(RGB(34, 40, 49));
    m_hDeadBrush = CreateSolidBrush(RGB(255, 255, 255));
    m_hTipBrush = CreateSolidBrush(RGB(255, 255, 255));
    m_hLeftPanelBrush = CreateSolidBrush(RGB(225, 235, 245));
    m_hGridPen = CreatePen(PS_SOLID, 1, RGB(210, 213, 220));

    // 创建字体
    m_hTitleFont = CreateFontW(-18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        VARIABLE_PITCH | FF_SWISS, L"Segoe UI");
    m_hTipFont = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        VARIABLE_PITCH | FF_SWISS, L"Segoe UI");
    m_hBtnFont = CreateFontW(-13, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        VARIABLE_PITCH | FF_SWISS, L"Segoe UI");
    m_hLeftKeyFont = CreateFontW(-14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        VARIABLE_PITCH | FF_SWISS, L"Segoe UI");
    m_hLeftDescFont = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        VARIABLE_PITCH | FF_SWISS, L"Segoe UI");

    return (m_hBackgroundBrush && m_hAliveBrush && m_hDeadBrush && m_hGridPen);
}

void Renderer::Cleanup() {
    if (m_hGridPen) DeleteObject(m_hGridPen);
    if (m_hBackgroundBrush) DeleteObject(m_hBackgroundBrush);
    if (m_hAliveBrush) DeleteObject(m_hAliveBrush);
    if (m_hDeadBrush) DeleteObject(m_hDeadBrush);
    if (m_hTipBrush) DeleteObject(m_hTipBrush);
    if (m_hLeftPanelBrush) DeleteObject(m_hLeftPanelBrush);
    if (m_hTitleFont) DeleteObject(m_hTitleFont);
    if (m_hTipFont) DeleteObject(m_hTipFont);
    if (m_hBtnFont) DeleteObject(m_hBtnFont);
    if (m_hLeftKeyFont) DeleteObject(m_hLeftKeyFont);
    if (m_hLeftDescFont) DeleteObject(m_hLeftDescFont);
}

void Renderer::Draw(HDC hdc, const LifeGame& game, const RECT* pDirty,
    bool showResetTip, int clientWidth, int clientHeight) {
    SetBkMode(hdc, TRANSPARENT);

    // 计算布局
    int cellSize, offX, offY, gridWpx, gridHpx;
    CalcLayout(game, cellSize, offX, offY, gridWpx, gridHpx, clientWidth, clientHeight);

    // 用柔和背景色填充整个客户区
    RECT clientRect = { 0, 0, clientWidth, clientHeight };
    FillRect(hdc, &clientRect, m_hBackgroundBrush);

    // 只绘制非控件区域
    RECT nonControlArea = clientRect;
    nonControlArea.left = LEFT_PANEL_WIDTH;
    nonControlArea.bottom -= STATUS_BAR_HEIGHT;

    RECT dirtyNonControl = *pDirty;
    if (dirtyNonControl.left < LEFT_PANEL_WIDTH)
        dirtyNonControl.left = LEFT_PANEL_WIDTH;
    if (dirtyNonControl.bottom > clientRect.bottom - STATUS_BAR_HEIGHT)
        dirtyNonControl.bottom = clientRect.bottom - STATUS_BAR_HEIGHT;

    if (dirtyNonControl.left < dirtyNonControl.right &&
        dirtyNonControl.top < dirtyNonControl.bottom) {
        DrawGrid(hdc, game, &dirtyNonControl, cellSize, offX, offY, gridWpx, gridHpx);
    }

    // 绘制左侧面板
    DrawLeftPanel(hdc, clientWidth, clientHeight);

    // 绘制状态栏
    DrawStatusBar(hdc, game, clientWidth, clientHeight);

    // 绘制重置提示
    if (showResetTip) {
        DrawResetTip(hdc, offX, offY, gridWpx, gridHpx);
    }
}

void Renderer::CalcLayout(const LifeGame& game, int& outCellSize, int& outOffsetX,
    int& outOffsetY, int& outGridWidthPx, int& outGridHeightPx,
    int clientWidth, int clientHeight) {
    // 网格区域可用宽高
    int availW = clientWidth - LEFT_PANEL_WIDTH;
    int availH = clientHeight - STATUS_BAR_HEIGHT;
    if (availW < 1) availW = 1;
    if (availH < 1) availH = 1;

    int cellSize = CELL_SIZE;
    int gridW = cellSize * game.GetWidth();
    int gridH = cellSize * game.GetHeight();

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

void Renderer::DrawGrid(HDC hdc, const LifeGame& game, const RECT* pDirty,
    int cellSize, int offX, int offY, int gridWpx, int gridHpx) {
    // 计算需要重绘的细胞范围
    int xStart = 0, xEnd = game.GetWidth(), yStart = 0, yEnd = game.GetHeight();
    if (pDirty) {
        xStart = (pDirty->left - offX) / cellSize;
        xEnd = (pDirty->right - offX + cellSize - 1) / cellSize;
        yStart = (pDirty->top - offY) / cellSize;
        yEnd = (pDirty->bottom - offY + cellSize - 1) / cellSize;
        if (xStart < 0) xStart = 0;
        if (yStart < 0) yStart = 0;
        if (xEnd > game.GetWidth()) xEnd = game.GetWidth();
        if (yEnd > game.GetHeight()) yEnd = game.GetHeight();
    }

    // 填充每个单元
    for (int y = yStart; y < yEnd && y < game.GetHeight(); y++) {
        for (int x = xStart; x < xEnd && x < game.GetWidth(); x++) {
            int left = offX + x * cellSize;
            int top = offY + y * cellSize;
            int right = left + cellSize;
            int bottom = top + cellSize;
            RECT cell = { left, top, right, bottom };
            FillRect(hdc, &cell, game.GetCell(x, y) ? m_hAliveBrush : m_hDeadBrush);
        }
    }

    // 绘制细线网格
    auto hOldPen = static_cast<HPEN>(SelectObject(hdc, m_hGridPen));
    for (int xi = 0; xi <= game.GetWidth(); xi++) {
        int xpos = offX + xi * cellSize;
        MoveToEx(hdc, xpos, offY, nullptr);
        LineTo(hdc, xpos, offY + gridHpx);
    }
    for (int yi = 0; yi <= game.GetHeight(); yi++) {
        int ypos = offY + yi * cellSize;
        MoveToEx(hdc, offX, ypos, nullptr);
        LineTo(hdc, offX + gridWpx, ypos);
    }
    SelectObject(hdc, hOldPen);
}

void Renderer::DrawLeftPanel(HDC hdc, int clientWidth, int clientHeight) {
    RECT leftPanel = { 0, 0, LEFT_PANEL_WIDTH, clientHeight };
    FillRect(hdc, &leftPanel, m_hLeftPanelBrush);

    SetBkMode(hdc, TRANSPARENT);
    int panelPaddingX = 10;
    int panelPaddingY = 12;
    int lineH = 24;

    struct Shortcut {
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

    int keyColW = 56;
    for (int i = 0; i < static_cast<int>(sizeof(sc) / sizeof(sc[0])); ++i) {
        int y = panelPaddingY + i * lineH;
        RECT keyR = { panelPaddingX, y, panelPaddingX + keyColW, y + lineH };
        RECT descR = { panelPaddingX + keyColW + 8, y, LEFT_PANEL_WIDTH - 12, y + lineH };

        auto hOld = static_cast<HFONT>(SelectObject(hdc, m_hLeftKeyFont));
        SetTextColor(hdc, RGB(0, 102, 204));
        DrawText(hdc, sc[i].key, -1, &keyR, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdc, m_hLeftDescFont);
        SetTextColor(hdc, RGB(70, 80, 90));
        DrawText(hdc, sc[i].desc, -1, &descR, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_WORDBREAK);
        SelectObject(hdc, hOld);
    }

    // 绘制标题
    int titleH = 36;
    int titleMargin = 12;
    RECT titleRect = {
        0, clientHeight - STATUS_BAR_HEIGHT - titleH - titleMargin, LEFT_PANEL_WIDTH,
        clientHeight - STATUS_BAR_HEIGHT - titleMargin
    };
    FillRect(hdc, &titleRect, m_hTipBrush);
    auto hOldFont = static_cast<HFONT>(SelectObject(hdc, m_hTitleFont));
    SetTextColor(hdc, RGB(34, 40, 49));
    DrawText(hdc, TEXT("生命游戏"), -1, &titleRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, hOldFont);
}

void Renderer::DrawStatusBar(HDC hdc, const LifeGame& game, int clientWidth, int clientHeight) {
    RECT statusRect = { 0, clientHeight - STATUS_BAR_HEIGHT, clientWidth, clientHeight };
    HBRUSH statusBg = CreateSolidBrush(RGB(230, 236, 240));
    FillRect(hdc, &statusRect, statusBg);

    SelectObject(hdc, m_hTipFont);
    SetTextColor(hdc, RGB(50, 60, 70));

    // 左侧状态信息
    TCHAR leftStatus[128];
    wsprintf(leftStatus, TEXT("状态: %s    速度: %d ms/帧"),
        game.IsRunning() ? TEXT("运行中") : TEXT("已暂停"), game.GetSpeed());
    RECT leftRect = { 8, clientHeight - STATUS_BAR_HEIGHT, clientWidth / 2, clientHeight };
    DrawText(hdc, leftStatus, -1, &leftRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    // 右侧显示网格尺寸和快捷键
    TCHAR rightStatus[128];
    wsprintf(rightStatus, TEXT("网格: %d x %d    空格: 开始/暂停"),
        game.GetWidth(), game.GetHeight());
    RECT rightRect = { clientWidth / 2, clientHeight - STATUS_BAR_HEIGHT, clientWidth - 8, clientHeight };
    DrawText(hdc, rightStatus, -1, &rightRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

    DeleteObject(statusBg);
}

void Renderer::DrawResetTip(HDC hdc, int offX, int offY, int gridWpx, int gridHpx) {
    auto tip = TEXT("网格已重置！按 G 生成随机细胞");
    auto hOld = static_cast<HFONT>(SelectObject(hdc, m_hTipFont));

    RECT measure = { 0, 0, 0, 0 };
    DrawText(hdc, tip, -1, &measure, DT_CALCRECT | DT_SINGLELINE);
    int tw = measure.right - measure.left + 20;
    int th = measure.bottom - measure.top + 10;

    RECT box = {
        offX + (gridWpx - tw) / 2, offY + (gridHpx - th) / 2,
        offX + (gridWpx + tw) / 2, offY + (gridHpx + th) / 2
    };

    // 背景与边框
    FillRect(hdc, &box, m_hTipBrush);
    HPEN hPenTip = CreatePen(PS_SOLID, 1, RGB(200, 50, 50));
    auto hOld2 = static_cast<HPEN>(SelectObject(hdc, hPenTip));
    Rectangle(hdc, box.left, box.top, box.right, box.bottom);

    SetTextColor(hdc, RGB(200, 30, 30));
    DrawText(hdc, tip, -1, &box, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, hOld2);
    DeleteObject(hPenTip);
    SelectObject(hdc, hOld);
}