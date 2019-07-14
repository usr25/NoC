/* moves.c
 * In charge of calculating all the moves for each piece in a given position, checks, 
 * castles, the ability to castle and the attacks on the king
 */

#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/memoization.h"
#include "../include/io.h"

inline uint64_t posKingMoves(Board* b, const int color)
{
    return getKingMoves(LSB_INDEX(b->piece[color][KING])) & b->color[color | 2];
}

inline uint64_t posKnightMoves(Board* b, const int color, const int lsb)
{
    return getKnightMoves(lsb) & b->color[color | 2];
}

uint64_t posPawnMoves(Board* b, const int color, const int lsb)
{
    uint64_t forward;
    if (color){
        if (POW2[lsb + 8] & b->allPieces)
            return getWhitePawnCaptures(lsb) & b->color[BLACK];

        forward = getWhitePawnMoves(lsb) & ~(b->color[WHITE] | b->color[BLACK]);
        return forward | (getWhitePawnCaptures(lsb) & b->color[BLACK]);
    }
    else{
        if (POW2[lsb - 8] & b->allPieces)
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
    const uint64_t inteUp = getUpMovesInt(lsb) & allPieces;
    const uint64_t inteDown = getDownMovesInt(lsb) & allPieces;
    const uint64_t inteRight = getRightMovesInt(lsb) & allPieces;
    const uint64_t inteLeft = getLeftMovesInt(lsb) & allPieces;
    
    uint64_t res = getStraMoves(lsb);   

    if (inteUp)
        res ^= getUpMoves(LSB_INDEX(inteUp));

    if (inteDown)
        res ^= getDownMoves(MSB_INDEX(inteDown));
    
    if (inteRight)
        res ^= getRightMoves(MSB_INDEX(inteRight));
    
    if (inteLeft)
        res ^= getLeftMoves(LSB_INDEX(inteLeft));

    return res;
}
uint64_t diagonal(const int lsb, const uint64_t allPieces)
{
    const uint64_t inteUpRight = getUpRightMovesInt(lsb) & allPieces;
    const uint64_t inteUpLeft = getUpLeftMovesInt(lsb) & allPieces;
    const uint64_t inteDownRight = getDownRightMovesInt(lsb) & allPieces;
    const uint64_t inteDownLeft = getDownLeftMovesInt(lsb) & allPieces;

    uint64_t res = getDiagMoves(lsb);

    if (inteUpRight)
        res ^= getUpRightMoves(LSB_INDEX(inteUpRight));
    
    if (inteUpLeft)
        res ^= getUpLeftMoves(LSB_INDEX(inteUpLeft));
    
    if (inteDownRight)
        res ^= getDownRightMoves(MSB_INDEX(inteDownRight));
    
    if (inteDownLeft)
        res ^= getDownLeftMoves(MSB_INDEX(inteDownLeft));

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
//Determines if the king can castle
//This function does NOT determine if the king is in check, since it isnt called from genCheckMoves
int canCastle(Board* b, const int color, const uint64_t forbidden) //Faster version, but forbidden has to be calculated
{
    int canK, canQ;

    uint64_t maskCheck = 0x30 * POW2[(1 ^ color) * 56];

    if (color)
    {
        canK = (b->posInfo & WCASTLEK) && (1   & b->piece[color][ROOK]) && ! (b->allPieces & C_MASK_WK) && ! (C_MASK_WK & forbidden);
        canQ = (b->posInfo & WCASTLEQ) && (128 & b->piece[color][ROOK]) && ! (b->allPieces & C_MASK_WQ) && ! (maskCheck & forbidden);
    }
    else
    {
        canK = (b->posInfo & BCASTLEK) && (POW2[56] & b->piece[color][ROOK]) && ! (b->allPieces & C_MASK_BK) && ! (C_MASK_BK & forbidden);
        canQ = (b->posInfo & BCASTLEQ) && (POW2[63] & b->piece[color][ROOK]) && ! (b->allPieces & C_MASK_BQ) && ! (maskCheck & forbidden);
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
uint64_t controlledKingPawnKnight(Board* b, const int opp)
{
    uint64_t temp;
    uint64_t res = getKingMoves(LSB_INDEX(b->piece[opp][KING]));

    temp = b->piece[opp][PAWN];
    while(temp)
    {
        res |= kingPawn(LSB_INDEX(temp), opp);
        REMOVE_LSB(temp);
    }
    temp = b->piece[opp][KNIGHT];
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
    const int opp = 1 ^ color;
    const int lsb = LSB_INDEX(b->piece[color][KING]);

    uint64_t res = 
        (b->piece[opp][PAWN] & kingPawn(lsb, color)) | (b->piece[opp][KNIGHT] & getKnightMoves(lsb));

    int num = POPCOUNT(res);
    int obstacle;

    uint64_t stra = b->piece[opp][QUEEN] | b->piece[opp][ROOK];
    uint64_t diag = b->piece[opp][QUEEN] | b->piece[opp][BISH];

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
    const int opp = 1 ^ kingsColor;

    if (b->piece[opp][KING] & getKingMoves(lsb)) return 1;
    if (b->piece[opp][PAWN] & kingPawn(lsb, kingsColor)) return 1;
    if (b->piece[opp][KNIGHT] & getKnightMoves(lsb)) return 1;
    
    const uint64_t stra = b->piece[opp][QUEEN] | b->piece[opp][ROOK];
    if (stra && (stra & straight(lsb, b->allPieces))) return 1;

    const uint64_t diag = b->piece[opp][QUEEN] | b->piece[opp][BISH];
    if (diag && (diag & diagonal(lsb, b->allPieces))) return 1;

    return 0;
}

inline int isInCheck(Board* b, const int kingsColor)
{
    return checkInPosition(b, LSB_INDEX(b->piece[kingsColor][KING]), kingsColor);
}
