#pragma once

#include <windows.h>
#include <commctrl.h>
#include "Game.h"
#include "Renderer.h"
#include "FileManager.h"
#include "HelpWindow.h"
#include "PatternPreview.h"

/**
 * @brief 用户界面控制器类 (UI Controller)
 *
 * 负责管理应用程序的所有标准 Windows 控件（按钮、文本框、下拉列表等）。
 * 它是 Model-View-Controller (MVC) 架构中的 Controller 部分之一（与 Application 共同承担）。
 *
 * 主要职责：
 * 1. 创建和初始化 UI 控件。
 * 2. 处理控件布局（响应窗口大小改变）。
 * 3. 处理用户交互（按钮点击、鼠标操作）。
 * 4. 协调各个子窗口（帮助窗口、设置对话框）。
 */
class UI {
public:
    /**
     * @brief 构造函数
     */
    UI();

    /**
     * @brief 析构函数
     */
    ~UI();

    /**
     * @brief 初始化 UI 系统
     *
     * 创建所有子控件，设置初始状态。
     * @param hInstance 应用程序实例句柄
     * @param hParent 父窗口句柄
     * @param game 游戏实例引用 (用于初始化控件数据)
     * @return true 初始化成功
     */
    bool Initialize(HINSTANCE hInstance, HWND hParent, LifeGame &game);

    /**
     * @brief 清理资源
     *
     * 销毁所有控件窗口。
     */
    void Cleanup();

    /**
     * @brief 重新布局控件
     *
     * 当主窗口大小改变时调用，调整所有控件的位置和大小。
     * 实现了响应式布局，确保控件始终位于左侧面板或正确的位置。
     *
     * @param clientWidth 客户区宽度
     * @param clientHeight 客户区高度
     */
    void LayoutControls(int clientWidth, int clientHeight);

    /**
     * @brief 更新窗口标题
     *
     * 根据当前游戏状态（如是否暂停、当前代数）更新主窗口标题栏。
     * @param hWnd 主窗口句柄
     * @param game 游戏实例
     */
    void UpdateWindowTitle(HWND hWnd, const LifeGame &game);

    /**
     * @brief 处理 WM_COMMAND 消息
     *
     * 响应按钮点击、下拉框选择等控件事件。
     *
     * @param id 控件 ID
     * @param code 通知代码
     * @param hWnd 主窗口句柄
     * @param game 游戏实例
     * @param pRenderer 渲染器指针 (可选，用于触发重绘)
     */
    void HandleCommand(int id, int code, HWND hWnd, LifeGame &game, Renderer *pRenderer = nullptr);

    /**
     * @brief 设置所有控件的字体
     *
     * 统一设置界面字体，保持风格一致。
     * @param hFont 字体句柄
     */
    void SetAllFonts(HFONT hFont);

    // ==========================================
    // 鼠标交互 (Mouse Interaction)
    // ==========================================

    /**
     * @brief 处理鼠标点击事件
     *
     * 将屏幕坐标转换为网格坐标，并执行相应的游戏操作（如放置细胞、放置图案）。
     *
     * @param x 鼠标 X 坐标
     * @param y 鼠标 Y 坐标
     * @param leftButton true=左键, false=右键
     * @param game 游戏实例
     * @param clientWidth 窗口宽度
     * @param clientHeight 窗口高度
     * @param pRenderer 渲染器指针
     * @return true 如果事件被处理
     */
    bool HandleMouseClick(int x, int y, bool leftButton, LifeGame &game,
                          int clientWidth, int clientHeight, Renderer *pRenderer = nullptr);

    /**
     * @brief 处理鼠标移动事件
     *
     * 用于实现拖拽绘制、预览图案跟随鼠标等功能。
     *
     * @param x 鼠标 X 坐标
     * @param y 鼠标 Y 坐标
     * @param game 游戏实例
     * @param clientWidth 窗口宽度
     * @param clientHeight 窗口高度
     * @param pRenderer 渲染器指针
     * @return true 如果需要重绘
     */
    bool HandleMouseMove(int x, int y, LifeGame &game,
                         int clientWidth, int clientHeight, Renderer *pRenderer = nullptr);

    /**
     * @brief 处理鼠标松开事件
     *
     * 结束拖拽操作。
     * @param leftButton true=左键, false=右键
     * @param pRenderer 渲染器指针
     */
    void HandleMouseUp(bool leftButton, Renderer *pRenderer = nullptr);

    /**
     * @brief 处理鼠标离开窗口事件
     *
     * 清除预览状态。
     * @param pRenderer 渲染器指针
     */
    void HandleMouseLeave(Renderer *pRenderer);

    // 状态查询
    bool IsDragging() const { return m_isDragging || m_isRightDragging; }

private:
    // 控件句柄
    HWND m_hRowsEdit; ///< 行数输入框
    HWND m_hColsEdit; ///< 列数输入框
    HWND m_hApplyBtn; ///< 应用大小按钮
    HWND m_hRowsLabel; ///< "行:" 标签
    HWND m_hColsLabel; ///< "列:" 标签
    HWND m_hPatternLabel; ///< "图案:" 标签
    HWND m_hPatternCombo; ///< 图案选择下拉框
    HWND m_hDescLabel; ///< 图案描述标签
    HWND m_hRuleLabel; ///< "规则:" 标签
    HWND m_hRuleCombo; ///< 规则选择下拉框
    HWND m_hSizeLabel; ///< "大小:" 标签
    HWND m_hSizeCombo; ///< 预设大小下拉框
    HWND m_hSaveBtn; ///< 保存按钮
    HWND m_hLoadBtn; ///< 加载按钮
    HWND m_hExportBtn; ///< 导出按钮
    HWND m_hSettingsBtn; ///< 设置按钮
    HWND m_hHelpBtn; ///< 帮助按钮
    HWND m_hUndoBtn; ///< 撤销按钮
    HWND m_hEraserBtn; ///< 橡皮擦按钮
    HWND m_hEraserSizeLabel; ///< 橡皮擦大小标签
    HWND m_hEraserSizeCombo; ///< 橡皮擦大小下拉框
    HWND m_hToolTip; ///< 工具提示控件

    FileManager m_fileManager; ///< 文件管理器实例
    HelpWindow m_helpWindow; ///< 帮助窗口实例
    PatternPreview m_preview; ///< 图案预览控件

    // 窗口子类化 (Subclassing)
    WNDPROC m_oldRowsProc;
    WNDPROC m_oldColsProc;
    WNDPROC m_oldApplyBtnProc;

    // 交互状态
    bool m_isDragging; ///< 是否正在左键拖拽
    bool m_isRightDragging; ///< 是否正在右键拖拽
    bool m_isPanning; ///< 是否正在平移视图 (中键或空格+拖拽)
    bool m_dragValue; ///< 拖拽时的目标状态 (绘制/擦除)
    bool m_applyHover; ///< 鼠标是否悬停在应用按钮上
    bool m_isEraserMode; ///< 是否处于橡皮擦模式
    int m_eraserSize; ///< 橡皮擦大小 (1, 3, 5, 7...)
    int m_lastGridX; ///< 上一次鼠标所在的网格 X 坐标
    int m_lastGridY; ///< 上一次鼠标所在的网格 Y 坐标
    int m_lastMouseX; ///< 上一次鼠标屏幕坐标 X
    int m_lastMouseY; ///< 上一次鼠标屏幕坐标 Y

    // 静态回调函数 (用于子类化控件)
    static LRESULT CALLBACK RowsEditProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    static LRESULT CALLBACK ColsEditProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    static LRESULT CALLBACK ApplyBtnProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    static UI *s_pInstance; ///< 单例指针 (用于回调函数访问成员)
};