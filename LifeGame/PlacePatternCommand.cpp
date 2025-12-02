#include "PlacePatternCommand.h"
#include "Game.h"

PlacePatternCommand::PlacePatternCommand(int x, int y, int patternIndex, const LifeGame& game)
	: m_x(x), m_y(y), m_patternIndex(patternIndex)
{
	const PatternData* pattern = game.GetPatternLibrary().GetPattern(m_patternIndex);
	if (pattern)
	{
		// Save the bounding box area
		for (int dy = 0; dy < pattern->height; ++dy)
		{
			for (int dx = 0; dx < pattern->width; ++dx)
			{
				int cx = m_x + dx;
				int cy = m_y + dy;
				// Note: We rely on GetCell to handle boundary checks/wrapping
				bool state = game.GetCell(cx, cy);
				m_affectedCells.push_back({cx, cy, state});
			}
		}
	}
}

void PlacePatternCommand::Execute(LifeGame& game)
{
	game.PlacePattern(m_x, m_y, m_patternIndex);
}

void PlacePatternCommand::Undo(LifeGame& game)
{
	for (const auto& cell : m_affectedCells)
	{
		game.SetCell(cell.x, cell.y, cell.oldState);
	}
}
