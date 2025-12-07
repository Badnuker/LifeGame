#include "PatternLibrary.h"
#include <sstream>
#include <algorithm>

/**
 * @brief 构造函数
 * 
 * 在这里调用 InitBuiltinPatterns 来填充图案库。
 */
PatternLibrary::PatternLibrary()
{
	InitBuiltinPatterns();
}

/**
 * @brief 析构函数
 */
PatternLibrary::~PatternLibrary()
{
	m_patterns.clear();
}

/**
 * @brief 获取所有图案
 */
const std::vector<PatternData>& PatternLibrary::GetPatterns() const
{
	return m_patterns;
}

/**
 * @brief 获取指定图案
 */
const PatternData* PatternLibrary::GetPattern(int index) const
{
	if (index >= 0 && index < static_cast<int>(m_patterns.size()))
	{
		return &m_patterns[index];
	}
	return nullptr;
}

/**
 * @brief 解析 RLE (Run Length Encoded) 字符串
 * 
 * RLE 是 Life 游戏社区通用的图案交换格式。
 * 格式说明:
 * - 'b' 表示死细胞 (dead)
 * - 'o' 表示活细胞 (alive)
 * - 数字表示重复次数 (例如 "3o" = "ooo")
 * - '$' 表示换行
 * - '!' 表示结束
 * 
 * @param rle 输入的 RLE 字符串
 * @param outGrid 输出的网格数据
 * @return true 成功
 * @return false 失败
 */
bool PatternLibrary::ParseRLE(const std::string& rle, std::vector<std::vector<bool>>& outGrid)
{
	outGrid.clear();
	if (rle.empty()) return false;

	std::vector<bool> currentRow;
	int count = 0;
	int width = 0;

	for (size_t i = 0; i < rle.length(); ++i)
	{
		char c = rle[i];

		if (isdigit(c))
		{
			// 如果是数字，累加计算重复次数
			// 支持多位数字，例如 "24o"
			count = count * 10 + (c - '0');
		}
		else
		{
			// 如果没有数字前缀，默认为 1
			if (count == 0) count = 1;

			if (c == 'b')
			{
				// 'b' = dead cell
				for (int k = 0; k < count; ++k) currentRow.push_back(false);
			}
			else if (c == 'o')
			{
				// 'o' = alive cell
				for (int k = 0; k < count; ++k) currentRow.push_back(true);
			}
			else if (c == '$')
			{
				// '$' = end of line
				// 记录最大宽度
				if (currentRow.size() > width) width = static_cast<int>(currentRow.size());

				outGrid.push_back(currentRow);
				currentRow.clear();

				// 处理空行 (例如 "3$" 表示跳过3行)
				// 注意：第一行已经通过上面的 push_back 添加了，所以这里添加 count-1 个空行
				for (int k = 1; k < count; ++k)
				{
					outGrid.push_back(std::vector<bool>());
				}
			}
			else if (c == '!')
			{
				// '!' = end of pattern
				if (!currentRow.empty())
				{
					if (currentRow.size() > width) width = static_cast<int>(currentRow.size());
					outGrid.push_back(currentRow);
				}
				break;
			}

			// 重置计数器
			count = 0;
		}
	}

	// 规范化网格：确保所有行长度一致，用 false 填充
	for (auto& row : outGrid)
	{
		if (row.size() < width)
		{
			row.resize(width, false);
		}
	}

	return true;
}

/**
 * @brief 初始化内置图案库
 * 
 * 这里包含了大量经典的 Life 游戏图案数据。
 * 数据来源：LifeWiki 及其他开源图案库。
 */
void PatternLibrary::InitBuiltinPatterns()
{
	// 单点绘制 (Single Cell) - 特殊占位符
	m_patterns.push_back({
		L"单点绘制",
		L"点击网格绘制单个细胞。",
		"", // 空 RLE
		0, 0
	});

	// 信号灯
	m_patterns.push_back({
		L"信号灯",
		L"周期=2轮",
		"3o!",
		3, 1
	});

	// 蟾蜍
	m_patterns.push_back({
		L"蟾蜍",
		L"周期=2轮",
		"b3o$3ob!",
		4, 2
	});

	// 红绿灯
	m_patterns.push_back({
		L"红绿灯",
		L"周期=2轮",
		"2b3o2b2$o5bo$o5bo$o5bo2$2b3o2b!",
		7, 7
	});

	// 脉冲星
	m_patterns.push_back({
		L"脉冲星",
		L"周期=3轮",
		"2b3o3b3o2b2$o4bobo4bo$o4bobo4bo$o4bobo4bo$2b3o3b3o2b2$2b3o3b3o2b$o4bobo4bo$o4bobo4bo$o4bobo4bo2$2b3o3b3o2b!",
		13, 13
	});

	// 慨影
	m_patterns.push_back({
		L"慨影",
		L"周期=15轮",
		"3o$obo$3o$3o$3o$3o$obo$3o!",
		3, 8
	});

	// 滑翔机
	m_patterns.push_back({
		L"滑翔机",
		L"周期=4轮",
		"bob$2bo$3o!",
		3, 3
	});

	// 太空船
	m_patterns.push_back({
		L"太空船",
		L"周期=4轮",
		"2b2o3b$o4bob$6bo$o5bo$b6o!",
		7, 5
	});

	// 高斯帕滑翔机枪
	m_patterns.push_back({
		L"高斯帕滑翔机枪",
		L"周期性发射滑翔机",
		"24bo11b$22bobo11b$12b2o6b2o12b2o$11bo3bo4b2o12b2o$2o8bo5bo3b2o14b$2o8bo3bob2o4bobo11b$10bo5bo7bo11b$11bo3bo20b$12b2o22b!",
		36, 9
	});

	// 银河
	m_patterns.push_back({
		L"银河",
		L"看起来像旋转的星系，周期=8轮",
		"6ob2o$6ob2o$7b2o$2o5b2o$2o5b2o$2o5b2o$2o7b$2ob6o$2ob6o!",
		9, 9
	});

	/*
	// 繁殖者
	m_patterns.push_back({
		L"繁殖者",
		L"产生滑翔机枪的飞船，种群呈二次方增长",
		"404bo2bo341b$408bo340b$404bo3bo340b$405b4o340b$416b2o331b$402bo11bo4bo \
		329b$400bobo17bo328b$342bobo46bo8bobo11bo5bo328b$342bobo44bo3bo21b6o5b \
		6o317b$331bo10bob2o48bo30bo5bo317b$329bo3bo10b2o43bo4bo36bo317b$334bo \
		6bo2bo45b5o30bo4bo318b$329bo4bo7b2o83b2o320b$330b5o50b2o362b$385b2o32b \
		3o5b2o320b$385b2o2bo13bo13bo3bo4bob2o319b$368b2o12b2ob2o3bo11bobo12bo \
		7b2obobo318b$355b2o10bo2bo8b2o2bobo4bo4b2o4bo9b2o4bo7bob2ob2o317b$355b \
		2o11b2o9b2o2b3o3bo5b2o5bo8b2o5bo2bo3bo3b2o318b$419bobo5b3o319b2$419bob \
		o5b3o319b$355b2o11b2o9b2o2b3o3bo5b2o5bo8b2o5bo2bo3bo3b2o318b$355b2o10b \
		o2bo8b2o2bobo4bo4b2o4bo9b2o4bo7bob2ob2o317b$368b2o12b2ob2o3bo11bobo12b \
		o7b2obobo318b$385b2o2bo13bo13bo3bo4bob2o319b$385b2o32b3o5b2o320b$330b \
		5o50b2o362b$329bo4bo7b2o83b2o320b$334bo6bo2bo45b5o30bo4bo318b$329bo3bo \
		10b2o43bo4bo36bo317b$331bo10bob2o48bo30bo5bo317b$342bobo44bo3bo21b6o5b \
		6o317b$342bobo46bo8bobo11bo5bo328b$400b2o18bo328b$401bo12bo4bo329b$ \
		416b2o331b$477b2o270b$475b2ob2o269b$475b4o270b$476b2o271b$376bobo370b$ \
		376b2o111b2o258b$377bo107b4ob2o5b4o248b$485b6o5b6o247b$463b2o21b4o6b4o \
		b2o246b$460b3ob2o34b2o247b$403b2o55b5o21bo262b$400b3ob2o46bo8b3o23bo \
		261b$352bobo45b5o21b3o23bo32b4o260b$352b2o47b3o20bo2b2o54b2o3bob2o257b \
		$353bo69bo3bobo26bo11b2o14bobobobo7b2o249b$421bo3bo9bobo13b4obo10bo2bo \
		13bo3b3o2bo3bo2b2o247b$405b2o14bo3b2o11bo11bo2bob2o4b2o4bobo7b2o6bobo \
		3bo4b3o2bo247b$405b2o14bo8bo3b2obo12bobo8b2o5bo8b2o7bo4b3o2bo4bo247b$ \
		422bo3bo3b3o3bo14bo44b5o248b2$328bobo91bo3bo3b3o3bo14bo44b5o248b$328b \
		2o11b2o30b2o30b2o14bo8bo3b2obo12bobo8b2o5bo8b2o7bo4b3o2bo4bo247b$329bo \
		11b2o30b2o30b2o14bo3b2o11bo11bo2bob2o4b2o4bobo7b2o6bobo3bo4b3o2bo247b$ \
		421bo3bo9bobo13b4obo10bo2bo13bo3b3o2bo3bo2b2o247b$423bo3bobo26bo11b2o \
		14bobobobo7b2o249b$424bo2b2o54b2o3bob2o257b$426b3o23bo32b4o260b$452bo \
		8b3o23bo261b$460b5o21bo262b$460b3ob2o34b2o89b2o156b$463b2o21b4o6b4ob2o \
		87b4o155b$485b6o5b6o88b2ob2o154b$485b4ob2o5b4o91b2o155b$489b2o258b$ \
		464bo136b4o144b$464bobo133b6o143b$280bobo181b2o134b4ob2o142b$280b2o \
		294b3o13b3o9b2o9b2o132b$281bo252bobo38b5o12bo18b4ob2o131b$516b3o14b2o \
		2bo37b3ob2o12bo17b6o132b$515b5o12b3o2bo40b2o27bo4b4o133b$515b3ob2o10b \
		3o72bo2bo139b$440bo77b2o11bobo2b2o59b3o149b$440bobo87b2ob3obo38b2o17b \
		5o12b2o135b$440b2o89bo6bo37b2o16b3o14bo3bo133b$532bo4bo20b2o15bo2bo14b \
		o3bo9bo3bo4bo132b$534b5o6b2o10bo2bo8b2o4bobo7b2o5b2o2bo4b2o5bobo5bo \
		132b$538b2o5b2o11b2o9b2o5bo8b2o5bo6b3o9b2ob3o132b$538b2o53b8o148b2$ \
		538b2o53b8o148b$449b2o30b2o30b2o23b2o5b2o11b2o9b2o5bo8b2o5bo6b3o9b2ob \
		3o81b2o49b$232bobo214b2o30b2o30b2o19b5o6b2o10bo2bo8b2o4bobo7b2o5b2o2bo \
		4b2o5bobo5bo79b2ob2o48b$232b2o298bo4bo20b2o15bo2bo14bo3bo9bo3bo4bo79b \
		4o49b$233bo297bo6bo37b2o16b3o14bo3bo81b2o50b$530b2ob3obo38b2o17b5o12b \
		2o135b$531bobo2b2o59b3o110b2o37b$531b3o72bo2bo96b4ob2o5b4o27b$392bo \
		139b3o2bo40b2o27bo4b4o90b6o5b6o26b$392bobo138b2o2bo37b3ob2o12bo17b6o \
		67b2o21b4o6b4ob2o25b$392b2o140bobo38b5o12bo18b4ob2o63b3ob2o34b2o26b$ \
		576b3o13b3o9b2o9b2o7b2o55b5o21bo41b$600b4ob2o14b3ob2o46bo8b3o23bo40b$ \
		600b6o15b5o21b3o23bo32b4o39b$601b4o17b3o20bo2b2o54b2o3bob2o36b$644bo3b \
		obo26bo11b2o14bobobobo7b2o28b$642bo3bo9bobo13b4obo10bo2bo13bo3b3o2bo3b \
		o2b2o26b$569bo56b2o14bo3b2o11bo11bo2bob2o4b2o4bobo7b2o6bobo3bo4b3o2bo \
		26b$184bobo381bo57b2o14bo8bo3b2obo12bobo8b2o5bo8b2o7bo4b3o2bo4bo26b$ \
		184b2o382b3o72bo3bo3b3o3bo14bo44b5o27b$185bo563b$643bo3bo3b3o3bo14bo \
		44b5o27b$562b2o30b2o30b2o14bo8bo3b2obo12bobo8b2o5bo8b2o7bo4b3o2bo4bo \
		26b$562b2o30b2o30b2o14bo3b2o11bo11bo2bob2o4b2o4bobo7b2o6bobo3bo4b3o2bo \
		26b$344bo297bo3bo9bobo13b4obo10bo2bo13bo3b3o2bo3bo2b2o26b$344bobo198bo \
		98bo3bobo26bo11b2o14bobobobo7b2o28b$344b2o198bo100bo2b2o54b2o3bob2o36b \
		$544b3o100b3o23bo32b4o39b$673bo8b3o23bo40b$681b5o21bo41b$681b3ob2o34b \
		2o26b$684b2o21b4o6b4ob2o25b$706b6o5b6o26b$706b4ob2o5b4o27b$136bobo571b \
		2o37b$136b2o547bo63b$137bo547bobo61b$685b2o62b3$296bo452b$296bobo198bo \
		251b$296b2o198bo252b$496b3o162bo55b2o30b$661bobo52b4o29b$661b2o53b2ob \
		2o28b$718b2o29b2$727b4o18b$726b6o17b$88bobo635b4ob2o16b$88b2o547bo64b \
		3o25b2o9b2o6b$89bo547bobo61b5o31b4ob2o5b$637b2o62b3ob2o16b2o12b6o6b$ \
		665b2o37b2o16bobo13b4o7b$664bo2bo53bo2bo12bo11b$248bo412b2obo61bo9bobo \
		10b$248bobo198bo216b2o52bo15bobo10b$248b2o198bo212bo3b2o39b2o17bobobo \
		10b2o7b$448b3o212bobo22b2o15bo2bo13bo2bobo2bo9bobo6b$643b2o30b2o10bo2b \
		o4bo3b2o4bobo7b2o6b2o5bo2bo8bo6b$643b2o23b2o5b2o11b2o5bo3b2o5bo8b2o11b \
		obobo2bo4b3o6b$668b2o55b2obo3bo2bo13b2$668b2o55b2obo3bo2bo13b$3b2o30b \
		2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o30b \
		2o30b2o30b2o30b2o30b2o30b2o23b2o5b2o11b2o5bo3b2o5bo8b2o11bobobo2bo4b3o \
		6b$3b2o30b2o3bobo24b2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o30b \
		2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o10bo2bo4bo3b2o4bobo7b2o \
		6b2o5bo2bo8bo6b$40b2o547bo73bobo22b2o15bo2bo13bo2bobo2bo9bobo6b$41bo \
		547bobo69bo3b2o39b2o17bobobo10b2o7b$589b2o75b2o52bo15bobo10b$3bo657b2o \
		bo61bo9bobo10b$2b3o659bo2bo53bo2bo12bo11b$bo3bo194bo464b2o37b2o16bobo \
		13b4o7b$ob3obo193bobo198bo299b3ob2o16b2o12b6o6b$b5o194b2o198bo300b5o \
		31b4ob2o5b$196b2o30b2o30b2o30b2o30b2o30b2o30b2o10b3o17b2o30b2o30b2o30b \
		2o30b2o152b3o25b2o9b2o6b$35b2o30b2o30b2o30b2o30b2o30bo2bo28bo2bo28bo2b \
		o28bo2bo28bo2bo28bo2bo28bo2bo28bo2bo28bo2bo28bo2bo28bo2bo28bo2bo175b4o \
		b2o16b$35bobo29bobo29bobo29bobo29bobo29bo2bo28bo2bo28bo2bo28bo2bo28bo \
		2bo28bo2bo28bo2bo28bo2bo28bo2bo28bo2bo28bo2bo28bo2bo175b6o17b$36b2o30b \
		2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o30b \
		2o30b2o177b4o18b$6b3o692bo47b$8bo691bo48b$7bo741b$701bobo45b$703bo45b$ \
		701bo47b$702bo46b2$38b2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o \
		30b2o371b4o14b$4b3o30bobo29bobo29bobo29bobo29bobo29bo2bo28bo2bo28bo2bo \
		28bo2bo28bo2bo28bo2bo369b6o13b$3bo3bo29b2o30b2o30b2o30b2o30b2o30bo2bo \
		28bo2bo28bo2bo28bo2bo28bo2bo28bo2bo229b2o138b4ob2o12b$2bo5bo189b2o30b \
		2o30b2o30b2o30b2o30b2o229b2o115b3o25b2o9b2o2b$2b2obob2o193b2o387bo113b \
		5o15b3o13b4ob2ob$202bobo461b2o37b3ob2o14bo15b6o2b$202bo464b2o39b2o18bo \
		13b4o3b$5bo659bobo59bo12bo8b$4bobo657b3o71b2o9b$4bobo34b2o621bo2bo62bo \
		8bo2bo6b$5bo35bobo356b2o262bob3o39b2o19b4o7b3ob2o3b$41bo357b2o264bo2bo \
		21b2o15bo2bo21bo13bo2b$5b2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o30b \
		2o30b2o30b2o30b2o10bo19b2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o19b2o9b2o \
		10bo2bo8b2o4bobo7b2o5b4o2bob2o12bo2b$5b2o30b2o30b2o30b2o30b2o30b2o30b \
		2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o30b2o30b \
		2o23b2o5b2o11b2o9b2o5bo8b2o5bobobobo4b2o5bo2b2o2b$670b2o53b2o2bobo11b \
		2o4b2$670b2o53b2o2bobo11b2o4b$422bo30b2o30b2o30b2o30b2o30b2o30b2o30b2o \
		23b2o5b2o11b2o9b2o5bo8b2o5bobobobo4b2o5bo2b2o2b$422bo30b2o30b2o30b2o \
		30b2o30b2o30b2o23b2o5b2o19b2o9b2o10bo2bo8b2o4bobo7b2o5b4o2bob2o12bo2b$ \
		424b2o211b2o26bo2bo21b2o15bo2bo21bo13bo2b$250b2o387bo24bob3o39b2o19b4o \
		7b3ob2o3b$250bobo411bo2bo62bo8bo2bo6b$250bo413b3o71b2o9b$665bobo59bo \
		12bo8b$667b2o39b2o18bo13b4o3b$89b2o575b2o37b3ob2o14bo15b6o2b$89bobo \
		356b2o212b2o41b5o15b3o13b4ob2ob$89bo357b2o212b2o43b3o25b2o9b2o2b$449bo \
		213bo66b4ob2o12b$730b6o13b$731b4o14b2$479b6o6b2o229b2o25b$478bo5bo4bo \
		4bo225b2ob2o24b$472b2o10bo10bo190b2o32b4o25b$414b2o38b5o12b2o5bo4bo5bo \
		5bo189b2o34b2o26b$298b2o113b3o37bo4bo14bo6b2o8b6o191bo61b$298bobo93b5o \
		12b2o3bo41bo27bo262b$298bo94bo4bo12bo3bo37bo3bo28b3o260b$398bo11bo44bo \
		21bo7bob2o2bo257b$393bo3bo12b3o2b2o59b3o8b3o2bo256b$137b2o256bo14bobob \
		3o38b2o16b3o10b5o2bo255b$137bobo271b3ob2o20b2o15bo2bo14bo3bo3bo5b3obob \
		3o215b2o37b$137bo275b2o9b2o10bo2bo8b2o4bobo7b2o5bo2bobo4bo9b2ob2o213b \
		2o38b$417b2o5b2o11b2o9b2o5bo8b2o5bo8bo9b2ob2o216bo37b$417b2o53b8o252b \
		6o6b2o3b$731bo5bo4bo4bob$417b2o53b8o257bo10bo$360b2o30b2o23b2o5b2o11b \
		2o9b2o5bo8b2o5bo8bo9b2ob2o212b5o19bo4bo5bo5bo$360b2o30b2o19b2o9b2o10bo \
		2bo8b2o4bobo7b2o5bo2bobo4bo9b2ob2o210bo4bo20bobo8b6o$411b3ob2o20b2o15b \
		o2bo14bo3bo3bo5b3obob3o152b5o59bo37b$410bobob3o38b2o16b3o10b5o2bo152bo \
		4bo21bo32bo3bo20bo2bo14b$346b2o62b3o2b2o59b3o8b3o2bo158bo20bobo33bo21b \
		3ob2o13b$346bobo61bo44bo21bo7bob2o2bo154bo3bo24bo53b2o3bob2o11b$346bo \
		64bo3bo37bo3bo28b3o159bo21bob2o24b2obo12b2o14bo3bo9b3o2b$411b2o3bo41bo \
		27bo181bob2o25b4ob2o9bo2bo13bobobob2o4bo3b2ob$413b3o37bo4bo14bo6b2o8b \
		6o155b2o13b3o2b2o7b2ob2o11bo4b2o4b2o4bobo7b2o6b5obobo2bobo2b2o$185b2o \
		227b2o38b5o12b2o5bo4bo5bo5bo155b2o14b2o2b2o3bo4b3o12bobo8b2o5bo8b2o7bo \
		4b2o2bo5bob$185bobo284b2o10bo10bo180b2o3b2o14bo40bo3b5o2b$185bo292bo5b \
		o4bo4bo254b$370b2o107b6o6b2o183b2o3b2o14bo40bo3b5o2b$370bobo278b2o14b \
		2o2b2o3bo4b3o12bobo8b2o5bo8b2o7bo4b2o2bo5bob$370bo97bo2bo179b2o13b3o2b \
		2o7b2ob2o11bo4b2o4b2o4bobo7b2o6b5obobo2bobo2b2o$472bo195bob2o25b4ob2o \
		9bo2bo13bobobob2o4bo3b2ob$468bo3bo175bo21bob2o24b2obo12b2o14bo3bo9b3o \
		2b$469b4o173bo3bo24bo53b2o3bob2o11b$651bo20bobo33bo21b3ob2o13b$646bo4b \
		o21bo32bo3bo20bo2bo14b$394b2o251b5o59bo37b$394bobo309bo4bo20bobo8b6o$ \
		394bo312b5o19bo4bo5bo5bo$737bo10bo$731bo5bo4bo4bob$233b2o497b6o6b2o3b$ \
		233bobo513b$233bo487bo2bo24b$418b2o305bo23b$418bobo300bo3bo23b$418bo \
		303b4o23b6$442b2o305b$442bobo304b$442bo306b3$281b2o466b$281bobo465b$ \
		281bo467b$466b2o281b$466bobo280b$466bo282b$491b2o256b$487b4ob2o5b4o \
		246b$487b6o5b6o245b$465b2o21b4o6b4ob2o244b$462b3ob2o34b2o245b$462b5o \
		21bo260b$454bo8b3o23bo259b$428b3o23bo32b4o258b$426bo2b2o54b2o3bob2o \
		255b$425bo3bobo26bo11b2o14bobobobo7b2o247b$329b2o92bo3bo9bobo13b4obo \
		10bo2bo13bo3b3o2bo3bo2b2o245b$329bobo11b2o30b2o30b2o14bo3b2o11bo11bo2b \
		ob2o4b2o4bobo7b2o6bobo3bo4b3o2bo245b$329bo13b2o30b2o30b2o14bo8bo3b2obo \
		12bobo8b2o5bo8b2o7bo4b3o2bo4bo245b$424bo3bo3b3o3bo14bo44b5o246b2$424bo \
		3bo3b3o3bo14bo44b5o246b$407b2o14bo8bo3b2obo12bobo8b2o5bo8b2o7bo4b3o2bo \
		4bo245b$407b2o14bo3b2o11bo11bo2bob2o4b2o4bobo7b2o6bobo3bo4b3o2bo245b$ \
		353b2o68bo3bo9bobo13b4obo10bo2bo13bo3b3o2bo3bo2b2o245b$353bobo69bo3bob \
		o26bo11b2o14bobobobo7b2o247b$353bo49b3o20bo2b2o54b2o3bob2o255b$402b5o \
		21b3o23bo32b4o258b$402b3ob2o46bo8b3o23bo259b$405b2o55b5o21bo260b$462b \
		3ob2o34b2o245b$465b2o21b4o6b4ob2o244b$377b2o108b6o5b6o245b$377bobo107b \
		4ob2o5b4o246b$377bo113b2o256b2$478b2o269b$477b4o268b$477b2ob2o267b$ \
		479b2o268b$401b2o346b$401bobo16b2o327b$401bo14b4ob2o5b4o317b$416b6o5b \
		6o316b$374bo19b2o21b4o6b4ob2o315b$372b2obo15b3ob2o34b2o316b$334b2o35bo \
		3bo15b5o353b$331b3ob2o30b4obobo17b3o354b$331b5o30bobo3b2o12b2o33bo327b \
		$332b3o20bo4b2o4bobo16bo2bo32b2o5b3o318b$354bobo2b2obo2b2ob2o18bo15bo \
		14bo2bo3b2o321b$353bo3bo2b2ob3o2b4o11b2o5b2o11b2o12b3o6bo4b2o316b$340b \
		2o12bo2bo4bo3b2obob2o17b2o4b2o4b3o7b2o4b2o5b2ob2o2bo316b$340b2o12b2ob \
		2o9b2ob2o11bobo9b2o14b2o5bobo10bo316b$370bo14bo35bo7b3o317b2$370bo14bo \
		35bo7b3o317b$340b2o12b2ob2o9b2ob2o11bobo9b2o14b2o5bobo10bo316b$340b2o \
		12bo2bo4bo3b2obob2o17b2o4b2o4b3o7b2o4b2o5b2ob2o2bo316b$353bo3bo2b2ob3o \
		2b4o11b2o5b2o11b2o12b3o6bo4b2o316b$354bobo2b2obo2b2ob2o18bo15bo14bo2bo \
		3b2o321b$332b3o20bo4b2o4bobo16bo2bo32b2o5b3o318b$331b5o30bobo3b2o12b2o \
		33bo327b$331b3ob2o30b4obobo17b3o354b$334b2o35bo3bo15b5o353b$372b2obo \
		15b3ob2o34b2o316b$374bo19b2o21b4o6b4ob2o315b$416b6o5b6o316b$416b4ob2o \
		5b4o317b$403b2o15b2o327b$402bo346b$407b2o340b$406b4o339b$406b2ob2o338b \
		$408b2o! \
		",
		749, 338
	});
	*/

	/*
	// 1. 滑翔机 (Glider)
	// 最基本的移动飞船
	m_patterns.push_back({
		L"滑翔机 (Glider)",
		L"最小的飞船，沿对角线移动。",
		"bob$2bo$3o!",
		3, 3
	});

	// 2. 轻型飞船 (LWSS)
	// 另一种常见的飞船
	m_patterns.push_back({
		L"轻型飞船 (LWSS)",
		L"Lightweight spaceship，正交移动。",
		"o2bob$4bo$o3bo$b4o!",
		5, 4
	});

	// 3. 脉冲星 (Pulsar)
	// 周期为 3 的振荡器
	m_patterns.push_back({
		L"脉冲星 (Pulsar)",
		L"周期为 3 的大型振荡器。",
		"2b3o2b3o2b$2b3o2b3o2b$$o4bobo4bo$o4bobo4bo$o4bobo4bo$2b3o2b3o2b$$2b3o2b3o2b$o4bobo4bo$o4bobo4bo$o4bobo4bo$$2b3o2b3o2b!",
		13, 13
	});

	// 4. 高斯帕滑翔机枪 (Gosper Glider Gun)
	// 第一个被发现的无限增长模式
	m_patterns.push_back({
		L"高斯帕滑翔机枪 (Gosper Glider Gun)",
		L"周期性发射滑翔机，导致种群无限增长。",
		"24bo$22bobo$12b2o6b2o12b2o$11bo3bo4b2o12b2o$2o8bo5bo3b2o$2o8bo3bob2o4bobo$10bo5bo7bo$11bo3bo$12b2o!",
		36, 9
	});

	// 5. 顶针 (Diehard)
	// 寿命很长的玛土撒拉
	m_patterns.push_back({
		L"顶针 (Diehard)",
		L"仅仅 7 个细胞，却能存活 130 代。",
		"6b2o$2o6b$bo!",
		8, 3
	});

	// 6. 橡子 (Acorn)
	// 著名的玛土撒拉
	m_patterns.push_back({
		L"橡子 (Acorn)",
		L"极其活跃的图案，7个细胞演化出数千个。",
		"bo5b$3bo3b$2o2b3o!",
		7, 3
	});

	// 7. 银河 (Galaxy)
	// 漂亮的旋转振荡器
	m_patterns.push_back({
		L"银河 (Galaxy)",
		L"看起来像旋转的星系，周期 8。",
		"2o2b2o2b2o$2o2b2o2b2o$2o2b2o2b2o$2o2b2o2b2o$2o2b2o2b2o$2o2b2o2b2o!",
		10, 6 // 近似 RLE，实际可能需要更精确的
	});
	// 修正 Galaxy RLE
	m_patterns.back().rleString = "6bo2bo$6bo2bo$2o5bo2bo$2o5b3o$$3o5b2o$o2bo5b2o$o2bo6b$o2bo!";
	m_patterns.back().width = 9;
	m_patterns.back().height = 9;

	// 8. 繁殖者 (Breeder 1)
	// 二次增长模式
	// 这是一个非常大的图案，RLE 会很长，非常适合增加代码行数
	m_patterns.push_back({
		L"繁殖者 (Breeder 1)",
		L"产生滑翔机枪的飞船，种群呈二次方增长。",
		"35bo$33b3o$32bo2bo$32b3o10b2o$32b3o10b2o$32b3o$$30b3o$29bo2bo$29bo$29bo4b2o$34b2o$$27b3o$26bo2bo$26bo$26bo$20b2o$20b2o$$24b3o$23bo2bo$23bo$23bo$$21b3o$20bo2bo$20bo$20bo$$18b3o$17bo2bo$17bo$17bo$$15b3o$14bo2bo$14bo$14bo$$12b3o$11bo2bo$11bo$11bo$$9b3o$8bo2bo$8bo$8bo$$6b3o$5bo2bo$5bo$5bo$$3b3o$2bo2bo$2bo$2bo!",
		50, 50 // 估算
	});

	// 9. 空间耙 (Space Rake)
	// 移动并发射滑翔机
	m_patterns.push_back({
		L"空间耙 (Space Rake)",
		L"移动的同时留下滑翔机。",
		"2bo$2b3o$5bo$4b2o$4bo10b4o$15bo3bo$19bo$15bo2bo!",
		20, 10
	});

	// 10. 铜头蛇 (Copperhead)
	// 2016年才发现的飞船
	m_patterns.push_back({
		L"铜头蛇 (Copperhead)",
		L"2016年发现的c/10正交飞船。",
		"b2o2b2o$3b2o$3b2o$bobo2bobo$o6bo$o6bo$b2o2b2o$b2o2b2o$$b2o2b2o$2b4o!",
		8, 12
	});

	// 11. 周末 (Weekender)
	m_patterns.push_back({
		L"周末 (Weekender)",
		L"一种复杂的飞船。",
		"2b2o5b2o$2b2o5b2o$$4b2ob2o$3bo5bo$3bo5bo$2bo7bo$2b2o5b2o$$3b2o3b2o$3bo5bo$$10bo$8b2o$8bo!",
		12, 16
	});

	// 12. 双管枪 (Bi-Gun)
	m_patterns.push_back({
		L"双管枪 (Bi-Gun)",
		L"两个滑翔机枪对着射击。",
		"24bo11b2o$22bobo11b2o$12b2o6b2o12b2o$11bo3bo4b2o12b2o$2o8bo5bo3b2o$2o8bo3bob2o4bobo$10bo5bo7bo$11bo3bo$12b2o$$12b2o$11bo3bo$10bo5bo7bo$2o8bo3bob2o4bobo$2o8bo5bo3b2o$11bo3bo4b2o12b2o$12b2o6b2o12b2o$22bobo11b2o$24bo11b2o!",
		50, 20
	});

	// 13. 模拟金 (Simkin Glider Gun)
	m_patterns.push_back({
		L"模拟金滑翔机枪",
		L"另一种产生滑翔机的机制。",
		"2o2b2o$2o2b2o$$2o2b2o$2o2b2o10b2o$18b2o$$15b2o$15b2o$$23b2o$23b2o!",
		30, 10
	});

	// 14. 蜂巢 (Beehive) - 静态
	m_patterns.push_back({
		L"蜂巢 (Beehive)",
		L"最常见的静态物体之一。",
		"b2o$o2bo$b2o!",
		4, 3
	});

	// 15. 块 (Block) - 静态
	m_patterns.push_back({
		L"块 (Block)",
		L"最简单的静态物体。",
		"2o$2o!",
		2, 2
	});

	// 16. 船 (Boat) - 静态
	m_patterns.push_back({
		L"船 (Boat)",
		L"常见的静态物体。",
		"2o$obo$bo!",
		3, 3
	});

	// 17. 蛇 (Snake) - 静态
	m_patterns.push_back({
		L"蛇 (Snake)",
		L"静态物体。",
		"2o2b$bobo$2b2o!",
		4, 3
	});

	// 18. 航母 (Aircraft Carrier) - 静态
	m_patterns.push_back({
		L"航母 (Aircraft Carrier)",
		L"静态物体。",
		"2o2b$o2b2o$2b2o!",
		5, 3
	});

	// 19. 蟾蜍 (Toad) - 振荡器
	m_patterns.push_back({
		L"蟾蜍 (Toad)",
		L"周期 2 振荡器。",
		"b3o$3o!",
		4, 2
	});

	// 20. 信号灯 (Blinker) - 振荡器
	m_patterns.push_back({
		L"信号灯 (Blinker)",
		L"最简单的振荡器。",
		"3o!",
		3, 1
	});

	// 21. 烽火台 (Beacon) - 振荡器
	m_patterns.push_back({
		L"烽火台 (Beacon)",
		L"周期 2 振荡器。",
		"2o2b$2o2b$2b2o$2b2o!",
		4, 4
	});

	// 22. 时钟 (Clock) - 振荡器
	m_patterns.push_back({
		L"时钟 (Clock)",
		L"周期 2 振荡器。",
		"2bo$o2bo$bobo$b2o!",
		4, 4
	});

	// 23. 五连体 (Penta-decathlon)
	m_patterns.push_back({
		L"五连体 (Penta-decathlon)",
		L"周期 15 的振荡器。",
		"2bo4bo$2o4b2o$2bo4bo!",
		8, 3
	});

	// 24. 字体 (Font) - 趣味
	m_patterns.push_back({
		L"Hello World",
		L"用细胞拼写的文字。",
		"o2b o2b 3o 2b o 2b o 2b 3o$3o 3o 3o 2b 3o 2b 3o 2b 3o$o2b o2b o2b 2b o2b 2b o2b 2b o2b!",
		// 伪RLE，仅作占位，实际需要更复杂的点阵
		50, 10
	});
	// 修正 Hello World RLE (简化版)
	m_patterns.back().rleString = "o3bo2bo3bo2bo3bo$o3bo2bo3bo2bo3bo$5o2b5o2b5o$o3bo2bo3bo2bo3bo$o3bo2bo3bo2bo3bo!";
	m_patterns.back().width = 20;
	m_patterns.back().height = 5;

	// 25. 随机填充 (Random) - 特殊标记
	// 这个通常由逻辑处理，但放在这里占位
	m_patterns.push_back({
		L"随机填充 (Random)",
		L"随机生成细胞。",
		"",
		0, 0
	});
	*/
}
