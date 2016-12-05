#ifndef __PUZZLE_EFFECTS_H__
#define __PUZZLE_EFFECTS_H__

namespace PuzzleEffects
{
    enum EdgeStatus { NONE = 0, LEFT = (1<<0), RIGHT = (1<<1), TOP = (1<<2), BOTTOM = (1<<3) };
}

#endif // __PUZZLE_EFFECTS_H__
