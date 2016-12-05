#ifndef __TOUCH_TYPES_H__
#define __TOUCH_TYPES_H__

#include <Interfaces/IEventListener.h>

namespace CMTouches
{
	enum TouchCategory
	{
		TC_NONE = 0,
		TC_PLAYFIELD,
		TC_MENU,
		TC_PAUSE,
		TC_DIALOGS,
		TC_COUNT
	};

	struct TouchNotice
	{
		IEventListener* focus;
		CCPoint pos;
		CCPoint prevPos;

		void reset(void) { focus = NULL; }
		bool hasFocus(void) const { return focus != NULL; }
		void retainFocus(IEventListener* listener) { if (focus == NULL) { focus = listener; } }
		void releaseFocus(IEventListener* listener) { if (listener == focus) { focus = NULL; } }
		float getTouchDeltaX(void) const { return pos.x - prevPos.x; }
		float getTouchDeltaY(void) const { return pos.y - prevPos.y; }
	};
}

#endif // __TOUCH_TYPES_H__
