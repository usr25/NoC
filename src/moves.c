#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/memoization.h"
#include "../include/io.h"

#include <stdio.h>


uint64_t posKingMoves(Board* b, const int color)
{
    return getKingMoves(LSB_INDEX(b->piece[color][KING])) & b->color[color + 2];
}

//TODO: Improve the while() by removing it
uint64_t posKnightMoves(Board* b, const int color, int index)
{
    uint64_t pos = b->piece[color][KNIGHT];
    while(index--) REMOVE_LSB(pos);

    return getKnightMoves(LSB_INDEX(pos)) & b->color[color + 2];
}

uint64_t posPawnMoves(Board* b, const int color, int index)
{
    uint64_t pos = b->piece[color][PAWN];
    while(index--) REMOVE_LSB(pos);

    int i = LSB_INDEX(pos);
    uint64_t forward;
    if (color){
        forward = getWhitePawnMoves(i) & (b->color[AV_WHITE] ^ b->color[BLACK]);
        if (i < 16 && (POW2[8 + i] & b->allPieces))
            return getWhitePawnCaptures(i) & b->color[1 ^ color];

        return forward | (getWhitePawnCaptures(i) & b->color[1 ^ color]);
    }
    else{
        forward = getBlackPawnMoves(i) & (b->color[AV_BLACK] ^ b->color[WHITE]);
        
        if (i > 47 && (POW2[i - 8] & b->allPieces))
            return getBlackPawnCaptures(i) & b->color[1 ^ color];
        
        return forward | (getBlackPawnCaptures(i) & b->color[1 ^ color]);
    }
}

uint64_t posRookMoves(Board* b, const int color, int index)
{
    uint64_t pos = b->piece[color][ROOK];
    while(index--) REMOVE_LSB(pos);
    int i = LSB_INDEX(pos);

    uint64_t inteUp = getUpMoves(i) & b->allPieces;
    uint64_t inteDown = getDownMoves(i) & b->allPieces;
    uint64_t inteRight = getRightMoves(i) & b->allPieces;
    uint64_t inteLeft = getLeftMoves(i) & b->allPieces;

    uint64_t res = 0;
    int obstacle;


    if (inteUp){
        obstacle = LSB_INDEX(inteUp);
        res |= getUpMoves(i) ^ getUpMoves(obstacle);
        res ^= POW2[obstacle] & b->color[color];
    } else res |= getUpMoves(i);
    
    if (inteDown){
        obstacle = MSB_INDEX(inteDown);
        res |= getDownMoves(i) ^ getDownMoves(obstacle);
        res ^= POW2[obstacle] & b->color[color];
    } else res |= getDownMoves(i);
    
    if (inteRight){
        obstacle = MSB_INDEX(inteRight);
        res |= getRightMoves(i) ^ getRightMoves(obstacle);
        res ^= POW2[obstacle] & b->color[color];
    } else res |= getRightMoves(i);
    
    if (inteLeft){
        obstacle = LSB_INDEX(inteLeft);
        res |= getLeftMoves(i) ^ getLeftMoves(obstacle);
        res ^= POW2[obstacle] & b->color[color];
    } else res |= getLeftMoves(i);


    return res;
}

uint64_t posBishMoves(Board* b, const int color, int index)
{
    uint64_t pos = b->piece[color][BISH];
    while(index--) REMOVE_LSB(pos);
    int i = LSB_INDEX(pos);

    uint64_t inteUpRight = getUpRightMoves(i) & b->allPieces;
    uint64_t inteUpLeft = getUpLeftMoves(i) & b->allPieces;
    uint64_t inteDownRight = getDownRightMoves(i) & b->allPieces;
    uint64_t inteDownLeft = getDownLeftMoves(i) & b->allPieces;

    uint64_t res = 0;
    int obstacle;


    if (inteUpRight){
        obstacle = LSB_INDEX(inteUpRight);
        res |= getUpRightMoves(i) ^ getUpRightMoves(obstacle);
        res ^= POW2[obstacle] & b->color[color];
    } else res |= getUpRightMoves(i);
    
    if (inteUpLeft){
        obstacle = LSB_INDEX(inteUpLeft);
        res |= getUpLeftMoves(i) ^ getUpLeftMoves(obstacle);
        res ^= POW2[obstacle] & b->color[color];
    } else res |= getUpLeftMoves(i);
    
    if (inteDownRight){
        obstacle = MSB_INDEX(inteDownRight);
        res |= getDownRightMoves(i) ^ getDownRightMoves(obstacle);
        res ^= POW2[obstacle] & b->color[color];
    } else res |= getDownRightMoves(i);
    
    if (inteDownLeft){
        obstacle = MSB_INDEX(inteDownLeft);
        res |= getDownLeftMoves(i) ^ getDownLeftMoves(obstacle);
        res ^= POW2[obstacle] & b->color[color];
    } else res |= getDownLeftMoves(i);


    return res;
}
uint64_t posQueenMoves(Board* b, const int color, int index)
{
    uint64_t pos = b->piece[color][QUEEN];
    while(index--) REMOVE_LSB(pos);
    int i = LSB_INDEX(pos);

    uint64_t inteUp = getUpMoves(i) & b->allPieces;
    uint64_t inteDown = getDownMoves(i) & b->allPieces;
    uint64_t inteRight = getRightMoves(i) & b->allPieces;
    uint64_t inteLeft = getLeftMoves(i) & b->allPieces;

    uint64_t inteUpRight = getUpRightMoves(i) & b->allPieces;
    uint64_t inteUpLeft = getUpLeftMoves(i) & b->allPieces;
    uint64_t inteDownRight = getDownRightMoves(i) & b->allPieces;
    uint64_t inteDownLeft = getDownLeftMoves(i) & b->allPieces;

    uint64_t res = 0;
    int obstacle;

    if (inteUp){
        obstacle = LSB_INDEX(inteUp);
        res |= getUpMoves(i) ^ getUpMoves(obstacle);
        res ^= POW2[obstacle] & b->color[color];
    } else res |= getUpMoves(i);
    
    if (inteDown){
        obstacle = MSB_INDEX(inteDown);
        res |= getDownMoves(i) ^ getDownMoves(obstacle);
        res ^= POW2[obstacle] & b->color[color];
    } else res |= getDownMoves(i);
    
    if (inteRight){
        obstacle = MSB_INDEX(inteRight);
        res |= getRightMoves(i) ^ getRightMoves(obstacle);
        res ^= POW2[obstacle] & b->color[color];
    } else res |= getRightMoves(i);
    
    if (inteLeft){
        obstacle = LSB_INDEX(inteLeft);
        res |= getLeftMoves(i) ^ getLeftMoves(obstacle);
        res ^= POW2[obstacle] & b->color[color];
    } else res |= getLeftMoves(i);
    

    if (inteUpRight){
        obstacle = LSB_INDEX(inteUpRight);
        res |= getUpRightMoves(i) ^ getUpRightMoves(obstacle);
        res ^= POW2[obstacle] & b->color[color];
    } else res |= getUpRightMoves(i);
    
    if (inteUpLeft){
        obstacle = LSB_INDEX(inteUpLeft);
        res |= getUpLeftMoves(i) ^ getUpLeftMoves(obstacle);
        res ^= POW2[obstacle] & b->color[color];
    } else res |= getUpLeftMoves(i);
    
    if (inteDownRight){
        obstacle = MSB_INDEX(inteDownRight);
        res |= getDownRightMoves(i) ^ getDownRightMoves(obstacle);
        res ^= POW2[obstacle] & b->color[color];
    } else res |= getDownRightMoves(i);
    
    if (inteDownLeft){
        obstacle = MSB_INDEX(inteDownLeft);
        res |= getDownLeftMoves(i) ^ getDownLeftMoves(obstacle);
        res ^= POW2[obstacle] & b->color[color];
    } else res |= getDownLeftMoves(i);

    return res;
}


//TODO: Improve this function
uint64_t kingStraight(const uint64_t pos, const uint64_t allPieces)
{
    int i = LSB_INDEX(pos);

    uint64_t inteUp = getUpMoves(i) & allPieces;
    uint64_t inteDown = getDownMoves(i) & allPieces;
    uint64_t inteRight = getRightMoves(i) & allPieces;
    uint64_t inteLeft = getLeftMoves(i) & allPieces;

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
uint64_t kingDiagonal(const uint64_t pos, const uint64_t allPieces)
{
    int i = LSB_INDEX(pos);

    uint64_t inteUpRight = getUpRightMoves(i) & allPieces;
    uint64_t inteUpLeft = getUpLeftMoves(i) & allPieces;
    uint64_t inteDownRight = getDownRightMoves(i) & allPieces;
    uint64_t inteDownLeft = getDownLeftMoves(i) & allPieces;

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

    pos = b->piece[kingsColor][KING];

    if (b->piece[1 ^ kingsColor][PAWN] & kingPawn(pos, kingsColor)) return PAWN;

    if (b->piece[1 ^ kingsColor][KNIGHT] & kingKnight(pos)) return KNIGHT;

    //TODO: Simplify this
    if (b->piece[1 ^ kingsColor][QUEEN] || b->piece[1 ^ kingsColor][ROOK])
    {
        straight = kingStraight(pos, b->allPieces);
        if (b->piece[1 ^ kingsColor][ROOK] & straight) return ROOK;
        if (b->piece[1 ^ kingsColor][QUEEN] & straight) return QUEEN;
    }
    if (b->piece[1 ^ kingsColor][QUEEN] || b->piece[1 ^ kingsColor][BISH])
    {
        diagonal = kingDiagonal(pos, b->allPieces);
        if (b->piece[1 ^ kingsColor][BISH] & diagonal) return BISH;
        if (b->piece[1 ^ kingsColor][QUEEN] & diagonal) return QUEEN;
    }

    return NO_PIECE;
}

/*
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
*/