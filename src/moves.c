#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/memoization.h"
#include "../include/io.h"

#include <stdio.h>

uint64_t posWhiteKingMoves(Board* b)
{
    return getKingMoves(LSB_INDEX(b->wKing)) & b->avWhite;
}
uint64_t posBlackKingMoves(Board* b)
{
    return getKingMoves(LSB_INDEX(b->bKing)) & b->avBlack;
}

uint64_t posKingMoves(Board* b, const int color)
{
    if (color)
        return posWhiteKingMoves(b);
    else
        return posBlackKingMoves(b);
}

//TODO: Improve the while() by removing it
uint64_t posKnightMoves(Board* b, const int color, int index)
{
    uint64_t pos = (color)?b->wKnight:b->bKnight;
    while(index--) REMOVE_LSB(pos);

    if (color)
        return getKnightMoves(LSB_INDEX(pos)) & b->avWhite;
    else
        return getKnightMoves(LSB_INDEX(pos)) & b->avBlack;
}

uint64_t posPawnMoves(Board* b, const int color, int index)
{
    uint64_t pos = (color)?b->wPawn:b->bPawn;
    while(index--) REMOVE_LSB(pos);

    int i = LSB_INDEX(pos);
    uint64_t forward;
    if (color){
        forward = getWhitePawnMoves(i) & (b->avWhite ^ b->black);
        if (i < 16 && (POW2[8 + i] & b->pieces))
            return getWhitePawnCaptures(i) & b->black;

        return forward | (getWhitePawnCaptures(i) & b->black);
    }
    else{
        forward = getBlackPawnMoves(i) & (b->avBlack ^ b->white);
        
        if (i > 47 && (POW2[i - 8] & b->pieces))
            return getBlackPawnCaptures(i) & b->white;
        
        return forward | (getBlackPawnCaptures(i) & b->white);
    }
}

uint64_t posRookMoves(Board* b, const int color, int index)
{
    uint64_t pos = (color)?b->wRook:b->bRook;
    while(index--) REMOVE_LSB(pos);
    int i = LSB_INDEX(pos);

    uint64_t inteUp = getUpMoves(i) & b->pieces;
    uint64_t inteDown = getDownMoves(i) & b->pieces;
    uint64_t inteRight = getRightMoves(i) & b->pieces;
    uint64_t inteLeft = getLeftMoves(i) & b->pieces;

    uint64_t res = 0;
    int obstacle;


    if (inteUp){
        obstacle = LSB_INDEX(inteUp);
        res |= getUpMoves(i) ^ getUpMoves(obstacle);
        res ^= color ? POW2[obstacle] & b->white : POW2[obstacle] & b->black;
    } else res |= getUpMoves(i);
    
    if (inteDown){
        obstacle = MSB_INDEX(inteDown);
        res |= getDownMoves(i) ^ getDownMoves(obstacle);
        res ^= color ? POW2[obstacle] & b->white : POW2[obstacle] & b->black;
    } else res |= getDownMoves(i);
    
    if (inteRight){
        obstacle = MSB_INDEX(inteRight);
        res |= getRightMoves(i) ^ getRightMoves(obstacle);
        res ^= color ? POW2[obstacle] & b->white : POW2[obstacle] & b->black;
    } else res |= getRightMoves(i);
    
    if (inteLeft){
        obstacle = LSB_INDEX(inteLeft);
        res |= getLeftMoves(i) ^ getLeftMoves(obstacle);
        res ^= color ? POW2[obstacle] & b->white : POW2[obstacle] & b->black;
    } else res |= getLeftMoves(i);


    return res;
}

uint64_t posBishMoves(Board* b, const int color, int index)
{
    uint64_t pos = (color)?b->wBish:b->bBish;
    while(index--) REMOVE_LSB(pos);
    int i = LSB_INDEX(pos);

    uint64_t inteUpRight = getUpRightMoves(i) & b->pieces;
    uint64_t inteUpLeft = getUpLeftMoves(i) & b->pieces;
    uint64_t inteDownRight = getDownRightMoves(i) & b->pieces;
    uint64_t inteDownLeft = getDownLeftMoves(i) & b->pieces;

    uint64_t res = 0;
    int obstacle;


    if (inteUpRight){
        obstacle = LSB_INDEX(inteUpRight);
        res |= getUpRightMoves(i) ^ getUpRightMoves(obstacle);
        res ^= color ? POW2[obstacle] & b->white : POW2[obstacle] & b->black;
    } else res |= getUpRightMoves(i);
    
    if (inteUpLeft){
        obstacle = LSB_INDEX(inteUpLeft);
        res |= getUpLeftMoves(i) ^ getUpLeftMoves(obstacle);
        res ^= color ? POW2[obstacle] & b->white : POW2[obstacle] & b->black;
    } else res |= getUpLeftMoves(i);
    
    if (inteDownRight){
        obstacle = MSB_INDEX(inteDownRight);
        res |= getDownRightMoves(i) ^ getDownRightMoves(obstacle);
        res ^= color ? POW2[obstacle] & b->white : POW2[obstacle] & b->black;
    } else res |= getDownRightMoves(i);
    
    if (inteDownLeft){
        obstacle = MSB_INDEX(inteDownLeft);
        res |= getDownLeftMoves(i) ^ getDownLeftMoves(obstacle);
        res ^= color ? POW2[obstacle] & b->white : POW2[obstacle] & b->black;
    } else res |= getDownLeftMoves(i);


    return res;
}
uint64_t posQueenMoves(Board* b, const int color, int index)
{
    uint64_t pos = (color)?b->wQueen:b->bQueen;
    while(index--) REMOVE_LSB(pos);
    int i = LSB_INDEX(pos);

    uint64_t inteUp = getUpMoves(i) & b->pieces;
    uint64_t inteDown = getDownMoves(i) & b->pieces;
    uint64_t inteRight = getRightMoves(i) & b->pieces;
    uint64_t inteLeft = getLeftMoves(i) & b->pieces;

    uint64_t inteUpRight = getUpRightMoves(i) & b->pieces;
    uint64_t inteUpLeft = getUpLeftMoves(i) & b->pieces;
    uint64_t inteDownRight = getDownRightMoves(i) & b->pieces;
    uint64_t inteDownLeft = getDownLeftMoves(i) & b->pieces;

    uint64_t res = 0;
    int obstacle;

    if (inteUp){
        obstacle = LSB_INDEX(inteUp);
        res |= getUpMoves(i) ^ getUpMoves(obstacle);
        res ^= color ? POW2[obstacle] & b->white : POW2[obstacle] & b->black;
    } else res |= getUpMoves(i);
    
    if (inteDown){
        obstacle = MSB_INDEX(inteDown);
        res |= getDownMoves(i) ^ getDownMoves(obstacle);
        res ^= color ? POW2[obstacle] & b->white : POW2[obstacle] & b->black;
    } else res |= getDownMoves(i);
    
    if (inteRight){
        obstacle = MSB_INDEX(inteRight);
        res |= getRightMoves(i) ^ getRightMoves(obstacle);
        res ^= color ? POW2[obstacle] & b->white : POW2[obstacle] & b->black;
    } else res |= getRightMoves(i);
    
    if (inteLeft){
        obstacle = LSB_INDEX(inteLeft);
        res |= getLeftMoves(i) ^ getLeftMoves(obstacle);
        res ^= color ? POW2[obstacle] & b->white : POW2[obstacle] & b->black;
    } else res |= getLeftMoves(i);
    

    if (inteUpRight){
        obstacle = LSB_INDEX(inteUpRight);
        res |= getUpRightMoves(i) ^ getUpRightMoves(obstacle);
        res ^= color ? POW2[obstacle] & b->white : POW2[obstacle] & b->black;
    } else res |= getUpRightMoves(i);
    
    if (inteUpLeft){
        obstacle = LSB_INDEX(inteUpLeft);
        res |= getUpLeftMoves(i) ^ getUpLeftMoves(obstacle);
        res ^= color ? POW2[obstacle] & b->white : POW2[obstacle] & b->black;
    } else res |= getUpLeftMoves(i);
    
    if (inteDownRight){
        obstacle = MSB_INDEX(inteDownRight);
        res |= getDownRightMoves(i) ^ getDownRightMoves(obstacle);
        res ^= color ? POW2[obstacle] & b->white : POW2[obstacle] & b->black;
    } else res |= getDownRightMoves(i);
    
    if (inteDownLeft){
        obstacle = MSB_INDEX(inteDownLeft);
        res |= getDownLeftMoves(i) ^ getDownLeftMoves(obstacle);
        res ^= color ? POW2[obstacle] & b->white : POW2[obstacle] & b->black;
    } else res |= getDownLeftMoves(i);


    return res;
}


//TODO: Improve this function
uint64_t kingStraight(Board* b, const uint64_t pos)
{
    int i = LSB_INDEX(pos);

    uint64_t inteUp = getUpMoves(i) & b->pieces;
    uint64_t inteDown = getDownMoves(i) & b->pieces;
    uint64_t inteRight = getRightMoves(i) & b->pieces;
    uint64_t inteLeft = getLeftMoves(i) & b->pieces;

    uint64_t res = 0;
    int obstacle;


    if (inteUp){
        obstacle = LSB_INDEX(inteUp);
        res |= getUpMoves(i) ^ getUpMoves(obstacle);
    } else res |= getUpMoves(i);
    
    if (inteDown){
        obstacle = MSB_INDEX(inteDown);
        res |= getDownMoves(i) ^ getDownMoves(obstacle);
    } else res |= getDownMoves(i);
    
    if (inteRight){
        obstacle = MSB_INDEX(inteRight);
        res |= getRightMoves(i) ^ getRightMoves(obstacle);
    } else res |= getRightMoves(i);
    
    if (inteLeft){
        obstacle = LSB_INDEX(inteLeft);
        res |= getLeftMoves(i) ^ getLeftMoves(obstacle);
    } else res |= getLeftMoves(i);

    return res;
}
uint64_t kingDiagonal(Board* b, const uint64_t pos)
{
    int i = LSB_INDEX(pos);

    uint64_t inteUpRight = getUpRightMoves(i) & b->pieces;
    uint64_t inteUpLeft = getUpLeftMoves(i) & b->pieces;
    uint64_t inteDownRight = getDownRightMoves(i) & b->pieces;
    uint64_t inteDownLeft = getDownLeftMoves(i) & b->pieces;

    uint64_t res = 0;
    int obstacle;


    if (inteUpRight){
        obstacle = LSB_INDEX(inteUpRight);
        res |= getUpRightMoves(i) ^ getUpRightMoves(obstacle);
    } else res |= getUpRightMoves(i);
    
    if (inteUpLeft){
        obstacle = LSB_INDEX(inteUpLeft);
        res |= getUpLeftMoves(i) ^ getUpLeftMoves(obstacle);
    } else res |= getUpLeftMoves(i);
    
    if (inteDownRight){
        obstacle = MSB_INDEX(inteDownRight);
        res |= getDownRightMoves(i) ^ getDownRightMoves(obstacle);
    } else res |= getDownRightMoves(i);
    
    if (inteDownLeft){
        obstacle = MSB_INDEX(inteDownLeft);
        res |= getDownLeftMoves(i) ^ getDownLeftMoves(obstacle);
    } else res |= getDownLeftMoves(i);

    return res;
}
static inline uint64_t kingKnight(const uint64_t pos)
{
    int i = LSB_INDEX(pos);
    return getKnightMoves(i);
}
static inline uint64_t kingPawn(const uint64_t pos, const int color)
{
    int i = LSB_INDEX(pos);
    return color ? getWhitePawnCaptures(i) : getBlackPawnCaptures(i);
}

int isInCheck(Board* b, const int kingsColor)
{
    int numPawns;
    uint64_t straight, diagonal;
    uint64_t pos;

    if (kingsColor)
    {
        pos = b->wKing;

        if (b->bPawn & kingPawn(pos, 1)) return PAWN;

        if (b->bKnight & kingKnight(pos)) return KNIGHT;

        if (numBQueen(b->numPieces) || numBRook(b->numPieces))
        {
            straight = kingStraight(b, pos);
            if (b->bRook & straight) return ROOK;
            if (b->bQueen & straight) return QUEEN;
        
        }
        if (numBQueen(b->numPieces) || numBBish(b->numPieces))
        {
            diagonal = kingDiagonal(b, pos);
            if (b->bQueen & diagonal) return QUEEN;
            if (b->bBish & diagonal) return BISH;
        }
    
    }
    else
    {
        pos = b->bKing;

        if (b->wPawn & kingPawn(pos, 0)) return PAWN;
        
        if (b->wKnight & kingKnight(pos)) return KNIGHT;

        if (numWQueen(b->numPieces) || numWRook(b->numPieces))
        {
            straight = kingStraight(b, pos);
            if (b->wRook & straight) return ROOK;
            if (b->wQueen & straight) return QUEEN;
        
        }
        if (numWQueen(b->numPieces) || numWBish(b->numPieces))
        {
            diagonal = kingDiagonal(b, pos);
            if (b->wQueen & diagonal) return QUEEN;
            if (b->wBish & diagonal) return BISH;
        }
    }

    return 0;
}

//It is assumed that the king is in check
//TODO: Not fully implemented, so far it kind of only detects available sqrs
int isMate(Board* b, const int kingsColor)
{
    uint64_t posEscapes, numPawn, numQueen, numRook, numBish, numKnight;

    if (kingsColor){
        posEscapes = posWhiteKingMoves(b);
        posEscapes &= ALL ^ posBlackKingMoves(b);
        //Get each pow2 in pos escapes and see if it is in check
    } 
}