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

int capturePiece(Board* b, const uint64_t pos, const int colorToCapture)
{
    const int targetPiece = pieceAt(b, pos, colorToCapture);

    if (targetPiece != NO_PIECE)
        b->piece[colorToCapture][targetPiece] ^= pos;

    return targetPiece;
}

static inline int kingMoved(const int color)
{
    return 0b111111 ^ (3 << ((color << 1) + 1));
}
static inline int rookMoved(const int color, const int from)
{
    if (from == 56 * (1 ^ color))
    {
        return 0b111111 ^ (1 << ((color << 1) + 1));
    }
    else if (from == 56 * (1 ^ color) + 7)
    {
        return 0b111111 ^ (2 << ((color << 1) + 1));
    }

    return 0b111111;
}

//It is assumed that the castling direction has already been decided, and is placed in move.castle (1 -> kingside, 2-> queenside)
void makeCastle(Board* b, Move move, const int color)
{
    uint64_t fromRook, toRook;
    const uint64_t fromKing = POW2[move.from], toKing = POW2[move.to];

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

    b->piece[color][KING] = toKing;
    
    b->piece[color][ROOK] |= toRook;
    b->piece[color][ROOK] ^= fromRook;

    b->color[color] |= toKing | toRook;
    b->color[color] ^= fromKing | fromRook;

    b->color[color | 2] = ALL ^ b->color[color];
    
    b->allPieces = b->color[WHITE] | b->color[BLACK];
}
void undoCastle(Board* b, Move move, const int color)
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

    b->piece[color][KING] = fromKing;
    
    b->piece[color][ROOK] |= fromRook;
    b->piece[color][ROOK] ^= toRook;

    b->color[color] |= fromKing | fromRook;
    b->color[color] ^= toKing | toRook;
    b->color[color | 2] = ALL ^ b->color[color];
}

void makePassand(Board* b, Move move, const int color)
{
    const uint64_t fromBit = POW2[move.from], toBit = POW2[move.to];
    const uint64_t pawnPos = POW2[move.enPass];
    
    //Move pawn
    b->piece[color][PAWN] |= toBit;
    b->piece[color][PAWN] ^= fromBit;

    b->color[color] |= toBit;
    b->color[color] ^= fromBit;
    b->color[color | 2] = ALL ^ b->color[color];

    //Capture pawn
    b->piece[1 ^ color][PAWN] ^= pawnPos;
    
    b->color[1 ^ color] ^= pawnPos;
    b->color[3 - color] |= pawnPos;

    b->allPieces = b->color[WHITE] | b->color[BLACK];
}
void undoPassand(Board* b, Move move, const int color)
{
    const uint64_t fromBit = POW2[move.from], toBit = POW2[move.to];
    const uint64_t pawnPos = POW2[move.enPass];
    
    //Pawned that moved
    b->piece[color][PAWN] ^= toBit;
    b->piece[color][PAWN] |= fromBit;

    b->color[color] ^= toBit;
    b->color[color] |= fromBit;
    b->color[color | 2] = ALL ^ b->color[color];

    //Captured pawn
    b->piece[1 ^ color][PAWN] |= pawnPos;
    
    b->color[1 ^ color] |= pawnPos;
    b->color[3 - color] ^= pawnPos;
}

void makeMove(Board* b, Move move, History* h)
{
    //Save the data
    h->posInfo = b->posInfo;
    h->allPieces = b->allPieces;
    h->enPass = b->enPass;

    if (move.pieceThatMoves == PAWN && move.to - move.from == (2 * h->color - 1) * 16)
        b->enPass = move.to;
    else
        b->enPass = 0;

    //En pass
    if (move.enPass)
    {
        makePassand(b, move, h->color);
        return;
    }
    //Castling
    else if (move.castle)
    {
        makeCastle(b, move, h->color);
        return;
    }

    //To remove ability to castle
    if (move.pieceThatMoves == KING)
        b->posInfo &= kingMoved(h->color);
    if (move.pieceThatMoves == ROOK)
        b->posInfo &= rookMoved(h->color, move.from);

    const uint64_t fromBit = POW2[move.from], toBit = POW2[move.to];

    //Promotion
    if (move.promotion && move.pieceThatMoves == PAWN)
        b->piece[h->color][move.promotion] |= toBit;
    else
        b->piece[h->color][move.pieceThatMoves] |= toBit;

    b->piece[h->color][move.pieceThatMoves] ^= fromBit;
    
    b->allPieces ^= fromBit;

    b->color[h->color] ^= fromBit;
    b->color[h->color] |= toBit;
    b->color[h->color | 2] = ALL ^ b->color[h->color];
    
    h->pieceCaptured = capturePiece(b, toBit, 1 ^ h->color);
    if (h->pieceCaptured != NO_PIECE)
    {
        b->color[1 ^ h->color] ^= toBit;
        b->color[3 - h->color] |= toBit;
        if (h->pieceCaptured == ROOK && move.to == 56 * h->color)
            b->posInfo &= rookMoved(1 ^ h->color, move.to);
        else if (h->pieceCaptured == ROOK && move.to == 56 * h->color + 7)
            b->posInfo &= rookMoved(1 ^ h->color, move.to);
    }
    //else
    //    b->allPieces |= toBit;
    
    b->allPieces = b->color[WHITE] | b->color[BLACK];
}

void undoMove(Board* b, Move move, History* h)
{
    //Unload the data
    b->posInfo = h->posInfo;
    b->allPieces = h->allPieces;
    b->enPass = h->enPass;
    
    if (move.castle)
    {
        undoCastle(b, move, h->color);
        return;
    }
    else if(move.enPass)
    {
        undoPassand(b, move, h->color);
        return;
    }
    const uint64_t fromBit = POW2[move.from], toBit = POW2[move.to];

    if (move.promotion && move.pieceThatMoves == PAWN)
        b->piece[h->color][move.promotion] ^= toBit;
    else
        b->piece[h->color][move.pieceThatMoves] ^= toBit;

    b->piece[h->color][move.pieceThatMoves] |= fromBit;

    b->color[h->color] |= fromBit;
    b->color[h->color] ^= toBit;
    
    b->color[h->color | 2] = ALL ^ b->color[h->color];//

    if (h->pieceCaptured != NO_PIECE)
    {
        b->piece[1 ^ h->color][h->pieceCaptured] |= toBit;
        b->color[1 ^ h->color] |= toBit;
        b->color[3 - h->color] ^= toBit;
    }
}