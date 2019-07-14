/* boardmoves.c
 * In charge of making changes to the actual board.
 * makeMove, undoMove are the most important functions
 */

#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"

//This file makes changes to the board, moves.c generates the moves themselves

//returns the piece CAPTURED
//returns -1 otherwise
inline int pieceAt(Board* const b, const uint64_t pos, const int color)
{
    if (pos & b->allPieces)
    {
        if (pos & b->piece[color][PAWN])     return PAWN;
        else if (pos & b->piece[color][ROOK]) return ROOK;
        else if (pos & b->piece[color][BISH]) return BISH;
        else if (pos & b->piece[color][KNIGHT]) return KNIGHT;
        else if (pos & b->piece[color][QUEEN]) return QUEEN;
        else if (pos & b->piece[color][KING]) return KING;
    }
    
    return NO_PIECE;
}

static inline int kingMoved(const int color)
{
    return 0b111111 ^ (3 << ((color << 1) + 1));
}
static inline int rookMoved(const int color, const int from)
{
    if (from == 56 * (1 ^ color))
        return 0b111111 ^ (1 << ((color << 1) + 1));

    else if (from == 56 * (1 ^ color) + 7)
        return 0b111111 ^ (2 << ((color << 1) + 1));

    return 0b111111;
}

static inline void setBit(Board* b, const uint64_t to, const int piece, const int color)
{
    b->piece[color][piece] |= to;
    b->color[color] |= to;
    b->color[color | 2] ^= to;
}
static inline void unsetBit(Board* b, const uint64_t from, const int piece, const int color)
{
    b->piece[color][piece] ^= from;
    b->color[color] ^= from;
    b->color[color | 2] |= from;
}

//It is assumed that the castling direction has already been decided, and is placed in move.castle (1 -> kingside, 2-> queenside)
static void makeCastle(Board* b, Move move, const int color)
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

    unsetBit(b, fromRook, ROOK, color);
    setBit(b, POW2[move.to], KING, color);
    setBit(b, toRook, ROOK, color);
}
static void undoCastle(Board* b, Move move, const int color)
{
    uint64_t fromRook, toRook;
    const uint64_t fromKing = POW2[move.from], toKing = POW2[move.to];

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

    unsetBit(b, toKing, KING, color);
    unsetBit(b, toRook, ROOK, color);
    setBit(b, fromRook, ROOK, color);
}

//Alters the board to make a move
void makeMove(Board* b, Move move, History* h)
{
    const uint64_t fromBit = POW2[move.from], toBit = POW2[move.to];

    //Save the data
    h->posInfo = b->posInfo;
    h->allPieces = b->allPieces;
    h->enPass = b->enPass;

    unsetBit(b, fromBit, move.pieceThatMoves, b->turn);
    b->enPass = 0;
    switch(move.pieceThatMoves)
    {
        case PAWN:
        if (move.to - move.from == (2 * b->turn - 1) << 4)
            b->enPass = move.to;
        if (move.enPass)
        {
            setBit(b, toBit, PAWN, b->turn);
            
            unsetBit(b, POW2[move.enPass], PAWN, 1 ^ b->turn);
        }
        else
        {
            if (move.promotion)
                setBit(b, toBit, move.promotion, b->turn);
            else
                setBit(b, toBit, PAWN, b->turn);
        }
        break;

        case KING:
        if (move.castle)
            makeCastle(b, move, b->turn);
        else
            setBit(b, toBit, KING, b->turn);

        b->posInfo &= kingMoved(b->turn);
        break;
        
        case ROOK:
        b->posInfo &= rookMoved(b->turn, move.from) & rookMoved(b->turn, move.to);

        default:
        setBit(b, toBit, move.pieceThatMoves, b->turn);
        break;
    }

    if (move.capture && move.capture != NO_PIECE)
        unsetBit(b, toBit, move.capture, 1 ^ b->turn);
    
    b->allPieces = b->color[WHITE] | b->color[BLACK];
    b->turn ^= 1;
}

//Alters the board to undo a move, (undo . make) should be the identity function 
void undoMove(Board* b, Move move, History* h)
{
    const uint64_t fromBit = POW2[move.from], toBit = POW2[move.to];

    b->posInfo = h->posInfo;
    b->allPieces = h->allPieces;
    b->enPass = h->enPass;
    
    b->turn ^= 1;

    setBit(b, fromBit, move.pieceThatMoves, b->turn);
    switch(move.pieceThatMoves)
    {
        case PAWN:
        if (move.enPass)
        {
            unsetBit(b, toBit, PAWN, b->turn);
            
            setBit(b, POW2[move.enPass], PAWN, 1 ^ b->turn);
        }
        else
        {
            if (move.promotion)
                unsetBit(b, toBit, move.promotion, b->turn);
            else
                unsetBit(b, toBit, PAWN, b->turn);
        }
        break;

        case KING:
        if (move.castle)
            undoCastle(b, move, b->turn);
        else
            unsetBit(b, toBit, KING, b->turn);
        break;

        default:
        unsetBit(b, toBit, move.pieceThatMoves, b->turn);
        break;
    }

    if (move.capture && move.capture != NO_PIECE)
        setBit(b, toBit, move.capture, 1 ^ b->turn);  
}