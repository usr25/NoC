#define VKING 99999
#define VQUEEN 900
#define VROOK 500
#define VBISH 325
#define VKNIGHT 300
#define VPAWN 100

#include "../include/global.h"
#include "../include/board.h"

#include <stdio.h>

int matrices(Board* b);
int allPiecesValue(Board* b);
int analyzePawnStructure(Board* b);
int multiply(int vals[64], uint64_t mask);

int castlingStructure();
int rookOnOpenFiles();
int knightCoordination(); //Two knights side by side are better
int bishopPair(); //Having a bishop pair but the opponent doesnt
int passedPawns();
int connectedPawns(); //Pawns lined up diagonally
int knightForks();
int materialHit(); //? 
int pins(); //?
int skewers();  //?

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
    10, 10, 10, 10,     10, 10, 10, 10,
    0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0,

    0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0,
    10, 10, 10, 10,     10, 10, 10, 10,
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
    -10, 10, 0, 0,     0, 0, 10, -10,
    -5, 5, 0, 15,     15, 0, 5, -5,

    0, 0, 0, 20,     20, 0, 0, 0,
    0, 3, 0, 10,     10, 0, 3, 0,
    5, 5, 5, 0,     0, 5, 5, 5,
    0, 0, 0, 0,     0, 0, 0, 0};

int bPawnMatrix[64] = 
   {0, 0, 0, 0,     0, 0, 0, 0,
    5, 5, 5, 0,     0, 5, 5, 5,
    0, 3, 0, 15,     15, 0, 3, 0,
    0, 0, 0, 20,     20, 0, 0, 0,

    -5, 5, 0, 10,     10, 0, 5, -5,
    -10, 10, 0, 0,     0, 0, 10, -10,
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

    val += multiply(wPawnMatrix, b->wPawns);
    val -= multiply(bPawnMatrix, b->bPawns);

    val += multiply(kingMatrix, b->wKing);
    val += multiply(queenMatrix, b->wQueen);
    val += multiply(rookMatrix, b->wRook);
    val += multiply(bishMatrix, b->wBish);
    val += multiply(knightMatrix, b->wKnight);

    val -= multiply(kingMatrix, b->bKing);
    val -= multiply(queenMatrix, b->bQueen);
    val -= multiply(rookMatrix, b->bRook);
    val -= multiply(bishMatrix, b->bBish);
    val -= multiply(knightMatrix, b->bKnight);

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

int multiply(int vals[64], uint64_t mask)
{
    int val = 0;
    int index = 63;

    do{
        if (mask & 1) val += vals[index];
        index--;
    }while(mask >>= 1);

    return val;
}

int testMatrix[64] =
    {0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0,

    0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 0, 0,
    0, 0, 0, 0,     0, 0, 2, -1};

int testE(uint64_t val)
{
    return multiply(testMatrix, val);
}