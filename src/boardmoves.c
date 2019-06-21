#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"

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

void makeMove(Board* b, Move* move)
{
    uint64_t fromBit = POW2[move->from], toBit = POW2[move->to];
    b->piece[move->color][move->pieceThatMoves] ^= fromBit;
    b->piece[move->color][move->pieceThatMoves] |= toBit;
    
    b->allPieces ^= fromBit;

    b->color[move->color] ^= fromBit;
    b->color[move->color | 2] |= fromBit;

    b->color[move->color] |= toBit;
    b->color[move->color | 2] ^= toBit;
    
    move->pieceCaptured = capturePiece(b, toBit, 1 ^ move->color);
    if (move->pieceCaptured != NO_PIECE)
    {
        b->color[1 ^ move->color] ^= toBit;
        b->color[3 - move->color] |= toBit;
    }
    else
        b->allPieces |= toBit;
}

void undoMove(Board* b, Move move)
{
    uint64_t fromBit = POW2[move.from], toBit = POW2[move.to];
    b->piece[move.color][move.pieceThatMoves] |= fromBit;
    b->piece[move.color][move.pieceThatMoves] ^= toBit;

    b->allPieces |= fromBit;

    b->color[move.color] |= fromBit;
    b->color[move.color] ^= toBit;
    b->color[move.color | 2] |= toBit;
    b->color[move.color | 2] ^= fromBit;

    if (move.pieceCaptured != NO_PIECE)
    {
        b->piece[1 ^ move.color][move.pieceCaptured] |= toBit;
        b->color[1 ^ move.color] |= toBit;
        b->color[3 - move.color] ^= toBit;
    }
    else
        b->allPieces ^= toBit;
}

//Generates all the moves and returns the number
int allMoves(Board* b, Move* list, uint64_t prevMovEnPass, const int color)
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
            list[numMoves++] = (Move) {.pieceThatMoves = KING, .from = LSB_INDEX(temp), .to = to, .color = color};;
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
            list[numMoves++] = (Move) {.pieceThatMoves = PAWN, .from = from, .to = to, .color = color};
        }
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
            list[numMoves++] = (Move) {.pieceThatMoves = QUEEN, .from = from, .to = to, .color = color};;
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
            list[numMoves++] = (Move) {.pieceThatMoves = ROOK, .from = from, .to = to, .color = color};;
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
            list[numMoves++] = (Move) {.pieceThatMoves = BISH, .from = from, .to = to, .color = color};
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
            list[numMoves++] = (Move) {.pieceThatMoves = KNIGHT, .from = from, .to = to, .color = color};
        }
    }

    return numMoves;
}
