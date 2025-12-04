#pragma once

#include <windows.h>
#include "Game.h"

/**
 * @brief 渲染器类 (Renderer)
 * 
 * 负责游戏的所有图形绘制工作，使用 Win32 GDI API。
 * 实现了赛博朋克风格的视觉效果，包括发光细胞、拖尾特效、HUD 界面等。
 * 它是 Model-View-Controller (MVC) 架构中的 View 部分。
 */
class Renderer
{
public:
	Renderer();
	~Renderer();

	/**
	 * @brief 初始化渲染器
	 * 
	 * 创建所有必要的 GDI 资源（画刷、画笔、字体）。
	 * @param hInstance 应用程序实例句柄
	 * @return true 初始化成功
	 */
	bool Initialize(HINSTANCE hInstance);

	/**
	 * @brief 清理资源
	 * 
	 * 释放所有 GDI 对象，防止内存泄漏。
	 */
	void Cleanup();

	/**
	 * @brief 更新设置
	 * 
	 * 当用户修改配色方案或网格设置时调用，重建画刷和画笔。
	 */
	void UpdateSettings(); // 新增：更新设置

	/**
	 * @brief 核心绘制函数
	 * 
	 * 每一帧调用一次，绘制整个游戏界面。
	 * 
	 * @param hdc 设备上下文句柄 (通常是双缓冲的内存 DC)
	 * @param game 游戏实例 (数据源)
	 * @param pDirty 脏矩形区域 (可选，用于局部重绘优化)
	 * @param showResetTip 是否显示重置提示
	 * @param clientWidth 窗口客户区宽度
	 * @param clientHeight 窗口客户区高度
	 */
	void Draw(HDC hdc, const LifeGame& game, const RECT* pDirty,
	          bool showResetTip = false, int clientWidth = 0, int clientHeight = 0);

	/**
	 * @brief 清除视觉残留
	 * 
	 * 重置拖尾效果的亮度网格，通常在重置游戏或加载存档时调用。
	 */
	void ClearVisuals();

	/**
	 * @brief 计算布局参数
	 * 
	 * 根据窗口大小和缩放比例，计算网格的显示位置和细胞大小。
	 * 实现了自动居中和自适应布局。
	 * 
	 * @param game 游戏实例
	 * @param outCellSize 输出：单个细胞的像素大小
	 * @param outOffsetX 输出：网格左上角 X 偏移
	 * @param outOffsetY 输出：网格左上角 Y 偏移
	 * @param outGridWidthPx 输出：网格总像素宽度
	 * @param outGridHeightPx 输出：网格总像素高度
	 * @param clientWidth 窗口宽度
	 * @param clientHeight 窗口高度
	 */
	void CalcLayout(const LifeGame& game, int& outCellSize, int& outOffsetX,
	                int& outOffsetY, int& outGridWidthPx, int& outGridHeightPx,
	                int clientWidth, int clientHeight);

	// 布局常量定义
	static constexpr int BASE_CELL_SIZE = 12; ///< 基础细胞大小 (未缩放时)
	static constexpr int STATUS_BAR_HEIGHT = 32; ///< 底部状态栏高度
	static constexpr int LEFT_PANEL_WIDTH = 260; ///< 左侧控制面板宽度

	// 获取资源供 Main.cpp 使用 (用于自绘控件)
	HBRUSH GetPanelBrush() const { return m_hLeftPanelBrush; }
	HBRUSH GetInputBrush() const { return m_hInputBgBrush; }
	COLORREF GetTextColor() const { return m_colText; }
	HFONT GetControlFont() const { return m_hControlFont; }

	/**
	 * @brief 设置预览状态
	 * 
	 * 用于在鼠标悬停时显示即将放置的图案或擦除范围。
	 * @param x 预览位置 X (网格坐标)
	 * @param y 预览位置 Y (网格坐标)
	 * @param patternIndex 图案索引
	 * @param isEraser 是否为橡皮擦模式
	 */
	void SetPreview(int x, int y, int patternIndex, bool isEraser);

	// ==========================================
	// 视图控制 (View Control)
	// ==========================================

	void SetScale(float scale); ///< 设置缩放比例
	float GetScale() const { return m_scale; }
	void SetOffset(int x, int y); ///< 设置视图偏移 (平移)
	void Pan(int dx, int dy); ///< 平移视图
	void Zoom(float factor, int centerX, int centerY); ///< 缩放视图 (factor > 1 放大, < 1 缩小)

private:
	// 内部绘制辅助函数
	void DrawGrid(HDC hdc, const LifeGame& game, const RECT* pDirty,
	              int cellSize, int offX, int offY, int gridWpx, int gridHpx,
	              int clientWidth, int clientHeight); // 增加裁剪参数
	void DrawPreview(HDC hdc, const LifeGame& game, int cellSize, int offX, int offY);
	void DrawHUD(HDC hdc, int offX, int offY, int gridWpx, int gridHpx);
	void DrawLeftPanel(HDC hdc, int clientWidth, int clientHeight, const LifeGame& game);
	void DrawStatusBar(HDC hdc, const LifeGame& game, int clientWidth, int clientHeight);
	void DrawResetTip(HDC hdc, int offX, int offY, int gridWpx, int gridHpx);
	void DrawBranding(HDC hdc, int x, int y, int w);
	void DrawStatistics(HDC hdc, const LifeGame& game, int x, int y, int w, int h);

	// 视觉增强数据 (Visual Enhancement)
	std::vector<float> m_visualGrid; ///< 存储每个细胞的亮度值 (0.0 - 1.0)，用于实现拖尾
	int m_visualW, m_visualH;
	void UpdateVisualGrid(const LifeGame& game); ///< 更新亮度网格，计算衰减

	// 视图状态 (View State)
	float m_scale; ///< 当前缩放比例
	int m_viewOffsetX; ///< 视图 X 偏移
	int m_viewOffsetY; ///< 视图 Y 偏移

	// GDI 资源句柄 (GDI Resources)
	HBRUSH m_hBackgroundBrush;
	HBRUSH m_hAliveBrush; // 核心亮色
	HBRUSH m_hGlowBrush; // 光晕色
	HBRUSH m_hDeadBrush;
	HBRUSH m_hTipBrush;
	HBRUSH m_hLeftPanelBrush;
	HBRUSH m_hInputBgBrush;

	// 预计算的衰减画刷数组 (用于拖尾效果)
	static constexpr int FADE_LEVELS = 10;
	HBRUSH m_fadeBrushes[FADE_LEVELS];

	HPEN m_hGridPen;
	HPEN m_hBorderPen;
	HPEN m_hHUDPen; // HUD 装饰线笔
	HPEN m_hGraphPen; // 统计图表笔

	// 字体资源
	HFONT m_hTitleFont;
	HFONT m_hTipFont;
	HFONT m_hBtnFont;
	HFONT m_hControlFont; // 新增：通用控件字体
	HFONT m_hLeftKeyFont;
	HFONT m_hLeftDescFont;
	HFONT m_hBrandingFont;
	HFONT m_hDataFont; // 数据显示字体

	// 颜色常量
	COLORREF m_colText;
	COLORREF m_colTextDim;
	COLORREF m_colHighlight;
	COLORREF m_colWarning; // 警示色

	// 预览状态
	int m_previewX;
	int m_previewY;
	int m_previewPatternIndex;
	bool m_isEraserPreview;
	HBRUSH m_hPreviewBrush; // 预览画刷
	HPEN m_hEraserPen; // 橡皮擦预览笔
};
