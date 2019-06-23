#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"
#include "../include/io.h"

//This file makes changes to the board, moves.c generates the moves themselves

#include <stdio.h>

//returns the piece CAPTURED
//returns -1 otherwise
//TODO: Add history
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
    int targetPiece = pieceAt(b, pos, colorToCapture);

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
    uint64_t fromKing = POW2[move.from], toKing = POW2[move.to];

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
    b->color[color | 2] |= fromKing | fromRook;
    b->color[color | 2] ^= toKing | toRook;

    b->allPieces |= toKing | toRook;
    b->allPieces ^= fromKing | fromRook;
}
void undoCastle(Board* b, Move move, const int color)
{
    uint64_t fromRook, toRook;
    uint64_t fromKing = POW2[move.from], toKing = POW2[move.to];

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
    b->color[color | 2] |= toKing | toRook;
    b->color[color | 2] ^= fromKing | fromRook;
}

void makePassand(Board* b, Move move, const int color)
{
    uint64_t fromBit = POW2[move.from], toBit = POW2[move.to];
    uint64_t pawnPos = POW2[move.enPass];
    //Move pawn
    b->piece[color][PAWN] |= toBit;
    b->piece[color][PAWN] ^= fromBit;

    b->color[color] |= toBit;
    b->color[color] ^= fromBit;
    b->color[color | 2] |= fromBit;
    b->color[color | 2] ^= toBit;

    //Remove pawn
    b->piece[1 ^ color][PAWN] ^= pawnPos;
    
    b->color[1 ^ color] ^= pawnPos;
    b->color[3 - color] |= pawnPos;

    b->allPieces ^= pawnPos | fromBit;
    b->allPieces |= toBit;
}
void undoPassand(Board* b, Move move, const int color)
{
    uint64_t fromBit = POW2[move.from], toBit = POW2[move.to];
    uint64_t pawnPos = POW2[move.enPass];
    //Move pawn
    b->piece[color][PAWN] ^= toBit;
    b->piece[color][PAWN] |= fromBit;

    b->color[color] ^= toBit;
    b->color[color] |= fromBit;
    b->color[color | 2] ^= fromBit;
    b->color[color | 2] |= toBit;

    //Remove pawn
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

    uint64_t fromBit = POW2[move.from], toBit = POW2[move.to];

    //Promotion
    if (move.promotion && move.pieceThatMoves == PAWN)
        b->piece[h->color][move.promotion] |= toBit;
    else
        b->piece[h->color][move.pieceThatMoves] |= toBit;

    b->piece[h->color][move.pieceThatMoves] ^= fromBit;
    
    b->allPieces ^= fromBit;

    b->color[h->color] ^= fromBit;
    b->color[h->color | 2] |= fromBit;

    b->color[h->color] |= toBit;
    b->color[h->color | 2] ^= toBit;
    
    h->pieceCaptured = capturePiece(b, toBit, 1 ^ h->color);
    if (h->pieceCaptured != NO_PIECE)
    {
        b->color[1 ^ h->color] ^= toBit;
        b->color[3 - h->color] |= toBit;
    }
    else
        b->allPieces |= toBit;
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
    uint64_t fromBit = POW2[move.from], toBit = POW2[move.to];

    if (move.promotion && move.pieceThatMoves == PAWN)
        b->piece[h->color][move.promotion] ^= toBit;
    else
        b->piece[h->color][move.pieceThatMoves] ^= toBit;

    b->piece[h->color][move.pieceThatMoves] |= fromBit;

    b->color[h->color] |= fromBit;
    b->color[h->color] ^= toBit;
    b->color[h->color | 2] |= toBit;
    b->color[h->color | 2] ^= fromBit;

    if (h->pieceCaptured != NO_PIECE)
    {
        b->piece[1 ^ h->color][h->pieceCaptured] |= toBit;
        b->color[1 ^ h->color] |= toBit;
        b->color[3 - h->color] ^= toBit;
    }
}

//Generates all the moves and returns the number
int allMoves(Board* b, Move* list, const int color)
{
    int numMoves = 0;
    int i, j, from, to, numPieces, popC;
    uint64_t temp, tempMoves;

    temp = b->piece[color][KING];
    if (temp)
    {
        tempMoves = posKingMoves(b, color);
        popC = POPCOUNT(tempMoves);
        for (j = 0; j < popC; ++j)
        {
            to = LSB_INDEX(tempMoves);
            REMOVE_LSB(tempMoves);
            list[numMoves++] = (Move) {.pieceThatMoves = KING, .from = LSB_INDEX(temp), .to = to};
        }
    }
    else
        return 0;

    temp = b->piece[color][PAWN];
    numPieces = POPCOUNT(temp);
    for (i = 0; i < numPieces; ++i)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);

        tempMoves = posPawnMoves(b, color, from);
        popC = POPCOUNT(tempMoves);
        for (j = 0; j < popC; ++j)
        {
            to = LSB_INDEX(tempMoves);
            REMOVE_LSB(tempMoves);
            if (to < 8 || to > 55)
            {
                list[numMoves++] = (Move) {.pieceThatMoves = PAWN, .from = from, .to = to, .promotion = KNIGHT};
                list[numMoves++] = (Move) {.pieceThatMoves = PAWN, .from = from, .to = to, .promotion = BISH};
                list[numMoves++] = (Move) {.pieceThatMoves = PAWN, .from = from, .to = to, .promotion = ROOK};
                list[numMoves++] = (Move) {.pieceThatMoves = PAWN, .from = from, .to = to, .promotion = QUEEN};
            }
            else
                list[numMoves++] = (Move) {.pieceThatMoves = PAWN, .from = from, .to = to};
        }


        if ((b->enPass - from == 1) && ((from & 7) != 7) && (b->piece[1 ^ color][PAWN] & POW2[b->enPass]))
            list[numMoves++] = (Move) {.pieceThatMoves = PAWN, .from = from, .to = from + 1 + (2 * color - 1) * 8, .enPass = b->enPass};
        
        else if ((b->enPass - from == -1) && ((from & 7) != 0) && (b->piece[1 ^ color][PAWN] & POW2[b->enPass]))
            list[numMoves++] = (Move) {.pieceThatMoves = PAWN, .from = from, .to = from - 1 + (2 * color - 1) * 8, .enPass = b->enPass};
    }
    
    temp = b->piece[color][QUEEN];
    numPieces = POPCOUNT(temp);
    for (i = 0; i < numPieces; ++i)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);
        tempMoves = posQueenMoves(b, color, from);
        popC = POPCOUNT(tempMoves);
        for (j = 0; j < popC; ++j)
        {
            to = LSB_INDEX(tempMoves);
            REMOVE_LSB(tempMoves);
            list[numMoves++] = (Move) {.pieceThatMoves = QUEEN, .from = from, .to = to};
        }
    }

    temp = b->piece[color][ROOK];
    numPieces = POPCOUNT(temp);
    for (i = 0; i < numPieces; ++i)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);
        tempMoves = posRookMoves(b, color, from);
        popC = POPCOUNT(tempMoves);
        for (j = 0; j < popC; ++j)
        {
            to = LSB_INDEX(tempMoves);
            REMOVE_LSB(tempMoves);
            list[numMoves++] = (Move) {.pieceThatMoves = ROOK, .from = from, .to = to};
        }
    }

    temp = b->piece[color][BISH];
    numPieces = POPCOUNT(temp);
    for (i = 0; i < numPieces; ++i)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);
        tempMoves = posBishMoves(b, color, from);
        popC = POPCOUNT(tempMoves);
        for (j = 0; j < popC; ++j)
        {
            to = LSB_INDEX(tempMoves);
            REMOVE_LSB(tempMoves);
            list[numMoves++] = (Move) {.pieceThatMoves = BISH, .from = from, .to = to};
        }
    }

    temp = b->piece[color][KNIGHT];
    numPieces = POPCOUNT(temp);
    for (i = 0; i < numPieces; ++i)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);
        tempMoves = posKnightMoves(b, color, from);
        popC = POPCOUNT(tempMoves);
        for (j = 0; j < popC; ++j)
        {
            to = LSB_INDEX(tempMoves);
            REMOVE_LSB(tempMoves);
            list[numMoves++] = (Move) {.pieceThatMoves = KNIGHT, .from = from, .to = to};
        }
    }

    int castle = canCastle(b, color);
    if (castle & 1)
        list[numMoves++] = castleKSide(color);
    if (castle & 2)
        list[numMoves++] = castleQSide(color);

    return numMoves;
}
