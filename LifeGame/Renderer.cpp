#include "Renderer.h"
#include "Game.h"
#include "Settings.h"
#include <tchar.h>
#include <stdio.h>

Renderer::Renderer()
	: m_visualW(0), m_visualH(0), m_hBackgroundBrush(nullptr),
	  m_hAliveBrush(nullptr), m_hGlowBrush(nullptr), m_hDeadBrush(nullptr),
	  m_hTipBrush(nullptr), m_hLeftPanelBrush(nullptr), m_hInputBgBrush(nullptr), m_hGridPen(nullptr),
	  m_hBorderPen(nullptr), m_hHUDPen(nullptr), m_hTitleFont(nullptr), m_hTipFont(nullptr),
	  m_hBtnFont(nullptr), m_hControlFont(nullptr), m_hLeftKeyFont(nullptr), m_hLeftDescFont(nullptr),
	  m_hBrandingFont(nullptr), m_hDataFont(nullptr)
{
	// 赛博朋克配色方案
	m_colText = RGB(220, 240, 255); // 亮白蓝
	m_colTextDim = RGB(100, 130, 150); // 灰蓝
	m_colHighlight = RGB(0, 255, 255); // 赛博青
	m_colWarning = RGB(255, 50, 80); // 警示红

	// 初始化衰减画刷数组为空
	for (int i = 0; i < FADE_LEVELS; ++i) m_fadeBrushes[i] = nullptr;
}

Renderer::~Renderer()
{
	Cleanup();
}

bool Renderer::Initialize(HINSTANCE hInstance)
{
	// 1. 基础画刷
	m_hBackgroundBrush = CreateSolidBrush(RGB(10, 12, 16)); // 极深空灰
	m_hAliveBrush = CreateSolidBrush(RGB(200, 255, 255)); // 核心：近乎白色的青
	m_hGlowBrush = CreateSolidBrush(RGB(0, 180, 255)); // 光晕：深青蓝
	m_hDeadBrush = CreateSolidBrush(RGB(10, 12, 16));
	m_hTipBrush = CreateSolidBrush(RGB(30, 35, 40));
	m_hLeftPanelBrush = CreateSolidBrush(RGB(20, 24, 28));
	m_hInputBgBrush = CreateSolidBrush(RGB(5, 8, 10));

	m_hGridPen = CreatePen(PS_SOLID, 1, RGB(30, 40, 50)); // 极淡的网格
	m_hBorderPen = CreatePen(PS_SOLID, 1, RGB(0, 100, 120)); // 边框
	m_hHUDPen = CreatePen(PS_SOLID, 2, RGB(0, 200, 220)); // HUD 装饰线
	m_hGraphPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 100)); // 统计图表笔 (绿色)

	// 2. 创建衰减画刷 (用于拖尾)
	// 从亮青色渐变到背景色
	for (int i = 0; i < FADE_LEVELS; ++i)
	{
		// 亮度从 10% 到 90% (0是背景，FADE_LEVELS是最大亮度)
		// 颜色插值：RGB(0, 180, 255) -> RGB(10, 12, 16)
		float ratio = static_cast<float>(i + 1) / (FADE_LEVELS + 2); // 稍微暗一点，不要太抢眼
		int r = 10 + static_cast<int>((0 - 10) * ratio);
		int g = 12 + static_cast<int>((180 - 12) * ratio);
		int b = 16 + static_cast<int>((255 - 16) * ratio);
		m_fadeBrushes[i] = CreateSolidBrush(RGB(r, g, b));
	}

	// 3. 字体 (字号调大)
	m_hTitleFont = CreateFontW(-36, 0, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE,
	                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
	                           VARIABLE_PITCH | FF_SWISS, L"Microsoft YaHei UI");
	m_hTipFont = CreateFontW(-20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
	                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
	                         VARIABLE_PITCH | FF_SWISS, L"Microsoft YaHei UI");
	m_hBtnFont = CreateFontW(-20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
	                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
	                         VARIABLE_PITCH | FF_SWISS, L"Microsoft YaHei UI");
	// 新增控件字体：稍微大一点，清晰
	m_hControlFont = CreateFontW(-22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
	                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
	                             VARIABLE_PITCH | FF_SWISS, L"Microsoft YaHei UI");

	m_hLeftKeyFont = CreateFontW(-24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
	                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
	                             VARIABLE_PITCH | FF_SWISS, L"Consolas");
	m_hLeftDescFont = CreateFontW(-20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
	                              DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
	                              VARIABLE_PITCH | FF_SWISS, L"Microsoft YaHei UI");
	m_hBrandingFont = CreateFontW(-24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
	                              DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
	                              VARIABLE_PITCH | FF_SWISS, L"Microsoft YaHei UI");
	m_hDataFont = CreateFontW(-18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
	                          DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
	                          VARIABLE_PITCH | FF_SWISS, L"Consolas");

	return (m_hBackgroundBrush && m_hAliveBrush && m_hGridPen);
}

void Renderer::Cleanup()
{
	if (m_hGridPen) DeleteObject(m_hGridPen);
	if (m_hBorderPen) DeleteObject(m_hBorderPen);
	if (m_hHUDPen) DeleteObject(m_hHUDPen);
	if (m_hGraphPen) DeleteObject(m_hGraphPen);
	if (m_hBackgroundBrush) DeleteObject(m_hBackgroundBrush);
	if (m_hAliveBrush) DeleteObject(m_hAliveBrush);
	if (m_hGlowBrush) DeleteObject(m_hGlowBrush);
	if (m_hDeadBrush) DeleteObject(m_hDeadBrush);
	if (m_hTipBrush) DeleteObject(m_hTipBrush);
	if (m_hLeftPanelBrush) DeleteObject(m_hLeftPanelBrush);
	if (m_hInputBgBrush) DeleteObject(m_hInputBgBrush);

	for (int i = 0; i < FADE_LEVELS; ++i)
		if (m_fadeBrushes[i]) DeleteObject(m_fadeBrushes[i]);

	if (m_hTitleFont) DeleteObject(m_hTitleFont);
	if (m_hTipFont) DeleteObject(m_hTipFont);
	if (m_hBtnFont) DeleteObject(m_hBtnFont);
	if (m_hControlFont) DeleteObject(m_hControlFont);
	if (m_hLeftKeyFont) DeleteObject(m_hLeftKeyFont);
	if (m_hLeftDescFont) DeleteObject(m_hLeftDescFont);
	if (m_hBrandingFont) DeleteObject(m_hBrandingFont);
	if (m_hDataFont) DeleteObject(m_hDataFont);
}

void Renderer::UpdateSettings()
{
	const auto& settings = SettingsManager::GetInstance().GetSettings();

	// 重建画刷和笔
	if (m_hBackgroundBrush) DeleteObject(m_hBackgroundBrush);
	m_hBackgroundBrush = CreateSolidBrush(settings.bgColor);

	if (m_hAliveBrush) DeleteObject(m_hAliveBrush);
	m_hAliveBrush = CreateSolidBrush(settings.cellColor);

	if (m_hGridPen) DeleteObject(m_hGridPen);
	m_hGridPen = CreatePen(PS_SOLID, settings.gridLineWidth, settings.gridColor);

	// 更新文字颜色
	m_colText = settings.textColor;
}

void Renderer::ClearVisuals()
{
	std::fill(m_visualGrid.begin(), m_visualGrid.end(), 0.0f);
}

void Renderer::UpdateVisualGrid(const LifeGame& game)
{
	int w = game.GetWidth();
	int h = game.GetHeight();

	// 如果尺寸变了，重置
	if (w != m_visualW || h != m_visualH)
	{
		m_visualW = w;
		m_visualH = h;
		m_visualGrid.assign(w * h, 0.0f);
	}

	// 衰减系数
	float decay = 0.15f;

	for (int y = 0; y < h; ++y)
	{
		for (int x = 0; x < w; ++x)
		{
			int idx = y * w + x;
			if (game.GetCell(x, y))
			{
				m_visualGrid[idx] = 1.0f; // 活细胞亮度拉满
			}
			else
			{
				if (m_visualGrid[idx] > 0.0f)
				{
					m_visualGrid[idx] -= decay;
					if (m_visualGrid[idx] < 0.0f) m_visualGrid[idx] = 0.0f;
				}
			}
		}
	}
}

void Renderer::Draw(HDC hdc, const LifeGame& game, const RECT* pDirty,
                    bool showResetTip, int clientWidth, int clientHeight)
{
	SetBkMode(hdc, TRANSPARENT);

	// 1. 更新视觉状态 (拖尾计算)
	UpdateVisualGrid(game);

	// 计算布局
	int cellSize, offX, offY, gridWpx, gridHpx;
	CalcLayout(game, cellSize, offX, offY, gridWpx, gridHpx, clientWidth, clientHeight);

	// 填充背景
	RECT clientRect = {0, 0, clientWidth, clientHeight};
	FillRect(hdc, &clientRect, m_hBackgroundBrush);

	// 绘制网格与细胞
	DrawGrid(hdc, game, pDirty, cellSize, offX, offY, gridWpx, gridHpx);

	const auto& settings = SettingsManager::GetInstance().GetSettings();

	// 绘制 HUD 装饰
	if (settings.showHUD)
	{
		DrawHUD(hdc, offX, offY, gridWpx, gridHpx);
	}

	// 绘制统计图表 (HUD 风格，位于网格右下角)
	// 用户要求：移动到左侧面板，不再在画布上绘制
	/*
	if (settings.showHistory)
	{
	    int statsW = 200;
	    int statsH = 100;
	    // 确保不超出网格
	    if (statsW < gridWpx && statsH < gridHpx) {
	        DrawStatistics(hdc, game, offX + gridWpx - statsW - 10, offY + gridHpx - statsH - 10, statsW, statsH);
	    }
	}
	*/

	// 绘制左侧面板 (包含统计图表)
	DrawLeftPanel(hdc, clientWidth, clientHeight, game);

	// 绘制底部状态栏
	DrawStatusBar(hdc, game, clientWidth, clientHeight);

	// 绘制水印
	DrawBranding(hdc, offX, offY + gridHpx + 10, gridWpx);

	// 绘制提示信息
	if (showResetTip)
	{
		DrawResetTip(hdc, offX, offY, gridWpx, gridHpx);
	}
}

void Renderer::CalcLayout(const LifeGame& game, int& outCellSize, int& outOffsetX,
                          int& outOffsetY, int& outGridWidthPx, int& outGridHeightPx,
                          int clientWidth, int clientHeight)
{
	int availW = clientWidth - LEFT_PANEL_WIDTH - 40; // 留出左右边际
	int availH = clientHeight - STATUS_BAR_HEIGHT - 40; // 留出上下边际
	if (availW < 1) availW = 1;
	if (availH < 1) availH = 1;

	int cols = game.GetWidth();
	int rows = game.GetHeight();
	if (cols < 1) cols = 1;
	if (rows < 1) rows = 1;

	// 动态计算单元格大小，使其填满可用区域
	int cellW = availW / cols;
	int cellH = availH / rows;
	int cellSize = (cellW < cellH) ? cellW : cellH;

	// 限制最小和最大尺寸
	if (cellSize < 1) cellSize = 1;
	// if (cellSize > 100) cellSize = 100; // 可选：限制最大尺寸

	int gridW = cellSize * cols;
	int gridH = cellSize * rows;

	// 居中显示
	int offX = LEFT_PANEL_WIDTH + 20 + (availW - gridW) / 2;
	int offY = 20 + (availH - gridH) / 2;

	if (offX < LEFT_PANEL_WIDTH) offX = LEFT_PANEL_WIDTH;
	if (offY < 0) offY = 0;

	outCellSize = cellSize;
	outOffsetX = offX;
	outOffsetY = offY;
	outGridWidthPx = gridW;
	outGridHeightPx = gridH;
}

void Renderer::DrawGrid(HDC hdc, const LifeGame& game, const RECT* pDirty,
                        int cellSize, int offX, int offY, int gridWpx, int gridHpx)
{
	// 绘制网格背景
	RECT gridRect = {offX, offY, offX + gridWpx, offY + gridHpx};
	FillRect(hdc, &gridRect, m_hBackgroundBrush);

	// 绘制网格线 (仅在格子足够大时绘制，避免密集恐惧症)
	const auto& settings = SettingsManager::GetInstance().GetSettings();
	if (settings.showGrid && cellSize >= 4) // 稍微放宽限制
	{
		auto hOldPen = static_cast<HPEN>(SelectObject(hdc, m_hGridPen));
		for (int xi = 0; xi <= game.GetWidth(); xi++)
		{
			int xpos = offX + xi * cellSize;
			MoveToEx(hdc, xpos, offY, nullptr);
			LineTo(hdc, xpos, offY + gridHpx);
		}
		for (int yi = 0; yi <= game.GetHeight(); yi++)
		{
			int ypos = offY + yi * cellSize;
			MoveToEx(hdc, offX, ypos, nullptr);
			LineTo(hdc, offX + gridWpx, ypos);
		}
		SelectObject(hdc, hOldPen);
	}

	// 绘制细胞 (带拖尾和辉光)
	int w = game.GetWidth();
	int h = game.GetHeight();

	// 确保 visualGrid 大小正确
	if (m_visualGrid.size() != w * h) return;

	for (int y = 0; y < h; ++y)
	{
		for (int x = 0; x < w; ++x)
		{
			float brightness = m_visualGrid[y * w + x];

			// 只有亮度大于0才绘制
			if (brightness > 0.01f)
			{
				int left = offX + x * cellSize;
				int top = offY + y * cellSize;
				RECT cell = {left, top, left + cellSize, top + cellSize};

				if (brightness >= 0.99f) // 活细胞 (全亮)
				{
					// 1. 绘制光晕 (稍微大一点)
					if (cellSize > 4)
					{
						RECT glow = cell;
						InflateRect(&glow, 1, 1); // 向外扩张1像素
						// 这里用实心画刷模拟光晕，如果有GDI+可以用半透明
						// 为了性能，我们只画一个稍暗的背景
						FillRect(hdc, &glow, m_hGlowBrush);
					}

					// 2. 绘制核心 (亮白青色)
					RECT core = cell;
					if (cellSize > 6) InflateRect(&core, -1, -1); // 稍微缩小
					FillRect(hdc, &core, m_hAliveBrush);
				}
				else // 拖尾 (衰减)
				{
					// 根据亮度选择画刷
					int level = static_cast<int>(brightness * FADE_LEVELS);
					if (level < 0) level = 0;
					if (level >= FADE_LEVELS) level = FADE_LEVELS - 1;

					if (m_fadeBrushes[level])
					{
						// 拖尾稍微缩小一点，产生"远去"的感觉
						RECT trail = cell;
						if (cellSize > 4) InflateRect(&trail, -1, -1);
						FillRect(hdc, &trail, m_fadeBrushes[level]);
					}
				}
			}
		}
	}
}

void Renderer::DrawHUD(HDC hdc, int offX, int offY, int gridWpx, int gridHpx)
{
	auto hOldPen = static_cast<HPEN>(SelectObject(hdc, m_hHUDPen));
	SelectObject(hdc, GetStockObject(NULL_BRUSH));

	int len = 20; // 拐角长度
	int gap = 5; // 间隙

	// 左上角
	MoveToEx(hdc, offX - gap, offY + len, nullptr);
	LineTo(hdc, offX - gap, offY - gap);
	LineTo(hdc, offX + len, offY - gap);

	// 右上角
	MoveToEx(hdc, offX + gridWpx - len, offY - gap, nullptr);
	LineTo(hdc, offX + gridWpx + gap, offY - gap);
	LineTo(hdc, offX + gridWpx + gap, offY + len);

	// 右下角
	MoveToEx(hdc, offX + gridWpx + gap, offY + gridHpx - len, nullptr);
	LineTo(hdc, offX + gridWpx + gap, offY + gridHpx + gap);
	LineTo(hdc, offX + gridWpx - len, offY + gridHpx + gap);

	// 左下角
	MoveToEx(hdc, offX + len, offY + gridHpx + gap, nullptr);
	LineTo(hdc, offX - gap, offY + gridHpx + gap);
	LineTo(hdc, offX - gap, offY + gridHpx - len);

	// 绘制一些装饰性的刻度
	HPEN hTickPen = CreatePen(PS_SOLID, 1, RGB(0, 100, 120));
	SelectObject(hdc, hTickPen);

	// 顶部刻度
	for (int i = 0; i < gridWpx; i += 50)
	{
		MoveToEx(hdc, offX + i, offY - gap - 2, nullptr);
		LineTo(hdc, offX + i, offY - gap - 6);
	}

	SelectObject(hdc, hOldPen);
	DeleteObject(hTickPen);
}

void Renderer::DrawLeftPanel(HDC hdc, int clientWidth, int clientHeight, const LifeGame& game)
{
	RECT leftPanel = {0, 0, LEFT_PANEL_WIDTH, clientHeight};
	FillRect(hdc, &leftPanel, m_hLeftPanelBrush);

	// 绘制右侧分割线
	HPEN hSplitPen = CreatePen(PS_SOLID, 1, RGB(50, 56, 64));
	auto hOldPen = static_cast<HPEN>(SelectObject(hdc, hSplitPen));
	MoveToEx(hdc, LEFT_PANEL_WIDTH - 1, 0, nullptr);
	LineTo(hdc, LEFT_PANEL_WIDTH - 1, clientHeight);
	SelectObject(hdc, hOldPen);
	DeleteObject(hSplitPen);

	SetBkMode(hdc, TRANSPARENT);
	int panelPaddingX = 16;
	int panelPaddingY = 20;
	int lineH = 28;

	// 1. 绘制统计图表 (位于快捷键上方)
	// 假设控件区域大约占用了顶部 600-700 像素 (根据 UI.cpp 的布局)
	// 我们把图表放在快捷键上方，快捷键放在底部

	int bottomMargin = STATUS_BAR_HEIGHT + 20;
	int shortcutH = 5 * lineH + 40; // 5行快捷键 + 标题
	int graphH = 100;
	int graphW = LEFT_PANEL_WIDTH - 2 * panelPaddingX;

	// 计算起始位置
	int shortcutStartY = clientHeight - bottomMargin - shortcutH;
	int graphStartY = shortcutStartY - graphH - 20; // 图表在快捷键上方 20px

	// 确保不覆盖顶部控件 (假设顶部控件占用 750px)
	if (graphStartY < 750)
	{
		// 如果空间不足，尝试压缩
		graphStartY = 750;
		shortcutStartY = graphStartY + graphH + 20;
	}

	// 绘制图表
	const auto& settings = SettingsManager::GetInstance().GetSettings();
	if (settings.showHistory)
	{
		DrawStatistics(hdc, game, panelPaddingX, graphStartY, graphW, graphH);
	}

	// 2. 绘制快捷键列表
	struct Shortcut
	{
		const TCHAR* key;
		const TCHAR* desc;
	};
	Shortcut sc[] = {
		{TEXT("SPACE"), L"开始/暂停"},
		{TEXT("R"), L"重置画布"},
		{TEXT("G"), L"随机生成"},
		{TEXT("+ / -"), L"速度调节"},
		{TEXT("ESC"), L"退出程序"}
	};

	int keyColW = 60;

	// 绘制一个小标题 "快捷操作"
	RECT titleR = {panelPaddingX, shortcutStartY, LEFT_PANEL_WIDTH, shortcutStartY + 30};
	SelectObject(hdc, m_hLeftDescFont);
	SetTextColor(hdc, m_colTextDim);
	DrawText(hdc, TEXT("快捷操作"), -1, &titleR, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	int listStartY = shortcutStartY + 30;
	for (int i = 0; i < static_cast<int>(sizeof(sc) / sizeof(sc[0])); ++i)
	{
		int y = listStartY + i * lineH;
		RECT keyR = {panelPaddingX, y, panelPaddingX + keyColW, y + lineH};
		RECT descR = {panelPaddingX + keyColW + 4, y, LEFT_PANEL_WIDTH - 10, y + lineH};

		auto hOld = static_cast<HFONT>(SelectObject(hdc, m_hLeftKeyFont));
		SetTextColor(hdc, m_colHighlight); // 高亮快捷键
		DrawText(hdc, sc[i].key, -1, &keyR, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

		SelectObject(hdc, m_hLeftDescFont);
		SetTextColor(hdc, m_colText); // 普通文本颜色
		DrawText(hdc, sc[i].desc, -1, &descR, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_WORDBREAK);
		SelectObject(hdc, hOld);
	}
}

void Renderer::DrawStatusBar(HDC hdc, const LifeGame& game, int clientWidth, int clientHeight)
{
	RECT statusRect = {0, clientHeight - STATUS_BAR_HEIGHT, clientWidth, clientHeight};
	HBRUSH statusBg = CreateSolidBrush(RGB(15, 18, 22));
	FillRect(hdc, &statusRect, statusBg);
	DeleteObject(statusBg);

	// 顶部高亮线
	HPEN hTopPen = CreatePen(PS_SOLID, 1, RGB(0, 100, 120));
	auto hOldPen = static_cast<HPEN>(SelectObject(hdc, hTopPen));
	MoveToEx(hdc, 0, clientHeight - STATUS_BAR_HEIGHT, nullptr);
	LineTo(hdc, clientWidth, clientHeight - STATUS_BAR_HEIGHT);
	SelectObject(hdc, hOldPen);
	DeleteObject(hTopPen);

	SelectObject(hdc, m_hDataFont);
	SetBkMode(hdc, TRANSPARENT);

	// 1. 状态指示灯
	bool running = game.IsRunning();
	HBRUSH hStatusBrush = CreateSolidBrush(running ? RGB(0, 255, 100) : RGB(255, 200, 0));
	RECT lightRect = {16, clientHeight - 20, 24, clientHeight - 12};
	FillRect(hdc, &lightRect, hStatusBrush);
	DeleteObject(hStatusBrush);

	SetTextColor(hdc, m_colText);
	RECT textRect = {32, clientHeight - STATUS_BAR_HEIGHT, 300, clientHeight};
	DrawText(hdc, running ? TEXT("Wuhan University") : TEXT("Wuhan University (PAUSED)"), -1, &textRect,
	         DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	// 2. 种群数量能量条
	int pop = game.GetPopulation();
	int maxPop = game.GetWidth() * game.GetHeight() / 2; // 估算最大值
	if (maxPop < 1) maxPop = 1;
	float ratio = static_cast<float>(pop) / maxPop;
	if (ratio > 1.0f) ratio = 1.0f;

	int barW = 200;
	int barH = 8;
	int barX = clientWidth / 2 - barW / 2;
	int barY = clientHeight - STATUS_BAR_HEIGHT / 2 - barH / 2;

	// 绘制槽
	HBRUSH hSlot = CreateSolidBrush(RGB(30, 40, 50));
	RECT slotRect = {barX, barY, barX + barW, barY + barH};
	FillRect(hdc, &slotRect, hSlot);
	DeleteObject(hSlot);

	// 绘制能量
	int fillW = static_cast<int>(barW * ratio);
	HBRUSH hEnergy = CreateSolidBrush(m_colHighlight);
	RECT energyRect = {barX, barY, barX + fillW, barY + barH};
	FillRect(hdc, &energyRect, hEnergy);
	DeleteObject(hEnergy);

	// 绘制文字
	TCHAR popText[64];
	_stprintf_s(popText, TEXT("POPULATION: %d"), pop);
	// 增加文本区域宽度，防止数字被截断
	RECT popTextRect = {barX + barW + 10, clientHeight - STATUS_BAR_HEIGHT, barX + barW + 250, clientHeight};
	SetTextColor(hdc, m_colHighlight);
	DrawText(hdc, popText, -1, &popTextRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	// 3. 右侧信息
	TCHAR rightStatus[128];
	_stprintf_s(rightStatus, TEXT("GRID: %dx%d | SPEED: %dms"),
	            game.GetWidth(), game.GetHeight(), game.GetSpeed());
	RECT rightRect = {clientWidth - 250, clientHeight - STATUS_BAR_HEIGHT, clientWidth - 16, clientHeight};
	SetTextColor(hdc, m_colTextDim);
	DrawText(hdc, rightStatus, -1, &rightRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
}

void Renderer::DrawBranding(HDC hdc, int x, int y, int w)
{
	// 在网格右下角绘制水印
	RECT r = {x, y, x + w, y + 30};
	SelectObject(hdc, m_hBrandingFont);
	SetTextColor(hdc, RGB(60, 70, 80)); // 很淡的颜色，不抢眼
	DrawText(hdc, L"武汉大学计算机弘毅班项目", -1, &r, DT_RIGHT | DT_TOP | DT_SINGLELINE);
}

void Renderer::DrawStatistics(HDC hdc, const LifeGame& game, int x, int y, int w, int h)
{
	// 绘制半透明背景 (模拟)
	// 由于 GDI 不支持 AlphaBlend (除非用 GdiAlphaBlend 且需要 msimg32.lib)，
	// 我们用深色背景代替
	HBRUSH hBg = CreateSolidBrush(RGB(10, 15, 20));
	RECT r = {x, y, x + w, y + h};
	FillRect(hdc, &r, hBg);
	DeleteObject(hBg);

	// 绘制边框
	HPEN hBorder = CreatePen(PS_SOLID, 1, RGB(0, 80, 100));
	auto hOldPen = static_cast<HPEN>(SelectObject(hdc, hBorder));
	SelectObject(hdc, GetStockObject(NULL_BRUSH));
	Rectangle(hdc, x, y, x + w, y + h);
	SelectObject(hdc, hOldPen);
	DeleteObject(hBorder);

	// 获取数据
	const auto& history = game.GetStatistics().GetPopulationHistory();
	if (history.size() < 2) return;

	int maxPop = game.GetStatistics().GetMaxPopulation();
	if (maxPop == 0) maxPop = 100; // 避免除零

	// 绘制曲线
	SelectObject(hdc, m_hGraphPen);

	int count = static_cast<int>(history.size());
	float stepX = static_cast<float>(w) / (count - 1);

	// 找到起始点
	// y坐标翻转：值越大越靠上
	// y = (y + h) - (val / max) * h

	auto getPt = [&](int i) -> POINT
	{
		int val = history[i];
		int px = x + static_cast<int>(i * stepX);
		int py = (y + h) - static_cast<int>((float)val / maxPop * (h - 10)) - 5; // 留出边距
		return {px, py};
	};

	POINT pt = getPt(0);
	MoveToEx(hdc, pt.x, pt.y, nullptr);

	for (int i = 1; i < count; ++i)
	{
		pt = getPt(i);
		LineTo(hdc, pt.x, pt.y);
	}

	// 绘制标题
	SelectObject(hdc, m_hDataFont);
	SetTextColor(hdc, RGB(0, 255, 100));
	RECT titleR = {x + 5, y + 5, x + w, y + 20};
	DrawText(hdc, TEXT("POPULATION HISTORY"), -1, &titleR, DT_LEFT | DT_TOP | DT_SINGLELINE);
}

void Renderer::DrawResetTip(HDC hdc, int offX, int offY, int gridWpx, int gridHpx)
{
	auto tip = L"已重置。按 G 键随机生成细胞";
	auto hOld = static_cast<HFONT>(SelectObject(hdc, m_hTipFont));

	RECT measure = {0, 0, 0, 0};
	DrawText(hdc, tip, -1, &measure, DT_CALCRECT | DT_SINGLELINE);
	int tw = measure.right - measure.left + 20;
	int th = measure.bottom - measure.top + 10;

	RECT box = {
		offX + (gridWpx - tw) / 2, offY + (gridHpx - th) / 2,
		offX + (gridWpx + tw) / 2, offY + (gridHpx + th) / 2
	};

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
