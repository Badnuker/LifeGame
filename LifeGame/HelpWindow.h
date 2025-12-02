#pragma once
#include <windows.h>
#include <vector>
#include <string>

struct HelpPage
{
	std::wstring title;
	std::wstring content;
};

class HelpWindow
{
public:
	HelpWindow();
	~HelpWindow();

	void Show(HWND hParent);

private:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

	void OnCreate(HWND hWnd);
	void OnPaint(HWND hWnd);
	void OnSize(HWND hWnd);
	void OnCommand(HWND hWnd, int id, int code);

	void DrawPage(HDC hdc, const RECT& rect);
	void UpdateFonts(); // 更新字体

	HWND m_hWnd;
	HWND m_hList; // 左侧列表
	HWND m_hZoomInBtn; // 放大按钮
	HWND m_hZoomOutBtn; // 缩小按钮
	int m_currentPage;
	float m_fontScale; // 字体缩放比例

	std::vector<HelpPage> m_pages;

	HFONT m_hTitleFont;
	HFONT m_hBodyFont;
};
