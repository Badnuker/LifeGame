#include "FileManager.h"
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>

FileManager::FileManager() {
}

FileManager::~FileManager() {
}

// 保存游戏状态到文件
bool FileManager::SaveGame(const std::wstring &filePath, const LifeGame &game) {
    // 使用宽字符路径打开文件
    FILE *fp = nullptr;
    // 使用 _wfopen_s 安全地打开文件，支持中文路径
    _wfopen_s(&fp, filePath.c_str(), L"w");
    if (!fp) {
        m_lastError = L"无法打开文件进行写入";
        return false;
    }

    // 1. 写入头部信息 (Header)
    fwprintf(fp, L"# LifeGame Save File v1.0\n");

    // 获取当前时间并写入注释
    time_t now = time(nullptr);
    tm tm_now;
    localtime_s(&tm_now, &now);
    fwprintf(fp, L"# Date: %04d-%02d-%02d %02d:%02d:%02d\n",
             tm_now.tm_year + 1900, tm_now.tm_mon + 1, tm_now.tm_mday,
             tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec);

    // 2. 写入基本参数 (Metadata)
    fwprintf(fp, L"WIDTH=%d\n", game.GetWidth());
    fwprintf(fp, L"HEIGHT=%d\n", game.GetHeight());
    fwprintf(fp, L"SPEED=%d\n", game.GetSpeed());

    fwprintf(fp, L"DATA_START\n");

    // 3. 写入网格数据 (Grid Data)
    // 为了可读性，使用字符矩阵表示：'O' 代表活细胞，'.' 代表死细胞
    // 这种格式虽然占用空间较大，但方便人工阅读和调试
    for (int y = 0; y < game.GetHeight(); ++y) {
        for (int x = 0; x < game.GetWidth(); ++x) {
            fputc(game.GetCell(x, y) ? 'O' : '.', fp);
        }
        fputc('\n', fp);
    }

    fwprintf(fp, L"DATA_END\n");

    fclose(fp);
    return true;
}

// 从文件加载游戏状态
bool FileManager::LoadGame(const std::wstring &filePath, LifeGame &game) {
    FILE *fp = nullptr;
    // 使用 _wfopen_s 打开文件进行读取
    _wfopen_s(&fp, filePath.c_str(), L"r");
    if (!fp) {
        m_lastError = L"无法打开文件进行读取";
        return false;
    }

    int width = 0, height = 0, speed = 100;
    bool readingData = false;
    int currentY = 0;

    // 简单的行读取缓冲区
    wchar_t buffer[4096];

    // 逐行读取文件内容
    while (fgetws(buffer, 4096, fp)) {
        std::wstring line(buffer);
        // 去除行尾的换行符
        if (!line.empty() && line.back() == '\n') line.pop_back();
        if (!line.empty() && line.back() == '\r') line.pop_back();

        // 跳过空行和注释行 (# 开头)
        if (line.empty() || line[0] == '#') continue;

        // 检测数据块开始标记
        if (line == L"DATA_START") {
            readingData = true;
            // 在读取数据前，根据解析出的宽高调整网格大小
            if (width > 0 && height > 0) {
                game.ResizeGrid(width, height);
                game.ResetGrid(); // 清空当前内容
                game.SetSpeed(speed);
            }
            continue;
        }

        // 检测数据块结束标记
        if (line == L"DATA_END") {
            break;
        }

        if (readingData) {
            // 解析网格数据行
            if (currentY < height) {
                for (int x = 0; x < width && x < static_cast<int>(line.length()); ++x) {
                    if (line[x] == 'O') {
                        game.SetCell(x, currentY, true);
                    }
                }
                currentY++;
            }
        } else {
            // 解析键值对 (Key-Value Pairs)
            size_t eqPos = line.find('=');
            if (eqPos != std::wstring::npos) {
                std::wstring key = line.substr(0, eqPos);
                std::wstring val = line.substr(eqPos + 1);

                if (key == L"WIDTH") width = _wtoi(val.c_str());
                else if (key == L"HEIGHT") height = _wtoi(val.c_str());
                else if (key == L"SPEED") speed = _wtoi(val.c_str());
            }
        }
    }

    fclose(fp);
    return true;
}

// 导出为 RLE 格式
bool FileManager::ExportRLE(const std::wstring &filePath, const LifeGame &game) {
    // 简单的 RLE (Run Length Encoded) 导出实现
    // RLE 格式文档: https://conwaylife.com/wiki/Run_Length_Encoded
    FILE *fp = nullptr;
    _wfopen_s(&fp, filePath.c_str(), L"w");
    if (!fp) return false;

    // 写入 RLE 头部
    fwprintf(fp, L"# Exported by LifeGame\n");
    fwprintf(fp, L"x = %d, y = %d, rule = B3/S23\n", game.GetWidth(), game.GetHeight());

    int runCount = 0;
    bool lastState = false; // 假设每行开始前是死细胞? 不，RLE 是连续的
    // RLE 通常逐行编码，行尾用 $

    for (int y = 0; y < game.GetHeight(); ++y) {
        runCount = 0;
        // 获取行首第一个细胞的状态
        bool currentState = game.GetCell(0, y);
        runCount = 1;

        // 遍历该行剩余细胞
        for (int x = 1; x < game.GetWidth(); ++x) {
            bool cell = game.GetCell(x, y);
            if (cell == currentState) {
                // 如果状态相同，计数加一
                runCount++;
            } else {
                // 如果状态改变，写入上一段的行程编码
                if (runCount > 1) fwprintf(fp, L"%d", runCount);
                fwprintf(fp, L"%c", currentState ? 'o' : 'b'); // o=活, b=死

                currentState = cell;
                runCount = 1;
            }
        }
        // 写入行尾最后一段
        if (runCount > 1) fwprintf(fp, L"%d", runCount);
        fwprintf(fp, L"%c", currentState ? 'o' : 'b');

        // 行结束标记
        if (y < game.GetHeight() - 1)
            fwprintf(fp, L"$");
        else
            fwprintf(fp, L"!"); // 文件结束标记

        // 为了美观，每几行换个行 (不影响 RLE 解析)
        if (y % 10 == 9) fwprintf(fp, L"\n");
    }

    fclose(fp);
    return true;
}
