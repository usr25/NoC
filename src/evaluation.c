/* evaluation.c
 * Returns the evaluation of a given position
 * eval < 0 -> It is good for black
 * eval > 0 -> It is good for white
 */

#define VQUEEN 950
#define VROOK 520
#define VBISH 335
#define VKNIGHT 310
#define VPAWN 100

//All the constants that begin with N_ are negative, so instead of C * (w - b) the operation is C * (b - w)
#define CONNECTED_ROOKS 35 //Bonus for having connected rooks
#define TWO_BISH 20 //Bonus for having the bishop pair
#define ROOK_OPEN_FILE 25 //Bonus for a rook on an open file (No same color pawns)
#define BISHOP_MOBILITY 2 //Bonus for sqares available to the bish
#define N_DOUBLED_PAWNS -40 //Penalization for doubled pawns (proportional to the pawns in line - 1)
#define PAWN_CHAIN 20 //Bonus for making a pawn chain
#define PAWN_PROTECTION 15 //Bonus for Bish / Knight protected by pawn
#define ATTACKED_BY_PAWN 20 //Bonus if a pawn can easily attack a piece
#define E_ADVANCED_KING 3 //Endgame, bonus for advanced king
#define E_ADVANCED_PAWN 6 //Endgame, bonus for advanced pawns
#define N_PIECE_SLOW_DEV -10 //-25 //Penalization for keeping the pieces in the back-rank
#define STABLE_KING 25 //Bonus for king in e1/8 or castled
#define PASSED_PAWN 30 //Bonus for passed pawns
#define N_ISOLATED_PAWN -20 //Penalization for isolated pawns
//TODO:
#define SAFE_KING 20 //Bonus for pawns surrounding the king
#define CLEAN_PAWN 20 //Bonus for a pawn that doesnt have any pieces in front
#define N_BACKWR_PAWN 20 //Penalization for a backwards pawn


#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/memoization.h"
#include "../include/io.h"

#include <stdio.h>

int matricesBeg(const Board b);
int matricesEnd(const Board b);
int allPiecesValue(const Board b);
int analyzePawnStructure(const Board b);
int pieceActivity(const Board b);
int endgameAnalysis(const Board b);
int pieceDevelopment(const Board b);
int multiply(int vals[64], uint64_t mask);
int pawns(const Board b);

int hasMatingMat(const Board b, int color);

int rookOnOpenFile(uint64_t wr, uint64_t wp, uint64_t br, uint64_t bp);
int connectedRooks(uint64_t wh, uint64_t bl, uint64_t all);
int twoBishops(uint64_t wh, uint64_t bl);
int bishopMobility(uint64_t wh, uint64_t bl, uint64_t all);

//TO implement:
int knightCoordination(); //Two knights side by side are better
int passedPawns();
int knightForks(); //Probably not necessary
int materialHit(); //? 
int pins(); //?
int skewers();  //?

int bishMatrix[64];
int knightMatrix[64];
int wPawnMatrix[64];
int ewPawnMatrix[64];
int bPawnMatrix[64];
int ebPawnMatrix[64];

inline int hasMatingMat(const Board b, int color)
{
    return b.piece[color][PAWN] || b.piece[color][ROOK] || b.piece[color][QUEEN] || POPCOUNT(b.piece[color][BISH] | b.piece[color][KNIGHT]) > 1;
}

int insuffMat(const Board b)
{
    switch(POPCOUNT(b.color[WHITE]))
    {
        case 1: 
            
            switch(POPCOUNT(b.color[BLACK]))
            {
                case 1:
                    return 1;
                case 2:
                    return b.piece[BLACK][BISH] || b.piece[BLACK][KNIGHT];
            }

            return 0;

        case 2:

            switch(POPCOUNT(b.color[BLACK]))
            {
                case 1:
                    return b.piece[WHITE][BISH] || b.piece[WHITE][KNIGHT];
                case 2:
                    return 
                              ((ODD_TILES & b.piece[WHITE][BISH]) && (ODD_TILES & b.piece[BLACK][BISH])
                            ||(EVEN_TILES & b.piece[WHITE][BISH]) && (EVEN_TILES & b.piece[BLACK][BISH]));
            }
            return 0;
    }

    return 0;
}

//It is considered an endgame if there are 7 pieces or less in each side, (< 10 taking into account the kings)
int eval(const Board b)
{
    if (POPCOUNT(b.allPieces ^ (b.piece[WHITE][PAWN] | b.piece[BLACK][PAWN])) < 10)
    {
        return   allPiecesValue(b)
                +matricesEnd(b)
                +endgameAnalysis(b)
                +pieceActivity(b)
                +pawns(b);
    }

    return   allPiecesValue(b)
            +pieceDevelopment(b)
            +matricesBeg(b)
            +pieceActivity(b)
            +pawns(b);
}

uint64_t pawnAttacks(uint64_t pawns, int color)
{
    uint64_t res = 0ULL;
    if (color)
    {
        while(pawns)
        {
            res |= getWhitePawnCaptures(LSB_INDEX(pawns));
            REMOVE_LSB(pawns);
        }
    }
    else
    {
        while(pawns)
        {
            res |= getBlackPawnCaptures(LSB_INDEX(pawns));
            REMOVE_LSB(pawns);
        }
    }
    return res;
}

//TODO?: Discriminate so that it is not necessary to develop both sides as to castle faster
//TODO: Disable piece_slow_dev if all the pieces have already moved or if it is the middlegame
inline int pieceDevelopment(const Board b)
{
    return 
         N_PIECE_SLOW_DEV * (POPCOUNT(0x66ULL & b.color[WHITE]) - POPCOUNT(0x6600000000000000ULL & b.color[BLACK]))
        +STABLE_KING * (((0x6b & b.piece[WHITE][KING]) != 0) - ((0x6b00000000000000 & b.piece[BLACK][KING]) != 0));
}

inline int pawns(const Board b)
{
    uint64_t wPawn = b.piece[WHITE][PAWN];
    uint64_t bPawn = b.piece[BLACK][PAWN];
    uint64_t attW = pawnAttacks(wPawn, WHITE);
    uint64_t attB = pawnAttacks(bPawn, BLACK);

    int isolW = 0, isolB = 0, passW = 0, passB = 0;
    int lsb;
    uint64_t tempW = wPawn, tempB = bPawn;
    while(tempW)
    {
        lsb = LSB_INDEX(tempW);
        isolW += (getPawnLanes(lsb) & wPawn) != 0;
        passW += (getWPassedPawn(lsb) & bPawn) == 0;
        REMOVE_LSB(tempW);
    }
    while(tempB)
    {
        lsb = LSB_INDEX(tempB);
        isolB += (getPawnLanes(lsb) & bPawn) != 0;
        passB += (getBPassedPawn(lsb) & wPawn) == 0;
        REMOVE_LSB(tempB);
    }

    return   PAWN_CHAIN * (POPCOUNT(wPawn & attW) - POPCOUNT(bPawn & attB))
            +PAWN_PROTECTION * (POPCOUNT(attW & (b.piece[WHITE][BISH] | b.piece[WHITE][KNIGHT])) - POPCOUNT(attB & (b.piece[BLACK][BISH] | b.piece[BLACK][KNIGHT])))
            +N_DOUBLED_PAWNS * (POPCOUNT(wPawn & (wPawn * 0x10100)) - POPCOUNT(bPawn & (bPawn >> 8 | bPawn >> 16)))
            +ATTACKED_BY_PAWN * (POPCOUNT((attW * 0x101) & b.color[BLACK]) - POPCOUNT((attB | (attB >> 8)) & b.color[WHITE]))
            +N_ISOLATED_PAWN * (isolW - isolB) 
            +PASSED_PAWN * (passW - passB);
}

inline int endgameAnalysis(const Board b)
{
    int wAvg = b.piece[WHITE][PAWN] ? 
    (LSB_INDEX(b.piece[WHITE][PAWN]) + MSB_INDEX(b.piece[WHITE][PAWN])) >> 1 
    : 0;
    int bAvg = b.piece[BLACK][PAWN] ? 
    (LSB_INDEX(b.piece[BLACK][PAWN]) + MSB_INDEX(b.piece[BLACK][PAWN])) >> 1 
    : 64;
    
    return 
        E_ADVANCED_KING * ((LSB_INDEX(b.piece[WHITE][KING]) >> 3) - ((63 - LSB_INDEX(b.piece[BLACK][KING])) >> 3))
        +E_ADVANCED_PAWN * ((wAvg >> 3) - ((64 - bAvg) >> 3));
}

inline int pieceActivity(const Board b)
{
    return   connectedRooks(b.piece[WHITE][ROOK], b.piece[BLACK][ROOK], b.allPieces ^ b.piece[WHITE][QUEEN] ^ b.piece[BLACK][QUEEN])
            +rookOnOpenFile(b.piece[WHITE][ROOK], b.piece[WHITE][PAWN], b.piece[BLACK][ROOK], b.piece[BLACK][PAWN])
            +twoBishops(b.piece[WHITE][BISH], b.piece[BLACK][BISH])
            +bishopMobility(b.piece[WHITE][BISH], b.piece[BLACK][BISH], b.allPieces);
}

inline int matricesBeg(const Board b)
{
    return   multiply(wPawnMatrix, b.piece[WHITE][PAWN]) - multiply(bPawnMatrix, b.piece[BLACK][PAWN])
            +multiply(bishMatrix, b.piece[WHITE][BISH]) - multiply(bishMatrix, b.piece[BLACK][BISH])
            +multiply(knightMatrix, b.piece[WHITE][KNIGHT]) - multiply(knightMatrix, b.piece[BLACK][KNIGHT]);
}
inline int matricesEnd(const Board b)
{
    return   multiply(ewPawnMatrix, b.piece[WHITE][PAWN]) - multiply(ebPawnMatrix, b.piece[BLACK][PAWN]);
}

inline int bishopMobility(uint64_t wh, uint64_t bl, uint64_t all)
{
    return BISHOP_MOBILITY * ((POPCOUNT(diagonal(MSB_INDEX(wh), all)) + POPCOUNT(diagonal(LSB_INDEX(wh), all))) 
                             -(POPCOUNT(diagonal(MSB_INDEX(bl), all)) + POPCOUNT(diagonal(LSB_INDEX(bl), all))));
}

inline int connectedRooks(uint64_t wh, uint64_t bl, uint64_t all)
{
    int res = 0;

    if (wh & (wh - 1))
    {
        int hi = MSB_INDEX(wh), lo = LSB_INDEX(wh);
        if ((hi >> 3) == (lo >> 3)) //Same row
        {
            res += LSB_INDEX(getLeftMoves(lo) & all) == hi;
        }
        else if((hi & 7) == (lo & 7)) //Same col
        {
            res += LSB_INDEX(getUpMoves(lo) & all) == hi;
        }
    }
    if (bl & (bl - 1))
    {
        int hi = MSB_INDEX(bl), lo = LSB_INDEX(bl);
        if ((hi >> 3) == (lo >> 3)) //Same row
        {
            res -= LSB_INDEX(getLeftMoves(lo) & all) == hi;
        }
        else if((hi & 7) == (lo & 7)) //Same col
        {
            res -= LSB_INDEX(getUpMoves(lo) & all) == hi;
        }
    }

    return CONNECTED_ROOKS * res;
}
inline int rookOnOpenFile(uint64_t wr, uint64_t wp, uint64_t br, uint64_t bp)
{
    int w = 0, b = 0;
    if (wr)
        w =  ((getVert(MSB_INDEX(wr) & 7) & wp) == 0)
            +((getVert(LSB_INDEX(wr) & 7) & wp) == 0);
    if (br)
        b =  ((getVert(MSB_INDEX(br) & 7) & bp) == 0)
            +((getVert(LSB_INDEX(br) & 7) & bp) == 0);

    return ROOK_OPEN_FILE * (w - b);
}
inline int twoBishops(uint64_t wh, uint64_t bl)
{
    return TWO_BISH * ((POPCOUNT(wh) == 2) - (POPCOUNT(bl) == 2));
}

int allPiecesValue(const Board bo)
{
    return   VQUEEN     *(POPCOUNT(bo.piece[1][QUEEN])    - POPCOUNT(bo.piece[0][QUEEN]))
            +VROOK      *(POPCOUNT(bo.piece[1][ROOK])     - POPCOUNT(bo.piece[0][ROOK]))
            +VBISH      *(POPCOUNT(bo.piece[1][BISH])     - POPCOUNT(bo.piece[0][BISH]))
            +VKNIGHT    *(POPCOUNT(bo.piece[1][KNIGHT])   - POPCOUNT(bo.piece[0][KNIGHT]))
            +VPAWN      *(POPCOUNT(bo.piece[1][PAWN])     - POPCOUNT(bo.piece[0][PAWN]));
}

int multiply(int vals[64], uint64_t mask)
{
    int val = 0;

    while(mask)
    {
        val += vals[63 - LSB_INDEX(mask)];
        REMOVE_LSB(mask);
    }

    return val;
}

int testEval(char* beg)
{
    int a;
    Board b = genFromFen(beg, &a);
    return eval(b);
}

int bishMatrix[64] = 
   {5, 5, 5, 5,     5, 5, 5, 5,     
    5, 13, 9, 5,     5, 9, 13, 5,
    11, 9, 9, -20, -20, 9, 9, 5,
    15, 11, 9, 5,   15, 11, 9, 5,
    
    5, 5, 5, 9,     5, 5, 5, 9,     
    9, 9, 9, -20,    -20, 9, 9, 9,
    11, 13, 9, 5,   5, 9, 13, 5,
    5, 5, 5, 5,     5, 5, 5, 5};

int knightMatrix[64] = 
    {-30, -5, -15, -15,  -15, -15, -5, -30,
    -15, 0, 0, 0,       0, 0, 0, -15,
    -8, 0, 15, 15,     15, 15, 0, -8,
    -15, 0, 15, 20,     20, 15, 0, -15,

    -15, 0, 15, 20,     20, 15, 0, -15,
    -8, 0, 15, 15,     15, 15, 0, -8,
    -15, 0, 0, 0,       0, 0, 0, -15,
    -30, -5, -15, -15,   -15, -15, -5, -30};

int wPawnMatrix[64] = 
   {0, 0, 0, 0,     0, 0, 0, 0,
    20, 50, 50, 50,     50, 50, 50, 20,
    -5, 3, 3, 3,     3, 3, 3, -5,
    -5, 1, 0, 2,     2, 0, 1, -5,

    4, 4, 4, 20,     20, 4, 4, 4,
    1, 5, 1, 7,     7, 1, 5, 1,
    5, 5, 5, -5,     -5, 5, 5, 5,
    0, 0, 0, 0,     0, 0, 0, 0};

int bPawnMatrix[64] = 
   {0, 0, 0, 0,     0, 0, 0, 0,
    5, 5, 5, -5,     -5, 5, 5, 5,
    1, 5, 1, 7,     7, 1, 5, 1,
    4, 4, 4, 20,     20, 4, 4, 4,

    -5, 1, 0, 2,     2, 0, 1, -5,
    -5, 3, 3, 3,     3, 3, 3, -5,
    20, 50, 50, 50,     50, 50, 50, 20,
    0, 0, 0, 0,     0, 0, 0, 0};

int ewPawnMatrix[64] = 
   {0, 0, 0, 0,     0, 0, 0, 0,
    50, 100, 100, 100,     100, 100, 100, 50,
    7, 15, 20, 25,     25, 20, 15, 7,
    5, 7, 7, 7,     7, 7, 7, 5,

    0, 0, 3, 5,     5, 3, 0, 0,
    -3, -3, -3, -3,     -3, -3, -3, -3,
    -10, -10, -10, -10,     -10, -10, -10, -10,
    0, 0, 0, 0,     0, 0, 0, 0};

int ebPawnMatrix[64] = 
   {0, 0, 0, 0,     0, 0, 0, 0,
    -10, -10, -10, -10,     -10, -10, -10, -10,
    -3, -3, -3, -3,     -3, -3, -3, -3,
    0, 0, 3, 5,     5, 3, 0, 0,

    5, 7, 7, 7,     7, 7, 7, 5,
    7, 15, 20, 25,     25, 20, 15, 7,
    50, 100, 100, 100,     100, 100, 100, 50,
    0, 0, 0, 0,     0, 0, 0, 0};