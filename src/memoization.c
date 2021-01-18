/* memoization.c
 * Pregenerates all the arrays with the movements to make it easier to access later
 * Since it is only called once and it is already fast, there is no need to further optimize
 */

#include "../include/global.h"
#include "../include/memoization.h"

uint64_t kingMoves[64];
uint64_t knightMoves[64];

uint64_t whitePawnMoves[64];
uint64_t whitePawnCaptures[64];
uint64_t blackPawnMoves[64];
uint64_t blackPawnCaptures[64];

/* ...Int are the intersection tiles, since it isnt really necessary to calculate the intersection
 * against a piece that is on the edge because the possible sqrs will stay the same
 */
uint64_t rightMoves[64];        uint64_t rightMovesInt[64];
uint64_t leftMoves[64];         uint64_t leftMovesInt[64];
uint64_t upMoves[64];           uint64_t upMovesInt[64];
uint64_t downMoves[64];         uint64_t downMovesInt[64];
uint64_t uprightMoves[64];      uint64_t uprightMovesInt[64];
uint64_t downrightMoves[64];    uint64_t downrightMovesInt[64];
uint64_t upleftMoves[64];       uint64_t upleftMovesInt[64];
uint64_t downleftMoves[64];     uint64_t downleftMovesInt[64];

uint64_t straMoves[64];         uint64_t straMovesInt[64];
uint64_t diagMoves[64];         uint64_t diagMovesInt[64];

uint64_t vert[8];
uint64_t horiz[8];

uint64_t pawnLanes[8];
uint64_t wPassedPawn[64];
uint64_t bPassedPawn[64];

uint64_t king2[64];

uint64_t POW2[64];

inline static const int GETX(const int i)
{return i & 7;} //i % 8
inline static const int GETY(const int i)
{return i >> 3;}//i / 8

inline static int ISVALID(const int x, const int y)
{return x >= 0 && x < 8 && y >= 0 && y < 8;}

void initializePOW2(void)
{
    for (int i = 0; i < 64; ++i) POW2[i] = 1ULL << i;
}

void genWhitePawnMoves(void)
{
    int i = 8;

    for (; i < 16; ++i)
        whitePawnMoves[i] = (POW2[i + 8] | POW2[i]) << 8;

    for (; i < 56; ++i)
        whitePawnMoves[i] = POW2[i + 8];
}
void genBlackPawnMoves(void)
{
    int i = 55;

    for (; i > 47; --i)
        blackPawnMoves[i] = ((1ULL << (i - 8)) | (1ULL << i)) >> 8;

    for (; i > 7; --i)
        blackPawnMoves[i] = 1ULL << (i - 8);
}
void genWhitePawnCaptures(void)
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
void genBlackPawnCaptures(void)
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

void genUpMoves(void)
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
void genDownMoves(void)
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
void genRightMoves(void)
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
void genLeftMoves(void)
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

void genUpRightMoves(void)
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
void genUpLeftMoves(void)
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
void genDownLeftMoves(void)
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
void genDownRightMoves(void)
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

void genStraDiagMoves(void)
{
    for (int i = 0; i < 64; ++i)
    {
        straMoves[i] = upMoves[i] | downMoves[i] | rightMoves[i] | leftMoves[i];
        diagMoves[i] = uprightMoves[i] | upleftMoves[i] | downrightMoves[i] | downleftMoves[i];

        straMovesInt[i] = upMovesInt[i] | downMovesInt[i] | rightMovesInt[i] | leftMovesInt[i];
        diagMovesInt[i] = uprightMovesInt[i] | upleftMovesInt[i] | downrightMovesInt[i] | downleftMovesInt[i];
    }
}


//Generates the king moves, it does NOT generate the castling moves
void genKingMoves(void)
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

void genKnightMoves(void)
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

void genVert(void)
{
    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            vert[i] |= 1ULL << (8 * j + i);
        }
    }
}
void genHoriz(void)
{
    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            horiz[i] |= 1ULL << (8 * i + j);
        }
    }
}

void genPawnLanes(void)
{
    pawnLanes[0] = vert[1];
    pawnLanes[7] = vert[6];

    for (int i = 1; i < 7; ++i)
    {
        pawnLanes[i] = vert[i - 1] | vert[i + 1];
    }
}

void genIntersectionArrs(void)
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

void genWPassedPawn(void)
{
    for (int i = 0; i < 64; ++i)
    {
        if ((i & 7) == 0)
            wPassedPawn[i] = upMoves[i] | upMoves[i + 1];
        else if ((i & 7) == 7)
            wPassedPawn[i] = upMoves[i - 1] | upMoves[i];
        else
            wPassedPawn[i] = upMoves[i - 1] | upMoves[i] | upMoves[i + 1];
    }
}

void genBPassedPawn(void)
{
    for (int i = 0; i < 64; ++i)
    {
        if ((i & 7) == 0)
            bPassedPawn[i] = downMoves[i] | downMoves[i + 1];
        else if ((i & 7) == 7)
            bPassedPawn[i] = downMoves[i - 1] | downMoves[i];
        else
            bPassedPawn[i] = downMoves[i - 1] | downMoves[i] | downMoves[i + 1];
    }
}

void genKing2(void)
{
    for (int i = 0; i < 64; ++i)
    {
        uint64_t ks = kingMoves[i];
        while(ks)
        {
            int index = LSB_INDEX(ks);
            king2[i] |= kingMoves[index];
            REMOVE_LSB(ks);
        }
        king2[i] &= ~kingMoves[i];
        king2[i] ^= 1ULL << i;
    }
}

void initMemo(void)
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

    genIntersectionArrs();
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
    genWPassedPawn();
    genBPassedPawn();
    genKing2();
}