#pragma once
#include "Command.h"
#include <vector>

struct CellState
{
	int x;
	int y;
	bool oldState;
};

class PlacePatternCommand : public Command
{
public:
	PlacePatternCommand(int x, int y, int patternIndex, const LifeGame& game);

	void Execute(LifeGame& game) override;
	void Undo(LifeGame& game) override;

private:
	int m_x;
	int m_y;
	int m_patternIndex;
	std::vector<CellState> m_affectedCells; // 存储受影响细胞的旧状态
};
