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
        forward = getWhitePawnMoves(lsb) & (ALL ^ b->color[WHITE] ^ b->color[BLACK]);
        if (lsb < 16 && (POW2[8 + lsb] & b->allPieces))
            return getWhitePawnCaptures(lsb) & b->color[BLACK];

        return forward | (getWhitePawnCaptures(lsb) & b->color[BLACK]);
    }
    else{
        forward = getBlackPawnMoves(lsb) & (ALL ^ b->color[BLACK] ^ b->color[WHITE]);
        
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


uint64_t kingStraight(const int lsb, const uint64_t allPieces)
{
    /*
    const uint64_t upMoves = getUpMoves(lsb);
    const uint64_t downMoves = getDownMoves(lsb);
    const uint64_t rightMoves = getRightMoves(lsb);
    const uint64_t leftMoves = getLeftMoves(lsb);
    */

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

static inline uint64_t kingPawn(const int lsb, const int color)
{
    return color ? getWhitePawnCaptures(lsb) : getBlackPawnCaptures(lsb);
}

//TODO: Maybe use an array to simplify?
int canCastle(Board* b, const int color)
{

    int lsb, canK, canQ;
    uint64_t maskK, maskQ, rookK, rookQ;

    if (color)
    {
        maskK = C_MASK_WK;
        maskQ = C_MASK_WQ;
        rookK = 1;
        rookQ = 128;

        canK = (b->posInfo & WCASTLEK) && (rookK & b->piece[color][ROOK]) && ((b->allPieces & maskK) == 0ULL);
        canQ = (b->posInfo & WCASTLEQ) && (rookQ & b->piece[color][ROOK]) && ((b->allPieces & maskQ) == 0ULL);
    }
    else
    {
        maskK = C_MASK_BK;
        maskQ = C_MASK_BQ;
        rookK = POW2[56];
        rookQ = POW2[63];

        canK = (b->posInfo & BCASTLEK) && (rookK & b->piece[color][ROOK]) && ((b->allPieces & maskK) == 0ULL);
        canQ = (b->posInfo & BCASTLEQ) && (rookQ & b->piece[color][ROOK]) && ((b->allPieces & maskQ) == 0ULL);
    }

    if (canK)
    {
        canK = isInCheck(b, color) == NO_PIECE;
        while(canK && maskK)
        {
            lsb = LSB_INDEX(maskK);
            REMOVE_LSB(maskK);
            if (checkInPosition(b, lsb, color) != NO_PIECE)
                canK = 0;
        }
    }
    if (canQ)
    {
        uint64_t maskToCastle = 0x30 * POW2[(1 ^ color) * 56]; //Squares which the king goes throught, in kingside they are the same as the mask
        canQ = isInCheck(b, color) == NO_PIECE;
        while(canQ && maskToCastle)
        {
            lsb = LSB_INDEX(maskToCastle);
            REMOVE_LSB(maskToCastle);
            if (checkInPosition(b, lsb, color) != NO_PIECE)
                canQ = 0;
        }
    }

    return (canQ << 1) | canK;
}
Move castleKSide(const int color)
{
    Move m;
    uint64_t from, to;
    if (color)
    {
        from = 3;
        to = 1;
    }
    else
    {
        from = 59;
        to = 57;
    }
    
    m = (Move) {.pieceThatMoves = KING, .from = from, .to = to, .castle = 1};

    return m;
}
Move castleQSide(const int color)
{
    Move m;
    uint64_t from, to;
    if (color)
    {
        from = 3;
        to = 5;
    }
    else
    {
        from = 59;
        to = 61;
    }
    
    m = (Move) {.pieceThatMoves = KING, .from = from, .to = to, .castle = 2};

    return m;
}


int checkInPosition(Board* b, const int lsb, const int kingsColor)
{
    uint64_t straight, diagonal;
    const int inverse = 1 ^ kingsColor;

    if (b->piece[inverse][KING] & getKingMoves(lsb)) return KING;
    
    if (b->piece[inverse][PAWN] & kingPawn(lsb, kingsColor)) return PAWN;

    if (b->piece[inverse][KNIGHT] & getKnightMoves(lsb)) return KNIGHT;
    //TODO: Simplify this?
    if (b->piece[inverse][QUEEN] || b->piece[inverse][ROOK])
    {
        straight = kingStraight(lsb, b->allPieces);

        if (b->piece[inverse][ROOK] & straight) return ROOK;
        if (b->piece[inverse][QUEEN] & straight) return QUEEN;
    }
    if (b->piece[inverse][QUEEN] || b->piece[inverse][BISH])
    {
        diagonal = kingDiagonal(lsb, b->allPieces);
        
        if (b->piece[inverse][BISH] & diagonal) return BISH;
        if (b->piece[inverse][QUEEN] & diagonal) return QUEEN;
    }

    return NO_PIECE;
}
int isInCheck(Board* b, const int kingsColor)
{
    return checkInPosition(b, LSB_INDEX(b->piece[kingsColor][KING]), kingsColor);
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