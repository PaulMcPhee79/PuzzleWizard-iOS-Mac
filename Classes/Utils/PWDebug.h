#ifndef __PW_DEBUG_H__
#define __PW_DEBUG_H__

class PWDebug
{
public:
    static int puzzleCount;
    static int puzzleBoardCount;
    static int tilePieceCount;
    static int tileCount;
    static int tileDecorationCount;
    static int humanPlayerPieceCount;
    static int mirrorPlayerPieceCount;
    static int playerCount;
    static int shieldCount;
    
private:
    PWDebug();
    PWDebug(const PWDebug& other);
	PWDebug& operator=(const PWDebug& rhs);
};
#endif // __PW_DEBUG_H__
