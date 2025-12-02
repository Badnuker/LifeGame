#pragma once
#include <windows.h>
#include <vector>
#include "PatternLibrary.h"

/**
 * @brief 图案预览控件
 * 
 * 在一个小窗口中绘制选中的图案预览。
 */
class PatternPreview
{
public:
	PatternPreview();
	~PatternPreview();

	bool Initialize(HINSTANCE hInstance, HWND hParent, int x, int y, int w, int h);
	void SetPattern(const PatternData* p);
	void Update();
	void Move(int x, int y, int w, int h); // 新增

private:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
	void OnPaint(HWND hWnd);

	HWND m_hWnd;
	const PatternData* m_pCurrentPattern;
	std::vector<std::vector<bool>> m_previewGrid;

	HBRUSH m_hBgBrush;
	HBRUSH m_hCellBrush;
};
