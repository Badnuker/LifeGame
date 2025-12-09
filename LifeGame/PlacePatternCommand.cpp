#include "PlacePatternCommand.h"
#include "Game.h"

// 构造函数：预先记录受影响区域的旧状态
PlacePatternCommand::PlacePatternCommand(int x, int y, int patternIndex, const LifeGame &game)
    : m_x(x), m_y(y), m_patternIndex(patternIndex) {
    const PatternData *pattern = game.GetPatternLibrary().GetPattern(m_patternIndex);
    if (pattern) {
        // 保存图案覆盖区域内的所有细胞状态 (Bounding Box Area)
        // 这样撤销时可以精确恢复，而不需要备份整个网格
        for (int dy = 0; dy < pattern->height; ++dy) {
            for (int dx = 0; dx < pattern->width; ++dx) {
                int cx = m_x + dx;
                int cy = m_y + dy;
                // 注意：GetCell 会处理边界检查
                bool state = game.GetCell(cx, cy);
                m_affectedCells.push_back({cx, cy, state});
            }
        }
    }
}

// 执行：调用游戏核心逻辑放置图案
void PlacePatternCommand::Execute(LifeGame &game) {
    game.PlacePattern(m_x, m_y, m_patternIndex);
}

// 撤销：逐个恢复受影响细胞的旧状态
void PlacePatternCommand::Undo(LifeGame &game) {
    for (const auto &cell: m_affectedCells) {
        game.SetCell(cell.x, cell.y, cell.oldState);
    }
}
