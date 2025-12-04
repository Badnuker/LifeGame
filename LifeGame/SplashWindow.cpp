#include "SplashWindow.h"
#include <tchar.h>
#include <cmath>
#include <algorithm>

// 链接 msimg32.lib 以使用 GradientFill (如果需要)
#pragma comment(lib, "msimg32.lib")

/**
 * @brief 构造函数
 * 
 * 初始化成员变量，设置默认动画参数。
 */
SplashWindow::SplashWindow()
	: m_hWnd(nullptr), m_mode(0), m_width(0), m_height(0),
	  m_frame(0), m_maxFrames(180), m_alpha(0.0f), // 3秒 @ 60fps
	  m_hTitleFont(nullptr), m_hSubFont(nullptr)
{
}

SplashWindow::~SplashWindow()
{
	if (m_hTitleFont) DeleteObject(m_hTitleFont);
	if (m_hSubFont) DeleteObject(m_hSubFont);
	if (IsWindow(m_hWnd)) DestroyWindow(m_hWnd);
}

/**
 * @brief 显示启动/退出画面
 * 
 * 创建一个全屏无边框窗口，并启动动画定时器。
 * 
 * @param hInstance 应用程序实例句柄
 * @param mode 0: 启动画面 (开机), 1: 退出画面 (关机)
 */
void SplashWindow::Show(HINSTANCE hInstance, int mode)
{
	m_mode = mode;
	m_frame = 0;
	m_maxFrames = 180; // 开机关机都设为3秒 (60fps * 3)

	// 注册窗口类
	WNDCLASS wc = {0};
	wc.lpfnWndProc = StaticWndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = TEXT("LifeGameSplash");
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
	RegisterClass(&wc);

	// 获取屏幕尺寸
	int screenW = GetSystemMetrics(SM_CXSCREEN);
	int screenH = GetSystemMetrics(SM_CYSCREEN);
	m_width = screenW;
	m_height = screenH;

	// 初始化星星
	InitStars(screenW, screenH);

	// 创建全屏无边框窗口
	m_hWnd = CreateWindowEx(
		WS_EX_TOPMOST | WS_EX_TOOLWINDOW, // 最顶层，不在任务栏显示
		TEXT("LifeGameSplash"),
		TEXT("Splash"),
		WS_POPUP | WS_VISIBLE,
		0, 0, screenW, screenH,
		nullptr, nullptr, hInstance, this
	);

	// 创建字体
	m_hTitleFont = CreateFontW(
		-80, 0, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
		VARIABLE_PITCH | FF_SWISS, L"Microsoft YaHei UI"
	);

	m_hSubFont = CreateFontW(
		-60, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
		VARIABLE_PITCH | FF_SWISS, L"Consolas"
	);

	// 启动定时器 60FPS
	SetTimer(m_hWnd, 1, 16, nullptr);
}

/**
 * @brief 运行消息循环
 * 
 * 阻塞直到窗口关闭。
 */
void SplashWindow::RunLoop()
{
	if (!m_hWnd) return;
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

/**
 * @brief 静态窗口过程
 * 
 * 将消息转发给实例的 WndProc。
 */
LRESULT CALLBACK SplashWindow::StaticWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SplashWindow* pThis = nullptr;
	if (uMsg == WM_NCCREATE)
	{
		auto pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
		pThis = reinterpret_cast<SplashWindow*>(pCreate->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
	}
	else
	{
		pThis = reinterpret_cast<SplashWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	}

	if (pThis) return pThis->WndProc(hWnd, uMsg, wParam, lParam);
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT SplashWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_PAINT:
		OnPaint(hWnd);
		return 0;
	case WM_TIMER:
		OnTimer(hWnd);
		return 0;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) DestroyWindow(hWnd); // 允许按ESC跳过
		return 0;
	case WM_DESTROY:
		m_hWnd = nullptr;
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/**
 * @brief 初始化星空
 * 
 * 生成大量随机分布的星星，用于 3D 飞行效果。
 */
void SplashWindow::InitStars(int width, int height)
{
	m_stars.clear();
	int count = 2500; // 大幅增加星星数量 (1000 -> 2500)
	for (int i = 0; i < count; ++i)
	{
		Star s;
		// 随机分布在 -width 到 width 之间
		s.x = static_cast<float>(rand() % (width * 2) - width);
		s.y = static_cast<float>(rand() % (height * 2) - height);
		s.z = static_cast<float>(rand() % 2000 + 1); // 深度
		s.speed = static_cast<float>(rand() % 20 + 10); // 提高基础速度
		s.size = rand() % 6 + 3; // 增大星星尺寸 (3-8)

		// 随机颜色 (增加一些变化)
		int type = rand() % 10;
		if (type < 6) s.color = RGB(200, 240, 255); // 蓝白
		else if (type < 9) s.color = RGB(255, 255, 255); // 纯白
		else s.color = RGB(200, 100, 255); // 紫色点缀

		m_stars.push_back(s);
	}
}

/**
 * @brief 更新动画状态
 * 
 * 计算每一帧的星星位置、旋转和透明度。
 */
void SplashWindow::UpdateStars()
{
	// 增加帧数
	m_frame++;
	if (m_frame >= m_maxFrames)
	{
		DestroyWindow(m_hWnd);
		return;
	}

	// 计算全局透明度/亮度
	float progress = static_cast<float>(m_frame) / m_maxFrames;

	if (m_mode == 0) // Startup
	{
		// 开机：背景一直保持，文字慢慢浮现
		// 这里 m_alpha 主要控制星星亮度
		if (progress < 0.1f) m_alpha = progress / 0.1f;
		else if (progress > 0.9f) m_alpha = 1.0f - (progress - 0.9f) / 0.1f;
		else m_alpha = 1.0f;
	}
	else // Shutdown
	{
		// 关机：整体慢慢变暗
		m_alpha = 1.0f - progress; // 线性变暗
	}

	// 更新星星位置
	if (m_mode == 0) // 开机：漩涡动画
	{
		float rotationSpeed = 0.02f; // 旋转速度
		// 随时间加速旋转
		rotationSpeed += (progress * 0.03f);

		float cosT = cos(rotationSpeed);
		float sinT = sin(rotationSpeed);

		for (auto& s : m_stars)
		{
			// 旋转 (绕Z轴)
			float nx = s.x * cosT - s.y * sinT;
			float ny = s.x * sinT + s.y * cosT;
			s.x = nx;
			s.y = ny;

			// 前进 (Warp)
			s.z -= s.speed * (1.0f + progress * 2.0f); // 慢慢加速

			// 循环
			if (s.z <= 1.0f)
			{
				s.z = 2000.0f;
				// 重置位置，防止中心空洞
				s.x = static_cast<float>(rand() % (m_width * 2) - m_width);
				s.y = static_cast<float>(rand() % (m_height * 2) - m_height);
			}
		}
	}
	else // 关机：原来的 Warp Speed，但反向或者加速离去？
	{
		// 保持原来的加速离去效果，配合变暗
		float speedMult = 3.0f + progress * 5.0f;

		for (auto& s : m_stars)
		{
			s.z -= s.speed * speedMult;
			if (s.z <= 1.0f)
			{
				s.z = 2000.0f;
				s.x = static_cast<float>(rand() % (m_width * 2) - m_width);
				s.y = static_cast<float>(rand() % (m_height * 2) - m_height);
			}
		}
	}
}

void SplashWindow::OnTimer(HWND hWnd)
{
	UpdateStars();
	InvalidateRect(hWnd, nullptr, FALSE);
}

/**
 * @brief 绘制窗口内容
 * 
 * 使用双缓冲技术绘制星空背景、文字和 LOGO。
 */
void SplashWindow::OnPaint(HWND hWnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);

	// 双缓冲
	HDC memDC = CreateCompatibleDC(hdc);
	HBITMAP hbm = CreateCompatibleBitmap(hdc, m_width, m_height);
	auto hOldBmp = static_cast<HBITMAP>(SelectObject(memDC, hbm));

	// 1. 绘制背景 (深空黑)
	RECT r = {0, 0, m_width, m_height};
	HBRUSH hBg = CreateSolidBrush(RGB(5, 5, 10));
	FillRect(memDC, &r, hBg);
	DeleteObject(hBg);

	// 2. 绘制星星
	DrawStars(memDC, m_width, m_height);

	// 3. 绘制文字
	SetBkMode(memDC, TRANSPARENT);

	// 根据 m_alpha 计算文字颜色
	// 开机时：文字独立淡入 + 上浮效果
	float textAlpha = m_alpha;
	int yOffset = 0;

	if (m_mode == 0)
	{
		float progress = static_cast<float>(m_frame) / m_maxFrames;

		// 0.0 - 0.15: 等待
		// 0.15 - 0.65: 淡入 + 上浮
		if (progress < 0.15f)
		{
			textAlpha = 0.0f;
			yOffset = 60;
		}
		else if (progress < 0.65f)
		{
			float t = (progress - 0.15f) / 0.5f;
			textAlpha = t;
			// EaseOutSine: sin(t * PI / 2)
			yOffset = static_cast<int>(60 * (1.0f - sin(t * 1.570796f)));
		}
		else
		{
			textAlpha = 1.0f;
			yOffset = 0;
		}

		// 最后淡出
		if (progress > 0.9f) textAlpha = 1.0f - (progress - 0.9f) / 0.1f;
	}

	int brightness = static_cast<int>(255 * textAlpha);
	COLORREF textColor = RGB(brightness, brightness, brightness);
	COLORREF subColor = RGB(0, static_cast<int>(brightness * 0.9), brightness); // 冷色系 (青蓝色)

	// 2.5 绘制 LOGO (滑翔机)
	// 放在标题上方
	if (m_mode == 0) // 仅开机显示 LOGO
	{
		int cellSize = 40; // 单元格大小 (放大)
		int logoW = 3 * cellSize;
		int logoH = 3 * cellSize;
		int logoX = (m_width - logoW) / 2;
		int logoY = m_height / 2 - 220 + yOffset; // 标题上方 + 浮动

		// 使用文字颜色作为 LOGO 颜色
		HBRUSH hLogoBrush = CreateSolidBrush(textColor);

		// 滑翔机图案 (3x3)
		// . * .
		// . . *
		// * * *
		struct Point
		{
			int x, y;
		};
		Point glider[] = {{1, 0}, {2, 1}, {0, 2}, {1, 2}, {2, 2}};

		for (const auto& p : glider)
		{
			RECT cell = {
				logoX + p.x * cellSize,
				logoY + p.y * cellSize,
				logoX + (p.x + 1) * cellSize - 4, // -4 间隙
				logoY + (p.y + 1) * cellSize - 4
			};
			FillRect(memDC, &cell, hLogoBrush);
		}
		DeleteObject(hLogoBrush);
	}

	// 标题
	SelectObject(memDC, m_hTitleFont);
	SetTextColor(memDC, textColor);

	RECT titleRect = {0, m_height / 2 - 60 + yOffset, m_width, m_height / 2 + 40 + yOffset};
	const TCHAR* title = (m_mode == 0) ? TEXT("LIFE GAME") : TEXT("SYSTEM HALTED");

	// 简单的发光效果 (绘制多次微小偏移)
	if (textAlpha > 0.5f)
	{
		SetTextColor(memDC, RGB(0, 100, 150)); // 蓝色光晕
		RECT glowRect = titleRect;
		OffsetRect(&glowRect, 2, 2);
		DrawText(memDC, title, -1, &glowRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}

	SetTextColor(memDC, textColor);
	DrawText(memDC, title, -1, &titleRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	// 副标题 / 版本号
	SelectObject(memDC, m_hSubFont);
	SetTextColor(memDC, subColor);

	RECT subRect = {0, m_height / 2 + 50 + yOffset, m_width, m_height / 2 + 150 + yOffset};
	TCHAR subText[64];
	if (m_mode == 0)
		_stprintf_s(subText, TEXT("WHUCS"));
	else
		_stprintf_s(subText, TEXT("WHUCS"));

	DrawText(memDC, subText, -1, &subRect, DT_CENTER | DT_TOP | DT_SINGLELINE);

	// 4. 绘制进度条 (仅开机)
	if (m_mode == 0)
	{
		int barW = 400;
		int barH = 4;
		int barX = (m_width - barW) / 2;
		int barY = m_height / 2 + 160 + yOffset;

		// 槽
		RECT slot = {barX, barY, barX + barW, barY + barH};
		HBRUSH hSlot = CreateSolidBrush(RGB(30, 30, 30));
		FillRect(memDC, &slot, hSlot);
		DeleteObject(hSlot);

		// 进度
		float progress = static_cast<float>(m_frame) / (m_maxFrames * 0.8f); // 提前一点满
		if (progress > 1.0f) progress = 1.0f;

		RECT fill = {barX, barY, barX + static_cast<int>(barW * progress), barY + barH};
		HBRUSH hFill = CreateSolidBrush(RGB(0, 200, 255));
		FillRect(memDC, &fill, hFill);
		DeleteObject(hFill);
	}

	// 拷贝到屏幕
	BitBlt(hdc, 0, 0, m_width, m_height, memDC, 0, 0, SRCCOPY);

	SelectObject(memDC, hOldBmp);
	DeleteObject(hbm);
	DeleteDC(memDC);
	EndPaint(hWnd, &ps);
}

/**
 * @brief 绘制星星
 * 
 * 实现 3D 投影和运动模糊效果。
 */
void SplashWindow::DrawStars(HDC hdc, int width, int height)
{
	int cx = width / 2;
	int cy = height / 2;

	// 使用 DC Pen 提高绘图效率 (Win2000+)
	auto hOldPen = static_cast<HPEN>(SelectObject(hdc, GetStockObject(DC_PEN)));

	for (const auto& s : m_stars)
	{
		// 3D 投影
		if (s.z <= 0) continue;

		float k = 500.0f / s.z; // 增大视场角 (FOV)
		int px = cx + static_cast<int>(s.x * k);
		int py = cy + static_cast<int>(s.y * k);

		if (px >= -100 && px < width + 100 && py >= -100 && py < height + 100)
		{
			// 亮度随距离衰减
			int alpha = static_cast<int>(255 * (1.0f - s.z / 2000.0f));
			if (alpha < 0) alpha = 0;

			// 结合全局 alpha
			alpha = static_cast<int>(alpha * m_alpha);

			COLORREF c = s.color;
			// 简单的颜色混合 (假设背景是深色的)
			int r = (GetRValue(c) * alpha) / 255;
			int g = (GetGValue(c) * alpha) / 255;
			int b = (GetBValue(c) * alpha) / 255;

			// 绘制拖尾 (Warp Speed 效果)
			// 计算"尾巴"的位置 (更远处的投影)
			// 速度越快，尾巴越长
			float speedMult = (m_mode == 0) ? 1.0f : 3.0f;
			float progress = static_cast<float>(m_frame) / m_maxFrames;
			if (m_mode == 0 && progress > 0.8f) speedMult = 8.0f; // 冲刺时更夸张

			// 只有当速度足够快时才画线
			if (speedMult > 2.0f)
			{
				float tailZ = s.z + s.speed * speedMult * 1.5f; // 尾巴在更深处
				float tailK = 500.0f / tailZ;
				int tx = cx + static_cast<int>(s.x * tailK);
				int ty = cy + static_cast<int>(s.y * tailK);

				// 使用画笔画粗线
				HPEN hPen = CreatePen(PS_SOLID, s.size / 2, RGB(r, g, b));
				auto hOld = static_cast<HPEN>(SelectObject(hdc, hPen));
				MoveToEx(hdc, px, py, nullptr);
				LineTo(hdc, tx, ty);
				SelectObject(hdc, hOld);
				DeleteObject(hPen);
			}
			else
			{
				// 绘制实心圆点，而不是单个像素
				HBRUSH hBrush = CreateSolidBrush(RGB(r, g, b));
				auto hOldBrush = static_cast<HBRUSH>(SelectObject(hdc, hBrush));

				// 根据 size 绘制圆
				int rSize = s.size;
				Ellipse(hdc, px - rSize / 2, py - rSize / 2, px + rSize / 2, py + rSize / 2);

				SelectObject(hdc, hOldBrush);
				DeleteObject(hBrush);

				if (s.size > 6)
				{
					// 大星星加十字光芒
					SetDCPenColor(hdc, RGB(r, g, b));
					MoveToEx(hdc, px - s.size, py, nullptr);
					LineTo(hdc, px + s.size, py);
					MoveToEx(hdc, px, py - s.size, nullptr);
					LineTo(hdc, px, py + s.size);
				}
			}
		}
	}
	SelectObject(hdc, hOldPen);
}
