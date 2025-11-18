#pragma once

#include <windows.h>
#include <commctrl.h>
#include "Game.h"
#include "Renderer.h"

class UI {
public:
    UI();
    ~UI();

    bool Initialize(HINSTANCE hInstance, HWND hParent, LifeGame& game);
    void Cleanup();

    void LayoutControls(int clientWidth, int clientHeight);
    void UpdateWindowTitle(HWND hWnd, const LifeGame& game);
    void HandleCommand(int id, int code, HWND hWnd, LifeGame& game);

    // 鼠标处理
    bool HandleMouseClick(int x, int y, bool leftButton, LifeGame& game,
        int clientWidth, int clientHeight);
    bool HandleMouseMove(int x, int y, LifeGame& game,
        int clientWidth, int clientHeight);
    void HandleMouseUp(bool leftButton);

    // 控件访问
    HWND GetRowsEdit() const { return m_hRowsEdit; }
    HWND GetColsEdit() const { return m_hColsEdit; }
    HWND GetApplyBtn() const { return m_hApplyBtn; }

    // 状态
    bool IsDragging() const { return m_isDragging || m_isRightDragging; }

private:
    // 控件句柄
    HWND m_hRowsEdit;
    HWND m_hColsEdit;
    HWND m_hApplyBtn;
    HWND m_hRowsLabel;
    HWND m_hColsLabel;
    HWND m_hToolTip;

    // 子类化过程
    WNDPROC m_oldRowsProc;
    WNDPROC m_oldColsProc;
    WNDPROC m_oldApplyBtnProc;

    // 鼠标状态
    bool m_isDragging;
    bool m_isRightDragging;
    bool m_dragValue;
    bool m_applyHover;

    // 子类化窗口过程
    static LRESULT CALLBACK RowsEditProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    static LRESULT CALLBACK ColsEditProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    static LRESULT CALLBACK ApplyBtnProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    // 实例指针用于静态回调
    static UI* s_pInstance;
};