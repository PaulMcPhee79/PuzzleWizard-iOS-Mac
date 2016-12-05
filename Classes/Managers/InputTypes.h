
#ifndef __INPUT_TYPES_H__
#define __INPUT_TYPES_H__

namespace CMInputs
{
    // Notes:
    // Focus State  : [8 bits category | 24 bits state]
    // Has Focus    : [8 bits category | 24 bits individual settings]
    static const int NUM_STATE_BITS                     = 24;
    static const uint FOCUS_CAT_MASK                    = 0xff000000;
    static const uint HAS_FOCUS_MASK                    = ~FOCUS_CAT_MASK;
    
    // Focus categories
    static const uint FOCUS_CAT_TITLE                   = 1 << NUM_STATE_BITS;
    static const uint FOCUS_CAT_MENU                    = 1 << (NUM_STATE_BITS + 1);
    static const uint FOCUS_CAT_PLAYFIELD               = 1 << (NUM_STATE_BITS + 2);
    static const uint FOCUS_CAT_ALL                     = FOCUS_CAT_MASK;
    
    // ***** Focus states *****
    static const uint FOCUS_STATE_NONE = 0;

    // FOCUS_CAT_TITLE
    static const uint FOCUS_STATE_TITLE                 = FOCUS_CAT_TITLE + 1;

    // FOCUS_CAT_MENU
    static const uint FOCUS_STATE_MENU                  = FOCUS_CAT_MENU + 1;
    static const uint FOCUS_STATE_PUZZLE_MENU           = FOCUS_CAT_MENU + 2;
    static const uint FOCUS_STATE_MENU_DIALOG           = FOCUS_CAT_MENU + 3;

    // FOCUS_CAT_PLAYFIELD
    static const uint FOCUS_STATE_PF_PLAYFIELD          = FOCUS_CAT_PLAYFIELD + 1;

    // ***** Individual bit settings *****
    
    // FOCUS_CAT_ALL
    static const uint HAS_FOCUS_ALL                     = FOCUS_CAT_ALL + HAS_FOCUS_MASK;

    // FOCUS_CAT_TITLE
    static const uint HAS_FOCUS_TITLE                   = FOCUS_CAT_TITLE + 0x1;

    // FOCUS_CAT_MENU
    static const uint HAS_FOCUS_MENU                    = FOCUS_CAT_MENU + 0x1;
    static const uint HAS_FOCUS_PUZZLE_MENU             = FOCUS_CAT_MENU + 0x2;
    static const uint HAS_FOCUS_MENU_DIALOG             = FOCUS_CAT_MENU + 0x4;
    static const uint HAS_FOCUS_MENU_ALL                = FOCUS_CAT_MENU + HAS_FOCUS_MASK;

    // FOCUS_CAT_PLAYFIELD
    static const uint HAS_FOCUS_BOARD                   = FOCUS_CAT_PLAYFIELD + 0x1;
    static const uint HAS_FOCUS_PAUSE_MENU              = FOCUS_CAT_PLAYFIELD + 0x2;
    static const uint HAS_FOCUS_PLAYFIELD_ALL           = FOCUS_CAT_PLAYFIELD + HAS_FOCUS_MASK;
    
}
#endif // __INPUT_TYPES_H__