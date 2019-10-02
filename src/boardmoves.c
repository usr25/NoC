/* boardmoves.c
 * In charge of making changes to the actual board.
 * makeMove, undoMove are the most important functions
 */

#include "../include/global.h"
#include "../include/memoization.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/magic.h"
#include "../include/boardmoves.h"

#include <assert.h>

/* Returns the piece in the determined sqr, pos has to be a bitboard (POW2)
 * Returns NO_PIECE (-1) if there is no piece
 */
inline int pieceAt(Board* const b, const uint64_t pos, const int color)
{
    if (pos & b->piece[color][PAWN])     return PAWN;
    else if (pos & b->piece[color][ROOK]) return ROOK;
    else if (pos & b->piece[color][BISH]) return BISH;
    else if (pos & b->piece[color][KNIGHT]) return KNIGHT;
    else if (pos & b->piece[color][QUEEN]) return QUEEN;
    else if (pos & b->piece[color][KING]) return KING;

    return NO_PIECE;
}

static inline int kingMoved(const int color)
{
    return ~ (3 << (color << 1));
}
static inline int rookMoved(const int color, const int from)
{
    if (from == 56 * (1 ^ color))
        return ~ (1 << (color << 1));

    else if (from == 56 * (1 ^ color) + 7)
        return ~ (2 << (color << 1));

    return 0b1111;
}

static inline void flipBits(Board* b, const uint64_t from, const int piece, const int color)
{
    b->piece[color][piece] ^= from;
    b->color[color] ^= from;
    b->color[color | 2] ^= from;
}

/* Flips the necessary bits for castling
 * PRE: The castling direction has been decided and saved to move.castle
 */
static inline void flipCastle(Board* b, const Move move, const int color)
{
    uint64_t fromRook, toRook;

    if (move.castle & 1) //Kingside
    {
        fromRook = POW2[move.to - 1];
        toRook = POW2[move.to + 1];
    }
    else //Queenside
    {
        fromRook = POW2[move.to + 2];
        toRook = POW2[move.to - 1];
    }

    flipBits(b, fromRook, ROOK, color);
    flipBits(b, POW2[move.to], KING, color);
    flipBits(b, toRook, ROOK, color);
}

/* Makes the actual move and saves the data into h to undo it later
 */
void makeMove(Board* b, const Move move, History* h)
{
    assert(move.from >= 0 && move.from < 64);
    assert(move.to >= 0 && move.to < 64);
    assert(move.piece >= KING && move.piece <= PAWN);

    const uint64_t fromBit = POW2[move.from], toBit = POW2[move.to];

    //Save the data
    h->castleInfo = b->castleInfo;
    h->allPieces = b->allPieces;
    h->enPass = b->enPass;
    h->fifty = b->fifty;

    b->enPass = 0;

    flipBits(b, fromBit, move.piece, b->turn);
    switch(move.piece)
    {
        case PAWN:
            if (move.to - move.from == (2 * b->turn - 1) * 16)
                b->enPass = move.to;
            if (move.enPass)
            {
                flipBits(b, toBit, PAWN, b->turn);
                flipBits(b, POW2[move.enPass], PAWN, 1 ^ b->turn);
            }
            else
            {
                if (move.promotion)
                    flipBits(b, toBit, move.promotion, b->turn);
                else
                    flipBits(b, toBit, PAWN, b->turn);
            }
            b->fifty = 0;
        break;

        case ROOK:
            if ((fromBit | toBit) & 0x8100000000000081ULL) //to reduce the number of calls
                b->castleInfo &= rookMoved(b->turn, move.from) & rookMoved(b->turn, move.to);
            flipBits(b, toBit, ROOK, b->turn);
            b->fifty++;
        break;

        case KING:
            if (move.castle)
                flipCastle(b, move, b->turn);
            else
                flipBits(b, toBit, KING, b->turn);

            b->castleInfo &= kingMoved(b->turn);
            b->fifty++;
        break;

        default:
            flipBits(b, toBit, move.piece, b->turn);
            b->fifty++;
        break;
    }

    b->turn ^= 1;

    if (move.capture > 0)
    {
        flipBits(b, toBit, move.capture, b->turn);
        b->fifty = 0;
    }

    b->allPieces = b->color[WHITE] | b->color[BLACK];
}

/* Equivalent to makeMove but doesn't need History, since it won't be undone
 */
void makePermaMove(Board* b, const Move move)
{
    assert(move.from >= 0 && move.from < 64);
    assert(move.to >= 0 && move.to < 64);
    assert(move.piece >= KING && move.piece <= PAWN);

    const uint64_t fromBit = POW2[move.from], toBit = POW2[move.to];

    b->enPass = 0;

    flipBits(b, fromBit, move.piece, b->turn);
    switch(move.piece)
    {
        case PAWN:
            if (move.to - move.from == (2 * b->turn - 1) << 4)
                b->enPass = move.to;
            if (move.enPass)
            {
                flipBits(b, toBit, PAWN, b->turn);
                flipBits(b, POW2[move.enPass], PAWN, 1 ^ b->turn);
            }
            else
            {
                if (move.promotion)
                    flipBits(b, toBit, move.promotion, b->turn);
                else
                    flipBits(b, toBit, PAWN, b->turn);
            }
            b->fifty = 0;
        break;

        case ROOK:
            if ((fromBit | toBit) & 0x8100000000000081ULL) //to reduce the number of calls
                b->castleInfo &= rookMoved(b->turn, move.from) & rookMoved(b->turn, move.to);
            flipBits(b, toBit, ROOK, b->turn);
            b->fifty++;
        break;

        case KING:
            if (move.castle)
                flipCastle(b, move, b->turn);
            else
                flipBits(b, toBit, KING, b->turn);

            b->castleInfo &= kingMoved(b->turn);
            b->fifty++;
        break;

        default:
            flipBits(b, toBit, move.piece, b->turn);
            b->fifty++;
        break;
    }

    b->turn ^= 1;

    if (move.capture > 0)
    {
        flipBits(b, toBit, move.capture, b->turn);
        b->fifty = 0;
    }

    b->allPieces = b->color[WHITE] | b->color[BLACK];
}

/* Alters the board to undo a move, (undo . make) should be the identity function 
 */
void undoMove(Board* b, const Move move, History* h)
{
    const uint64_t fromBit = POW2[move.from], toBit = POW2[move.to];

    b->castleInfo = h->castleInfo;
    b->allPieces = h->allPieces;
    b->enPass = h->enPass;
    b->fifty = h->fifty;

    if (move.capture > 0)
        flipBits(b, toBit, move.capture, b->turn);

    b->turn ^= 1;

    flipBits(b, fromBit, move.piece, b->turn);
    switch(move.piece)
    {
        case PAWN:
            if (move.enPass)
            {
                flipBits(b, toBit, PAWN, b->turn);
                flipBits(b, POW2[move.enPass], PAWN, 1 ^ b->turn);
            }
            else
            {
                if (move.promotion)
                    flipBits(b, toBit, move.promotion, b->turn);
                else
                    flipBits(b, toBit, PAWN, b->turn);
            }
        break;

        case KING:
            if (move.castle)
                flipCastle(b, move, b->turn);
            else
                flipBits(b, toBit, KING, b->turn);
        break;

        default:
            flipBits(b, toBit, move.piece, b->turn);
        break;
    }
}

/* Returns if a move is legal in the given position
 * PRE: The move hasnt been applied to the board and is a valid move
 * It currently doesn't work with enPass or castle, they are returned as false (0)
 */
int isValid(Board b, const Move m)
{
    uint64_t toBB = POW2[m.to], fromBB = POW2[m.from];
    if ((fromBB & b.color[b.turn]) == 0)
        return 0;
    if (pieceAt(&b, fromBB, b.turn) == NO_PIECE)
        return 0;
    if (m.capture > 0 && pieceAt(&b, toBB, 1 ^ b.turn) != m.capture)
        return 0;
    //Look at the move
    switch (m.piece)
    {
        case PAWN:
            if ((toBB & posPawnMoves(&b, b.turn, m.from)) == 0)
                return 0;
            break;
        case KNIGHT:
            if ((toBB & getKnightMoves(m.from)) == 0)
                return 0;
            break;
        case BISH:
            if ((toBB & getBishMagicMoves(m.from, b.allPieces)) == 0)
                return 0;
            break;
        case ROOK:
            if ((toBB & getRookMagicMoves(m.from, b.allPieces)) == 0)
                return 0;
            break;
        case QUEEN:
            if ((toBB & (getRookMagicMoves(m.from, b.allPieces) | getBishMagicMoves(m.from, b.allPieces))) == 0)
                return 0;
            break;
        case KING:
            if ((toBB & getKingMoves(m.from)) == 0)
                return 0;
            break;

        default:
            return 0;
    }

    //To avoid other errors, see if the king is in check
    makePermaMove(&b, m);
    return !isInCheck(&b, 1 ^ b.turn);
}