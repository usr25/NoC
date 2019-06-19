#include "../include/global.h"
#include "../include/board.h"
#include "../include/boardmoves.h"
#include "../include/node.h"

//This file makes changes to the board, moves.c generates the moves

#include <stdio.h>

//returns the piece CAPTURED and its color
//returns 0 otherwise
unsigned int makeMoveWhite(Board* b, const unsigned int piece, const int from, const int to)
{
    uint64_t fromBit = POW2[from], toBit = POW2[to];
    switch (piece)
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

    unsigned int capturedPiece = captureBlackPiece(b, toBit);
    if (piece) 
    {
        b->black ^= toBit;
        b->avBlack |= toBit;
    }
    return piece;
}
unsigned int makeMoveBlack(Board* b, const unsigned int piece, const int from, const int to)
{
    uint64_t fromBit = POW2[from], toBit = POW2[to];
    switch (piece)
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

    unsigned int capturedPiece = captureWhitePiece(b, toBit);
    if (piece) 
    {
        b->white ^= toBit;
        b->avWhite |= toBit;
    }
    
    return piece;
}
//The move was made by white
void undoMoveWhite(Board* b, const int pieceMoved, const int pieceCaptured, const int from, const int to)
{
    uint64_t fromBit = POW2[from], toBit = POW2[to];
    switch (pieceMoved)
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

    int change = 1;

    switch (pieceCaptured)
    {
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

        default:
        change = 0;
    }

    if (change)
    {
        b->black |= toBit;
        b->avBlack ^= toBit;
    }
}
void undoMoveBlack(Board* b, const int pieceMoved, const int pieceCaptured, const int from, const int to)
{
    uint64_t fromBit = POW2[from], toBit = POW2[to];
    switch (pieceMoved)
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

    int change = 1;

    switch (pieceCaptured)
    {
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

        default:
        change = 0;
    }

    if (change)
    {
        b->white |= toBit;
        b->avWhite ^= toBit;
    }
}