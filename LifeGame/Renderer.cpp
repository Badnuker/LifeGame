#include "Renderer.h"
#include "Game.h"
#include "Settings.h"
#include <tchar.h>
#include <stdio.h>
#include <algorithm>

/**
 * @brief 构造函数
 * 
 * 初始化所有成员变量，设置默认的赛博朋克配色方案。
 * 此时尚未创建 GDI 资源，资源创建在 Initialize 中进行。
 */
Renderer::Renderer()
    : m_visualW(0), m_visualH(0), m_scale(1.0f),
      m_viewOffsetX(0), m_viewOffsetY(0), m_hBackgroundBrush(nullptr),
      m_hAliveBrush(nullptr), m_hGlowBrush(nullptr), m_hDeadBrush(nullptr), m_hTipBrush(nullptr),
      m_hLeftPanelBrush(nullptr), m_hInputBgBrush(nullptr), m_hGridPen(nullptr), m_hBorderPen(nullptr),
      m_hHUDPen(nullptr), m_hTitleFont(nullptr), m_hTipFont(nullptr), m_hBtnFont(nullptr),
      m_hControlFont(nullptr), m_hLeftKeyFont(nullptr),
      m_hLeftDescFont(nullptr), m_hBrandingFont(nullptr), m_hDataFont(nullptr), m_previewX(-1),
      m_previewY(-1), m_previewPatternIndex(-1),
      m_isEraserPreview(false), m_eraserSize(1), m_hPreviewBrush(nullptr), m_hEraserPen(nullptr) {
    // 赛博朋克配色方案
    m_colText = RGB(220, 240, 255); // 亮白蓝
    m_colTextDim = RGB(100, 130, 150); // 灰蓝
    m_colHighlight = RGB(0, 255, 255); // 赛博青
    m_colWarning = RGB(255, 50, 80); // 警示红

    // 初始化衰减画刷数组为空
    for (int i = 0; i < FADE_LEVELS; ++i) m_fadeBrushes[i] = nullptr;
}

Renderer::~Renderer() {
    Cleanup();
}

/**
 * @brief 初始化 GDI 资源
 *
 * 创建画刷、画笔和字体。
 *
 * @param hInstance 应用程序实例句柄
 * @return true 如果关键资源创建成功
 */
bool Renderer::Initialize(HINSTANCE hInstance) {
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
    for (int i = 0; i < FADE_LEVELS; ++i) {
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
    m_hBrandingFont = CreateFontW(-16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                  DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                  VARIABLE_PITCH | FF_SWISS, L"Microsoft YaHei UI");
    m_hDataFont = CreateFontW(-18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                              DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                              VARIABLE_PITCH | FF_SWISS, L"Consolas");

    // 4. 预览资源
    // 使用阴影线画刷模拟半透明
    m_hPreviewBrush = CreateHatchBrush(HS_BDIAGONAL, RGB(100, 255, 255));
    m_hEraserPen = CreatePen(PS_SOLID, 2, RGB(255, 50, 50));

    return (m_hBackgroundBrush && m_hAliveBrush && m_hGridPen);
}

/**
 * @brief 清理资源
 */
void Renderer::Cleanup() {
    if (m_hGridPen) DeleteObject(m_hGridPen);
    if (m_hBorderPen) DeleteObject(m_hBorderPen);
    if (m_hHUDPen) DeleteObject(m_hHUDPen);
    if (m_hGraphPen) DeleteObject(m_hGraphPen);
    if (m_hEraserPen) DeleteObject(m_hEraserPen); // 新增
    if (m_hBackgroundBrush) DeleteObject(m_hBackgroundBrush);
    if (m_hAliveBrush) DeleteObject(m_hAliveBrush);
    if (m_hGlowBrush) DeleteObject(m_hGlowBrush);
    if (m_hDeadBrush) DeleteObject(m_hDeadBrush);
    if (m_hTipBrush) DeleteObject(m_hTipBrush);
    if (m_hLeftPanelBrush) DeleteObject(m_hLeftPanelBrush);
    if (m_hInputBgBrush) DeleteObject(m_hInputBgBrush);
    if (m_hPreviewBrush) DeleteObject(m_hPreviewBrush); // 新增

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

/**
 * @brief 更新设置
 *
 * 当用户在设置对话框中更改颜色或网格设置时调用。
 */
void Renderer::UpdateSettings() {
    const auto &settings = SettingsManager::GetInstance().GetSettings();

    // 重建画刷和笔
    if (m_hBackgroundBrush) DeleteObject(m_hBackgroundBrush);
    m_hBackgroundBrush = CreateSolidBrush(settings.bgColor);

    if (m_hAliveBrush) DeleteObject(m_hAliveBrush);
    m_hAliveBrush = CreateSolidBrush(settings.cellColor);

    // 更新光晕画刷 (基于活细胞颜色，稍微暗一点)
    if (m_hGlowBrush) DeleteObject(m_hGlowBrush);
    int r = GetRValue(settings.cellColor);
    int g = GetGValue(settings.cellColor);
    int b = GetBValue(settings.cellColor);
    // 简单的变暗处理
    m_hGlowBrush = CreateSolidBrush(RGB(r * 0.8, g * 0.8, b * 0.8));

    // 更新衰减画刷
    for (int i = 0; i < FADE_LEVELS; ++i) {
        if (m_fadeBrushes[i]) DeleteObject(m_fadeBrushes[i]);

        // 重新计算渐变: 从 cellColor 到 bgColor
        float ratio = static_cast<float>(i + 1) / (FADE_LEVELS + 2);

        int bgR = GetRValue(settings.bgColor);
        int bgG = GetGValue(settings.bgColor);
        int bgB = GetBValue(settings.bgColor);

        int newR = bgR + static_cast<int>((r - bgR) * ratio);
        int newG = bgG + static_cast<int>((g - bgG) * ratio);
        int newB = bgB + static_cast<int>((b - bgB) * ratio);

        m_fadeBrushes[i] = CreateSolidBrush(RGB(newR, newG, newB));
    }

    if (m_hGridPen) DeleteObject(m_hGridPen);
    m_hGridPen = CreatePen(PS_SOLID, settings.gridLineWidth, settings.gridColor);

    // 更新文字颜色
    m_colText = settings.textColor;
}

/**
 * @brief 清除视觉残留
 */
void Renderer::ClearVisuals() {
    std::fill(m_visualGrid.begin(), m_visualGrid.end(), 0.0f);
}

/**
 * @brief 更新视觉网格 (计算拖尾)
 *
 * 遍历所有细胞，如果细胞存活，亮度设为 1.0。
 * 如果细胞死亡，亮度逐渐衰减，形成拖尾效果。
 */
void Renderer::UpdateVisualGrid(const LifeGame &game) {
    int w = game.GetWidth();
    int h = game.GetHeight();

    // 如果尺寸变了，重置
    if (w != m_visualW || h != m_visualH) {
        m_visualW = w;
        m_visualH = h;
        m_visualGrid.assign(w * h, 0.0f);
    }

    // 衰减系数
    float decay = 0.15f;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = y * w + x;
            if (game.GetCell(x, y)) {
                m_visualGrid[idx] = 1.0f; // 活细胞亮度拉满
            } else {
                if (m_visualGrid[idx] > 0.0f) {
                    m_visualGrid[idx] -= decay; // 死细胞亮度衰减
                    if (m_visualGrid[idx] < 0.0f) m_visualGrid[idx] = 0.0f;
                }
            }
        }
    }
}

void Renderer::SetPreview(int x, int y, int patternIndex, bool isEraser, int eraserSize) {
    m_previewX = x;
    m_previewY = y;
    m_previewPatternIndex = patternIndex;
    m_isEraserPreview = isEraser;
    m_eraserSize = eraserSize;
}

void Renderer::SetScale(float scale) {
    if (scale < 0.1f) scale = 0.1f;
    if (scale > 10.0f) scale = 10.0f;
    m_scale = scale;
}

void Renderer::SetOffset(int x, int y) {
    m_viewOffsetX = x;
    m_viewOffsetY = y;
}

void Renderer::Pan(int dx, int dy) {
    m_viewOffsetX += dx;
    m_viewOffsetY += dy;
}

void Renderer::Zoom(float factor, int centerX, int centerY) {
    float oldScale = m_scale;
    float newScale = oldScale * factor;
    if (newScale < 0.1f) newScale = 0.1f;
    if (newScale > 10.0f) newScale = 10.0f;

    if (newScale == oldScale) return;

    m_scale = newScale;
}

/**
 * @brief 核心绘制函数
 *
 * 每一帧调用一次，负责绘制整个游戏界面。
 */
void Renderer::Draw(HDC hdc, const LifeGame &game, const RECT *pDirty,
                    bool showResetTip, int clientWidth, int clientHeight) {
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
    DrawGrid(hdc, game, pDirty, cellSize, offX, offY, gridWpx, gridHpx, clientWidth, clientHeight);

    // 绘制预览 (新增)
    DrawPreview(hdc, game, cellSize, offX, offY);

    const auto &settings = SettingsManager::GetInstance().GetSettings();

    // 绘制 HUD 装饰
    if (settings.showHUD) {
        DrawHUD(hdc, offX, offY, gridWpx, gridHpx);
    }

    // 绘制左侧面板 (包含统计图表)
    DrawLeftPanel(hdc, clientWidth, clientHeight, game);

    // 绘制底部状态栏
    DrawStatusBar(hdc, game, clientWidth, clientHeight);

    // 绘制水印
    DrawBranding(hdc, offX, offY + gridHpx + 10, gridWpx);

    // 绘制提示信息
    if (showResetTip) {
        DrawResetTip(hdc, offX, offY, gridWpx, gridHpx);
    }
}

/**
 * @brief 绘制预览
 *
 * 在鼠标位置绘制即将放置的图案预览或橡皮擦范围。
 */
void Renderer::DrawPreview(HDC hdc, const LifeGame &game, int cellSize, int offX, int offY) {
    if (m_previewX < 0 || m_previewY < 0 || m_previewX >= game.GetWidth() || m_previewY >= game.GetHeight())
        return;

    if (m_isEraserPreview) {
        // 绘制红色边框表示擦除范围
        int halfSize = m_eraserSize / 2;
        int startX = m_previewX - halfSize;
        int startY = m_previewY - halfSize;

        auto hOldPen = static_cast<HPEN>(SelectObject(hdc, m_hEraserPen));
        SelectObject(hdc, GetStockObject(NULL_BRUSH));

        // 绘制每个要擦除的格子
        for (int dy = 0; dy < m_eraserSize; ++dy) {
            for (int dx = 0; dx < m_eraserSize; ++dx) {
                int cellX = startX + dx;
                int cellY = startY + dy;

                // 检查边界
                if (cellX < 0 || cellX >= game.GetWidth() || cellY < 0 || cellY >= game.GetHeight())
                    continue;

                int left = offX + cellX * cellSize;
                int top = offY + cellY * cellSize;
                RECT r = {left, top, left + cellSize, top + cellSize};

                Rectangle(hdc, r.left, r.top, r.right, r.bottom);

                // 画个叉
                MoveToEx(hdc, r.left, r.top, nullptr);
                LineTo(hdc, r.right, r.bottom);
                MoveToEx(hdc, r.right, r.top, nullptr);
                LineTo(hdc, r.left, r.bottom);
            }
        }

        SelectObject(hdc, hOldPen);
    } else {
        // 绘制图案预览
        const PatternData *p = game.GetPatternLibrary().GetPattern(m_previewPatternIndex);

        // 如果是单点绘制 (index 0) 或找不到图案，只画一个点
        if (m_previewPatternIndex <= 0 || !p) {
            int left = offX + m_previewX * cellSize;
            int top = offY + m_previewY * cellSize;
            RECT r = {left, top, left + cellSize, top + cellSize};
            FillRect(hdc, &r, m_hPreviewBrush);
        } else {
            // 解析并绘制图案
            std::vector<std::vector<bool> > grid;

            // 只画一个矩形框表示范围
            int w = p->width;
            int h = p->height;
            int left = offX + m_previewX * cellSize;
            int top = offY + m_previewY * cellSize;
            int right = left + w * cellSize;
            int bottom = top + h * cellSize;

            RECT r = {left, top, right, bottom};
            FrameRect(hdc, &r, m_hPreviewBrush);

            // 填充左上角表示起始点
            RECT start = {left, top, left + cellSize, top + cellSize};
            FillRect(hdc, &start, m_hPreviewBrush);
        }
    }
}

/**
 * @brief 计算布局
 *
 * 根据窗口大小和缩放比例，计算网格的显示位置和细胞大小。
 * 实现了自动居中和自适应布局。
 */
void Renderer::CalcLayout(const LifeGame &game, int &outCellSize, int &outOffsetX,
                          int &outOffsetY, int &outGridWidthPx, int &outGridHeightPx,
                          int clientWidth, int clientHeight) {
    int availW = clientWidth - LEFT_PANEL_WIDTH - 40;
    int availH = clientHeight - STATUS_BAR_HEIGHT - 40;
    if (availW < 1) availW = 1;
    if (availH < 1) availH = 1;

    int cols = game.GetWidth();
    int rows = game.GetHeight();
    if (cols < 1) cols = 1;
    if (rows < 1) rows = 1;

    // 基础单元格大小 (不缩放时的大小)
    int baseCellSize = BASE_CELL_SIZE;

    // 1. 计算"最佳适应"的大小
    int fitCellW = availW / cols;
    int fitCellH = availH / rows;
    int fitSize = (fitCellW < fitCellH) ? fitCellW : fitCellH;
    if (fitSize < 1) fitSize = 1;

    // 2. 应用缩放
    float rawSize = static_cast<float>(fitSize);
    if (rawSize < 2.0f) rawSize = 2.0f; // 最小基础大小

    // 如果网格巨大，fitSize 只有 1 或 0。这时候基础大小应该设为比如 10，然后允许用户缩放。
    if (cols > 500 || rows > 500) rawSize = 10.0f;

    int cellSize = static_cast<int>(rawSize * m_scale);
    if (cellSize < 1) cellSize = 1;

    int gridW = cellSize * cols;
    int gridH = cellSize * rows;

    // 居中显示 + 偏移
    int offX = LEFT_PANEL_WIDTH + 20 + (availW - gridW) / 2 + m_viewOffsetX;
    int offY = 20 + (availH - gridH) / 2 + m_viewOffsetY;

    outCellSize = cellSize;
    outOffsetX = offX;
    outOffsetY = offY;
    outGridWidthPx = gridW;
    outGridHeightPx = gridH;
}

/**
 * @brief 绘制网格和细胞
 *
 * 实现了视锥裁剪优化，只绘制可见区域的网格和细胞。
 */
void Renderer::DrawGrid(HDC hdc, const LifeGame &game, const RECT *pDirty,
                        int cellSize, int offX, int offY, int gridWpx, int gridHpx,
                        int clientWidth, int clientHeight) {
    // 裁剪优化：只绘制可见区域
    // 可见区域：[LEFT_PANEL_WIDTH, 0] 到 [clientWidth, clientHeight - STATUS_BAR_HEIGHT]
    int viewL = LEFT_PANEL_WIDTH;
    int viewT = 0;
    int viewR = clientWidth;
    int viewB = clientHeight - STATUS_BAR_HEIGHT;

    // 计算可见的网格索引范围
    int startCol = (viewL - offX) / cellSize;
    int endCol = (viewR - offX) / cellSize + 1;
    int startRow = (viewT - offY) / cellSize;
    int endRow = (viewB - offY) / cellSize + 1;

    if (startCol < 0) startCol = 0;
    if (endCol > game.GetWidth()) endCol = game.GetWidth();
    if (startRow < 0) startRow = 0;
    if (endRow > game.GetHeight()) endRow = game.GetHeight();

    if (startCol >= endCol || startRow >= endRow) return; // 不可见

    // 绘制网格背景 (仅绘制可见部分)
    int drawL = max(offX, viewL);
    int drawT = max(offY, viewT);
    int drawR = min(offX + gridWpx, viewR);
    int drawB = min(offY + gridHpx, viewB);

    if (drawR > drawL && drawB > drawT) {
        RECT gridRect = {drawL, drawT, drawR, drawB};
        FillRect(hdc, &gridRect, m_hBackgroundBrush);
    }

    // 绘制网格线
    const auto &settings = SettingsManager::GetInstance().GetSettings();
    if (settings.showGrid && cellSize >= 4) {
        auto hOldPen = static_cast<HPEN>(SelectObject(hdc, m_hGridPen));

        // 只绘制可见范围内的线
        for (int xi = startCol; xi <= endCol; xi++) {
            int xpos = offX + xi * cellSize;
            if (xpos >= viewL && xpos <= viewR) {
                MoveToEx(hdc, xpos, max(offY, viewT), nullptr);
                LineTo(hdc, xpos, min(offY + gridHpx, viewB));
            }
        }
        for (int yi = startRow; yi <= endRow; yi++) {
            int ypos = offY + yi * cellSize;
            if (ypos >= viewT && ypos <= viewB) {
                MoveToEx(hdc, max(offX, viewL), ypos, nullptr);
                LineTo(hdc, min(offX + gridWpx, viewR), ypos);
            }
        }
        SelectObject(hdc, hOldPen);
    }

    // 绘制细胞
    int w = game.GetWidth();
    int h = game.GetHeight();

    // 确保 visualGrid 大小正确
    if (m_visualGrid.size() != w * h) return;

    for (int y = startRow; y < endRow; ++y) {
        for (int x = startCol; x < endCol; ++x) {
            float brightness = m_visualGrid[y * w + x];

            if (brightness > 0.01f) {
                int left = offX + x * cellSize;
                int top = offY + y * cellSize;
                // 简单的可见性检查 (其实上面循环已经保证了大部分)
                if (left >= viewR || top >= viewB || left + cellSize <= viewL || top + cellSize <= viewT)
                    continue;

                RECT cell = {left, top, left + cellSize, top + cellSize};

                if (brightness >= 0.99f) {
                    if (cellSize > 4) {
                        RECT glow = cell;
                        InflateRect(&glow, 1, 1);
                        FillRect(hdc, &glow, m_hGlowBrush);
                    }
                    RECT core = cell;
                    if (cellSize > 6) InflateRect(&core, -1, -1);
                    FillRect(hdc, &core, m_hAliveBrush);
                } else {
                    int level = static_cast<int>(brightness * FADE_LEVELS);
                    if (level < 0) level = 0;
                    if (level >= FADE_LEVELS) level = FADE_LEVELS - 1;

                    if (m_fadeBrushes[level]) {
                        RECT trail = cell;
                        if (cellSize > 4) InflateRect(&trail, -1, -1);
                        FillRect(hdc, &trail, m_fadeBrushes[level]);
                    }
                }
            }
        }
    }
}

/**
 * @brief 绘制 HUD 装饰线
 */
void Renderer::DrawHUD(HDC hdc, int offX, int offY, int gridWpx, int gridHpx) {
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
    for (int i = 0; i < gridWpx; i += 50) {
        MoveToEx(hdc, offX + i, offY - gap - 2, nullptr);
        LineTo(hdc, offX + i, offY - gap - 6);
    }

    SelectObject(hdc, hOldPen);
    DeleteObject(hTickPen);
}

/**
 * @brief 绘制左侧控制面板
 *
 * 包含统计图表、快捷键列表等。
 */
void Renderer::DrawLeftPanel(HDC hdc, int clientWidth, int clientHeight, const LifeGame &game) {
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
    int bottomMargin = STATUS_BAR_HEIGHT + 20;
    int shortcutH = 5 * lineH + 40; // 5行快捷键 + 标题
    int graphH = 100;
    int graphW = LEFT_PANEL_WIDTH - 2 * panelPaddingX;

    // 计算起始位置
    int shortcutStartY = clientHeight - bottomMargin - shortcutH;
    int graphStartY = shortcutStartY - graphH - 20; // 图表在快捷键上方 20px

    // 确保不覆盖顶部控件 (假设顶部控件占用 750px)
    if (graphStartY < 750) {
        // 如果空间不足，尝试压缩
        graphStartY = 750;
        shortcutStartY = graphStartY + graphH + 20;
    }

    // 绘制图表
    const auto &settings = SettingsManager::GetInstance().GetSettings();
    if (settings.showHistory) {
        DrawStatistics(hdc, game, panelPaddingX, graphStartY, graphW, graphH);
    }

    // 2. 绘制快捷键列表
    struct Shortcut {
        const TCHAR *key;
        const TCHAR *desc;
    };
    Shortcut sc[] = {
        {TEXT("左键"), L"绘制"},
        {TEXT("右键"), L"拖动"},
        {TEXT("SPACE"), L"开始/暂停"},
        {TEXT("R"), L"重置画布"},
        {TEXT("G"), L"随机生成"},
        {TEXT("+ / -"), L"速度调节"},
        {TEXT("ESC"), L"退出程序"}
    };

    int keyColW = 90; // 增加宽度以完整显示快捷键文字

    // 绘制一个小标题 "快捷操作"
    RECT titleR = {panelPaddingX, shortcutStartY, LEFT_PANEL_WIDTH, shortcutStartY + 30};
    SelectObject(hdc, m_hLeftDescFont);
    SetTextColor(hdc, m_colTextDim);
    DrawText(hdc, TEXT("快捷操作"), -1, &titleR, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    int listStartY = shortcutStartY + 30;
    for (int i = 0; i < static_cast<int>(sizeof(sc) / sizeof(sc[0])); ++i) {
        int y = listStartY + i * lineH;
        RECT keyR = {panelPaddingX, y, panelPaddingX + keyColW, y + lineH};
        RECT descR = {panelPaddingX + keyColW + 4, y, LEFT_PANEL_WIDTH - 10, y + lineH};

        auto hOld = static_cast<HFONT>(SelectObject(hdc, m_hLeftKeyFont));
        SetTextColor(hdc, m_colHighlight); // 高亮快捷键
        DrawText(hdc, sc[i].key, -1, &keyR, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        SelectObject(hdc, m_hLeftDescFont);
        SetTextColor(hdc, m_colText); // 普通文本颜色
        DrawText(hdc, sc[i].desc, -1, &descR, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdc, hOld);
    }
}

/**
 * @brief 绘制底部状态栏
 */
void Renderer::DrawStatusBar(HDC hdc, const LifeGame &game, int clientWidth, int clientHeight) {
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

/**
 * @brief 绘制水印
 */
void Renderer::DrawBranding(HDC hdc, int x, int y, int w) {
    // 在网格右下角绘制水印
    RECT r = {x, y, x + w, y + 30};
    SelectObject(hdc, m_hBrandingFont);
    SetTextColor(hdc, RGB(60, 70, 80)); // 很淡的颜色，不抢眼
    DrawText(hdc, L"武汉大学计算机弘毅班项目", -1, &r, DT_RIGHT | DT_TOP | DT_SINGLELINE);
}

/**
 * @brief 绘制统计图表
 */
void Renderer::DrawStatistics(HDC hdc, const LifeGame &game, int x, int y, int w, int h) {
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
    const auto &history = game.GetStatistics().GetPopulationHistory();
    if (history.size() < 2) return;

    int maxPop = game.GetStatistics().GetMaxPopulation();
    if (maxPop == 0) maxPop = 100; // 避免除零

    // 绘制曲线
    SelectObject(hdc, m_hGraphPen);

    int count = static_cast<int>(history.size());
    float stepX = static_cast<float>(w) / (count - 1);

    auto getPt = [&](int i) -> POINT {
        int val = history[i];
        int px = x + static_cast<int>(i * stepX);
        int py = (y + h) - static_cast<int>(static_cast<float>(val) / maxPop * (h - 10)) - 5; // 留出边距
        return {px, py};
    };

    POINT pt = getPt(0);
    MoveToEx(hdc, pt.x, pt.y, nullptr);

    for (int i = 1; i < count; ++i) {
        pt = getPt(i);
        LineTo(hdc, pt.x, pt.y);
    }
}

/**
 * @brief 绘制重置提示
 */
void Renderer::DrawResetTip(HDC hdc, int offX, int offY, int gridWpx, int gridHpx) {
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