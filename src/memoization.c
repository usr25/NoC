unsigned long long kingMoves[64];
unsigned long long queenMoves[64];
unsigned long long rookMoves[64];
unsigned long long bishMoves[64];
unsigned long long knightMoves[64];
//It can be made to be 64 - 8 but the impact in memory is tiny
unsigned long long whitePawnMoves[64];
unsigned long long whitePawnCaptures[64];
unsigned long long blackPawnMoves[64];
unsigned long long blackPawnCaptures[64];

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
    int x, y, i, j;
    unsigned long long pos;
    for (i = 0; i < 64; ++i)
    {
        pos = 0;
        x = GETX(i);
        y = GETY(i);

        for (int x0 = 0; x0 < 8; ++x0)
        {
            if (x0 != x)
                pos |= 1ULL << (y * 8 + x0);
        }
        for (int y0 = 0; y0 < 8; ++y0)
        {
            if (y0 != y)
                pos |= 1ULL << (y0 * 8 + x);
        }

        for (j = 1; x + j < 8 && y + j < 8; ++j)
            pos |= 1ULL << ((y + j) * 8 + x + j);

        for (j = 1; x - j > -1 && y + j < 8; ++j)
            pos |= 1ULL << ((y + j) * 8 + x - j);
        
        for (j = 1; x + j < 8 && y - j > -1; ++j)
            pos |= 1ULL << ((y - j) * 8 + x + j);
        
        for (j = 1; x - j > -1 && y - j > -1; ++j)
            pos |= 1ULL << ((y - j) * 8 + x - j);

        queenMoves[i] = pos;
    }
}


void genRookMoves()
{
    int x, y, i;
    unsigned long long pos;
    for (i = 0; i < 64; ++i)
    {
        pos = 0;
        x = GETX(i);
        y = GETY(i);

        for (int x0 = 0; x0 < 8; ++x0)
        {
            if (x0 != x)
                pos |= 1ULL << (y * 8 + x0);
        }
        for (int y0 = 0; y0 < 8; ++y0)
        {
            if (y0 != y)
                pos |= 1ULL << (y0 * 8 + x);
        }
        rookMoves[i] = pos;
    }
}
void genBishMoves()
{
    int x, y, i, j;
    unsigned long long pos;
    for (i = 0; i < 64; ++i)
    {
        pos = 0;
        x = GETX(i);
        y = GETY(i);

        for (j = 1; x + j < 8 && y + j < 8; ++j)
            pos |= 1ULL << ((y + j) * 8 + x + j);

        for (j = 1; x - j > -1 && y + j < 8; ++j)
            pos |= 1ULL << ((y + j) * 8 + x - j);
        
        for (j = 1; x + j < 8 && y - j > -1; ++j)
            pos |= 1ULL << ((y - j) * 8 + x + j);
        
        for (j = 1; x - j > -1 && y - j > -1; ++j)
            pos |= 1ULL << ((y - j) * 8 + x - j);
        
        bishMoves[i] = pos;
    }
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
    genKingMoves();
    genQueenMoves();
    genRookMoves();
    genBishMoves();
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

unsigned long long getWhitePawnMoves(unsigned int index)
{return whitePawnMoves[index];}
unsigned long long getWhitePawnCaptures(unsigned int index)
{return whitePawnCaptures[index];}
unsigned long long getBlackPawnMoves(unsigned int index)
{return blackPawnMoves[index];}
unsigned long long getBlackPawnCaptures(unsigned int index)
{return blackPawnCaptures[index];}
