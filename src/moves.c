#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/memoization.h"
#include "../include/io.h"

#include <stdio.h>


uint64_t posKingMoves(Board* b, const int color)
{
    return getKingMoves(LSB_INDEX(b->piece[color][KING])) & b->color[color | 2];
}

uint64_t posKnightMoves(Board* b, const int color, const int lsb)
{
    return getKnightMoves(lsb) & b->color[color | 2];
}

uint64_t posPawnMoves(Board* b, const int color, const int lsb)
{
    uint64_t forward;
    if (color){
        forward = getWhitePawnMoves(lsb) & (b->color[AV_WHITE] ^ b->color[BLACK]);
        if (lsb < 16 && (POW2[8 + lsb] & b->allPieces))
            return getWhitePawnCaptures(lsb) & b->color[BLACK];

        return forward | (getWhitePawnCaptures(lsb) & b->color[BLACK]);
    }
    else{
        forward = getBlackPawnMoves(lsb) & (b->color[AV_BLACK] ^ b->color[WHITE]);
        
        if (lsb > 47 && (POW2[lsb - 8] & b->allPieces))
            return getBlackPawnCaptures(lsb) & b->color[WHITE];
        
        return forward | (getBlackPawnCaptures(lsb) & b->color[WHITE]);
    }
}

uint64_t posRookMoves(Board* b, const int color, const int lsb)
{
    const uint64_t inteUp = getUpMoves(lsb) & b->allPieces;
    const uint64_t inteDown = getDownMoves(lsb) & b->allPieces;
    const uint64_t inteRight = getRightMoves(lsb) & b->allPieces;
    const uint64_t inteLeft = getLeftMoves(lsb) & b->allPieces;

    uint64_t res = 0;
    int obstacle;

    if (inteUp){
        obstacle = LSB_INDEX(inteUp);
        res |= getUpMoves(lsb) ^ getUpMoves(obstacle);
        res ^= POW2[obstacle] & b->color[color];
    } else res |= getUpMoves(lsb);
    
    if (inteDown){
        obstacle = MSB_INDEX(inteDown);
        res |= getDownMoves(lsb) ^ getDownMoves(obstacle);
        res ^= POW2[obstacle] & b->color[color];
    } else res |= getDownMoves(lsb);
    
    if (inteRight){
        obstacle = MSB_INDEX(inteRight);
        res |= getRightMoves(lsb) ^ getRightMoves(obstacle);
        res ^= POW2[obstacle] & b->color[color];
    } else res |= getRightMoves(lsb);
    
    if (inteLeft){
        obstacle = LSB_INDEX(inteLeft);
        res |= getLeftMoves(lsb) ^ getLeftMoves(obstacle);
        res ^= POW2[obstacle] & b->color[color];
    } else res |= getLeftMoves(lsb);

    return res;
}

uint64_t posBishMoves(Board* b, const int color, const int lsb)
{
    const uint64_t inteUpRight = getUpRightMoves(lsb) & b->allPieces;
    const uint64_t inteUpLeft = getUpLeftMoves(lsb) & b->allPieces;
    const uint64_t inteDownRight = getDownRightMoves(lsb) & b->allPieces;
    const uint64_t inteDownLeft = getDownLeftMoves(lsb) & b->allPieces;

    uint64_t res = 0;
    int obstacle;

    if (inteUpRight){
        obstacle = LSB_INDEX(inteUpRight);
        res |= getUpRightMoves(lsb) ^ getUpRightMoves(obstacle);
        res ^= POW2[obstacle] & b->color[color];
    } else res |= getUpRightMoves(lsb);
    
    if (inteUpLeft){
        obstacle = LSB_INDEX(inteUpLeft);
        res |= getUpLeftMoves(lsb) ^ getUpLeftMoves(obstacle);
        res ^= POW2[obstacle] & b->color[color];
    } else res |= getUpLeftMoves(lsb);
    
    if (inteDownRight){
        obstacle = MSB_INDEX(inteDownRight);
        res |= getDownRightMoves(lsb) ^ getDownRightMoves(obstacle);
        res ^= POW2[obstacle] & b->color[color];
    } else res |= getDownRightMoves(lsb);
    
    if (inteDownLeft){
        obstacle = MSB_INDEX(inteDownLeft);
        res |= getDownLeftMoves(lsb) ^ getDownLeftMoves(obstacle);
        res ^= POW2[obstacle] & b->color[color];
    } else res |= getDownLeftMoves(lsb);

    return res;
}
uint64_t posQueenMoves(Board* b, const int color, const int lsb)
{
    return posBishMoves(b, color, lsb) | posRookMoves(b, color, lsb);
}


//TODO: Improve this function
uint64_t kingStraight(const int lsb, const uint64_t allPieces)
{
    const uint64_t inteUp = getUpMoves(lsb) & allPieces;
    const uint64_t inteDown = getDownMoves(lsb) & allPieces;
    const uint64_t inteRight = getRightMoves(lsb) & allPieces;
    const uint64_t inteLeft = getLeftMoves(lsb) & allPieces;

    uint64_t res = 0;
    int obstacle;

    if (inteUp){
        obstacle = LSB_INDEX(inteUp);
        res |= getUpMoves(lsb) ^ getUpMoves(obstacle);
    } else res |= getUpMoves(lsb);
    
    if (inteDown){
        obstacle = MSB_INDEX(inteDown);
        res |= getDownMoves(lsb) ^ getDownMoves(obstacle);
    } else res |= getDownMoves(lsb);
    
    if (inteRight){
        obstacle = MSB_INDEX(inteRight);
        res |= getRightMoves(lsb) ^ getRightMoves(obstacle);
    } else res |= getRightMoves(lsb);
    
    if (inteLeft){
        obstacle = LSB_INDEX(inteLeft);
        res |= getLeftMoves(lsb) ^ getLeftMoves(obstacle);
    } else res |= getLeftMoves(lsb);

    return res;
}
uint64_t kingDiagonal(const int lsb, const uint64_t allPieces)
{
    const uint64_t inteUpRight = getUpRightMoves(lsb) & allPieces;
    const uint64_t inteUpLeft = getUpLeftMoves(lsb) & allPieces;
    const uint64_t inteDownRight = getDownRightMoves(lsb) & allPieces;
    const uint64_t inteDownLeft = getDownLeftMoves(lsb) & allPieces;

    uint64_t res = 0;
    int obstacle;


    if (inteUpRight){
        obstacle = LSB_INDEX(inteUpRight);
        res |= getUpRightMoves(lsb) ^ getUpRightMoves(obstacle);
    } else res |= getUpRightMoves(lsb);
    
    if (inteUpLeft){
        obstacle = LSB_INDEX(inteUpLeft);
        res |= getUpLeftMoves(lsb) ^ getUpLeftMoves(obstacle);
    } else res |= getUpLeftMoves(lsb);
    
    if (inteDownRight){
        obstacle = MSB_INDEX(inteDownRight);
        res |= getDownRightMoves(lsb) ^ getDownRightMoves(obstacle);
    } else res |= getDownRightMoves(lsb);
    
    if (inteDownLeft){
        obstacle = MSB_INDEX(inteDownLeft);
        res |= getDownLeftMoves(lsb) ^ getDownLeftMoves(obstacle);
    } else res |= getDownLeftMoves(lsb);

    return res;
}
static inline uint64_t kingKnight(const int lsb)
{
    return getKnightMoves(lsb);
}
static inline uint64_t kingPawn(const int lsb, const int color)
{
    return color ? getWhitePawnCaptures(lsb) : getBlackPawnCaptures(lsb);
}

int isInCheck(Board* b, const int kingsColor)
{
    int numPawns, lsb;
    uint64_t straight, diagonal;
    uint64_t pos;

    pos = b->piece[kingsColor][KING];

    lsb = LSB_INDEX(pos);

    if (b->piece[1 ^ kingsColor][PAWN] & kingPawn(lsb, kingsColor)) return PAWN;

    if (b->piece[1 ^ kingsColor][KNIGHT] & kingKnight(lsb)) return KNIGHT;

    //TODO: Simplify this
    if (b->piece[1 ^ kingsColor][QUEEN] || b->piece[1 ^ kingsColor][ROOK])
    {
        straight = kingStraight(lsb, b->allPieces);
        if (b->piece[1 ^ kingsColor][ROOK] & straight) return ROOK;
        if (b->piece[1 ^ kingsColor][QUEEN] & straight) return QUEEN;
    }
    if (b->piece[1 ^ kingsColor][QUEEN] || b->piece[1 ^ kingsColor][BISH])
    {
        diagonal = kingDiagonal(lsb, b->allPieces);
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