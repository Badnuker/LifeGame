#pragma once

#include <windows.h>
#include "Game.h"

class Renderer
{
public:
	Renderer();
	~Renderer();

	bool Initialize(HINSTANCE hInstance);
	void Cleanup();
	void UpdateSettings(); // 新增：更新设置

	// 核心绘制函数
	void Draw(HDC hdc, const LifeGame& game, const RECT* pDirty,
	          bool showResetTip = false, int clientWidth = 0, int clientHeight = 0);

	// 清除视觉残留 (用于重置时)
	void ClearVisuals();

	// 计算布局参数
	void CalcLayout(const LifeGame& game, int& outCellSize, int& outOffsetX,
	                int& outOffsetY, int& outGridWidthPx, int& outGridHeightPx,
	                int clientWidth, int clientHeight);

	// 布局常量
	static constexpr int CELL_SIZE = 12; // 稍微调大一点，更清晰
	static constexpr int STATUS_BAR_HEIGHT = 32;
	static constexpr int LEFT_PANEL_WIDTH = 260; // 加宽一点以容纳更多信息

	// 获取资源供 Main.cpp 使用
	HBRUSH GetPanelBrush() const { return m_hLeftPanelBrush; }
	HBRUSH GetInputBrush() const { return m_hInputBgBrush; }
	COLORREF GetTextColor() const { return m_colText; }
	HFONT GetControlFont() const { return m_hControlFont; } // 新增：获取控件字体

private:
	void DrawGrid(HDC hdc, const LifeGame& game, const RECT* pDirty,
	              int cellSize, int offX, int offY, int gridWpx, int gridHpx);
	void DrawHUD(HDC hdc, int offX, int offY, int gridWpx, int gridHpx); // 新增：绘制HUD装饰
	void DrawLeftPanel(HDC hdc, int clientWidth, int clientHeight, const LifeGame& game);
	void DrawStatusBar(HDC hdc, const LifeGame& game, int clientWidth, int clientHeight);
	void DrawResetTip(HDC hdc, int offX, int offY, int gridWpx, int gridHpx);
	void DrawBranding(HDC hdc, int x, int y, int w);
	void DrawStatistics(HDC hdc, const LifeGame& game, int x, int y, int w, int h); // 新增：绘制统计图表

	// 视觉增强数据
	std::vector<float> m_visualGrid; // 存储每个细胞的亮度值 (0.0 - 1.0)
	int m_visualW, m_visualH;
	void UpdateVisualGrid(const LifeGame& game); // 更新亮度衰减

	// GDI 资源句柄
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

	// 字体
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
};
