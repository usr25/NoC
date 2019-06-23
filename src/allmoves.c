#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"
#include "../include/allmoves.h"

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