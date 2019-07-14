/* memoization.c
 * Pregenerates all the arrays with the movements to make it easier to access later
 */

#include "../include/global.h"
#include "../include/memoization.h"

static inline int GETX(int i)
{return i & 7;} //i % 8
static inline int GETY(int i)
{return i >> 3;}//i / 8

static inline int ISVALID(int x, int y)
{return x >= 0 && x < 8 && y >= 0 && y < 8;}

void initializePOW2()
{
    for (int i = 0; i < 64; ++i) POW2[i] = 1ULL << i;
}

void genWhitePawnMoves()
{   
    int i = 8;

    for (; i < 16; ++i)
        whitePawnMoves[i] = (POW2[i + 8] | POW2[i]) << 8;

    for (; i < 56; ++i)
        whitePawnMoves[i] = POW2[i + 8];
}
void genBlackPawnMoves()
{   
    int i = 55;

    for (; i > 47; --i)
        blackPawnMoves[i] = ((1ULL << (i - 8)) | (1ULL << i)) >> 8;

    for (; i > 7; --i)
        blackPawnMoves[i] = 1ULL << (i - 8);
}
void genWhitePawnCaptures()
{
    for (int i = 0; i < 56; ++i)
    {
        if (i % 8 == 0)
            whitePawnCaptures[i] = 1ULL << (i + 9);
        else if (i % 8 == 7)
            whitePawnCaptures[i] = 1ULL << (i + 7);
        else
            whitePawnCaptures[i] = ((1ULL << 2) | 1ULL) << (i + 7);
    }
}
void genBlackPawnCaptures()
{
    for (int i = 63; i > 7; --i)
    {
        if (i % 8 == 0)
            blackPawnCaptures[i] = 1ULL << (i - 7);
        else if (i % 8 == 7)
            blackPawnCaptures[i] = 1ULL << (i - 9);
        else
            blackPawnCaptures[i] = ((1ULL << 2) | 1ULL) << (i - 9);
    }
}

void genUpMoves()
{
    int x, y;
    uint64_t pos;
    for (int i = 0; i < 64; ++i)
    {
        pos = 0;
        x = GETX(i);
        y = GETY(i);

        for (int y0 = y + 1; y0 < 8; ++y0)
            pos |= 1ULL << (y0 * 8 + x);

        upMoves[i] = pos;
    }
}
void genDownMoves()
{
    int x, y;
    uint64_t pos;
    for (int i = 0; i < 64; ++i)
    {
        pos = 0;
        x = GETX(i);
        y = GETY(i);

        for (int y0 = y - 1; y0 > -1; --y0)
            pos |= 1ULL << (y0 * 8 + x);

        downMoves[i] = pos;
    }
}
void genRightMoves()
{
    int x, y;
    uint64_t pos;
    for (int i = 0; i < 64; ++i)
    {
        pos = 0;
        x = GETX(i);
        y = GETY(i);

        for (int x0 = x - 1; x0 > -1; --x0)
            pos |= 1ULL << (y * 8 + x0);

        rightMoves[i] = pos;
    }
}
void genLeftMoves()
{
    int x, y;
    uint64_t pos;
    for (int i = 0; i < 64; ++i)
    {
        pos = 0;
        x = GETX(i);
        y = GETY(i);

        for (int x0 = x + 1; x0 < 8; ++x0)
            pos |= 1ULL << (y * 8 + x0);

        leftMoves[i] = pos;
    }
}

void genUpRightMoves()
{
    int x, y;
    uint64_t pos;
    for (int i = 0; i < 64; ++i)
    {
        pos = 0;
        x = GETX(i);
        y = GETY(i);

        for (int j = 1; ISVALID(y + j, x - j); ++j)
            pos |= 1ULL << ((y + j) * 8 + x - j);

        uprightMoves[i] = pos;
    }
}
void genUpLeftMoves()
{
    int x, y;
    uint64_t pos;
    for (int i = 0; i < 64; ++i)
    {
        pos = 0;
        x = GETX(i);
        y = GETY(i);

        for (int j = 1; ISVALID(y + j, x + j); ++j)
            pos |= 1ULL << ((y + j) * 8 + x + j);

        upleftMoves[i] = pos;
    }
}
void genDownLeftMoves()
{
    int x, y;
    uint64_t pos;
    for (int i = 0; i < 64; ++i)
    {
        pos = 0;
        x = GETX(i);
        y = GETY(i);

        for (int j = 1; ISVALID(y - j, x + j); ++j)
            pos |= 1ULL << ((y - j) * 8 + x + j);

        downleftMoves[i] = pos;
    }
}
void genDownRightMoves()
{
    int x, y;
    uint64_t pos;
    for (int i = 0; i < 64; ++i)
    {
        pos = 0;
        x = GETX(i);
        y = GETY(i);

        for (int j = 1; ISVALID(y - j, x - j); ++j)
            pos |= 1ULL << ((y - j) * 8 + x - j);

        downrightMoves[i] = pos;
    }
}

void genStraDiagMoves()
{
    for (int i = 0; i < 64; ++i)
    {
        straMoves[i] = upMoves[i] | downMoves[i] | rightMoves[i] | leftMoves[i];
        diagMoves[i] = uprightMoves[i] | upleftMoves[i] | downrightMoves[i] | downleftMoves[i];
    }
}


//Generates the king moves, it does NOT implement castling
void genKingMoves()
{
    int x, y, i;
    uint64_t pos;
    for (i = 0; i < 64; ++i)
    {
        pos = 0;
        x = GETX(i);
        y = GETY(i);

        if (ISVALID(x + 1, y + 1)) pos |= 1ULL << ((y + 1) * 8 + x + 1);
        if (ISVALID(x - 1, y + 1)) pos |= 1ULL << ((y + 1) * 8 + x - 1);
        if (ISVALID(x + 1, y - 1)) pos |= 1ULL << ((y - 1) * 8 + x + 1);
        if (ISVALID(x - 1, y - 1)) pos |= 1ULL << ((y - 1) * 8 + x - 1);
        if (ISVALID(x + 1, y)) pos |= 1ULL << (y * 8 + x + 1);
        if (ISVALID(x - 1, y)) pos |= 1ULL << (y * 8 + x - 1);
        if (ISVALID(x, y + 1)) pos |= 1ULL << ((y + 1) * 8 + x);
        if (ISVALID(x, y - 1)) pos |= 1ULL << ((y - 1) * 8 + x);

        kingMoves[i] = pos;
    }
}

void genKnightMoves()
{
    int x, y, i;
    uint64_t pos;
    for (i = 0; i < 64; ++i)
    {
        pos = 0;
        x = GETX(i);
        y = GETY(i);

        if (ISVALID(x + 1, y + 2)) pos |= 1ULL << ((y + 2) * 8 + x + 1);
        if (ISVALID(x - 1, y + 2)) pos |= 1ULL << ((y + 2) * 8 + x - 1);
        if (ISVALID(x + 1, y - 2)) pos |= 1ULL << ((y - 2) * 8 + x + 1);
        if (ISVALID(x - 1, y - 2)) pos |= 1ULL << ((y - 2) * 8 + x - 1);
        if (ISVALID(x + 2, y + 1)) pos |= 1ULL << ((y + 1) * 8 + x + 2);
        if (ISVALID(x - 2, y + 1)) pos |= 1ULL << ((y + 1) * 8 + x - 2);
        if (ISVALID(x + 2, y - 1)) pos |= 1ULL << ((y - 1) * 8 + x + 2);
        if (ISVALID(x - 2, y - 1)) pos |= 1ULL << ((y - 1) * 8 + x - 2);

        knightMoves[i] |= pos;
    }
}

void genVert()
{
    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            vert[i] |= 1ULL << (8 * j + i);
        }
    }
}
void genHoriz()
{
    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            horiz[i] |= 1ULL << (8 * i + j);
        }
    }
}

void genPawnLanes()
{
    pawnLanes[0] = vert[1];
    pawnLanes[7] = vert[6];

    for (int i = 1; i < 7; ++i)
    {
        pawnLanes[i] = vert[i - 1] | vert[i + 1];
    }
}

void genIntersectionArrs()
{
    for (int i = 0; i < 64; ++i)
    {
        rightMovesInt[i] = rightMoves[i] & 0x7e7e7e7e7e7e7e7e;
        leftMovesInt[i] = leftMoves[i] & 0x7e7e7e7e7e7e7e7e;
        upMovesInt[i] = upMoves[i] & 0xffffffffffff00;
        downMovesInt[i] = downMoves[i] & 0xffffffffffff00;

        uprightMovesInt[i] = uprightMoves[i] & 0x7e7e7e7e7e7e00;
        upleftMovesInt[i] = upleftMoves[i] & 0x7e7e7e7e7e7e00;
        downrightMovesInt[i] = downrightMoves[i] & 0x7e7e7e7e7e7e00;
        downleftMovesInt[i] = downleftMoves[i] & 0x7e7e7e7e7e7e00;
    }
}

void initialize()
{
    initializePOW2();

    genRightMoves();
    genLeftMoves();
    genUpMoves();
    genDownMoves();
    
    genUpRightMoves();
    genUpLeftMoves();
    genDownRightMoves();
    genDownLeftMoves();

    genStraDiagMoves();

    genKingMoves();
    genKnightMoves();

    genWhitePawnMoves();
    genWhitePawnCaptures();
    genBlackPawnMoves();
    genBlackPawnCaptures();

    genVert();
    genHoriz();

    genPawnLanes();

    genIntersectionArrs();
}