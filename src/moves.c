/* moves.c
 * In charge of calculating all the moves for each piece in a given position, checks, 
 * castles, the ability to castle and the attacks on the king
 */

#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/memoization.h"
#include "../include/io.h"
#include "../include/magic.h"

/* How it works:
 * 1- Get all the relevant bits for the attack using a mask getMoveTypeInt(index) & allPieces, it is not neccessary to include the bits of the blocking pieces in the mask
 * 2- Multiply * magicType[index]
 * 3- Bitshift the result 64 - 12 for rook and 64 - 9 for bish
 * 4- Refer to the respective array and return getTypeMoves[index][multShifted]
 * The return WILL include the blocking sqrs, so that the mask b.color[opp] can be applied
 * to include the opp pieces for captures
 */

inline uint64_t posKingMoves(Board* b, const int color)
{
    return b->color[color | 2] & getKingMoves(LSB_INDEX(b->piece[color][KING]));
}
inline uint64_t posKnightMoves(Board* b, const int color, const int lsb)
{
    return b->color[color | 2] & getKnightMoves(lsb);
}
inline uint64_t posRookMoves(Board* b, const int color, const int lsb)
{
    return b->color[color | 2] & getRookMagicMoves(lsb, b->allPieces);
}
inline uint64_t posBishMoves(Board* b, const int color, const int lsb)
{
    return b->color[color | 2] & getBishMagicMoves(lsb, b->allPieces);
}
inline uint64_t posQueenMoves(Board* b, const int color, const int lsb)
{
    return b->color[color | 2] & (getRookMagicMoves(lsb, b->allPieces) | getBishMagicMoves(lsb, b->allPieces));
}
/*
 * All the possible pawn moves without including enPass
 */
uint64_t posPawnMoves(Board* b, const int color, const int lsb)
{
    if (color)
    {
        if (POW2[lsb + 8] & b->allPieces) //If there is a piece blocking the square ahead, to avoid problems when it can move 2 sqrs
            return getWhitePawnCaptures(lsb) & b->color[BLACK];

        return (getWhitePawnMoves(lsb) & ~b->allPieces) | (getWhitePawnCaptures(lsb) & b->color[BLACK]);
    }
    else
    {
        if (POW2[lsb - 8] & b->allPieces) //If there is a piece blocking the square ahead, to avoid problems when it can move 2 sqrs
            return getBlackPawnCaptures(lsb) & b->color[WHITE];

        return (getBlackPawnMoves(lsb) & ~b->allPieces) | (getBlackPawnCaptures(lsb) & b->color[WHITE]);
    }
}

/*
 * All the sliding pieces movements are calculated by checking if there is an
 * intersection with the move direction in the board, if there is, remove all
 * the following tiles.
 * The piece blocking is included in the resulting bitboard
 * Eg.: Q - - r - -  => Q 1 1 r(1) 0 0
 */
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

static inline uint64_t pawnCaptures(const int lsb, const int color)
{
    return color ? getWhitePawnCaptures(lsb) : getBlackPawnCaptures(lsb);
}

/*
 * Determines if the king can castle
 * This function does NOT determine if the king is in check, since it is only called from genCheckMoves
 */
int canCastle(Board* b, const int color, const uint64_t forbidden)
{
    int canK, canQ;

    uint64_t obstacles = b->allPieces | forbidden;

    if (color)
    {
        canK = (b->castleInfo & WCASTLEK) && (1    & b->piece[color][ROOK]) && ! (obstacles & C_MASK_WK);
        canQ = (b->castleInfo & WCASTLEQ) && (0x80 & b->piece[color][ROOK]) && ! (obstacles & 0x30) && ! (b->allPieces & C_MASK_WQ);
    }
    else
    {
        canK = (b->castleInfo & BCASTLEK) && (0x100000000000000ULL  & b->piece[color][ROOK]) && ! (obstacles & C_MASK_BK);
        canQ = (b->castleInfo & BCASTLEQ) && (0x8000000000000000ULL & b->piece[color][ROOK]) && ! (obstacles & 0x3000000000000000ULL) && ! (b->allPieces & C_MASK_BQ);
    }

    return (canQ << 1) | canK;
}
inline Move castleKSide(const int color)
{
    return (Move) {.piece = KING, .from = 56 * (1 ^ color) + 3, .to = 56 * (1 ^ color) + 1, .castle = 1};
}
inline Move castleQSide(const int color)
{ 
    return (Move) {.piece = KING, .from = 56 * (1 ^ color) + 3, .to = 56 * (1 ^ color) + 5, .castle = 2};
}

//Tiles controlled by the opp king / pawns / knights
uint64_t controlledKingPawnKnight(Board* b, const int opp)
{
    uint64_t temp;
    uint64_t res = getKingMoves(LSB_INDEX(b->piece[opp][KING]));

    if (opp)
        res |= ((b->piece[WHITE][PAWN] << 9) & 0xfefefefefefefefe) | ((b->piece[WHITE][PAWN] << 7) & 0x7f7f7f7f7f7f7f7f);
    else
        res |= ((b->piece[BLACK][PAWN] >> 9) & 0x7f7f7f7f7f7f7f7f) | ((b->piece[BLACK][PAWN] >> 7) & 0xfefefefefefefefe);

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
    
    temp = b->piece[color][QUEEN];
    while(temp)
    {
        res |= getRookMagicMoves(LSB_INDEX(temp), obstacles) | getBishMagicMoves(LSB_INDEX(temp), obstacles);
        REMOVE_LSB(temp);
    }
    temp = b->piece[color][ROOK];
    while(temp)
    {
        res |= getRookMagicMoves(LSB_INDEX(temp), obstacles);
        REMOVE_LSB(temp);
    }
    temp = b->piece[color][BISH];
    while(temp)
    {
        res |= getBishMagicMoves(LSB_INDEX(temp), obstacles);
        REMOVE_LSB(temp);
    }

    return res;
}

/*Any piece placed in one of this tiles will stop a check, either by capturing or by pinning itself
 */
AttacksOnK getCheckTiles(Board* b, const int color)
{
    const int opp = 1 ^ color;
    const int lsb = LSB_INDEX(b->piece[color][KING]);

    uint64_t res = 
        (b->piece[opp][PAWN] & pawnCaptures(lsb, color)) | (b->piece[opp][KNIGHT] & getKnightMoves(lsb));

    int num = res != 0;

    uint64_t stra = (b->piece[opp][QUEEN] | b->piece[opp][ROOK]) & getRookMagicMoves(lsb, b->allPieces);
    uint64_t diag = (b->piece[opp][QUEEN] | b->piece[opp][BISH]) & getBishMagicMoves(lsb, b->allPieces);

    if (stra)
    {
        const uint64_t inteUp = getUpMoves(lsb) & stra;
        const uint64_t inteDown = getDownMoves(lsb) & stra;
        const uint64_t inteRight = getRightMoves(lsb) & stra;
        const uint64_t inteLeft = getLeftMoves(lsb) & stra;

        if (inteUp)
        {
            ++num;
            res |= getUpMoves(lsb) ^ getUpMoves(LSB_INDEX(inteUp));
        }
        if (inteDown)
        {
            ++num;
            res |= getDownMoves(lsb) ^ getDownMoves(LSB_INDEX(inteDown));
        }
        if (inteRight)
        {
            ++num;
            res |= getRightMoves(lsb) ^ getRightMoves(LSB_INDEX(inteRight));
        }
        if (inteLeft)
        {
            ++num;
            res |= getLeftMoves(lsb) ^ getLeftMoves(LSB_INDEX(inteLeft));
        }
    }

    if (diag && num < 2)
    {
        const uint64_t inteUpRight = getUpRightMoves(lsb) & diag;
        const uint64_t inteUpLeft = getUpLeftMoves(lsb) & diag;
        const uint64_t inteDownRight = getDownRightMoves(lsb) & diag;
        const uint64_t inteDownLeft = getDownLeftMoves(lsb) & diag;

        if (inteUpRight)
        {
            ++num;
            res |= getUpRightMoves(lsb) ^ getUpRightMoves(LSB_INDEX(inteUpRight));
        }
        if (inteUpLeft)
        {
            ++num;
            res |= getUpLeftMoves(lsb) ^ getUpLeftMoves(LSB_INDEX(inteUpLeft));
        }
        if (inteDownRight)
        {
            ++num;
            res |= getDownRightMoves(lsb) ^ getDownRightMoves(LSB_INDEX(inteDownRight));
        }
        if (inteDownLeft)
        {
            ++num;
            res |= getDownLeftMoves(lsb) ^ getDownLeftMoves(LSB_INDEX(inteDownLeft));
        }
    }

    return (AttacksOnK) {.tiles = res, .num = num};
}

inline int slidingCheck(Board* b, const int kingsColor)
{
    const int lsb = LSB_INDEX(b->piece[kingsColor][KING]);

    uint64_t stra = b->piece[1 ^ kingsColor][QUEEN] | b->piece[1 ^ kingsColor][ROOK];
    if (stra && (stra & getRookMagicMoves(lsb, b->allPieces))) return 1;

    uint64_t diag = b->piece[1 ^ kingsColor][QUEEN] | b->piece[1 ^ kingsColor][BISH];
    if (diag && (diag & getBishMagicMoves(lsb, b->allPieces))) return 1;

    return 0;
}

inline int isInCheck(Board* b, const int kingsColor)
{
    const int lsb = LSB_INDEX(b->piece[kingsColor][KING]);
    const int opp = 1 ^ kingsColor;

    if (b->piece[opp][KING] & getKingMoves(lsb)) return 1;
    if (b->piece[opp][PAWN] & pawnCaptures(lsb, kingsColor)) return 1;
    if (b->piece[opp][KNIGHT] & getKnightMoves(lsb)) return 1;
    
    uint64_t stra = b->piece[opp][QUEEN] | b->piece[opp][ROOK];
    if (stra && (stra & getRookMagicMoves(lsb, b->allPieces))) return 1;

    uint64_t diag = b->piece[opp][QUEEN] | b->piece[opp][BISH];
    if (diag && (diag & getBishMagicMoves(lsb, b->allPieces))) return 1;

    return 0;
}