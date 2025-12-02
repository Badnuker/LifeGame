// 确保在 Vista 及以上版本编译
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include "Application.h"
#include "Resource.h"
#include <string>
#include <sstream>
#include <tchar.h>

Application::Application()
	: m_showResetTip(false), m_timerId(0), m_tipTimerId(0),
	  m_clientWidth(0), m_clientHeight(0)
{
}

Application::~Application()
{
}

int Application::Run(HINSTANCE hInstance, int nCmdShow, LPSTR lpCmdLine)
{
	// 1. 启用高DPI感知
	// 防止在高分屏下界面模糊
	SetProcessDPIAware();

	// 2. 解析命令行参数
	int gridWidth = 160; // 默认中等大小
	int gridHeight = 120;
	ParseCommandLine(lpCmdLine, gridWidth, gridHeight);

	// 3. 初始化核心子系统
	m_game = std::make_unique<LifeGame>(gridWidth, gridHeight);
	m_renderer = std::make_unique<Renderer>();
	m_ui = std::make_unique<UI>();

	// 4. 注册窗口类
	WNDCLASS wc = {0};
	wc.lpfnWndProc = StaticWndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = TEXT("LifeGameWindow");
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = nullptr; // 使用双缓冲，不需要背景刷，减少闪烁

	if (!RegisterClass(&wc))
	{
		MessageBox(nullptr, TEXT("窗口注册失败！"), TEXT("错误"), MB_ICONERROR);
		return 1;
	}

	// 5. 初始化渲染器
	if (!m_renderer->Initialize(hInstance))
	{
		MessageBox(nullptr, TEXT("渲染器初始化失败！"), TEXT("错误"), MB_ICONERROR);
		return 1;
	}

	// 6. 创建主窗口
	RECT windowRect = CalcInitialWindowRect();
	HWND hWnd = CreateWindow(
		TEXT("LifeGameWindow"),
		TEXT("LifeGame (Win32 GDI)"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, // 允许最大化
		CW_USEDEFAULT, CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL, NULL, hInstance, this // 将 this 指针传递给窗口过程
	);

	if (!hWnd)
	{
		MessageBox(nullptr, TEXT("窗口创建失败！"), TEXT("错误"), MB_ICONERROR);
		return 1;
	}

	// 7. 初始化 UI 控件
	if (!m_ui->Initialize(hInstance, hWnd, *m_game))
	{
		MessageBox(nullptr, TEXT("UI初始化失败！"), TEXT("错误"), MB_ICONERROR);
		return 1;
	}

	// 设置统一字体
	m_ui->SetAllFonts(m_renderer->GetControlFont());

	// 8. 初始布局与显示
	RECT client;
	GetClientRect(hWnd, &client);
	m_clientWidth = client.right - client.left;
	m_clientHeight = client.bottom - client.top;

	m_ui->LayoutControls(m_clientWidth, m_clientHeight);
	m_ui->UpdateWindowTitle(hWnd, *m_game);

	ShowWindow(hWnd, SW_MAXIMIZE); // 启动时最大化
	UpdateWindow(hWnd);

	// 9. 进入消息循环
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK Application::StaticWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Application* pApp = nullptr;
	if (uMsg == WM_NCCREATE)
	{
		// 在窗口创建时，提取 this 指针并存储在 GWLP_USERDATA 中
		auto pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
		pApp = reinterpret_cast<Application*>(pCreate->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pApp));
	}
	else
	{
		// 后续消息从 GWLP_USERDATA 获取指针
		pApp = reinterpret_cast<Application*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	}

	if (pApp)
	{
		return pApp->WndProc(hWnd, uMsg, wParam, lParam);
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT Application::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_PAINT:
		OnPaint(hWnd);
		break;
	case WM_ERASEBKGND:
		return 1; // 防止闪烁
	case WM_SIZE:
		OnSize(hWnd, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_TIMER:
		OnTimer(hWnd, wParam);
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MOUSEMOVE:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		OnMouse(hWnd, uMsg, wParam, lParam);
		break;
	case WM_KEYDOWN:
		OnKeyDown(hWnd, wParam);
		break;
	case WM_COMMAND:
		OnCommand(hWnd, LOWORD(wParam), HIWORD(wParam));
		break;
	case WM_DRAWITEM:
		OnDrawItem(lParam);
		return TRUE;
	case WM_CTLCOLORSTATIC:
		{
			auto hdc = (HDC)wParam;
			SetTextColor(hdc, m_renderer->GetTextColor());
			SetBkMode(hdc, TRANSPARENT);
			return (LRESULT)m_renderer->GetPanelBrush();
		}
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORLISTBOX:
		{
			auto hdc = (HDC)wParam;
			SetTextColor(hdc, m_renderer->GetTextColor());
			SetBkColor(hdc, RGB(15, 18, 22));
			return (LRESULT)m_renderer->GetInputBrush();
		}
	case WM_USER + 1: // 设置更新消息
		if (m_renderer)
		{
			m_renderer->UpdateSettings();
			InvalidateRect(hWnd, nullptr, TRUE);
		}
		break;
	case WM_CLOSE:
		if (MessageBox(hWnd, TEXT("确定要退出吗？"), TEXT("提示"), MB_YESNO | MB_ICONQUESTION) == IDYES)
		{
			DestroyWindow(hWnd);
		}
		break;
	case WM_DESTROY:
		OnDestroy(hWnd);
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

void Application::OnPaint(HWND hWnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);

	// 创建内存 DC 进行双缓冲绘图
	HDC memDC = CreateCompatibleDC(hdc);
	HBITMAP hbm = CreateCompatibleBitmap(hdc, m_clientWidth, m_clientHeight);
	auto hOldBmp = static_cast<HBITMAP>(SelectObject(memDC, hbm));

	// 调用渲染器绘制所有内容
	m_renderer->Draw(memDC, *m_game, &ps.rcPaint, m_showResetTip, m_clientWidth, m_clientHeight);

	// 将内存 DC 内容拷贝到屏幕
	BitBlt(hdc, 0, 0, m_clientWidth, m_clientHeight, memDC, 0, 0, SRCCOPY);

	// 清理资源
	SelectObject(memDC, hOldBmp);
	DeleteObject(hbm);
	DeleteDC(memDC);
	EndPaint(hWnd, &ps);
}

void Application::OnTimer(HWND hWnd, WPARAM timerId)
{
	if (timerId == 1) // 游戏循环定时器
	{
		if (m_game->IsRunning())
		{
			m_game->UpdateGrid();
			// 仅重绘网格区域和状态栏，优化性能
			// 但由于有拖尾效果和 HUD，重绘整个右侧区域比较安全
			RECT r;
			GetClientRect(hWnd, &r);
			r.left = Renderer::LEFT_PANEL_WIDTH;
			InvalidateRect(hWnd, &r, FALSE);
		}
	}
	else if (timerId == 2) // 提示信息定时器
	{
		m_showResetTip = false;
		m_tipTimerId = 0;
		InvalidateRect(hWnd, nullptr, FALSE);
		KillTimer(hWnd, 2);
	}
}

void Application::OnMouse(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int x = LOWORD(lParam);
	int y = HIWORD(lParam);

	// 如果有提示信息显示，点击时隐藏
	if ((uMsg == WM_LBUTTONDOWN || (uMsg == WM_MOUSEMOVE && m_ui->IsDragging())) && m_showResetTip)
	{
		m_showResetTip = false;
		if (m_tipTimerId)
		{
			KillTimer(hWnd, 2);
			m_tipTimerId = 0;
		}
		InvalidateRect(hWnd, nullptr, FALSE);
	}

	bool needRepaint = false;
	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
		if (m_ui->HandleMouseClick(x, y, true, *m_game, m_clientWidth, m_clientHeight))
		{
			SetCapture(hWnd);
			needRepaint = true;
		}
		break;
	case WM_RBUTTONDOWN:
		if (m_ui->HandleMouseClick(x, y, false, *m_game, m_clientWidth, m_clientHeight))
		{
			SetCapture(hWnd);
			needRepaint = true;
		}
		break;
	case WM_MOUSEMOVE:
		if (m_ui->HandleMouseMove(x, y, *m_game, m_clientWidth, m_clientHeight))
			needRepaint = true;
		break;
	case WM_LBUTTONUP:
		m_ui->HandleMouseUp(true);
		if (!m_ui->IsDragging()) ReleaseCapture();
		break;
	case WM_RBUTTONUP:
		m_ui->HandleMouseUp(false);
		if (!m_ui->IsDragging()) ReleaseCapture();
		break;
	}

	if (needRepaint) InvalidateRect(hWnd, nullptr, FALSE);
}

void Application::OnKeyDown(HWND hWnd, WPARAM key)
{
	switch (key)
	{
	case VK_SPACE:
		m_game->ToggleRunning();
		RestartTimer(hWnd);
		m_ui->UpdateWindowTitle(hWnd, *m_game);
		break;
	case 'R':
		m_game->Pause();
		KillTimer(hWnd, m_timerId);
		if (m_tipTimerId) KillTimer(hWnd, 2);
		m_game->ResetGrid();
		m_renderer->ClearVisuals(); // 清除视觉残留
		m_showResetTip = true;
		m_tipTimerId = SetTimer(hWnd, 2, 2000, nullptr);
		InvalidateRect(hWnd, nullptr, FALSE);
		m_ui->UpdateWindowTitle(hWnd, *m_game);
		break;
	case 'G':
		m_game->Pause();
		KillTimer(hWnd, m_timerId);
		m_game->InitGrid();
		m_renderer->ClearVisuals(); // 清除视觉残留
		InvalidateRect(hWnd, nullptr, FALSE);
		m_ui->UpdateWindowTitle(hWnd, *m_game);
		break;
	case VK_ADD:
	case 0xBB: // +
		m_game->IncreaseSpeed();
		if (m_game->IsRunning()) RestartTimer(hWnd);
		m_ui->UpdateWindowTitle(hWnd, *m_game);
		break;
	case VK_SUBTRACT:
	case 0xBD: // -
		m_game->DecreaseSpeed();
		if (m_game->IsRunning()) RestartTimer(hWnd);
		m_ui->UpdateWindowTitle(hWnd, *m_game);
		break;
	case VK_ESCAPE:
		DestroyWindow(hWnd);
		break;
	}
}

void Application::OnCommand(HWND hWnd, int id, int code)
{
	if (m_ui)
	{
		m_ui->HandleCommand(id, code, hWnd, *m_game, m_renderer.get());
		InvalidateRect(hWnd, nullptr, TRUE);
	}
}

void Application::OnDrawItem(LPARAM lParam)
{
	auto pdis = (LPDRAWITEMSTRUCT)lParam;
	// 委托给 UI 或 Renderer 处理？
	// 目前 Main.cpp 里的逻辑是硬编码的，这里保留
	if (pdis && pdis->CtlID == ID_APPLY_BTN)
	{
		HDC hdc = pdis->hDC;
		RECT rc = pdis->rcItem;

		HBRUSH hBg = CreateSolidBrush(RGB(20, 24, 28));
		FillRect(hdc, &rc, hBg);
		DeleteObject(hBg);

		HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 180, 220));
		auto hOldPen = static_cast<HPEN>(SelectObject(hdc, hPen));
		auto hOldBrush = static_cast<HBRUSH>(SelectObject(hdc, GetStockObject(NULL_BRUSH)));

		Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);

		if (pdis->itemState & ODS_SELECTED)
		{
			HBRUSH hPress = CreateSolidBrush(RGB(0, 60, 80));
			FillRect(hdc, &rc, hPress);
			DeleteObject(hPress);
		}

		SetTextColor(hdc, RGB(0, 220, 255));
		SetBkMode(hdc, TRANSPARENT);
		DrawText(hdc, TEXT("应用设置"), -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		SelectObject(hdc, hOldPen);
		SelectObject(hdc, hOldBrush);
		DeleteObject(hPen);
	}
	// 其他按钮 (颜色选择等) 也可以在这里绘制
	else if (pdis && (pdis->CtlID == 101 || pdis->CtlID == 102 || pdis->CtlID == 103)) // SettingsDialog 按钮
	{
		// SettingsDialog 是模态的，它的消息不会传到这里
		// 除非 SettingsDialog 是非模态的子窗口
	}
}

void Application::OnSize(HWND hWnd, int width, int height)
{
	m_clientWidth = width;
	m_clientHeight = height;
	if (m_ui) m_ui->LayoutControls(m_clientWidth, m_clientHeight);
	InvalidateRect(hWnd, nullptr, TRUE);
}

void Application::OnDestroy(HWND hWnd)
{
	KillTimer(hWnd, m_timerId);
	if (m_tipTimerId) KillTimer(hWnd, 2);
	PostQuitMessage(0);
}

void Application::RestartTimer(HWND hWnd)
{
	if (m_timerId) KillTimer(hWnd, m_timerId);
	if (m_game && m_game->IsRunning())
	{
		m_timerId = SetTimer(hWnd, 1, m_game->GetSpeed(), nullptr);
	}
}

void Application::ParseCommandLine(LPSTR lpCmdLine, int& w, int& h)
{
	if (!lpCmdLine || lpCmdLine[0] == '\0') return;
	std::string cmd(lpCmdLine);
	std::istringstream iss(cmd);
	std::string tok;
	while (iss >> tok)
	{
		if ((tok == "-w" || tok == "-cols") && (iss >> tok)) w = atoi(tok.c_str());
		else if ((tok == "-h" || tok == "-rows") && (iss >> tok)) h = atoi(tok.c_str());
	}
}

RECT Application::CalcInitialWindowRect()
{
	int desiredClientW = 1200;
	int desiredClientH = 900; // 调整默认高度，避免过高

	RECT r = {0, 0, desiredClientW, desiredClientH};
	AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);
	return r;
}
