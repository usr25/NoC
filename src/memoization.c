/* memoization.c
 * Pregenerates all the arrays with the movements to make it easier to access later
 */

#include "../include/global.h"
#include "../include/memoization.h"

uint64_t kingMoves[64];
uint64_t knightMoves[64];

uint64_t rightMoves[64];
uint64_t leftMoves[64];
uint64_t upMoves[64];
uint64_t downMoves[64];

uint64_t uprightMoves[64];
uint64_t downrightMoves[64];
uint64_t upleftMoves[64];
uint64_t downleftMoves[64];

uint64_t straMoves[64];
uint64_t diagMoves[64];

//It can be made to be 64 - 8 but the impact in memory is tiny and the runtime performance would worsen
uint64_t whitePawnMoves[64];
uint64_t whitePawnCaptures[64];
uint64_t blackPawnMoves[64];
uint64_t blackPawnCaptures[64];

uint64_t vert[8];
uint64_t horiz[8];
uint64_t mainDiag[15];
uint64_t sndDiag[15];

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
    //genMainDiag();
    //genSndDiag();
}

uint64_t getKingMoves(int index)
{return kingMoves[index];}
uint64_t getKnightMoves(int index)
{return knightMoves[index];}

uint64_t getUpMoves(int index)
{return upMoves[index];}
uint64_t getDownMoves(int index)
{return downMoves[index];}
uint64_t getRightMoves(int index)
{return rightMoves[index];}
uint64_t getLeftMoves(int index)
{return leftMoves[index];}

uint64_t getUpRightMoves(int index)
{return uprightMoves[index];}
uint64_t getUpLeftMoves(int index)
{return upleftMoves[index];}
uint64_t getDownRightMoves(int index)
{return downrightMoves[index];}
uint64_t getDownLeftMoves(int index)
{return downleftMoves[index];}

inline uint64_t getStraMoves(int index)
{return straMoves[index];}
inline uint64_t getDiagMoves(int index)
{return diagMoves[index];}

uint64_t getWhitePawnMoves(int index)
{return whitePawnMoves[index];}
uint64_t getWhitePawnCaptures(int index)
{return whitePawnCaptures[index];}
uint64_t getBlackPawnMoves(int index)
{return blackPawnMoves[index];}
uint64_t getBlackPawnCaptures(int index)
{return blackPawnCaptures[index];}

uint64_t getVert(int index)
{return vert[index];}
uint64_t getHoriz(int index)
{return horiz[index];}
uint64_t getMainDiag(int index)
{return mainDiag[index];}
uint64_t getSndDiag(int index)
{return sndDiag[index];}