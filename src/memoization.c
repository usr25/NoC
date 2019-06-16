
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

void genWhitePawnMoves()
{   
    int i = 8;

    for (; i < 16; ++i)
        whitePawnMoves[i] = ((1ULL << (i + 8)) + (1ULL << i)) << 8;

    for (; i < 56; ++i)
        whitePawnMoves[i] = 1ULL << (i + 8);
}
void genBlackPawnMoves()
{   
    int i = 55;

    for (; i > 47; --i)
        blackPawnMoves[i] = ((1ULL << (i - 8)) + (1ULL << i)) >> 8;

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
            whitePawnCaptures[i] = ((1ULL << 2) + 1ULL) << (i + 7);
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
            blackPawnCaptures[i] = ((1ULL << 2) + 1ULL) << (i - 9);
    }
}

void initialize()
{
    /*
    generateKingMoves();
    generateQueenMoves();
    generateRookMoves();
    generateBishMoves();
    generateKnightMoves();
    */
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
