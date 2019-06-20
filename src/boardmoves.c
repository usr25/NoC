#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"

//This file makes changes to the board, moves.c generates the moves

#include <stdio.h>

//returns the piece CAPTURED and its color
//returns 0 otherwise
void makeMoveWhite(Board* b, Move* move, int* history)
{
    uint64_t fromBit = POW2[move->from], toBit = POW2[move->to];
    switch (move->pieceThatMoves)
    {
        case KING:
        b->wKing ^= fromBit;
        b->wKing |= toBit;
        break;

        case QUEEN:
        b->wQueen ^= fromBit;
        b->wQueen |= toBit;
        break;
        
        case ROOK:
        b->wRook ^= fromBit;
        b->wRook |= toBit;
        break;
        
        case BISH:
        b->wBish ^= fromBit;
        b->wBish |= toBit;
        break;
        
        case KNIGHT:
        b->wKnight ^= fromBit;
        b->wKnight |= toBit;
        break;
        
        case PAWN:
        b->wPawn ^= fromBit;
        b->wPawn |= toBit;
        break;
    }
    b->white ^= fromBit;
    b->avWhite |= fromBit;
    b->white |= toBit;
    b->avWhite ^= toBit;
    b->pieces ^= fromBit;

    move->pieceCaptured = captureBlackPiece(b, toBit);
    if (move->pieceCaptured)
    {
        b->black ^= toBit;
        b->avBlack |= toBit;
    }
}
void makeMoveBlack(Board* b, Move* move, int* history)
{
    uint64_t fromBit = POW2[move->from], toBit = POW2[move->to];
    switch (move->pieceThatMoves)
    {
        case KING:
        b->bKing ^= fromBit;
        b->bKing |= toBit;
        break;

        case QUEEN:
        b->bQueen ^= fromBit;
        b->bQueen |= toBit;
        break;
        
        case ROOK:
        b->bRook ^= fromBit;
        b->bRook |= toBit;
        break;
        
        case BISH:
        b->bBish ^= fromBit;
        b->bBish |= toBit;
        break;
        
        case KNIGHT:
        b->bKnight ^= fromBit;
        b->bKnight |= toBit;
        break;
        
        case PAWN:
        b->bPawn ^= fromBit;
        b->bPawn |= toBit;
        break;
    }

    b->black ^= fromBit;
    b->avBlack |= fromBit;
    b->black |= toBit;
    b->avBlack ^= toBit;
    b->pieces ^= fromBit;

    move->pieceCaptured = captureWhitePiece(b, toBit);
    if (move->pieceCaptured)
    {
        b->white ^= toBit;
        b->avWhite |= toBit;
    }
}
//The move was made by white
void undoMoveWhite(Board* b, Move* move, int* history)
{
    uint64_t fromBit = POW2[move->from], toBit = POW2[move->to];
    switch (move->pieceThatMoves)
    {
        case KING:
        b->wKing |= fromBit;
        b->wKing ^= toBit;
        break;

        case QUEEN:
        b->wQueen |= fromBit;
        b->wQueen ^= toBit;
        break;
        
        case ROOK:
        b->wRook |= fromBit;
        b->wRook ^= toBit;
        break;
        
        case BISH:
        b->wBish |= fromBit;
        b->wBish ^= toBit;
        break;
        
        case KNIGHT:
        b->wKnight |= fromBit;
        b->wKnight ^= toBit;
        break;
        
        case PAWN:
        b->wPawn |= fromBit;
        b->wPawn ^= toBit;
        break;
    }
    b->white |= fromBit;
    b->white ^= toBit;
    b->avWhite |= toBit;
    b->avWhite ^= fromBit;
    b->pieces |= fromBit;

    int change = 1;

    switch (move->pieceCaptured)
    {
        case 0: //To avoid checking against the other cases
        change = 0;
        break;

        case KING:
        b->bKing |= toBit;
        break;

        case QUEEN:
        b->bQueen |= toBit;
        incrBQueen(b);
        break;
        
        case ROOK:
        b->bRook |= toBit;
        incrBRook(b);
        break;
        
        case BISH:
        b->bBish |= toBit;
        incrBBish(b);
        break;
        
        case KNIGHT:
        b->bKnight |= toBit;
        incrBKnight(b);
        break;
        
        case PAWN:
        b->bPawn |= toBit;
        incrBPawn(b);
        break;

    }

    if (change)
    {
        b->black |= toBit;
        b->avBlack ^= toBit;
    }
}
void undoMoveBlack(Board* b, Move* move, int* history)
{
    uint64_t fromBit = POW2[move->from], toBit = POW2[move->to];
    switch (move->pieceThatMoves)
    {
        case KING:
        b->bKing |= fromBit;
        b->bKing ^= toBit;
        break;

        case QUEEN:
        b->bQueen |= fromBit;
        b->bQueen ^= toBit;
        break;
        
        case ROOK:
        b->bRook |= fromBit;
        b->bRook ^= toBit;
        break;
        
        case BISH:
        b->bBish |= fromBit;
        b->bBish ^= toBit;
        break;
        
        case KNIGHT:
        b->bKnight |= fromBit;
        b->bKnight ^= toBit;
        break;
        
        case PAWN:
        b->bPawn |= fromBit;
        b->bPawn ^= toBit;
        break;
    }
    b->black |= fromBit;
    b->black ^= toBit;
    b->avBlack |= toBit;
    b->avBlack ^= fromBit;
    b->pieces |= fromBit;

    int change = 1;

    switch (move->pieceCaptured)
    {
        case 0:
        change = 0;
        break;

        case KING:
        b->wKing |= toBit;
        break;

        case QUEEN:
        b->wQueen |= toBit;
        incrWQueen(b);
        break;
        
        case ROOK:
        b->wRook |= toBit;
        incrWRook(b);
        break;
        
        case BISH:
        b->wBish |= toBit;
        incrWBish(b);
        break;
        
        case KNIGHT:
        b->wKnight |= toBit;
        incrWKnight(b);
        break;
        
        case PAWN:
        b->wPawn |= toBit;
        incrWPawn(b);
        break;
    }

    if (change)
    {
        b->white |= toBit;
        b->avWhite ^= toBit;
    }
}

//Generates all the moves and returns the number
int allMovesWhite(Board* b, Move* list, uint64_t prevMovEnPass)
{
    int numMoves = 0;
    int i, j, from, to, popC;
    uint64_t temp, tempMoves;

    temp = b->wKing;
    tempMoves = posKingMoves(b, 1);
    popC = POPCOUNT(tempMoves);
    for (j = 0; j < popC; ++j)
    {
        to = LSB_INDEX(tempMoves);
        REMOVE_LSB(tempMoves);
        list[numMoves++] = (Move) {.pieceThatMoves = KING, .from = from, .to = to, .color = 1};   
    }

    temp = b->wPawn;
    for (i = 0; i < numWPawn(b->numPieces); ++i)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);
        tempMoves = posPawnMoves(b, 1, i);
        popC = POPCOUNT(tempMoves);
        for (j = 0; j < popC; ++j)
        {
            to = LSB_INDEX(tempMoves);
            REMOVE_LSB(tempMoves);
            list[numMoves++] = (Move) {.pieceThatMoves = PAWN, .from = from, .to = to, .color = 1};   
        }
    }

    temp = b->wQueen;
    for (i = 0; i < numWQueen(b->numPieces); ++i)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);
        tempMoves = posQueenMoves(b, 1, i);
        popC = POPCOUNT(tempMoves);
        for (j = 0; j < popC; ++j)
        {
            to = LSB_INDEX(tempMoves);
            REMOVE_LSB(tempMoves);
            list[numMoves++] = (Move) {.pieceThatMoves = QUEEN, .from = from, .to = to, .color = 1};   
        }
    }

    temp = b->wRook;
    for (i = 0; i < numWRook(b->numPieces); ++i)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);
        tempMoves = posRookMoves(b, 1, i);
        popC = POPCOUNT(tempMoves);
        for (j = 0; j < popC; ++j)
        {
            to = LSB_INDEX(tempMoves);
            REMOVE_LSB(tempMoves);
            list[numMoves++] = (Move) {.pieceThatMoves = ROOK, .from = from, .to = to, .color = 1};   
        }
    }

    temp = b->wBish;
    for (i = 0; i < numWBish(b->numPieces); ++i)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);
        tempMoves = posBishMoves(b, 1, i);
        popC = POPCOUNT(tempMoves);
        for (j = 0; j < popC; ++j)
        {
            to = LSB_INDEX(tempMoves);
            REMOVE_LSB(tempMoves);
            list[numMoves++] = (Move) {.pieceThatMoves = BISH, .from = from, .to = to, .color = 1};   
        }
    }

    temp = b->wKnight;
    for (i = 0; i < numWKnight(b->numPieces); ++i)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);
        tempMoves = posKnightMoves(b, 1, i);
        popC = POPCOUNT(tempMoves);
        for (j = 0; j < popC; ++j)
        {
            to = LSB_INDEX(tempMoves);
            REMOVE_LSB(tempMoves);
            list[numMoves++] = (Move) {.pieceThatMoves = KNIGHT, .from = from, .to = to, .color = 1};   
        }
    }

    return numMoves;
}
int allMovesBlack(Board* b, Move* list, uint64_t prevMovEnPass)
{
    int numMoves = 0;
    int i, j, from, to, popC;
    uint64_t temp, tempMoves;

    temp = b->bKing;
    tempMoves = posKingMoves(b, 0);
    popC = POPCOUNT(tempMoves);
    for (j = 0; j < popC; ++j)
    {
        to = LSB_INDEX(tempMoves);
        REMOVE_LSB(tempMoves);
        list[numMoves++] = (Move) {.pieceThatMoves = KING, .from = from, .to = to, .color = 0};   
    }
    

    temp = b->bPawn;
    for (i = 0; i < numBPawn(b->numPieces); ++i)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);
        tempMoves = posPawnMoves(b, 0, i);
        popC = POPCOUNT(tempMoves);
        for (j = 0; j < popC; ++j)
        {
            to = LSB_INDEX(tempMoves);
            REMOVE_LSB(tempMoves);
            list[numMoves++] = (Move) {.pieceThatMoves = PAWN, .from = from, .to = to, .color = 0};   
        }
    }

    temp = b->bQueen;
    for (i = 0; i < numBQueen(b->numPieces); ++i)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);
        tempMoves = posQueenMoves(b, 0, i);
        popC = POPCOUNT(tempMoves);
        for (j = 0; j < popC; ++j)
        {
            to = LSB_INDEX(tempMoves);
            REMOVE_LSB(tempMoves);
            list[numMoves++] = (Move) {.pieceThatMoves = QUEEN, .from = from, .to = to, .color = 0};   
        }
    }

    temp = b->bRook;
    for (i = 0; i < numBRook(b->numPieces); ++i)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);
        tempMoves = posRookMoves(b, 0, i);
        popC = POPCOUNT(tempMoves);
        for (j = 0; j < popC; ++j)
        {
            to = LSB_INDEX(tempMoves);
            REMOVE_LSB(tempMoves);
            list[numMoves++] = (Move) {.pieceThatMoves = ROOK, .from = from, .to = to, .color = 0};   
        }
    }

    temp = b->bBish;
    for (i = 0; i < numBBish(b->numPieces); ++i)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);
        tempMoves = posBishMoves(b, 0, i);
        popC = POPCOUNT(tempMoves);
        for (j = 0; j < popC; ++j)
        {
            to = LSB_INDEX(tempMoves);
            REMOVE_LSB(tempMoves);
            list[numMoves++] = (Move) {.pieceThatMoves = BISH, .from = from, .to = to, .color = 0};   
        }
    }

    temp = b->bKnight;
    for (i = 0; i < numBKnight(b->numPieces); ++i)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);
        tempMoves = posKnightMoves(b, 0, i);
        popC = POPCOUNT(tempMoves);
        for (j = 0; j < popC; ++j)
        {
            to = LSB_INDEX(tempMoves);
            REMOVE_LSB(tempMoves);
            list[numMoves++] = (Move) {.pieceThatMoves = KNIGHT, .from = from, .to = to, .color = 0};   
        }
    }

    return numMoves;
}