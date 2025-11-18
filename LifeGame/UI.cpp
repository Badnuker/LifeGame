#include "UI.h"
#include "Resource.h"
#include <tchar.h>

UI* UI::s_pInstance = nullptr;

UI::UI()
    : m_hRowsEdit(nullptr), m_hColsEdit(nullptr), m_hApplyBtn(nullptr),
    m_hRowsLabel(nullptr), m_hColsLabel(nullptr), m_hToolTip(nullptr),
    m_oldRowsProc(nullptr), m_oldColsProc(nullptr), m_oldApplyBtnProc(nullptr),
    m_isDragging(false), m_isRightDragging(false), m_dragValue(true),
    m_applyHover(false) {
    s_pInstance = this;
}

UI::~UI() {
    Cleanup();
    if (s_pInstance == this) {
        s_pInstance = nullptr;
    }
}

bool UI::Initialize(HINSTANCE hInstance, HWND hParent, LifeGame& game) {
    // 创建控件
    int leftX = 16, leftY = 32, labelW = 32, editW = 72, editH = 22, gapY = 12;

    m_hColsLabel = CreateWindowEx(0, TEXT("STATIC"), TEXT("列"),
        WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE,
        leftX, leftY, labelW, editH, hParent,
        nullptr, hInstance, nullptr);

    m_hColsEdit = CreateWindowEx(0, TEXT("EDIT"), nullptr,
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOVSCROLL | WS_TABSTOP,
        leftX + labelW + 8, leftY, editW, editH, hParent,
        (HMENU)ID_COLS_EDIT, hInstance, nullptr);

    leftY += editH + gapY;
    m_hRowsLabel = CreateWindowEx(0, TEXT("STATIC"), TEXT("行"),
        WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE,
        leftX, leftY, labelW, editH, hParent,
        nullptr, hInstance, nullptr);

    m_hRowsEdit = CreateWindowEx(0, TEXT("EDIT"), nullptr,
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOVSCROLL | WS_TABSTOP,
        leftX + labelW + 8, leftY, editW, editH, hParent,
        (HMENU)ID_ROWS_EDIT, hInstance, nullptr);

    leftY += editH + gapY;
    m_hApplyBtn = CreateWindowEx(0, TEXT("BUTTON"), TEXT("Apply"),
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | WS_TABSTOP,
        leftX, leftY, labelW + editW + 8, editH, hParent,
        (HMENU)ID_APPLY_BTN, hInstance, nullptr);

    // 设置初始编辑框文本
    TCHAR tmpbuf[32];
    wsprintf(tmpbuf, TEXT("%d"), game.GetHeight());
    SetWindowText(m_hRowsEdit, tmpbuf);
    wsprintf(tmpbuf, TEXT("%d"), game.GetWidth());
    SetWindowText(m_hColsEdit, tmpbuf);

    // 子类化 Edit 控件
    m_oldRowsProc = (WNDPROC)SetWindowLongPtr(m_hRowsEdit, GWLP_WNDPROC, (LONG_PTR)RowsEditProc);
    m_oldColsProc = (WNDPROC)SetWindowLongPtr(m_hColsEdit, GWLP_WNDPROC, (LONG_PTR)ColsEditProc);
    m_oldApplyBtnProc = (WNDPROC)SetWindowLongPtr(m_hApplyBtn, GWLP_WNDPROC, (LONG_PTR)ApplyBtnProc);

    // 创建 tooltip
    INITCOMMONCONTROLSEX icex = { sizeof(icex), ICC_WIN95_CLASSES };
    InitCommonControlsEx(&icex);
    m_hToolTip = CreateWindowEx(0, TOOLTIPS_CLASS, nullptr, WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        hParent, nullptr, hInstance, nullptr);
    if (m_hToolTip) {
        TOOLINFO ti = { 0 };
        ti.cbSize = sizeof(ti);
        ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
        ti.hwnd = hParent;
        ti.uId = (UINT_PTR)m_hApplyBtn;
        ti.lpszText = (LPTSTR)TEXT("应用行/列设置 (Enter) ");
        SendMessage(m_hToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
    }

    return (m_hRowsEdit && m_hColsEdit && m_hApplyBtn);
}

void UI::Cleanup() {
    // 恢复子类化
    if (m_hRowsEdit && m_oldRowsProc)
        SetWindowLongPtr(m_hRowsEdit, GWLP_WNDPROC, (LONG_PTR)m_oldRowsProc);
    if (m_hColsEdit && m_oldColsProc)
        SetWindowLongPtr(m_hColsEdit, GWLP_WNDPROC, (LONG_PTR)m_oldColsProc);
    if (m_hApplyBtn && m_oldApplyBtnProc)
        SetWindowLongPtr(m_hApplyBtn, GWLP_WNDPROC, (LONG_PTR)m_oldApplyBtnProc);

    // 销毁 tooltip
    if (m_hToolTip) DestroyWindow(m_hToolTip);
}

void UI::LayoutControls(int clientWidth, int clientHeight) {
    if (!m_hRowsEdit || !m_hColsEdit || !m_hApplyBtn || !m_hRowsLabel || !m_hColsLabel)
        return;

    int shortcutCount = 5;
    int panelPaddingY = 12;
    int lineH = 24;
    int leftX = 16;
    int leftY = panelPaddingY + shortcutCount * lineH + 24;
    int labelW = 32, editW = 72, editH = 22, gapY = 12;

    SetWindowPos(m_hColsLabel, nullptr, leftX, leftY, labelW, editH, SWP_NOZORDER);
    SetWindowPos(m_hColsEdit, nullptr, leftX + labelW + 8, leftY, editW, editH, SWP_NOZORDER);
    leftY += editH + gapY;
    SetWindowPos(m_hRowsLabel, nullptr, leftX, leftY, labelW, editH, SWP_NOZORDER);
    SetWindowPos(m_hRowsEdit, nullptr, leftX + labelW + 8, leftY, editW, editH, SWP_NOZORDER);
    leftY += editH + gapY;
    SetWindowPos(m_hApplyBtn, nullptr, leftX, leftY, labelW + editW + 8, editH, SWP_NOZORDER);
}

void UI::UpdateWindowTitle(HWND hWnd, const LifeGame& game) {
    TCHAR title[200];
    wsprintf(title, TEXT("生命游戏 - %s | 速度：%d ms/帧"),
        game.IsRunning() ? TEXT("运行中") : TEXT("已暂停"),
        game.GetSpeed());
    SetWindowText(hWnd, title);
}

void UI::HandleCommand(int id, int code, HWND hWnd, LifeGame& game) {
    if (id == ID_APPLY_BTN && code == BN_CLICKED) {
        TCHAR buf[64];
        int newRows = game.GetHeight();
        int newCols = game.GetWidth();

        if (m_hRowsEdit) {
            GetWindowText(m_hRowsEdit, buf, _countof(buf));
            int v = _ttoi(buf);
            if (v > 0) newRows = v;
        }
        if (m_hColsEdit) {
            GetWindowText(m_hColsEdit, buf, _countof(buf));
            int v = _ttoi(buf);
            if (v > 0) newCols = v;
        }

        // 限制范围
        if (newCols < 4) newCols = 4;
        if (newRows < 4) newRows = 4;
        if (newCols > 120) newCols = 120;
        if (newRows > 80) newRows = 80;

        game.ResizeGrid(newCols, newRows);

        // 计算所需窗口大小并调整
        constexpr int CELL_SIZE = 10;
        constexpr int STATUS_BAR_HEIGHT = 28;
        constexpr int LEFT_PANEL_WIDTH = 150;

        int gridWpx = CELL_SIZE * game.GetWidth();
        int gridHpx = CELL_SIZE * game.GetHeight();
        int topControlsH = 44;
        int desiredClientW = LEFT_PANEL_WIDTH + gridWpx + 20;
        int desiredClientH = gridHpx + STATUS_BAR_HEIGHT + topControlsH + 10;

        constexpr int MIN_VISIBLE_COLS = 40;
        constexpr int MIN_VISIBLE_ROWS = 40;
        int minGridWpx = CELL_SIZE * MIN_VISIBLE_COLS;
        int minClientW = LEFT_PANEL_WIDTH + minGridWpx + 20;
        int minGridHpx = CELL_SIZE * MIN_VISIBLE_ROWS;
        int minClientH = minGridHpx + STATUS_BAR_HEIGHT + topControlsH + 10;

        if (desiredClientW < minClientW) desiredClientW = minClientW;
        if (desiredClientH < minClientH) desiredClientH = minClientH;

        RECT wr = { 0, 0, desiredClientW, desiredClientH };
        AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, FALSE);
        int winW = wr.right - wr.left;
        int winH = wr.bottom - wr.top;

        SetWindowPos(hWnd, nullptr, 0, 0, winW, winH, SWP_NOMOVE | SWP_NOZORDER);

        // 更新编辑框内容
        wsprintf(buf, TEXT("%d"), game.GetHeight());
        SetWindowText(m_hRowsEdit, buf);
        wsprintf(buf, TEXT("%d"), game.GetWidth());
        SetWindowText(m_hColsEdit, buf);

        UpdateWindowTitle(hWnd, game);
        SetFocus(hWnd);
    }
}

bool UI::HandleMouseClick(int x, int y, bool leftButton, LifeGame& game,
    int clientWidth, int clientHeight) {
    constexpr int CELL_SIZE = 10;
    constexpr int STATUS_BAR_HEIGHT = 28;
    constexpr int LEFT_PANEL_WIDTH = 150;

    // 计算布局
    int availW = clientWidth - LEFT_PANEL_WIDTH;
    int availH = clientHeight - STATUS_BAR_HEIGHT;
    if (availW < 1) availW = 1;
    if (availH < 1) availH = 1;

    int cellSize = CELL_SIZE;
    int gridW = cellSize * game.GetWidth();
    int gridH = cellSize * game.GetHeight();
    int offX = LEFT_PANEL_WIDTH + (availW - gridW) / 2;
    int offY = (availH - gridH) / 2;
    if (offX < LEFT_PANEL_WIDTH) offX = LEFT_PANEL_WIDTH;
    if (offY < 0) offY = 0;

    if (x >= offX && x < offX + gridW && y >= offY && y < offY + gridH) {
        int cellX = (x - offX) / cellSize;
        int cellY = (y - offY) / cellSize;

        if (cellX >= 0 && cellX < game.GetWidth() && cellY >= 0 && cellY < game.GetHeight()) {
            if (leftButton) {
                // 左键染色
                game.SetCell(cellX, cellY, true);
                m_isDragging = true;
                m_dragValue = true;
                return true;
            }
            else {
                // 右键擦除
                game.SetCell(cellX, cellY, false);
                m_isRightDragging = true;
                return true;
            }
        }
    }
    return false;
}

bool UI::HandleMouseMove(int x, int y, LifeGame& game,
    int clientWidth, int clientHeight) {
    if (!m_isDragging && !m_isRightDragging) return false;

    constexpr int CELL_SIZE = 10;
    constexpr int STATUS_BAR_HEIGHT = 28;
    constexpr int LEFT_PANEL_WIDTH = 150;

    int availW = clientWidth - LEFT_PANEL_WIDTH;
    int availH = clientHeight - STATUS_BAR_HEIGHT;
    if (availW < 1) availW = 1;
    if (availH < 1) availH = 1;

    int cellSize = CELL_SIZE;
    int gridW = cellSize * game.GetWidth();
    int gridH = cellSize * game.GetHeight();
    int offX = LEFT_PANEL_WIDTH + (availW - gridW) / 2;
    int offY = (availH - gridH) / 2;
    if (offX < LEFT_PANEL_WIDTH) offX = LEFT_PANEL_WIDTH;
    if (offY < 0) offY = 0;

    if (x >= offX && x < offX + gridW && y >= offY && y < offY + gridH) {
        int cellX = (x - offX) / cellSize;
        int cellY = (y - offY) / cellSize;

        if (cellX >= 0 && cellX < game.GetWidth() && cellY >= 0 && cellY < game.GetHeight()) {
            bool target = m_isDragging; // 左键染色，右键擦除
            if (game.GetCell(cellX, cellY) != target) {
                game.SetCell(cellX, cellY, target);
                return true;
            }
        }
    }
    return false;
}

void UI::HandleMouseUp(bool leftButton) {
    if (leftButton) {
        if (m_isDragging) {
            m_isDragging = false;
        }
    }
    else {
        if (m_isRightDragging) {
            m_isRightDragging = false;
        }
    }
}

// 静态回调函数
LRESULT CALLBACK UI::RowsEditProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_KEYDOWN && wp == VK_RETURN) {
        if (s_pInstance && s_pInstance->m_hApplyBtn)
            SendMessage(s_pInstance->m_hApplyBtn, BM_CLICK, 0, 0);
        return 0;
    }
    if (s_pInstance && s_pInstance->m_oldRowsProc)
        return CallWindowProc(s_pInstance->m_oldRowsProc, hwnd, msg, wp, lp);
    return DefWindowProc(hwnd, msg, wp, lp);
}

LRESULT CALLBACK UI::ColsEditProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_KEYDOWN && wp == VK_RETURN) {
        if (s_pInstance && s_pInstance->m_hApplyBtn)
            SendMessage(s_pInstance->m_hApplyBtn, BM_CLICK, 0, 0);
        return 0;
    }
    if (s_pInstance && s_pInstance->m_oldColsProc)
        return CallWindowProc(s_pInstance->m_oldColsProc, hwnd, msg, wp, lp);
    return DefWindowProc(hwnd, msg, wp, lp);
}

LRESULT CALLBACK UI::ApplyBtnProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (!s_pInstance) return DefWindowProc(hwnd, msg, wp, lp);

    if (msg == WM_MOUSEMOVE) {
        if (!s_pInstance->m_applyHover) {
            s_pInstance->m_applyHover = true;
            InvalidateRect(GetParent(hwnd), nullptr, TRUE);
        }
        TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, hwnd, 0 };
        TrackMouseEvent(&tme);
    }
    else if (msg == WM_MOUSELEAVE) {
        if (s_pInstance->m_applyHover) {
            s_pInstance->m_applyHover = false;
            InvalidateRect(GetParent(hwnd), nullptr, TRUE);
        }
    }

    if (s_pInstance->m_oldApplyBtnProc)
        return CallWindowProc(s_pInstance->m_oldApplyBtnProc, hwnd, msg, wp, lp);
    return DefWindowProc(hwnd, msg, wp, lp);
}