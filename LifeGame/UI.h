#pragma once

#include <windows.h>
#include <commctrl.h>
#include "Game.h"
#include "Renderer.h"
#include "FileManager.h"
#include "HelpWindow.h"
#include "PatternPreview.h"

class UI {
public:
    UI();

    ~UI();

    bool Initialize(HINSTANCE hInstance, HWND hParent, LifeGame &game);

    void Cleanup();

    void LayoutControls(int clientWidth, int clientHeight);

    void UpdateWindowTitle(HWND hWnd, const LifeGame &game);

    void HandleCommand(int id, int code, HWND hWnd, LifeGame &game, Renderer *pRenderer = nullptr);

    void SetAllFonts(HFONT hFont); // 新增：设置所有控件字体

    // 鼠标交互
    bool HandleMouseClick(int x, int y, bool leftButton, LifeGame &game,
                          int clientWidth, int clientHeight, Renderer *pRenderer = nullptr);

    bool HandleMouseMove(int x, int y, LifeGame &game,
                         int clientWidth, int clientHeight, Renderer *pRenderer = nullptr);

    void HandleMouseUp(bool leftButton, Renderer *pRenderer = nullptr);

    void HandleMouseLeave(Renderer *pRenderer); // 新增：处理鼠标离开事件

    // 状态查询
    bool IsDragging() const { return m_isDragging || m_isRightDragging; }

private:
    // 控件句柄
    HWND m_hRowsEdit;
    HWND m_hColsEdit;
    HWND m_hApplyBtn;
    HWND m_hRowsLabel;
    HWND m_hColsLabel;
    HWND m_hPatternLabel; // 新增
    HWND m_hPatternCombo; // 新增
    HWND m_hDescLabel; // 新增：图案简介
    HWND m_hRuleLabel; // 新增
    HWND m_hRuleCombo; // 新增
    HWND m_hSizeLabel; // 新增
    HWND m_hSizeCombo; // 新增
    HWND m_hSaveBtn; // 新增：保存按钮
    HWND m_hLoadBtn; // 新增：加载按钮
    HWND m_hExportBtn; // 新增：导出按钮
    HWND m_hSettingsBtn; // 新增：设置按钮
    HWND m_hHelpBtn; // 新增：帮助按钮
    HWND m_hUndoBtn; // 新增：撤销按钮
    HWND m_hEraserBtn; // 新增：橡皮擦按钮
    HWND m_hEraserSizeLabel; // 新增：橡皮擦大小标签
    HWND m_hEraserSizeCombo; // 新增：橡皮擦大小下拉框
    HWND m_hMoveBtn; // 新增：移动按钮
    HWND m_hToolTip;

    FileManager m_fileManager; // 新增：文件管理器实例
    HelpWindow m_helpWindow; // 新增：帮助窗口实例
    PatternPreview m_preview; // 新增：图案预览

    // 窗口子类化
    WNDPROC m_oldRowsProc;
    WNDPROC m_oldColsProc;
    WNDPROC m_oldApplyBtnProc;

    // 交互状态
    bool m_isDragging;
    bool m_isRightDragging;
    bool m_isPanning; // 新增：平移状态
    bool m_dragValue;
    bool m_applyHover;
    bool m_isEraserMode;
    int m_eraserSize; // 新增：橡皮擦大小 (1, 3, 5, 7...)
    bool m_isMoveMode; // 新增：移动模式
    int m_lastGridX;
    int m_lastGridY;
    int m_lastMouseX; // 新增：上一次鼠标屏幕坐标X
    int m_lastMouseY; // 新增：上一次鼠标屏幕坐标Y

    // 静态回调函数
    static LRESULT CALLBACK RowsEditProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    static LRESULT CALLBACK ColsEditProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    static LRESULT CALLBACK ApplyBtnProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    static UI *s_pInstance;
};
