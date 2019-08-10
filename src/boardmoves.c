/* boardmoves.c
 * In charge of making changes to the actual board.
 * makeMove, undoMove are the most important functions
 */

#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"

/*
 * Returns the piece in the determined sqr, pos has to be a bitboard (POW2)
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

    return 0b111111;
}

static inline void flipBits(Board* b, const uint64_t from, const int piece, const int color)
{
    b->piece[color][piece] ^= from;
    b->color[color] ^= from;
    b->color[color | 2] ^= from;
}

//It is assumed that the castling direction has already been decided, and is placed in move.castle (1 -> kingside, 2-> queenside)
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

//Alters the board to make a move
void makeMove(Board* b, const Move move, History* h)
{
    const uint64_t fromBit = POW2[move.from], toBit = POW2[move.to];

    //Save the data
    h->castleInfo = b->castleInfo;
    h->allPieces = b->allPieces;
    h->enPass = b->enPass;
    h->fifty = b->fifty;

    b->enPass = 0;

    flipBits(b, fromBit, move.pieceThatMoves, b->turn);
    switch(move.pieceThatMoves)
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
            b->fifty++;
            flipBits(b, toBit, move.pieceThatMoves, b->turn);
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

//Alters the board to undo a move, (undo . make) should be the identity function 
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

    flipBits(b, fromBit, move.pieceThatMoves, b->turn);
    switch(move.pieceThatMoves)
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
            flipBits(b, toBit, move.pieceThatMoves, b->turn);
        break;
    }
}