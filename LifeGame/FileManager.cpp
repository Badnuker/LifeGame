#include "FileManager.h"
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>

FileManager::FileManager()
{
}

FileManager::~FileManager()
{
}

bool FileManager::SaveGame(const std::wstring& filePath, const LifeGame& game)
{
	// 使用宽字符路径打开文件
	// 注意：std::ofstream 在 Windows 上对宽字符路径的支持取决于编译器版本
	// MinGW 可能需要特殊处理，这里尝试直接转换或使用 _wfopen

	// 为了兼容性，我们使用 _wfopen_s 和 C 风格 I/O，或者使用 std::ofstream 的扩展
	// 这里使用 std::ofstream 配合转换后的路径 (如果路径包含中文可能会有问题)
	// 更好的方法是使用 _wfopen

	FILE* fp = nullptr;
	_wfopen_s(&fp, filePath.c_str(), L"w");
	if (!fp)
	{
		m_lastError = L"无法打开文件进行写入";
		return false;
	}

	// 写入头部信息
	fwprintf(fp, L"# LifeGame Save File v1.0\n");

	// 获取当前时间
	time_t now = time(nullptr);
	tm tm_now;
	localtime_s(&tm_now, &now);
	fwprintf(fp, L"# Date: %04d-%02d-%02d %02d:%02d:%02d\n",
	         tm_now.tm_year + 1900, tm_now.tm_mon + 1, tm_now.tm_mday,
	         tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec);

	// 写入基本参数
	fwprintf(fp, L"WIDTH=%d\n", game.GetWidth());
	fwprintf(fp, L"HEIGHT=%d\n", game.GetHeight());
	fwprintf(fp, L"SPEED=%d\n", game.GetSpeed());

	// 写入规则 (这里我们只保存索引，或者保存规则字符串更好)
	// 暂时保存索引，但这依赖于规则列表不变
	// 理想情况是保存规则字符串
	// fwprintf(fp, L"RULE_INDEX=%d\n", game.GetCurrentRuleIndex()); // 需要在 Game 中添加 GetCurrentRuleIndex

	fwprintf(fp, L"DATA_START\n");

	// 写入网格数据 (使用简单的 RLE 变体或直接 01 矩阵)
	// 为了可读性，使用 01 矩阵，每行一个换行
	// 对于大网格，这会很大，但易于实现
	for (int y = 0; y < game.GetHeight(); ++y)
	{
		for (int x = 0; x < game.GetWidth(); ++x)
		{
			fputc(game.GetCell(x, y) ? 'O' : '.', fp);
		}
		fputc('\n', fp);
	}

	fwprintf(fp, L"DATA_END\n");

	fclose(fp);
	return true;
}

bool FileManager::LoadGame(const std::wstring& filePath, LifeGame& game)
{
	FILE* fp = nullptr;
	_wfopen_s(&fp, filePath.c_str(), L"r");
	if (!fp)
	{
		m_lastError = L"无法打开文件进行读取";
		return false;
	}

	int width = 0, height = 0, speed = 100;
	bool readingData = false;
	int currentY = 0;

	// 简单的行读取缓冲区
	wchar_t buffer[4096];
	// 注意：如果一行超过 4096 字符 (即宽度 > 4096)，这里会出问题
	// 但我们限制了宽度 2000，所以应该没问题

	while (fgetws(buffer, 4096, fp))
	{
		std::wstring line(buffer);
		// 去除换行符
		if (!line.empty() && line.back() == '\n') line.pop_back();
		if (!line.empty() && line.back() == '\r') line.pop_back();

		if (line.empty() || line[0] == '#') continue;

		if (line == L"DATA_START")
		{
			readingData = true;
			// 在读取数据前，调整网格大小
			if (width > 0 && height > 0)
			{
				game.ResizeGrid(width, height);
				game.ResetGrid(); // 清空当前内容
				game.SetSpeed(speed);
			}
			continue;
		}

		if (line == L"DATA_END")
		{
			break;
		}

		if (readingData)
		{
			if (currentY < height)
			{
				for (int x = 0; x < width && x < static_cast<int>(line.length()); ++x)
				{
					if (line[x] == 'O')
					{
						game.SetCell(x, currentY, true);
					}
				}
				currentY++;
			}
		}
		else
		{
			// 解析键值对
			size_t eqPos = line.find('=');
			if (eqPos != std::wstring::npos)
			{
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

bool FileManager::ExportRLE(const std::wstring& filePath, const LifeGame& game)
{
	// 简单的 RLE 导出实现
	FILE* fp = nullptr;
	_wfopen_s(&fp, filePath.c_str(), L"w");
	if (!fp) return false;

	fwprintf(fp, L"# Exported by LifeGame\n");
	fwprintf(fp, L"x = %d, y = %d, rule = B3/S23\n", game.GetWidth(), game.GetHeight());

	int runCount = 0;
	bool lastState = false; // 假设每行开始前是死细胞? 不，RLE 是连续的
	// RLE 通常逐行编码，行尾用 $

	for (int y = 0; y < game.GetHeight(); ++y)
	{
		runCount = 0;
		// 行首状态
		bool currentState = game.GetCell(0, y);
		runCount = 1;

		for (int x = 1; x < game.GetWidth(); ++x)
		{
			bool cell = game.GetCell(x, y);
			if (cell == currentState)
			{
				runCount++;
			}
			else
			{
				// 写入上一段
				if (runCount > 1) fwprintf(fp, L"%d", runCount);
				fwprintf(fp, L"%c", currentState ? 'o' : 'b');

				currentState = cell;
				runCount = 1;
			}
		}
		// 写入行尾最后一段
		if (runCount > 1) fwprintf(fp, L"%d", runCount);
		fwprintf(fp, L"%c", currentState ? 'o' : 'b');

		// 行结束
		if (y < game.GetHeight() - 1)
			fwprintf(fp, L"$");
		else
			fwprintf(fp, L"!");

		// 为了美观，每几行换个行
		if (y % 10 == 9) fwprintf(fp, L"\n");
	}

	fclose(fp);
	return true;
}
