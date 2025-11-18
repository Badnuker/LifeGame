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

	void Draw(HDC hdc, const LifeGame& game, const RECT* pDirty,
	          bool showResetTip = false, int clientWidth = 0, int clientHeight = 0);

	void CalcLayout(const LifeGame& game, int& outCellSize, int& outOffsetX,
	                int& outOffsetY, int& outGridWidthPx, int& outGridHeightPx,
	                int clientWidth, int clientHeight);

	// GDI 资源访问
	HBRUSH GetAliveBrush() const { return m_hAliveBrush; }
	HBRUSH GetDeadBrush() const { return m_hDeadBrush; }
	HPEN GetGridPen() const { return m_hGridPen; }

private:
	void DrawGrid(HDC hdc, const LifeGame& game, const RECT* pDirty,
	              int cellSize, int offX, int offY, int gridWpx, int gridHpx);
	void DrawLeftPanel(HDC hdc, int clientWidth, int clientHeight);
	void DrawStatusBar(HDC hdc, const LifeGame& game, int clientWidth, int clientHeight);
	void DrawResetTip(HDC hdc, int offX, int offY, int gridWpx, int gridHpx);

	// GDI 资源
	HBRUSH m_hBackgroundBrush;
	HBRUSH m_hAliveBrush;
	HBRUSH m_hDeadBrush;
	HBRUSH m_hTipBrush;
	HBRUSH m_hLeftPanelBrush;
	HPEN m_hGridPen;
	HFONT m_hTitleFont;
	HFONT m_hTipFont;
	HFONT m_hBtnFont;
	HFONT m_hLeftKeyFont;
	HFONT m_hLeftDescFont;

	// 常量
	static constexpr int CELL_SIZE = 10;
	static constexpr int STATUS_BAR_HEIGHT = 28;
	static constexpr int LEFT_PANEL_WIDTH = 150;
};
