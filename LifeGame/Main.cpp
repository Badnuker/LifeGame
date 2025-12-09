#include <windows.h>
#include "Application.h"

#ifdef _MSC_VER
#pragma comment(lib, "comctl32.lib")
#endif

/**
 * @brief 应用程序入口点
 * 
 * 标准 Win32 应用程序入口函数。
 * 负责实例化 Application 对象并启动主循环。
 */
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
                   _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
    // 实例化应用程序对象
    Application app;

    // 运行应用程序
    return app.Run(hInstance, nCmdShow, lpCmdLine);
}
