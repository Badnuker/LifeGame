#pragma once
#include <windows.h>
#include <vector>
#include "PatternLibrary.h"

/**
 * @brief 图案预览控件 (Pattern Preview Control)
 * 
 * 这是一个自定义的 Win32 子窗口控件，用于在 UI 上实时显示选中图案的缩略图。
 * 当用户在图案列表中选择不同的项时，此控件会解析对应的 RLE 数据并绘制出来，
 * 帮助用户在放置前直观地看到图案形状。
 */
class PatternPreview
{
public:
	PatternPreview();
	~PatternPreview();

	/**
	 * @brief 初始化预览控件
	 * 
	 * 注册窗口类并创建子窗口。
	 * @param hInstance 应用程序实例句柄
	 * @param hParent 父窗口句柄
	 * @param x 控件 X 坐标
	 * @param y 控件 Y 坐标
	 * @param w 控件宽度
	 * @param h 控件高度
	 * @return true 初始化成功
	 */
	bool Initialize(HINSTANCE hInstance, HWND hParent, int x, int y, int w, int h);

	/**
	 * @brief 设置当前要预览的图案
	 * 
	 * 解析图案的 RLE 数据并触发重绘。
	 * @param p 指向图案数据的指针，如果为 nullptr 则清空预览
	 */
	void SetPattern(const PatternData* p);

	/**
	 * @brief 强制更新显示
	 * 
	 * 触发窗口重绘消息 (WM_PAINT)。
	 */
	void Update();

	/**
	 * @brief 移动或调整控件大小
	 * 
	 * @param x 新 X 坐标
	 * @param y 新 Y 坐标
	 * @param w 新宽度
	 * @param h 新高度
	 */
	void Move(int x, int y, int w, int h);

private:
	/**
	 * @brief 静态窗口过程
	 * 
	 * 处理预览窗口的消息。
	 */
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

	/**
	 * @brief 绘图处理函数
	 * 
	 * 执行实际的绘制逻辑：计算缩放比例、居中偏移，并绘制网格。
	 */
	void OnPaint(HWND hWnd);

	HWND m_hWnd; ///< 预览窗口句柄
	const PatternData* m_pCurrentPattern; ///< 当前持有的图案数据指针
	std::vector<std::vector<bool>> m_previewGrid; ///< 解析后的图案网格缓存

	HBRUSH m_hBgBrush; ///< 背景画刷 (深色)
	HBRUSH m_hCellBrush; ///< 细胞画刷 (亮青色)
};
