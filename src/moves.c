/* moves.c
 * In charge of calculating all the moves for each piece in a given position, checks, 
 * castles, the ability to castle and the attacks on the king
 */

#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/memoization.h"
#include "../include/io.h"

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
        if (lsb < 16 && (POW2[8 + lsb] & b->allPieces))
            return getWhitePawnCaptures(lsb) & b->color[BLACK];

        forward = getWhitePawnMoves(lsb) & ~(b->color[WHITE] | b->color[BLACK]);
        return forward | (getWhitePawnCaptures(lsb) & b->color[BLACK]);
    }
    else{
        if (lsb > 47 && (POW2[lsb - 8] & b->allPieces))
            return getBlackPawnCaptures(lsb) & b->color[WHITE];

        forward = getBlackPawnMoves(lsb) & ~(b->color[BLACK] | b->color[WHITE]);
        return forward | (getBlackPawnCaptures(lsb) & b->color[WHITE]);
    }
}

/*
 * All the sliding pieces movements are calculated by checking if there is an
 * intersection with the move direction in a clead board, if there is, remove all
 * the following tiles and, depending on the colors, remove the tile of the obstacle
 * Eg.: Q - - r - -  => Q 1 1 r(1) - -
 */

//Possible rook moves
uint64_t straight(const int lsb, const uint64_t allPieces)
{
    const uint64_t inteUp = getUpMoves(lsb) & allPieces;
    const uint64_t inteDown = getDownMoves(lsb) & allPieces;
    const uint64_t inteRight = getRightMoves(lsb) & allPieces;
    const uint64_t inteLeft = getLeftMoves(lsb) & allPieces;
    
    uint64_t res = getStraMoves(lsb);   
    int obstacle;

    if (inteUp){
        obstacle = LSB_INDEX(inteUp);
        res ^= getUpMoves(obstacle);
    }

    if (inteDown){
        obstacle = MSB_INDEX(inteDown);
        res ^= getDownMoves(obstacle);
    }
    
    if (inteRight){
        obstacle = MSB_INDEX(inteRight);
        res ^= getRightMoves(obstacle);
    }
    
    if (inteLeft){
        obstacle = LSB_INDEX(inteLeft);
        res ^= getLeftMoves(obstacle);
    }

    return res;
}
uint64_t diagonal(const int lsb, const uint64_t allPieces)
{
    const uint64_t inteUpRight = getUpRightMoves(lsb) & allPieces;
    const uint64_t inteUpLeft = getUpLeftMoves(lsb) & allPieces;
    const uint64_t inteDownRight = getDownRightMoves(lsb) & allPieces;
    const uint64_t inteDownLeft = getDownLeftMoves(lsb) & allPieces;

    uint64_t res = getDiagMoves(lsb);
    int obstacle;

    if (inteUpRight){
        obstacle = LSB_INDEX(inteUpRight);
        res ^= getUpRightMoves(obstacle);
    }
    
    if (inteUpLeft){
        obstacle = LSB_INDEX(inteUpLeft);
        res ^= getUpLeftMoves(obstacle);
    }
    
    if (inteDownRight){
        obstacle = MSB_INDEX(inteDownRight);
        res ^= getDownRightMoves(obstacle);
    }
    
    if (inteDownLeft){
        obstacle = MSB_INDEX(inteDownLeft);
        res ^= getDownLeftMoves(obstacle);
    }

    return res;
}

inline uint64_t posRookMoves(Board* b, const int color, const int lsb)
{
    return b->color[color | 2] & straight(lsb, b->allPieces);
}
inline uint64_t posBishMoves(Board* b, const int color, const int lsb)
{
    return b->color[color | 2] & diagonal(lsb, b->allPieces);
}
inline uint64_t posQueenMoves(Board* b, const int color, const int lsb)
{
    return b->color[color | 2] & (straight(lsb, b->allPieces) | diagonal(lsb, b->allPieces));
}

static inline uint64_t kingPawn(const int lsb, const int color)
{
    return color ? getWhitePawnCaptures(lsb) : getBlackPawnCaptures(lsb);
}
//Slower version, only for tests
//TODO: Update tests and remove
int canCastleCheck(Board* b, const int color)
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
        canK = ! isInCheck(b, color);
        while(canK && maskK)
        {
            if (checkInPosition(b, LSB_INDEX(maskK), color))
                canK = 0;
            REMOVE_LSB(maskK);
        }
    }
    if (canQ)
    {
        uint64_t maskToCastle = 0x30 * POW2[(1 ^ color) * 56]; //Squares which the king goes throught, in kingside they are the same as the mask
        canQ = ! isInCheck(b, color);
        while(canQ && maskToCastle)
        {
            if (checkInPosition(b, LSB_INDEX(maskToCastle), color))
                canQ = 0;
            REMOVE_LSB(maskToCastle);
        }
    }

    return (canQ << 1) | canK;
}
//Determines if the king can castle
int canCastle(Board* b, const int color, const uint64_t forbidden) //Faster version, but forbidden has to be calculated
{
    int lsb, canK, canQ;
    uint64_t maskK, maskQ, rookK, rookQ;

    uint64_t maskCheck = 0x30 * POW2[(1 ^ color) * 56];

    if (color)
    {
        maskK = C_MASK_WK;
        maskQ = C_MASK_WQ;
        rookK = 1;   //2^0
        rookQ = 128; //2^7

        canK = (b->posInfo & WCASTLEK) && (rookK & b->piece[color][ROOK]) && ((b->allPieces & maskK) == 0ULL) && ((maskK & forbidden) == 0ULL);
        canQ = (b->posInfo & WCASTLEQ) && (rookQ & b->piece[color][ROOK]) && ((b->allPieces & maskQ) == 0ULL) && ((maskCheck & forbidden) == 0ULL);
    }
    else
    {
        maskK = C_MASK_BK;
        maskQ = C_MASK_BQ;
        rookK = POW2[56];
        rookQ = POW2[63];

        canK = (b->posInfo & BCASTLEK) && (rookK & b->piece[color][ROOK]) && ((b->allPieces & maskK) == 0ULL) && ((maskK & forbidden) == 0ULL);
        canQ = (b->posInfo & BCASTLEQ) && (rookQ & b->piece[color][ROOK]) && ((b->allPieces & maskQ) == 0ULL) && ((maskCheck & forbidden) == 0ULL);
    }

    return (canQ << 1) | canK;
}
inline Move castleKSide(const int color)
{
    return (Move) {.pieceThatMoves = KING, .from = 56 * (1 ^ color) + 3, .to = 56 * (1 ^ color) + 1, .castle = 1};
}
inline Move castleQSide(const int color)
{ 
    return (Move) {.pieceThatMoves = KING, .from = 56 * (1 ^ color) + 3, .to = 56 * (1 ^ color) + 5, .castle = 2};
}

//Tiles controlled by the opp king / pawns / knights
uint64_t controlledKingPawnKnight(Board* b, const int inverse)
{
    uint64_t temp;
    uint64_t res = getKingMoves(LSB_INDEX(b->piece[inverse][KING]));

    temp = b->piece[inverse][PAWN];
    while(temp)
    {
        res |= kingPawn(LSB_INDEX(temp), inverse);
        REMOVE_LSB(temp);
    }
    temp = b->piece[inverse][KNIGHT];
    while(temp)
    {
        res |= getKnightMoves(LSB_INDEX(temp));
        REMOVE_LSB(temp);
    }

    return res;
}

uint64_t allSlidingAttacks(Board* b, const int color, const uint64_t obstacles)
{
    uint64_t temp, res = 0ULL;
    int lsb;
    
    temp = b->piece[color][QUEEN];
    while(temp)
    {
        lsb = LSB_INDEX(temp);
        REMOVE_LSB(temp);
        res |= straight(lsb, obstacles) | diagonal(lsb, obstacles);
    }
    temp = b->piece[color][ROOK];
    while(temp)
    {
        res |= straight(LSB_INDEX(temp), obstacles);
        REMOVE_LSB(temp);
    }
    temp = b->piece[color][BISH];
    while(temp)
    {
        res |= diagonal(LSB_INDEX(temp), obstacles);
        REMOVE_LSB(temp);
    }

    return res;
}

//Any piece placed in one of this tiles will stop a check, either by capture or by pinning itself
AttacksOnK getCheckTiles(Board* b, const int color)
{
    const int inverse = 1 ^ color;
    const int lsb = LSB_INDEX(b->piece[color][KING]);

    uint64_t res = 
        (b->piece[inverse][PAWN] & kingPawn(lsb, color)) | (b->piece[inverse][KNIGHT] & getKnightMoves(lsb));

    int num = POPCOUNT(res);
    int obstacle;

    uint64_t stra = b->piece[inverse][QUEEN] | b->piece[inverse][ROOK];
    uint64_t diag = b->piece[inverse][QUEEN] | b->piece[inverse][BISH];

    if (stra)
    {
        const uint64_t inteUp = getUpMoves(lsb) & b->allPieces;
        const uint64_t inteDown = getDownMoves(lsb) & b->allPieces;
        const uint64_t inteRight = getRightMoves(lsb) & b->allPieces;
        const uint64_t inteLeft = getLeftMoves(lsb) & b->allPieces;

        if (inteUp){
            obstacle = LSB_INDEX(inteUp);
            if (POW2[obstacle] & stra){
                ++num;
                res |= getUpMoves(lsb) ^ getUpMoves(obstacle);
            }
        }
        if (inteDown){
            obstacle = MSB_INDEX(inteDown);
            if (POW2[obstacle] & stra){
                ++num;
                res |= getDownMoves(lsb) ^ getDownMoves(obstacle);
            }
        }
        if (inteRight){
            obstacle = MSB_INDEX(inteRight);
            if (POW2[obstacle] & stra){
                ++num;
                res |= getRightMoves(lsb) ^ getRightMoves(obstacle);
            }
        }
        if (inteLeft){
            obstacle = LSB_INDEX(inteLeft);
            if (POW2[obstacle] & stra){
                ++num;
                res |= getLeftMoves(lsb) ^ getLeftMoves(obstacle);
            }
        }
    }

    if (diag)
    {
        const uint64_t inteUpRight = getUpRightMoves(lsb) & b->allPieces;
        const uint64_t inteUpLeft = getUpLeftMoves(lsb) & b->allPieces;
        const uint64_t inteDownRight = getDownRightMoves(lsb) & b->allPieces;
        const uint64_t inteDownLeft = getDownLeftMoves(lsb) & b->allPieces;

        if (inteUpRight){
            obstacle = LSB_INDEX(inteUpRight);
            if (POW2[obstacle] & diag){
                ++num;
                res |= getUpRightMoves(lsb) ^ getUpRightMoves(obstacle);
            }
        }
        
        if (inteUpLeft){
            obstacle = LSB_INDEX(inteUpLeft);
            if (POW2[obstacle] & diag){
                ++num;
                res |= getUpLeftMoves(lsb) ^ getUpLeftMoves(obstacle);
            }
        }
        if (inteDownRight){
            obstacle = MSB_INDEX(inteDownRight);
            if (POW2[obstacle] & diag){
                ++num;
                res |= getDownRightMoves(lsb) ^ getDownRightMoves(obstacle);
            }
        }
        if (inteDownLeft){
            obstacle = MSB_INDEX(inteDownLeft);
            if (POW2[obstacle] & diag){
                ++num;
                res |= getDownLeftMoves(lsb) ^ getDownLeftMoves(obstacle);
            }
        }
    }

    return (AttacksOnK) {.tiles = res, .num = num};
}

int checkInPosition(Board* b, const int lsb, const int kingsColor)
{
    const int inverse = 1 ^ kingsColor;

    if (b->piece[inverse][KING] & getKingMoves(lsb)) return 1;
    if (b->piece[inverse][PAWN] & kingPawn(lsb, kingsColor)) return 1;
    if (b->piece[inverse][KNIGHT] & getKnightMoves(lsb)) return 1;
    
    const uint64_t stra = b->piece[inverse][QUEEN] | b->piece[inverse][ROOK];
    if (stra && (stra & straight(lsb, b->allPieces))) return 1;

    const uint64_t diag = b->piece[inverse][QUEEN] | b->piece[inverse][BISH];
    if (diag && (diag & diagonal(lsb, b->allPieces))) return 1;

    return 0;
}
inline int isInCheck(Board* b, const int kingsColor)
{
    return checkInPosition(b, LSB_INDEX(b->piece[kingsColor][KING]), kingsColor);
}