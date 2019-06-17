unsigned long long kingMoves[64];
unsigned long long queenMoves[64];
unsigned long long rookMoves[64];
unsigned long long bishMoves[64];
unsigned long long knightMoves[64];

unsigned long long rightMoves[64];
unsigned long long leftMoves[64];
unsigned long long upMoves[64];
unsigned long long downMoves[64];

unsigned long long uprightMoves[64];
unsigned long long downrightMoves[64];
unsigned long long upleftMoves[64];
unsigned long long downleftMoves[64];
//It can be made to be 64 - 8 but the impact in memory is tiny
unsigned long long whitePawnMoves[64];
unsigned long long whitePawnCaptures[64];
unsigned long long blackPawnMoves[64];
unsigned long long blackPawnCaptures[64];

#include <stdio.h>

static inline int GETX(int i)
{return i % 8;}
static inline int GETY(int i)
{return i / 8;}

static inline int ISVALID(int x, int y)
{return x >= 0 && x < 8 && y >= 0 && y < 8;}

void genWhitePawnMoves()
{   
    int i = 8;

    for (; i < 16; ++i)
        whitePawnMoves[i] = ((1ULL << (i + 8)) | (1ULL << i)) << 8;

    for (; i < 56; ++i)
        whitePawnMoves[i] = 1ULL << (i + 8);
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
    int i = 8;
    for (; i < 56; ++i)
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
    int i = 55;
    for (; i > 7; --i)
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
    unsigned long long pos;
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
    unsigned long long pos;
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
    unsigned long long pos;
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
    unsigned long long pos;
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
    unsigned long long pos;
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
    unsigned long long pos;
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
    unsigned long long pos;
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
    unsigned long long pos;
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

//Generates the king moves, it does NOT implement castling
void genKingMoves()
{
    int x, y, i;
    unsigned long long pos;
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
void genQueenMoves()
{
    for (int i = 0; i < 64; ++i)
        queenMoves[i] = rookMoves[i] | bishMoves[i];
}


void genRookMoves()
{
    for (int i = 0; i < 64; ++i)
        rookMoves[i] = rightMoves[i] | leftMoves[i] | upMoves[i] | downMoves[i];
}
void genBishMoves()
{
    for (int i = 0; i < 64; ++i)
        bishMoves[i] = uprightMoves[i] | upleftMoves[i] | downrightMoves[i] | downleftMoves[i]; 
}
void genKnightMoves()
{
    int x, y, i;
    unsigned long long pos;
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

void initialize()
{
    genRightMoves();
    genLeftMoves();
    genUpMoves();
    genDownMoves();
    
    genUpRightMoves();
    genUpLeftMoves();
    genDownRightMoves();
    genDownLeftMoves();

    genKingMoves();
    genRookMoves();
    genBishMoves();
    genQueenMoves();
    genKnightMoves();

    genWhitePawnMoves();
    genWhitePawnCaptures();
    genBlackPawnMoves();
    genBlackPawnCaptures();
}

unsigned long long getKingMoves(unsigned int index)
{return kingMoves[index];}
unsigned long long getQueenMoves(unsigned int index)
{return queenMoves[index];}
unsigned long long getRookMoves(unsigned int index)
{return rookMoves[index];}
unsigned long long getBishMoves(unsigned int index)
{return bishMoves[index];}
unsigned long long getKnightMoves(unsigned int index)
{return knightMoves[index];}

unsigned long long getUpMoves(unsigned int index)
{return upMoves[index];}
unsigned long long getDownMoves(unsigned int index)
{return downMoves[index];}
unsigned long long getRightMoves(unsigned int index)
{return rightMoves[index];}
unsigned long long getLeftMoves(unsigned int index)
{return leftMoves[index];}

unsigned long long getUpRightMoves(unsigned int index)
{return uprightMoves[index];}
unsigned long long getUpLeftMoves(unsigned int index)
{return upleftMoves[index];}
unsigned long long getDownRightMoves(unsigned int index)
{return downrightMoves[index];}
unsigned long long getDownLeftMoves(unsigned int index)
{return downleftMoves[index];}

unsigned long long getWhitePawnMoves(unsigned int index)
{return whitePawnMoves[index];}
unsigned long long getWhitePawnCaptures(unsigned int index)
{return whitePawnCaptures[index];}
unsigned long long getBlackPawnMoves(unsigned int index)
{return blackPawnMoves[index];}
unsigned long long getBlackPawnCaptures(unsigned int index)
{return blackPawnCaptures[index];}
