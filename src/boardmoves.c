#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"

//This file makes changes to the board, moves.c generates the moves themselves

//returns the piece CAPTURED
//returns -1 otherwise
int pieceAt(Board* const b, const uint64_t pos, const int color)
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

static inline void moveTo(Board* b, const uint64_t to, const int piece, const int color)
{
    b->piece[color][piece] |= to;
    b->color[color] |= to;
    b->color[color | 2] ^= to;
}
static inline void moveFrom(Board* b, const uint64_t from, const int piece, const int color)
{
    b->piece[color][piece] ^= from;
    b->color[color] ^= from;
    b->color[color | 2] |= from;
}

//It is assumed that the castling direction has already been decided, and is placed in move.castle (1 -> kingside, 2-> queenside)
static void makeCastle(Board* b, Move move, const int color)
{
    uint64_t fromRook, toRook;

    b->posInfo &= kingMoved(color);

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

    moveFrom(b, POW2[move.from], KING, color);
    moveFrom(b, fromRook, ROOK, color);
    moveTo(b, POW2[move.to], KING, color);
    moveTo(b, toRook, ROOK, color);
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

    moveFrom(b, toKing, KING, color);
    moveFrom(b, toRook, ROOK, color);
    moveTo(b, fromKing, KING, color);
    moveTo(b, fromRook, ROOK, color);
}

void makeMove(Board* b, Move move, History* h)
{
    //Save the data
    h->posInfo = b->posInfo;
    h->allPieces = b->allPieces;
    h->enPass = b->enPass;

    if (move.pieceThatMoves == PAWN && move.to - move.from == (2 * b->turn - 1) * 16)
        b->enPass = move.to;
    else
        b->enPass = 0;

    if (move.enPass)
    {
        moveFrom(b, POW2[move.from], PAWN, b->turn);
        moveTo(b, POW2[move.to], PAWN, b->turn);
        
        moveFrom(b, POW2[move.enPass], PAWN, 1 ^ b->turn);
    }
    else if (move.castle)
    {
        makeCastle(b, move, b->turn);
    }
    else
    {
        //To remove ability to castle
        if (move.pieceThatMoves == KING)
            b->posInfo &= kingMoved(b->turn);
        if (move.pieceThatMoves == ROOK)
            b->posInfo &= rookMoved(b->turn, move.from);

        const uint64_t toBit = POW2[move.to];

        moveFrom(b, POW2[move.from], move.pieceThatMoves, b->turn);

        if (move.promotion && move.pieceThatMoves == PAWN)
            moveTo(b, toBit, move.promotion, b->turn);
        else
            moveTo(b, toBit, move.pieceThatMoves, b->turn);

        h->pieceCaptured = pieceAt(b, toBit, 1 ^ b->turn);
        if (h->pieceCaptured != NO_PIECE)
        {
            moveFrom(b, toBit, h->pieceCaptured, 1 ^ b->turn);

            if (h->pieceCaptured == ROOK && move.to == 56 * b->turn)
                b->posInfo &= 0b111110 ^ (1 << (((1 ^ b->turn) << 1) + 1));
            else if (h->pieceCaptured == ROOK && move.to == 56 * b->turn + 7)
                b->posInfo &= 0b111110 ^ (2 << (((1 ^ b->turn) << 1) + 1));
        }
    }
    
    b->allPieces = b->color[WHITE] | b->color[BLACK];
    b->turn ^= 1;
}

void undoMove(Board* b, Move move, History* h)
{
    //Unload the data
    b->posInfo = h->posInfo;
    b->allPieces = h->allPieces;
    b->enPass = h->enPass;
    
    b->turn ^= 1;
    
    if (move.castle)
    {
        undoCastle(b, move, b->turn);
    }
    else if(move.enPass)
    {
        moveFrom(b, POW2[move.to], PAWN, b->turn);
        moveTo(b, POW2[move.from], PAWN, b->turn);

        moveTo(b, POW2[move.enPass], PAWN, 1 ^ b->turn);
    }
    else
    {
        const uint64_t toBit = POW2[move.to];

        moveTo(b, POW2[move.from], move.pieceThatMoves, b->turn);

        if (move.promotion && move.pieceThatMoves == PAWN)
            moveFrom(b, toBit, move.promotion, b->turn);
        else
            moveFrom(b, toBit, move.pieceThatMoves, b->turn);

        if (h->pieceCaptured != NO_PIECE)
            moveTo(b, toBit, h->pieceCaptured, 1 ^ b->turn);
    }
}