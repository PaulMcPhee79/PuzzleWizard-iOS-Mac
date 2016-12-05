#ifndef __SHIELD_MANAGER_H__
#define __SHIELD_MANAGER_H__

#include "cocos2d.h"
#include <Prop/CroppedProp.h>
#include <Puzzle/View/Playfield/Effects/Shield.h>
#include <Interfaces/IEventListener.h>
USING_NS_CC;

class ShieldManager : public CroppedProp, public IEventListener
{
public:
	ShieldManager(int category, int numColumns, int numRows, const CCRect& viewableRegion);
	virtual ~ShieldManager(void);

	Shield* shieldForKey(int key);
	Shield* addShield(int key, int tileIndex, const CCPoint& shieldOrigin);
	void removeShield(int key);
	void withdrawAll(bool playSound);

	virtual void onEvent(int evType, void* evData);
	virtual void advanceTime(float dt);

private:
    bool isIndexSurroundedBy(int index, int other, int range) const;
    int rowForIndex(int index) const;
    int columnForIndex(int index) const;
    int calcRowDelta(int index, int other) const;
    int calcColumnDelta(int index, int other) const;
	void updateFoci(void);

    int mNumColumns;
    int mNumRows;
	std::map<int, Shield*> mShields;
};
#endif // __SHIELD_MANAGER_H__
