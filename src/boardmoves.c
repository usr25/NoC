#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"

//This file makes changes to the board, moves.c generates the moves

#include <stdio.h>

//returns the piece CAPTURED and its color
//returns 0 otherwise
//TODO: Add history

void makeMove(Board* b, Move* move, const int colorToMove)
{
    uint64_t fromBit = POW2[move->from], toBit = POW2[move->to];
    b->piece[colorToMove][move->pieceThatMoves] ^= fromBit;
    b->piece[colorToMove][move->pieceThatMoves] |= toBit;
    
    b->allPieces ^= fromBit;

    b->color[colorToMove] ^= fromBit;
    b->color[colorToMove + 2] |= fromBit;

    b->color[colorToMove] |= toBit;
    b->color[colorToMove + 2] ^= toBit;
    
    move->pieceCaptured = capturePiece(b, toBit, 1 ^ colorToMove);
    if (move->pieceCaptured != NO_PIECE)
    {
        b->color[1 ^ colorToMove] ^= toBit;
        b->color[3 - colorToMove] |= toBit;
    }
    else
        b->allPieces |= toBit;
}

void undoMove(Board* b, Move move, const int colorThatPlayed)
{
    uint64_t fromBit = POW2[move.from], toBit = POW2[move.to];
    b->piece[colorThatPlayed][move.pieceThatMoves] |= fromBit;
    b->piece[colorThatPlayed][move.pieceThatMoves] ^= toBit;

    b->allPieces |= fromBit;

    b->color[colorThatPlayed] |= fromBit;
    b->color[colorThatPlayed] ^= toBit;
    b->color[colorThatPlayed + 2] |= toBit;
    b->color[colorThatPlayed + 2] ^= fromBit;

    if (move.pieceCaptured != NO_PIECE)
    {
        b->piece[1 ^ colorThatPlayed][move.pieceCaptured] |= toBit;
        b->color[1 ^ colorThatPlayed] |= toBit;
        b->color[3 - colorThatPlayed] ^= toBit;
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

    temp = b->piece[color][PAWN];
    numPieces = POPCOUNT(temp);
    for (i = 0; i < numPieces; ++i)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);
        tempMoves = posPawnMoves(b, color, i);
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
        tempMoves = posQueenMoves(b, color, i);
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
        tempMoves = posRookMoves(b, color, i);
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
        tempMoves = posBishMoves(b, color, i);
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
        tempMoves = posKnightMoves(b, color, i);
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
