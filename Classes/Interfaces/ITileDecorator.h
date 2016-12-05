#ifndef __ITILE_DECORATOR_H__
#define __ITILE_DECORATOR_H__

class TilePiece;

class ITileDecorator
{
public:
	virtual ~ITileDecorator(void) { }
	virtual GLubyte decoratorValueForKey(uint key) = 0;
	virtual void decorationDidChange(TilePiece* tilePiece) { };
};
#endif // __ITILE_DECORATOR_H__
