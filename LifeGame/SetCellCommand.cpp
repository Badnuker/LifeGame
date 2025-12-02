#include "SetCellCommand.h"
#include "Game.h"

SetCellCommand::SetCellCommand(int x, int y, bool newState, bool oldState)
	: m_x(x), m_y(y), m_newState(newState), m_oldState(oldState)
{
}

void SetCellCommand::Execute(LifeGame& game)
{
	game.SetCell(m_x, m_y, m_newState);
}

void SetCellCommand::Undo(LifeGame& game)
{
	game.SetCell(m_x, m_y, m_oldState);
}
