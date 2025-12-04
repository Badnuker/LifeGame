// 确保在 Vista 及以上版本编译 (为了支持某些现代 API)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include "Application.h"
#include "Resource.h"
#include <string>
#include <sstream>
#include <tchar.h>

// 构造函数：初始化成员变量
Application::Application()
	: m_showResetTip(false), m_timerId(0), m_tipTimerId(0),
	  m_clientWidth(0), m_clientHeight(0)
{
}

Application::~Application()
{
}

// 应用程序主入口逻辑
int Application::Run(HINSTANCE hInstance, int nCmdShow, LPSTR lpCmdLine)
{
	// 1. 启用高DPI感知
	// 防止在高分屏 (High DPI) 显示器下界面模糊，确保像素级清晰度
	SetProcessDPIAware();

	// 0. 显示开机画面 (Splash Screen)
	// 在主窗口创建前显示一个无边框的启动窗口，提升用户体验
	{
		SplashWindow splash;
		splash.Show(hInstance, 0); // 0 = Startup (开机模式)
		splash.RunLoop(); // 进入独立的模态消息循环，直到动画结束
	}

	// 2. 解析命令行参数
	// 允许用户通过命令行自定义初始网格大小，默认 160x120
	int gridWidth = 160;
	int gridHeight = 120;
	ParseCommandLine(lpCmdLine, gridWidth, gridHeight);

	// 3. 初始化核心子系统
	// 使用 std::make_unique 创建智能指针，自动管理内存
	m_game = std::make_unique<LifeGame>(gridWidth, gridHeight); // 游戏逻辑模型
	m_renderer = std::make_unique<Renderer>(); // 渲染器
	m_ui = std::make_unique<UI>(); // UI 控制器

	// 4. 注册窗口类
	WNDCLASS wc = {0};
	wc.lpfnWndProc = StaticWndProc; // 设置静态窗口过程
	wc.hInstance = hInstance;
	wc.lpszClassName = TEXT("LifeGameWindow");
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = nullptr; // 关键：设置为 nullptr 以禁用系统背景擦除，使用双缓冲防止闪烁

	if (!RegisterClass(&wc))
	{
		MessageBox(nullptr, TEXT("窗口注册失败！"), TEXT("错误"), MB_ICONERROR);
		return 1;
	}

	// 5. 初始化渲染器 (加载 GDI 资源，如画刷、字体等)
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
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, // WS_CLIPCHILDREN 防止父窗口重绘覆盖子控件
		CW_USEDEFAULT, CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL, NULL, hInstance, this // 关键：将 'this' 指针作为创建参数传递，以便在 StaticWndProc 中获取
	);

	if (!hWnd)
	{
		MessageBox(nullptr, TEXT("窗口创建失败！"), TEXT("错误"), MB_ICONERROR);
		return 1;
	}

	// 7. 初始化 UI 控件 (创建按钮、输入框等子窗口)
	if (!m_ui->Initialize(hInstance, hWnd, *m_game))
	{
		MessageBox(nullptr, TEXT("UI初始化失败！"), TEXT("错误"), MB_ICONERROR);
		return 1;
	}

	// 设置统一字体：让所有 UI 控件使用渲染器中定义的高质量字体
	m_ui->SetAllFonts(m_renderer->GetControlFont());

	// 8. 初始布局与显示
	RECT client;
	GetClientRect(hWnd, &client);
	m_clientWidth = client.right - client.left;
	m_clientHeight = client.bottom - client.top;

	m_ui->LayoutControls(m_clientWidth, m_clientHeight); // 根据窗口大小调整控件位置
	m_ui->UpdateWindowTitle(hWnd, *m_game); // 更新标题栏状态

	ShowWindow(hWnd, SW_MAXIMIZE); // 启动时默认最大化
	UpdateWindow(hWnd);

	// 9. 进入主消息循环
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return static_cast<int>(msg.wParam);
}

// 静态窗口过程：消息分发中心
LRESULT CALLBACK Application::StaticWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Application* pApp = nullptr;
	if (uMsg == WM_NCCREATE)
	{
		// 在窗口创建的早期 (WM_NCCREATE)，提取 CreateWindow 传入的 this 指针
		auto pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
		pApp = reinterpret_cast<Application*>(pCreate->lpCreateParams);
		// 将 this 指针存储在窗口的用户数据中 (GWLP_USERDATA)
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pApp));
	}
	else
	{
		// 对于后续消息，直接从窗口用户数据中取回 this 指针
		pApp = reinterpret_cast<Application*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	}

	// 如果成功获取到 Application 实例，则调用成员函数 WndProc
	if (pApp)
	{
		return pApp->WndProc(hWnd, uMsg, wParam, lParam);
	}
	// 否则调用默认窗口过程
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// 成员窗口过程：实际的业务逻辑处理
LRESULT Application::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_PAINT:
		OnPaint(hWnd); // 处理绘图
		break;
	case WM_ERASEBKGND:
		return 1; // 返回 1 告诉系统"我已经擦除背景了" (实际上什么都没做)，防止闪烁
	case WM_SIZE:
		OnSize(hWnd, LOWORD(lParam), HIWORD(lParam)); // 处理窗口大小改变
		break;
	case WM_TIMER:
		OnTimer(hWnd, wParam); // 处理定时器
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MOUSEMOVE:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		OnMouse(hWnd, uMsg, wParam, lParam); // 处理鼠标交互
		break;
	case WM_MOUSELEAVE: // 新增：处理鼠标离开窗口
		if (m_ui && m_renderer)
		{
			m_ui->HandleMouseLeave(m_renderer.get());
			InvalidateRect(hWnd, nullptr, FALSE);
		}
		break;
	case WM_MOUSEWHEEL:
		OnMouseWheel(hWnd, wParam, lParam); // 处理滚轮缩放
		break;
	case WM_KEYDOWN:
		OnKeyDown(hWnd, wParam); // 处理键盘快捷键
		break;
	case WM_COMMAND:
		OnCommand(hWnd, LOWORD(wParam), HIWORD(wParam)); // 处理 UI 控件命令
		break;
	case WM_DRAWITEM:
		OnDrawItem(lParam); // 处理自绘控件
		return TRUE;
	case WM_CTLCOLORSTATIC:
		{
			// 设置静态文本控件的颜色 (透明背景)
			auto hdc = (HDC)wParam;
			SetTextColor(hdc, m_renderer->GetTextColor());
			SetBkMode(hdc, TRANSPARENT);
			return (LRESULT)m_renderer->GetPanelBrush();
		}
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORLISTBOX:
		{
			// 设置编辑框和列表框的颜色 (深色背景)
			auto hdc = (HDC)wParam;
			SetTextColor(hdc, m_renderer->GetTextColor());
			SetBkColor(hdc, RGB(15, 18, 22));
			return (LRESULT)m_renderer->GetInputBrush();
		}
	case WM_USER + 1: // 自定义消息：设置已更新
		if (m_renderer)
		{
			m_renderer->UpdateSettings(); // 通知渲染器更新画刷
			InvalidateRect(hWnd, nullptr, TRUE); // 触发重绘
		}
		break;
	case WM_CLOSE:
		// 拦截关闭消息，显示关机画面
		// if (MessageBox(hWnd, TEXT("确定要退出吗？"), TEXT("提示"), MB_YESNO | MB_ICONQUESTION) == IDYES)
		{
			// 隐藏主窗口，避免在关机动画时看到它
			ShowWindow(hWnd, SW_HIDE);

			// 显示关机画面 (Shutdown Screen)
			SplashWindow splash;
			splash.Show(GetModuleHandle(nullptr), 1); // 1 = Shutdown (关机模式)
			splash.RunLoop();

			DestroyWindow(hWnd); // 销毁窗口，触发 WM_DESTROY
		}
		break;
	case WM_DESTROY:
		OnDestroy(hWnd); // 退出程序
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

// 绘图处理函数
void Application::OnPaint(HWND hWnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);

	// 双缓冲技术 (Double Buffering)：
	// 1. 创建一个与屏幕兼容的内存 DC (画布)
	HDC memDC = CreateCompatibleDC(hdc);
	// 2. 创建一个与屏幕兼容的位图 (画布纸)
	HBITMAP hbm = CreateCompatibleBitmap(hdc, m_clientWidth, m_clientHeight);
	// 3. 将位图选入内存 DC
	auto hOldBmp = static_cast<HBITMAP>(SelectObject(memDC, hbm));

	// 4. 在内存 DC 上进行所有的绘制操作 (调用渲染器)
	m_renderer->Draw(memDC, *m_game, &ps.rcPaint, m_showResetTip, m_clientWidth, m_clientHeight);

	// 5. 将绘制好的内存 DC 一次性拷贝 (BitBlt) 到屏幕 DC
	BitBlt(hdc, 0, 0, m_clientWidth, m_clientHeight, memDC, 0, 0, SRCCOPY);

	// 6. 清理资源
	SelectObject(memDC, hOldBmp);
	DeleteObject(hbm);
	DeleteDC(memDC);
	EndPaint(hWnd, &ps);
}

// 定时器处理函数
void Application::OnTimer(HWND hWnd, WPARAM timerId)
{
	if (timerId == 1) // 游戏循环定时器 (ID=1)
	{
		if (m_game->IsRunning())
		{
			m_game->UpdateGrid(); // 核心逻辑：计算下一代

			// 优化重绘：
			// 之前只重绘右侧网格，导致左侧统计图不更新。
			// 现在重绘整个客户区 (r.left = 0)，确保所有动态元素都能实时刷新。
			RECT r;
			GetClientRect(hWnd, &r);
			r.left = 0;
			InvalidateRect(hWnd, &r, FALSE); // FALSE 表示不擦除背景，直接覆盖
		}
	}
	else if (timerId == 2) // 提示信息定时器 (ID=2)
	{
		// 提示显示时间到，隐藏提示
		m_showResetTip = false;
		m_tipTimerId = 0;
		InvalidateRect(hWnd, nullptr, FALSE);
		KillTimer(hWnd, 2);
	}
}

// 鼠标交互处理
void Application::OnMouse(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int x = LOWORD(lParam);
	int y = HIWORD(lParam);

	// 如果有提示信息显示，点击任意位置隐藏它
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
		// 左键点击：绘制细胞
		if (m_ui->HandleMouseClick(x, y, true, *m_game, m_clientWidth, m_clientHeight, m_renderer.get()))
		{
			SetCapture(hWnd); // 捕获鼠标，确保拖拽出窗口也能收到消息
			needRepaint = true;
		}
		break;
	case WM_RBUTTONDOWN:
		// 右键点击：擦除细胞
		if (m_ui->HandleMouseClick(x, y, false, *m_game, m_clientWidth, m_clientHeight, m_renderer.get()))
		{
			SetCapture(hWnd);
			needRepaint = true;
		}
		break;
	case WM_MOUSEMOVE:
		// 追踪鼠标离开事件
		{
			TRACKMOUSEEVENT tme = {sizeof(TRACKMOUSEEVENT), TME_LEAVE, hWnd, 0};
			TrackMouseEvent(&tme);
		}
		// 鼠标移动：处理拖拽绘制或悬停预览
		if (m_ui->HandleMouseMove(x, y, *m_game, m_clientWidth, m_clientHeight, m_renderer.get()))
			needRepaint = true;
		break;
	case WM_LBUTTONUP:
		m_ui->HandleMouseUp(true, m_renderer.get());
		if (!m_ui->IsDragging()) ReleaseCapture(); // 释放鼠标捕获
		break;
	case WM_RBUTTONUP:
		m_ui->HandleMouseUp(false, m_renderer.get());
		if (!m_ui->IsDragging()) ReleaseCapture();
		break;
	}

	if (needRepaint) InvalidateRect(hWnd, nullptr, FALSE);
}

// 鼠标滚轮处理：缩放视图
void Application::OnMouseWheel(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
	POINT pt;
	pt.x = static_cast<short>(LOWORD(lParam));
	pt.y = static_cast<short>(HIWORD(lParam));
	ScreenToClient(hWnd, &pt); // 将屏幕坐标转换为客户区坐标

	if (m_renderer)
	{
		// 向上滚放大 (1.1倍)，向下滚缩小 (0.9倍)
		float scaleFactor = (zDelta > 0) ? 1.1f : 0.9f;
		m_renderer->Zoom(scaleFactor, pt.x, pt.y);
		InvalidateRect(hWnd, nullptr, FALSE);
	}
}

// 键盘快捷键处理
void Application::OnKeyDown(HWND hWnd, WPARAM key)
{
	switch (key)
	{
	case VK_SPACE: // 空格：暂停/开始
		m_game->ToggleRunning();
		RestartTimer(hWnd);
		m_ui->UpdateWindowTitle(hWnd, *m_game);
		break;
	case 'R': // R键：重置 (清空)
		m_game->Pause();
		KillTimer(hWnd, m_timerId);
		if (m_tipTimerId) KillTimer(hWnd, 2);
		m_game->ResetGrid();
		m_renderer->ClearVisuals(); // 清除视觉残留 (拖尾)
		m_showResetTip = true;
		m_tipTimerId = SetTimer(hWnd, 2, 2000, nullptr); // 2秒后隐藏提示
		InvalidateRect(hWnd, nullptr, FALSE);
		m_ui->UpdateWindowTitle(hWnd, *m_game);
		break;
	case 'G': // G键：随机生成
		m_game->Pause();
		KillTimer(hWnd, m_timerId);
		m_game->InitGrid();
		m_renderer->ClearVisuals();
		InvalidateRect(hWnd, nullptr, FALSE);
		m_ui->UpdateWindowTitle(hWnd, *m_game);
		break;
	case VK_ADD:
	case 0xBB: // +键：加速
		m_game->IncreaseSpeed();
		if (m_game->IsRunning()) RestartTimer(hWnd);
		m_ui->UpdateWindowTitle(hWnd, *m_game);
		break;
	case VK_SUBTRACT:
	case 0xBD: // -键：减速
		m_game->DecreaseSpeed();
		if (m_game->IsRunning()) RestartTimer(hWnd);
		m_ui->UpdateWindowTitle(hWnd, *m_game);
		break;
	case VK_ESCAPE:
		// ESC 键：退出程序 (触发关机流程)
		SendMessage(hWnd, WM_CLOSE, 0, 0);
		break;
	}
}

// UI 命令处理 (按钮、菜单等)
void Application::OnCommand(HWND hWnd, int id, int code)
{
	if (m_ui)
	{
		// 将命令委托给 UI 类处理
		m_ui->HandleCommand(id, code, hWnd, *m_game, m_renderer.get());
		InvalidateRect(hWnd, nullptr, TRUE);
	}
}

// 自绘控件处理
void Application::OnDrawItem(LPARAM lParam)
{
	auto pdis = (LPDRAWITEMSTRUCT)lParam;
	// 绘制"应用设置"按钮 (ID_APPLY_BTN)
	if (pdis && pdis->CtlID == ID_APPLY_BTN)
	{
		HDC hdc = pdis->hDC;
		RECT rc = pdis->rcItem;

		// 绘制背景
		HBRUSH hBg = CreateSolidBrush(RGB(20, 24, 28));
		FillRect(hdc, &rc, hBg);
		DeleteObject(hBg);

		// 绘制边框
		HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 180, 220));
		auto hOldPen = static_cast<HPEN>(SelectObject(hdc, hPen));
		auto hOldBrush = static_cast<HBRUSH>(SelectObject(hdc, GetStockObject(NULL_BRUSH)));

		Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);

		// 绘制按下状态
		if (pdis->itemState & ODS_SELECTED)
		{
			HBRUSH hPress = CreateSolidBrush(RGB(0, 60, 80));
			FillRect(hdc, &rc, hPress);
			DeleteObject(hPress);
		}

		// 绘制文字
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

// 窗口大小改变处理
void Application::OnSize(HWND hWnd, int width, int height)
{
	m_clientWidth = width;
	m_clientHeight = height;
	// 通知 UI 重新布局控件
	if (m_ui) m_ui->LayoutControls(m_clientWidth, m_clientHeight);
	InvalidateRect(hWnd, nullptr, TRUE);
}

// 窗口销毁处理
void Application::OnDestroy(HWND hWnd)
{
	KillTimer(hWnd, m_timerId);
	if (m_tipTimerId) KillTimer(hWnd, 2);
	PostQuitMessage(0); // 发送 WM_QUIT 消息，结束消息循环
}

// 重启定时器
void Application::RestartTimer(HWND hWnd)
{
	if (m_timerId) KillTimer(hWnd, m_timerId);
	if (m_game && m_game->IsRunning())
	{
		// 根据当前游戏速度设置定时器间隔
		m_timerId = SetTimer(hWnd, 1, m_game->GetSpeed(), nullptr);
	}
}

// 解析命令行参数
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

// 计算初始窗口大小
RECT Application::CalcInitialWindowRect()
{
	int desiredClientW = 1200;
	int desiredClientH = 900; // 调整默认高度，避免过高

	RECT r = {0, 0, desiredClientW, desiredClientH};
	// 根据客户区大小计算包含标题栏和边框的窗口大小
	AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);
	return r;
}
