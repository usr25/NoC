#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/memoization.h"
#include "../include/boardmoves.h"
#include "../include/allmoves.h"
#include "../include/io.h"

#include <stdio.h>

//Generates all the moves and returns the number
//TODO: Compress all this code
int allMoves(Board* b, Move* list, const int color)
{
    int numMoves = 0;
    int i, j, from, to, numPieces, popC;
    uint64_t temp, tempMoves;

    temp = b->piece[color][KING];
    if (temp)
    {
        tempMoves = posKingMoves(b, color);
        while(tempMoves)
        {
            to = LSB_INDEX(tempMoves);
            REMOVE_LSB(tempMoves);
            list[numMoves++] = (Move) {.pieceThatMoves = KING, .from = LSB_INDEX(temp), .to = to};
        }
    }
    else
        return 0;

    temp = b->piece[color][PAWN];
    while(temp)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);

        tempMoves = posPawnMoves(b, color, from);
        while (tempMoves)
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
    while(temp)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);
        tempMoves = posQueenMoves(b, color, from);
        while(tempMoves)
        {
            to = LSB_INDEX(tempMoves);
            REMOVE_LSB(tempMoves);
            list[numMoves++] = (Move) {.pieceThatMoves = QUEEN, .from = from, .to = to};
        }
    }

    temp = b->piece[color][ROOK];
    while(temp)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);
        tempMoves = posRookMoves(b, color, from);
        while(tempMoves)
        {
            to = LSB_INDEX(tempMoves);
            REMOVE_LSB(tempMoves);
            list[numMoves++] = (Move) {.pieceThatMoves = ROOK, .from = from, .to = to};
        }
    }


    temp = b->piece[color][BISH];
    while(temp)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);
        tempMoves = posBishMoves(b, color, from);

        while(tempMoves)
        {
            to = LSB_INDEX(tempMoves);
            REMOVE_LSB(tempMoves);
            list[numMoves++] = (Move) {.pieceThatMoves = BISH, .from = from, .to = to};   
        }
    }


    temp = b->piece[color][KNIGHT];
    while(temp)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);
        tempMoves = posKnightMoves(b, color, from);
        while(tempMoves)
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

uint64_t pinnedPieces(Board* b, const int color)
{
    /*
    1- Trace moves from the queen as if it were a queen but separating each dir
    2- Only pay attention to the lines that the first intersection is with a piece of the king's color
    3- Retrace from that piece in the direction and detect if there is a Rook / Bish / Queen
    4- Return a bitboard of 1s where each 1 is a pinned piece, this will be used as a mask
    */
    uint64_t res = 0;
    uint64_t retrace;

    const int lsb = LSB_INDEX(b->piece[color][KING]);
    int obstacle;

    const uint64_t inteUp = getUpMoves(lsb) & b->allPieces;
    const uint64_t inteDown = getDownMoves(lsb) & b->allPieces;
    const uint64_t inteRight = getRightMoves(lsb) & b->allPieces;
    const uint64_t inteLeft = getLeftMoves(lsb) & b->allPieces;

    const uint64_t inteUpRight = getUpRightMoves(lsb) & b->allPieces;
    const uint64_t inteUpLeft = getUpLeftMoves(lsb) & b->allPieces;
    const uint64_t inteDownRight = getDownRightMoves(lsb) & b->allPieces;
    const uint64_t inteDownLeft = getDownLeftMoves(lsb) & b->allPieces;

    uint64_t stra = b->piece[1 ^ color][QUEEN] | b->piece[1 ^ color][ROOK];
    uint64_t diag = b->piece[1 ^ color][QUEEN] | b->piece[1 ^ color][BISH];

    if (inteUp)
    {
        obstacle = LSB_INDEX(inteUp);
        if (POW2[obstacle] & b->color[color])
        {
            retrace = getUpMoves(obstacle) & b->allPieces;
            if (POW2[LSB_INDEX(retrace)] & stra)
                res |= POW2[obstacle];
        }
    }
    if (inteDown)
    {
        obstacle = MSB_INDEX(inteDown);
        if (POW2[obstacle] & b->color[color])
        {
            retrace = getDownMoves(obstacle) & b->allPieces;
            if (POW2[MSB_INDEX(retrace)] & stra)
                res |= POW2[obstacle];
        }
    }
    if (inteRight)
    {
        obstacle = MSB_INDEX(inteRight);
        if (POW2[obstacle] & b->color[color])
        {
            retrace = getRightMoves(obstacle) & b->allPieces;
            if (POW2[MSB_INDEX(retrace)] & stra)
                res |= POW2[obstacle];
        }
    }
    if (inteLeft)
    {
        obstacle = LSB_INDEX(inteLeft);
        if (POW2[obstacle] & b->color[color])
        {
            retrace = getLeftMoves(obstacle) & b->allPieces;
            if (POW2[LSB_INDEX(retrace)] & stra)
                res |= POW2[obstacle];
        }
    }
    
    if (inteUpRight){
        obstacle = LSB_INDEX(inteUpRight);
        if (POW2[obstacle] & b->color[color])
        {
            retrace = getUpRightMoves(obstacle) & b->allPieces;
            if (POW2[LSB_INDEX(retrace)] & diag)
                res |= POW2[obstacle];
        }
    }
    if (inteUpLeft){
        obstacle = LSB_INDEX(inteUpLeft);
        if (POW2[obstacle] & b->color[color])
        {
            retrace = getUpLeftMoves(obstacle) & b->allPieces;
            if (POW2[LSB_INDEX(retrace)] & diag)
                res |= POW2[obstacle];
        }
    }
    if (inteDownRight){
        obstacle = MSB_INDEX(inteDownRight);
        if (POW2[obstacle] & b->color[color])
        {
            retrace = getDownRightMoves(obstacle) & b->allPieces;
            if (POW2[MSB_INDEX(retrace)] & diag)
                res |= POW2[obstacle];
        }
    }
    if (inteDownLeft){
        obstacle = MSB_INDEX(inteDownLeft);
        if (POW2[obstacle] & b->color[color])
        {
            retrace = getDownLeftMoves(obstacle) & b->allPieces;
            if (POW2[MSB_INDEX(retrace)] & diag)
                res |= POW2[obstacle];
        }
    }

    return res;
}

static inline int moveIsValid(Board* b, Move m, History h)
{
    makeMove(b, m, &h);
    int chk = isInCheck(b, h.color);
    undoMove(b, m, &h);
    return chk == NO_PIECE;
}

int legalMoves(Board* b, Move* list, const int color)
{
    History h = (History) {.color = color};

    int kingIndex = LSB_INDEX(b->piece[color][KING]);
    int numMoves = 0, from, to;
    uint64_t temp, tempMoves;
    uint64_t kingPawnKnight = controlledKingPawnKnight(b, 1 ^ color);
    uint64_t xRay = xRaySquares(b, 1 ^ color) | kingPawnKnight;
    uint64_t forbidden = forbiddenSquares(b, 1 ^ color) | kingPawnKnight;
    
    if ((xRay & b->piece[color][KING]) == 0ULL) //The king isnt in check and even if a piece moves he wont be
    {
        tempMoves = posKingMoves(b, color) & (ALL ^ forbidden);
        while(tempMoves)
        {
            to = LSB_INDEX(tempMoves);
            REMOVE_LSB(tempMoves);
            list[numMoves++] = (Move) {.pieceThatMoves = KING, .from = LSB_INDEX(temp), .to = to};
        }

        temp = b->piece[color][PAWN];
        while(temp)
        {
            from = LSB_INDEX(temp);
            REMOVE_LSB(temp);

            tempMoves = posPawnMoves(b, color, from);
            while (tempMoves)
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
        while(temp)
        {
            from = LSB_INDEX(temp);
            REMOVE_LSB(temp);
            tempMoves = posQueenMoves(b, color, from);
            while(tempMoves)
            {
                to = LSB_INDEX(tempMoves);
                REMOVE_LSB(tempMoves);
                list[numMoves++] = (Move) {.pieceThatMoves = QUEEN, .from = from, .to = to};
            }
        }

        temp = b->piece[color][ROOK];
        while(temp)
        {
            from = LSB_INDEX(temp);
            REMOVE_LSB(temp);
            tempMoves = posRookMoves(b, color, from);
            while(tempMoves)
            {
                to = LSB_INDEX(tempMoves);
                REMOVE_LSB(tempMoves);
                list[numMoves++] = (Move) {.pieceThatMoves = ROOK, .from = from, .to = to};
            }
        }


        temp = b->piece[color][BISH];
        while(temp)
        {
            from = LSB_INDEX(temp);
            REMOVE_LSB(temp);
            tempMoves = posBishMoves(b, color, from);

            while(tempMoves)
            {
                to = LSB_INDEX(tempMoves);
                REMOVE_LSB(tempMoves);
                list[numMoves++] = (Move) {.pieceThatMoves = BISH, .from = from, .to = to};   
            }
        }


        temp = b->piece[color][KNIGHT];
        while(temp)
        {
            from = LSB_INDEX(temp);
            REMOVE_LSB(temp);
            tempMoves = posKnightMoves(b, color, from);
            while(tempMoves)
            {
                to = LSB_INDEX(tempMoves);
                REMOVE_LSB(tempMoves);
                list[numMoves++] = (Move) {.pieceThatMoves = KNIGHT, .from = from, .to = to};
            }
        }
    }
    else if((forbidden & b->piece[color][KING]) == 0ULL) //The king isnt in check but if a piece moves he will be
    {
        uint64_t pinned = pinnedPieces(b, color);
        int isPinned;
        Move m;

        tempMoves = posKingMoves(b, color) & (ALL ^ forbidden);
        while(tempMoves)
        {
            to = LSB_INDEX(tempMoves);
            REMOVE_LSB(tempMoves);
            list[numMoves++] = (Move) {.pieceThatMoves = KING, .from = LSB_INDEX(temp), .to = to};
        }

        temp = b->piece[color][PAWN];
        while(temp)
        {
            from = LSB_INDEX(temp);
            REMOVE_LSB(temp);
            isPinned = pinned & POW2[from];
            tempMoves = posPawnMoves(b, color, from);

            while (tempMoves)
            {
                to = LSB_INDEX(tempMoves);
                REMOVE_LSB(tempMoves);

                if (isPinned)
                {
                    m = (Move) {.pieceThatMoves = PAWN, .from = from, .to = to};
                    if (moveIsValid(b, m, h))
                        list[numMoves++] = m;
                    
                }
                else if (to < 8 || to > 55)
                {
                    list[numMoves++] = (Move) {.pieceThatMoves = PAWN, .from = from, .to = to, .promotion = KNIGHT};
                    list[numMoves++] = (Move) {.pieceThatMoves = PAWN, .from = from, .to = to, .promotion = BISH};
                    list[numMoves++] = (Move) {.pieceThatMoves = PAWN, .from = from, .to = to, .promotion = ROOK};
                    list[numMoves++] = (Move) {.pieceThatMoves = PAWN, .from = from, .to = to, .promotion = QUEEN};
                }
                else
                    list[numMoves++] = (Move) {.pieceThatMoves = PAWN, .from = from, .to = to};
            }

            if (isPinned)
            {
                if ((b->enPass - from == 1) && ((from & 7) != 7) && (b->piece[1 ^ color][PAWN] & POW2[b->enPass])){
                    m = (Move) {.pieceThatMoves = PAWN, .from = from, .to = from + 1 + (2 * color - 1) * 8, .enPass = b->enPass};
                    if (moveIsValid(b, m, h))
                        list[numMoves++] = m;
                }
                else if ((b->enPass - from == -1) && ((from & 7) != 0) && (b->piece[1 ^ color][PAWN] & POW2[b->enPass])){
                    m = (Move) {.pieceThatMoves = PAWN, .from = from, .to = from - 1 + (2 * color - 1) * 8, .enPass = b->enPass};
                    if (moveIsValid(b, m, h))
                        list[numMoves++] = m;
                }
            }
            if ((b->enPass - from == 1) && ((from & 7) != 7) && (b->piece[1 ^ color][PAWN] & POW2[b->enPass]))
                list[numMoves++] = (Move) {.pieceThatMoves = PAWN, .from = from, .to = from + 1 + (2 * color - 1) * 8, .enPass = b->enPass};
            else if ((b->enPass - from == -1) && ((from & 7) != 0) && (b->piece[1 ^ color][PAWN] & POW2[b->enPass]))
                list[numMoves++] = (Move) {.pieceThatMoves = PAWN, .from = from, .to = from - 1 + (2 * color - 1) * 8, .enPass = b->enPass};
        }
        
        temp = b->piece[color][QUEEN];
        while(temp)
        {
            from = LSB_INDEX(temp);
            REMOVE_LSB(temp);
            isPinned = pinned & POW2[from];
            tempMoves = posQueenMoves(b, color, from);
            while(tempMoves)
            {
                to = LSB_INDEX(tempMoves);
                REMOVE_LSB(tempMoves);
                m = (Move) {.pieceThatMoves = QUEEN, .from = from, .to = to};
                if (isPinned)
                {
                    if (moveIsValid(b, m, h))
                        list[numMoves++] = m;
                }
                else
                    list[numMoves++] = m;
            }
        }

        temp = b->piece[color][ROOK];
        while(temp)
        {
            from = LSB_INDEX(temp);
            REMOVE_LSB(temp);
            isPinned = pinned & POW2[from];
            tempMoves = posRookMoves(b, color, from);
            while(tempMoves)
            {
                to = LSB_INDEX(tempMoves);
                REMOVE_LSB(tempMoves);
                m = (Move) {.pieceThatMoves = ROOK, .from = from, .to = to};
                if (isPinned)
                {
                    if (moveIsValid(b, m, h))
                        list[numMoves++] = m;
                }
                else
                    list[numMoves++] = m;
            }
        }


        temp = b->piece[color][BISH];
        while(temp)
        {
            from = LSB_INDEX(temp);
            REMOVE_LSB(temp);
            isPinned = pinned & POW2[from];
            tempMoves = posBishMoves(b, color, from);
            while(tempMoves)
            {
                to = LSB_INDEX(tempMoves);
                REMOVE_LSB(tempMoves);
                m = (Move) {.pieceThatMoves = BISH, .from = from, .to = to};   
                if (isPinned)
                {
                    if (moveIsValid(b, m, h))
                        list[numMoves++] = m;
                }
                else
                    list[numMoves++] = m;
            }
        }


        temp = b->piece[color][KNIGHT];
        while(temp)
        {
            from = LSB_INDEX(temp);
            REMOVE_LSB(temp);
            isPinned = pinned & POW2[from];
            tempMoves = posKnightMoves(b, color, from);
            while(tempMoves)
            {
                to = LSB_INDEX(tempMoves);
                REMOVE_LSB(tempMoves);
                m = (Move) {.pieceThatMoves = KNIGHT, .from = from, .to = to};
                if (isPinned)
                {
                    if (moveIsValid(b, m, h))
                        list[numMoves++] = m;
                }
                else
                    list[numMoves++] = m;
            }
        }
    }
    else
    {
        Move m;

        tempMoves = posKingMoves(b, color) & (ALL ^ forbidden);
        while(tempMoves)
        {
            to = LSB_INDEX(tempMoves);
            REMOVE_LSB(tempMoves);
            list[numMoves++] = (Move) {.pieceThatMoves = KING, .from = LSB_INDEX(temp), .to = to};
        }

        temp = b->piece[color][PAWN];
        while(temp)
        {
            from = LSB_INDEX(temp);
            REMOVE_LSB(temp);
            tempMoves = posPawnMoves(b, color, from);

            while (tempMoves)
            {
                to = LSB_INDEX(tempMoves);
                REMOVE_LSB(tempMoves);

                if (to < 8 || to > 55)
                {
                    m = (Move) {.pieceThatMoves = PAWN, .from = from, .to = to, .promotion = KNIGHT};
                    if (moveIsValid(b, m, h))
                        list[numMoves++] = m;
                    m = (Move) {.pieceThatMoves = PAWN, .from = from, .to = to, .promotion = BISH};
                    if (moveIsValid(b, m, h))
                        list[numMoves++] = m;
                    m = (Move) {.pieceThatMoves = PAWN, .from = from, .to = to, .promotion = ROOK};
                    if (moveIsValid(b, m, h))
                        list[numMoves++] = m;
                    m = (Move) {.pieceThatMoves = PAWN, .from = from, .to = to, .promotion = QUEEN};
                    if (moveIsValid(b, m, h))
                        list[numMoves++] = m;
                }
                else{
                    m = (Move) {.pieceThatMoves = PAWN, .from = from, .to = to};
                    if (moveIsValid(b, m, h))
                        list[numMoves++] = m;
                }
            }

            if ((b->enPass - from == 1) && ((from & 7) != 7) && (b->piece[1 ^ color][PAWN] & POW2[b->enPass])){
                m = (Move) {.pieceThatMoves = PAWN, .from = from, .to = from + 1 + (2 * color - 1) * 8, .enPass = b->enPass};
                if (moveIsValid(b, m, h))
                    list[numMoves++] = m;
            }
            else if ((b->enPass - from == -1) && ((from & 7) != 0) && (b->piece[1 ^ color][PAWN] & POW2[b->enPass])){
                m = (Move) {.pieceThatMoves = PAWN, .from = from, .to = from - 1 + (2 * color - 1) * 8, .enPass = b->enPass};
                if (moveIsValid(b, m, h))
                    list[numMoves++] = m;
            }
        }
        
        temp = b->piece[color][QUEEN];
        while(temp)
        {
            from = LSB_INDEX(temp);
            REMOVE_LSB(temp);
            tempMoves = posQueenMoves(b, color, from);
            while(tempMoves)
            {
                to = LSB_INDEX(tempMoves);
                REMOVE_LSB(tempMoves);
                m = (Move) {.pieceThatMoves = QUEEN, .from = from, .to = to};
                if (moveIsValid(b, m, h))
                    list[numMoves++] = m;
            }
        }

        temp = b->piece[color][ROOK];
        while(temp)
        {
            from = LSB_INDEX(temp);
            REMOVE_LSB(temp);
            tempMoves = posRookMoves(b, color, from);
            while(tempMoves)
            {
                to = LSB_INDEX(tempMoves);
                REMOVE_LSB(tempMoves);
                m = (Move) {.pieceThatMoves = ROOK, .from = from, .to = to};
                if (moveIsValid(b, m, h))
                    list[numMoves++] = m;
            }
        }


        temp = b->piece[color][BISH];
        while(temp)
        {
            from = LSB_INDEX(temp);
            REMOVE_LSB(temp);
            tempMoves = posBishMoves(b, color, from);
            while(tempMoves)
            {
                to = LSB_INDEX(tempMoves);
                REMOVE_LSB(tempMoves);
                m = (Move) {.pieceThatMoves = BISH, .from = from, .to = to};   
                if (moveIsValid(b, m, h))
                    list[numMoves++] = m;
            }
        }


        temp = b->piece[color][KNIGHT];
        while(temp)
        {
            from = LSB_INDEX(temp);
            REMOVE_LSB(temp);
            tempMoves = posKnightMoves(b, color, from);
            while(tempMoves)
            {
                to = LSB_INDEX(tempMoves);
                REMOVE_LSB(tempMoves);
                m = (Move) {.pieceThatMoves = KNIGHT, .from = from, .to = to};
                if (moveIsValid(b, m, h))
                    list[numMoves++] = m;
            }
        }
    }

    //TODO: Improve this function because we know the state of the king and the checking squares
    int castle = canCastle(b, color);
    if (castle & 1)
        list[numMoves++] = castleKSide(color);
    if (castle & 2)
        list[numMoves++] = castleQSide(color);

    return numMoves;
}