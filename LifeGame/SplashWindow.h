#pragma once
#include <windows.h>
#include <vector>
#include <string>

class SplashWindow
{
public:
	SplashWindow();
	~SplashWindow();

	// mode: 0 = Startup (开机), 1 = Shutdown (关机)
	void Show(HINSTANCE hInstance, int mode);

	// 运行消息循环直到动画结束
	void RunLoop();

private:
	static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void OnPaint(HWND hWnd);
	void OnTimer(HWND hWnd);

	// 星空效果
	void InitStars(int width, int height);
	void UpdateStars();
	void DrawStars(HDC hdc, int width, int height);

	HWND m_hWnd;
	int m_mode; // 0: Startup, 1: Shutdown
	int m_width;
	int m_height;

	// 动画状态
	int m_frame;
	int m_maxFrames;
	float m_alpha; // 0.0 - 1.0 (模拟透明度或亮度)

	struct Star
	{
		float x, y, z;
		float speed;
		int size;
		COLORREF color;
	};

	std::vector<Star> m_stars;

	HFONT m_hTitleFont;
	HFONT m_hSubFont;
};
