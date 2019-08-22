#include "../include/global.h"
#include "../include/memoization.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/mate.h"
#include "../include/io.h"

#include <stdio.h>
#include <stdlib.h>

static inline int rookAttByK(const int wrLSB, const int wrCol, const int wKingIsHigher)
{
    if (wKingIsHigher)
    {
        if (wrLSB < 56)
            return 56 + wrCol;
        else
            return wrCol;
    }
    else
    {
        if (wrLSB > 8)
            return wrCol;
        else
            return 56 + wrCol;
    }
}

/* Shifts the rook all the way to the bottom or to the top
 */
static inline int shiftRook(const int wrLSB, const int wrCol)
{
    if ((wrLSB >> 3) < 4) //It is on the lower half of the board
        return 56 + wrCol;
    return wrCol;
}
static inline int rookCutsOff(const int lkCol, const int wrCol, const int wKingIsToTheRight)
{
    if (wKingIsToTheRight)
        return wrCol - lkCol == -1;
    else
        return wrCol - lkCol == 1;
}
static inline int makeWaitingMove(const int wrRow, const int wrCol, const int lkRow, const int wKingIsHigher)
{
    if(wKingIsHigher)
    {
        if (wrRow > lkRow)
            return 8 * (wrRow - 1) + wrCol;
        else if (wrRow < lkRow - 3)
            return 8 * (wrRow + 1) + wrCol;
        else
            return shiftRook(8 * wrRow + wrCol, wrCol);
    }
    else
    {
        if (wrRow < lkRow)
            return 8 * (wrRow + 1) + wrCol;
        else if (wrRow > lkRow + 3)
            return 8 * (wrRow - 1) + wrCol;
        else
            return shiftRook(8 * wrRow + wrCol, wrCol);
    }
}

//This algorithm is made to be simple, not efficient
/* How it works:
 * The algorithm starts checking conditions to see which the best move to play
 * 1- If the rook is attacked by the opp king, shift it to the top / bottom
 * 2- If the kings are in opposition and the rook is safe to move, give check to force the king back
 * 3- If none of this conditions apply, then the rook hasnt yet cutoff the king, determine a side to mate and move
 *    the rook to cutoff the king, to avoid problems with opposition, always move the rook between the 2 kings 
 *    column-wise, despite this being inefficient
 * 4- If the rook is safe and cuts off the king but the kings arent in opposition, keep the king a knights move away from the enemy king
 *    and, if necessary, make a waiting move with the rook
 */
Move rookMate(const Board b)
{
    const int color = b.turn;

    const int wkLSB = LSB_INDEX(b.piece[color][KING]);
    const int wrLSB = LSB_INDEX(b.piece[color][ROOK]);
    const int lkLSB = LSB_INDEX(b.piece[1 ^ color][KING]);

    const uint64_t lkMoves = getKingMoves(lkLSB);
    const uint64_t wkMoves = getKingMoves(wkLSB);
    const uint64_t wrMoves = straight(wrLSB, b.piece[color][KING]);

    const int wkCol = wkLSB & 7;
    const int wrCol = wrLSB & 7;
    const int lkCol = lkLSB & 7;
    
    const int wrRow = wrLSB / 8;
    const int wkRow = wkLSB / 8;
    const int lkRow = lkLSB / 8;

    const int kingsDist = abs(wkLSB - lkLSB);

    const int opposition = kingsDist == 2 || ((kingsDist == 10 || kingsDist == 6) && (lkLSB < 8 || lkLSB > 55)); //Opposition or the los king is on the edge besides the win king
    const int wKingIsHigher = wkLSB / 8 > lkLSB / 8;
    const int wKingIsToTheRight = (wkLSB & 7) < (lkLSB & 7);

    const uint64_t safeRookSquares = (~lkMoves) | wkMoves;

    Move move;

    if (b.piece[color][ROOK] & lkMoves) //The rook is attacked by the los king
    {
        move = (Move) {.piece = ROOK, .from = wrLSB, .to = rookAttByK(wrLSB, wrCol, wKingIsHigher)};
    }
    else if (opposition)
    {
        uint64_t target = POW2[8 * wrRow + lkCol];//Try to put the king in check, if not possible place the rook betw the kings or shift it
        if (target & safeRookSquares)
            move = (Move) {.piece = ROOK, .from = wrLSB, .to = 8 * wrRow + lkCol};
        else
            move = (Move) {.piece = ROOK, .from = wrLSB, .to = 8 * wrRow + (wkCol + lkCol) / 2};
    }
    else if (rookCutsOff(lkCol, wrCol, wKingIsToTheRight)) //The rook is already cutting off the opp king
    {
        if (kingsDist == 10 || kingsDist == 6)
            move = (Move) {.piece = ROOK, .from = wrLSB, .to = makeWaitingMove(wrRow, wrCol, lkRow, wKingIsHigher)};
        else if (abs(wkCol - lkCol) == 2) //rook is betw and k is looking for opposition, chase the king
            move = (Move) {.piece = KING, .from = wkLSB, .to = 8 * (wkRow - (2 * wKingIsHigher - 1)) + wkCol};
        else
            move = (Move) {.piece = KING, .from = wkLSB, .to = 8 * (wkRow - (2 * wKingIsHigher - 1)) + wkCol + (2 * wKingIsToTheRight - 1)};
    }
    else if (wrCol == wkCol)
    {
        move = (Move) {.piece = KING, .from = wkLSB, .to = 8 * wkRow + wkCol + (2 * wKingIsToTheRight - 1)};
    }
    else //Place the rook as to cut off the opp king
    {
        uint64_t sqrsToCutOff = getVert(lkCol - (2 * wKingIsToTheRight - 1)) & safeRookSquares;
        uint64_t intersect = sqrsToCutOff & wrMoves;

        if (intersect)
            move = (Move) {.piece = ROOK, .from = wrLSB, .to = LSB_INDEX(intersect)};
        else
            move = (Move) {.piece = ROOK, .from = wrLSB, .to = shiftRook(wrLSB, wrCol)};
    }


    return move;
}

int kingIsStalemate(const int wkLSB, const int wqLSB, const int lkLSB)
{
    return (getKingMoves(lkLSB) & ~(getKingMoves(wkLSB) | straight(wqLSB, POW2[wkLSB]) | diagonal(wqLSB, POW2[wkLSB]))) == 0ULL;
}

Move queenMate(const Board b)
{
    const int color = b.turn;

    const int wkLSB = LSB_INDEX(b.piece[color][KING]);
    const int wqLSB = LSB_INDEX(b.piece[color][QUEEN]);
    const int lkLSB = LSB_INDEX(b.piece[1 ^ color][KING]);

    const int wkRow = wkLSB / 8;
    const int wqRow = wqLSB / 8;
    const int lkRow = lkLSB / 8;

    const int lkCol = lkLSB & 7;
    const int wqCol = wqLSB & 7;

    const uint64_t wkMoves = getKingMoves(wkLSB);
    const uint64_t lkMoves = getKingMoves(lkLSB);
    const uint64_t wqMoves = straight(wqLSB, POW2[wkLSB]) | diagonal(wqLSB, POW2[wkLSB]);
    const uint64_t safeSquares = (~lkMoves) | wkMoves;
    const int queenIsKnightAway = (getKnightMoves(lkLSB) & POW2[wqLSB]) != 0;

    const int wKingIsHigher = wkLSB / 8 > lkLSB / 8;
    const int wKingIsToTheRight = (wkLSB & 7) < (lkLSB & 7);
    const int lKingInCorner = (lkRow == 0 || lkRow == 7) && (lkCol == 0 || lkCol == 7);


    Move move;

    if (lKingInCorner && queenIsKnightAway)//Possible stalemate
    {
        if (POW2[lkLSB + 16 * wKingIsHigher - 8] & wkMoves) //There can be mate
            move = (Move) {.piece = QUEEN, .from = wqLSB, .to = lkLSB + 16 * wKingIsHigher - 8};
        else //Back off queen
        {
            int to;
            
            if (wqRow == 1 || wqRow == 6)
                to = wqLSB + 2 * (wqCol == 2) - 1;
            else
                to = wqLSB + 16 * (wqRow == 2) - 8;

            move = (Move) {.piece = QUEEN, .from = wqLSB, .to = to};
        }
    }
    else if (lkRow == 0 || lkRow == 7) 
    {
        if (abs(wqRow - lkRow) == 1) //The queen is cutting off the king
        {
            if (abs(wkRow - lkRow) == 2)
            {
                if (POW2[lkLSB + 16 * wKingIsHigher - 8] & wkMoves)
                    move = (Move) {.piece = QUEEN, .from = wqLSB, .to = lkLSB + 16 * wKingIsHigher - 8};
                else
                    move = (Move) {.piece = KING, .from = wkLSB, .to = wkLSB + 2 * wKingIsToTheRight - 1};
            }
            else //Get the king closer
                move = (Move) {.piece = KING, .from = wkLSB, .to = wkLSB + 2 * wKingIsToTheRight - 1 - 16 * wKingIsHigher + 8};
        }
        else //Get the queen to cut off the king
        {
            //It currently stalemates the king if hes in the corner
            uint64_t cutOffSqrs = wqMoves & getHoriz(2 * (lkRow == 0) - 1) & safeSquares;
            move = (Move) {.piece = QUEEN, .from = wkLSB, .to = LSB_INDEX(cutOffSqrs)};
        }
    }
    else //Use the queen to corner the king
    {
        
    }

    return move;
}