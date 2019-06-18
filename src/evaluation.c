#define VKING 99999
#define VQUEEN 900
#define VROOK 500
#define VBISH 325
#define VKNIGHT 300
#define VPAWN 100

#define VLASTRANK 300

#define SEVENTH_RANK_MASK 0xff000000000000
#define SECOND_RANK_MASK 0xff00

#include "../include/board.h"
#include "../include/global.h"

int matrices(Board* b);
int allPiecesValue(Board* b);
int analyzePawnStructure(Board* b);
int multiply(int vals[64], unsigned long long mask, int comparison);

int kingMatrix[64] = 
   {0, 8, 8, 0,     0, 4, 10, 5,
    0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, -4, -5,   -5, -5, -4, 0,
    0, 0, -5, -10,  -10, -5, 0, 0,

    0, 0, -5, -10,  -10, -5, 0, 0,
    0, 0, -4, -5,   -5, -4, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0,
    0, 8, 8, 0,     0, 4, 10, 5};

int queenMatrix[64] = 
   {0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0,

    0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0};

int rookMatrix[64] = 
   {0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0,

    0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0};

int bishMatrix[64] = 
   {5, 5, 5, 9,     5, 5, 5, 9,     
    9, 9, 11, 5,    9, 9, 11, 5,
    11, 13, 9, 5,   11, 13, 9, 5,
    15, 11, 9, 5,   15, 11, 9, 5,
    
    5, 5, 5, 9,     5, 5, 5, 9,     
    9, 9, 11, 5,    9, 9, 11, 5,
    11, 13, 9, 5,   11, 13, 9, 5,
    15, 11, 9, 5,   15, 11, 9, 5};

int knightMatrix[64] = 
   {-50, -10, -10, -10, -10, -10, -10, -50,
    -10, 0, 0, 0,       0, 0, 0, -10,
    -10, 0, 10, 15,     15, 10, 0, -10,
    -10, 0, 15, 20,     20, 15, 0, -10,

    -10, 0, 15, 20,     20, 15, 0, -10,
    -10, 0, 10, 15,     15, 10, 0, -10,
    -10, 0, 0, 0,       0, 0, 0, -10,
    -50, 0, 0, 0,       0, 0, 0, -50};

int wPawnMatrix[64] = 
   {0, 0, 0, 0,     0, 0, 0, 0,
    175, 200, 200, 200,     200, 200, 200, 175,
    0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 15,     15, 0, 0, 0,

    0, 0, 0, 20,     20, 0, 0, 0,
    0, 0, 0, 10,     10, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0};

int bPawnMatrix[64] = 
   {0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0,

    0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0,
    175, 200, 200, 200,     200, 200, 200, 175,
    0, 0, 0, 0,     0, 0, 0, 0};

int evaluate(Board b)
{
    int val = 0;

    val += allPiecesValue(&b);
    val += matrices(&b);

    //checks

    return val;
}

int matrices(Board* b)
{
    int val = 0;

    val += multiply(wPawnMatrix, b->wPawns, 1);
    val += multiply(bPawnMatrix, b->bPawns, -1);

    val += multiply(kingMatrix, b->wKing, 1);
    val += multiply(queenMatrix, b->wQueen, 1);
    val += multiply(rookMatrix, b->wRook, 1);
    val += multiply(bishMatrix, b->wBish, 1);
    val += multiply(knightMatrix, b->wKnight, 1);

    val += multiply(kingMatrix, b->bKing, -1);
    val += multiply(queenMatrix, b->bQueen, -1);
    val += multiply(rookMatrix, b->bRook, -1);
    val += multiply(bishMatrix, b->bBish, -1);
    val += multiply(knightMatrix, b->bKnight, -1);

    return val;
}

int allPiecesValue(Board* bo)
{
    int k = VKING *     (numWKing(bo->numPieces)     - numBKing(bo->numPieces));
    int q = VQUEEN *    (numWQueen(bo->numPieces)    - numBQueen(bo->numPieces));
    int r = VROOK *     (numWRook(bo->numPieces)     - numBRook(bo->numPieces));
    int b = VBISH *     (numWBish(bo->numPieces)     - numBBish(bo->numPieces));
    int n = VKNIGHT *   (numWKnight(bo->numPieces)   - numBKnight(bo->numPieces));
    int p = VPAWN *     (numWPawn(bo->numPieces)     - numBPawn(bo->numPieces));

    return k + q + r + b + n + p;
}

int multiply(int vals[64], unsigned long long mask, int comparison)
{
    int val = 0;
    int index = 0;
    do
    {
        if (mask & 1) val += comparison * vals[index];
        ++index;
    }while(mask >>= 1);

    return val;
}