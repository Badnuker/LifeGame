#pragma comment(lib, "comctl32.lib")

#include <windows.h>
#include <sstream>
#include <string>
#include "Game.h"
#include "Renderer.h"
#include "UI.h"
#include "Resource.h"

// 全局对象
LifeGame* g_pGame = nullptr;
Renderer* g_pRenderer = nullptr;
UI* g_pUI = nullptr;

// 全局状态
bool g_showResetTip = false;
UINT_PTR g_timerId = 0;
UINT_PTR g_tipTimerId = 0;
int g_clientWidth = 0;
int g_clientHeight = 0;

// 前向声明
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void RestartTimer(HWND hWnd);

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
                   _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	// 解析命令行参数
	int gridWidth = 80;
	int gridHeight = 60;

	if (lpCmdLine && lpCmdLine[0] != '\0')
	{
		std::string cmd(lpCmdLine);
		std::istringstream iss(cmd);
		std::string tok;
		while (iss >> tok)
		{
			if ((tok == "-w" || tok == "-cols" || tok == "--width") && (iss >> tok))
			{
				int v = atoi(tok.c_str());
				if (v > 0) gridWidth = v;
			}
			else if ((tok == "-h" || tok == "-rows" || tok == "--height") && (iss >> tok))
			{
				int v = atoi(tok.c_str());
				if (v > 0) gridHeight = v;
			}
			else
			{
				auto pos = tok.find('=');
				if (pos != std::string::npos)
				{
					std::string key = tok.substr(0, pos);
					std::string val = tok.substr(pos + 1);
					if ((key == "w" || key == "cols" || key == "width") && !val.empty())
					{
						int v = atoi(val.c_str());
						if (v > 0) gridWidth = v;
					}
					else if ((key == "h" || key == "rows" || key == "height") && !val.empty())
					{
						int v = atoi(val.c_str());
						if (v > 0) gridHeight = v;
					}
				}
			}
		}
	}

	// 创建全局对象
	g_pGame = new LifeGame(gridWidth, gridHeight);
	g_pRenderer = new Renderer();
	g_pUI = new UI();

	// 注册窗口类
	WNDCLASS wc = {0};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = TEXT("LifeGameWindow");
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = nullptr;

	if (!RegisterClass(&wc))
	{
		MessageBox(nullptr, TEXT("窗口类注册失败！"), TEXT("错误"), MB_ICONERROR);
		return 1;
	}

	// 初始化渲染器
	if (!g_pRenderer->Initialize(hInstance))
	{
		MessageBox(nullptr, TEXT("渲染器初始化失败！"), TEXT("错误"), MB_ICONERROR);
		return 1;
	}

	// 计算窗口大小
	constexpr int CELL_SIZE = 10;
	constexpr int STATUS_BAR_HEIGHT = 28;
	constexpr int LEFT_PANEL_WIDTH = 150;

	int gridWpx = CELL_SIZE * g_pGame->GetWidth();
	int gridHpx = CELL_SIZE * g_pGame->GetHeight();
	int topControlsH = 44;
	int desiredClientW = LEFT_PANEL_WIDTH + gridWpx + 20;
	int desiredClientH = gridHpx + STATUS_BAR_HEIGHT + topControlsH + 10;

	constexpr int MIN_VISIBLE_COLS = 40;
	constexpr int MIN_VISIBLE_ROWS = 40;
	int minGridWpx = CELL_SIZE * MIN_VISIBLE_COLS;
	int minClientW = LEFT_PANEL_WIDTH + minGridWpx + 20;
	int minGridHpx = CELL_SIZE * MIN_VISIBLE_ROWS;
	int minClientH = minGridHpx + STATUS_BAR_HEIGHT + topControlsH + 10;

	if (desiredClientW < minClientW) desiredClientW = minClientW;
	if (desiredClientH < minClientH) desiredClientH = minClientH;

	RECT windowRect = {0, 0, desiredClientW, desiredClientH};
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, FALSE);

	// 创建窗口
	HWND hWnd = CreateWindow(
		TEXT("LifeGameWindow"),
		TEXT("生命游戏"),
		WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL, NULL, hInstance, NULL
	);

	if (!hWnd)
	{
		MessageBox(nullptr, TEXT("窗口创建失败！"), TEXT("错误"), MB_ICONERROR);
		return 1;
	}

	// 初始化UI
	if (!g_pUI->Initialize(hInstance, hWnd, *g_pGame))
	{
		MessageBox(nullptr, TEXT("UI初始化失败！"), TEXT("错误"), MB_ICONERROR);
		return 1;
	}

	// 获取客户区尺寸并布局控件
	RECT client;
	GetClientRect(hWnd, &client);
	g_clientWidth = client.right - client.left;
	g_clientHeight = client.bottom - client.top;
	g_pUI->LayoutControls(g_clientWidth, g_clientHeight);
	g_pUI->UpdateWindowTitle(hWnd, *g_pGame);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	SetFocus(hWnd);

	// 消息循环
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// 清理资源
	delete g_pUI;
	delete g_pRenderer;
	delete g_pGame;

	return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!g_pGame || !g_pRenderer || !g_pUI)
	{
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	switch (uMsg)
	{
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);

			// 更新客户区尺寸
			RECT clientRect;
			GetClientRect(hWnd, &clientRect);
			g_clientWidth = clientRect.right - clientRect.left;
			g_clientHeight = clientRect.bottom - clientRect.top;

			// 双缓冲绘制
			HDC memDC = CreateCompatibleDC(hdc);
			HBITMAP hbm = CreateCompatibleBitmap(hdc, g_clientWidth, g_clientHeight);
			auto hOldBmp = static_cast<HBITMAP>(SelectObject(memDC, hbm));

			g_pRenderer->Draw(memDC, *g_pGame, &ps.rcPaint, g_showResetTip, g_clientWidth, g_clientHeight);

			BitBlt(hdc, 0, 0, g_clientWidth, g_clientHeight, memDC, 0, 0, SRCCOPY);

			SelectObject(memDC, hOldBmp);
			DeleteObject(hbm);
			DeleteDC(memDC);
			EndPaint(hWnd, &ps);
			break;
		}

	case WM_ERASEBKGND:
		return 1;

	case WM_SIZE:
		g_clientWidth = LOWORD(lParam);
		g_clientHeight = HIWORD(lParam);
		g_pUI->LayoutControls(g_clientWidth, g_clientHeight);
		InvalidateRect(hWnd, nullptr, TRUE);
		break;

	case WM_TIMER:
		if (wParam == 1)
		{
			if (g_pGame->IsRunning())
			{
				g_pGame->UpdateGrid();
				InvalidateRect(hWnd, nullptr, FALSE);
			}
		}
		else if (wParam == 2)
		{
			g_showResetTip = false;
			g_tipTimerId = 0;
			InvalidateRect(hWnd, nullptr, FALSE);
			KillTimer(hWnd, 2);
		}
		break;

	case WM_LBUTTONDOWN:
		{
			if (g_showResetTip)
			{
				g_showResetTip = false;
				if (g_tipTimerId != 0)
				{
					KillTimer(hWnd, 2);
					g_tipTimerId = 0;
				}
				InvalidateRect(hWnd, nullptr, FALSE);
			}

			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			if (g_pUI->HandleMouseClick(x, y, true, *g_pGame, g_clientWidth, g_clientHeight))
			{
				SetCapture(hWnd);
				InvalidateRect(hWnd, nullptr, FALSE);
			}
			break;
		}

	case WM_RBUTTONDOWN:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			if (g_pUI->HandleMouseClick(x, y, false, *g_pGame, g_clientWidth, g_clientHeight))
			{
				SetCapture(hWnd);
				InvalidateRect(hWnd, nullptr, FALSE);
			}
			break;
		}

	case WM_MOUSEMOVE:
		{
			if (g_showResetTip && g_pUI->IsDragging())
			{
				g_showResetTip = false;
				if (g_tipTimerId != 0)
				{
					KillTimer(hWnd, 2);
					g_tipTimerId = 0;
				}
				InvalidateRect(hWnd, nullptr, FALSE);
			}

			if (g_pUI->HandleMouseMove(LOWORD(lParam), HIWORD(lParam), *g_pGame, g_clientWidth, g_clientHeight))
			{
				InvalidateRect(hWnd, nullptr, FALSE);
			}
			break;
		}

	case WM_LBUTTONUP:
		g_pUI->HandleMouseUp(true);
		if (!g_pUI->IsDragging())
		{
			ReleaseCapture();
		}
		break;

	case WM_RBUTTONUP:
		g_pUI->HandleMouseUp(false);
		if (!g_pUI->IsDragging())
		{
			ReleaseCapture();
		}
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_SPACE:
			g_pGame->ToggleRunning();
			if (g_pGame->IsRunning())
			{
				RestartTimer(hWnd);
			}
			else
			{
				KillTimer(hWnd, g_timerId);
			}
			g_pUI->UpdateWindowTitle(hWnd, *g_pGame);
			break;

		case 'R':
			g_pGame->Pause();
			KillTimer(hWnd, g_timerId);
			if (g_tipTimerId != 0)
			{
				KillTimer(hWnd, 2);
			}
			g_pGame->ResetGrid();
			g_showResetTip = true;
			g_tipTimerId = SetTimer(hWnd, 2, 2000, nullptr);
			InvalidateRect(hWnd, nullptr, FALSE);
			g_pUI->UpdateWindowTitle(hWnd, *g_pGame);
			break;

		case 'G':
			g_pGame->Pause();
			KillTimer(hWnd, g_timerId);
			g_pGame->InitGrid();
			InvalidateRect(hWnd, nullptr, FALSE);
			g_pUI->UpdateWindowTitle(hWnd, *g_pGame);
			break;

		case VK_ADD:
		case 0xBB:
			g_pGame->IncreaseSpeed();
			if (g_pGame->IsRunning()) RestartTimer(hWnd);
			g_pUI->UpdateWindowTitle(hWnd, *g_pGame);
			break;

		case VK_SUBTRACT:
		case 0xBD:
			g_pGame->DecreaseSpeed();
			if (g_pGame->IsRunning()) RestartTimer(hWnd);
			g_pUI->UpdateWindowTitle(hWnd, *g_pGame);
			break;

		case VK_ESCAPE:
			DestroyWindow(hWnd);
			break;
		}
		break;

	case WM_COMMAND:
		g_pUI->HandleCommand(LOWORD(wParam), HIWORD(wParam), hWnd, *g_pGame);
		InvalidateRect(hWnd, nullptr, TRUE);
		break;

	case WM_DRAWITEM:
		{
			auto pdis = (LPDRAWITEMSTRUCT)lParam;
			if (pdis && pdis->CtlID == ID_APPLY_BTN)
			{
				HDC hdc = pdis->hDC;
				RECT rc = pdis->rcItem;

				COLORREF base = RGB(40, 120, 200);
				COLORREF hoverCol = RGB(70, 150, 230);
				COLORREF pressCol = RGB(20, 90, 170);
				COLORREF fill = base;

				// 简化按钮状态判断
				if (pdis->itemState & ODS_SELECTED)
				{
					fill = pressCol;
				}
				else
				{
					// 使用静态变量跟踪悬停状态
					static bool s_applyHover = false;
					// 这里可以添加更复杂的悬停检测逻辑
					fill = base;
				}

				HBRUSH hbr = CreateSolidBrush(fill);
				FillRect(hdc, &rc, hbr);
				DeleteObject(hbr);

				HPEN hPen = CreatePen(PS_SOLID, 1, RGB(15, 60, 120));
				auto hOld = static_cast<HPEN>(SelectObject(hdc, hPen));
				MoveToEx(hdc, rc.left, rc.top, nullptr);
				LineTo(hdc, rc.right - 1, rc.top);
				LineTo(hdc, rc.right - 1, rc.bottom - 1);
				LineTo(hdc, rc.left, rc.bottom - 1);
				LineTo(hdc, rc.left, rc.top);
				SelectObject(hdc, hOld);
				DeleteObject(hPen);

				SetTextColor(hdc, RGB(255, 255, 255));
				SetBkMode(hdc, TRANSPARENT);
				RECT txt = rc;
				DrawText(hdc, TEXT("应用"), -1, &txt, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				return TRUE;
			}
			break;
		}

	case WM_CLOSE:
		if (MessageBox(hWnd, TEXT("确定退出吗？"), TEXT("提示"), MB_YESNO) == IDYES)
		{
			DestroyWindow(hWnd);
		}
		break;

	case WM_DESTROY:
		KillTimer(hWnd, g_timerId);
		if (g_tipTimerId != 0)
		{
			KillTimer(hWnd, 2);
		}
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

void RestartTimer(HWND hWnd)
{
	if (g_timerId != 0)
	{
		KillTimer(hWnd, g_timerId);
	}
	if (g_pGame)
	{
		g_timerId = SetTimer(hWnd, 1, g_pGame->GetSpeed(), nullptr);
	}
}
