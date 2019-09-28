#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include "../include/global.h"
#include "../include/board.h"
#include "../include/memoization.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"
#include "../include/sort.h"
#include "../include/magic.h"

#define NUM_KM 2

static int smallestAttackerSqr(const Board* b, const int sqr, const int col);
inline int seeCapture(Board b, const Move m);
__attribute__((hot)) int see(Board* b, const int to, const int pieceAtSqr);

static const Move NOMOVE = (Move) {.from = -1, .to = -1};
const int pVal[6] = {1500, VQUEEN, VROOK, VBISH, VKNIGHT, VPAWN};
Move killerMoves[99][NUM_KM];

inline int compMoves(const Move* m1, const Move* m2)
{
    return m1->from == m2->from && m1->to == m2->to;
}

void initKM(void)
{
    for (int i = 0; i < 99; ++i)
    {
        for (int j = 0; j < NUM_KM; ++j)
            killerMoves[i][j] = NOMOVE;
    }
}

inline int seeCapture(Board b, const Move m)
{
    makePermaMove(&b, m);
    return pVal[m.capture] - see(&b, m.to, m.piece);
}
int see(Board* b, const int sqr, const int pieceAtSqr)
{
    const int col = b->turn;
    const int from = smallestAttackerSqr(b, sqr, 1 ^ col);

    int value = 0;

    if (from != -1)
    {
        const int attacker = pieceAt(b, POW2[from], col);

        const Move move = (Move){.from=from, .to=sqr, .piece=attacker, .capture = pieceAtSqr, .castle=0};

        makePermaMove(b, move);
        const int score = pVal[pieceAtSqr] - see(b, sqr, attacker);
        value = (score > 0)? score : 0;
    }

    return value;
}

static int smallestAttackerSqr(const Board* b, const int sqr, const int col)
{
    const int opp = 1 ^ col;
    const uint64_t obst = b->allPieces;

    uint64_t temp = 0, bishMag = 0, rookMag = 0;
    if (col)
    {
        temp = getWhitePawnCaptures(sqr) & b->piece[BLACK][PAWN];
        if (temp)
            return LSB_INDEX(temp);
    }
    else
    {
        temp = getBlackPawnCaptures(sqr) & b->piece[WHITE][PAWN];
        if (temp)
            return LSB_INDEX(temp);
    }

    temp = getKnightMoves(sqr) & b->piece[opp][KNIGHT];
    if (temp)
        return LSB_INDEX(temp);

    if ((b->piece[opp][BISH] | b->piece[opp][QUEEN]) & getDiagMoves(sqr))
        bishMag = getBishMagicMoves(sqr, obst);

    temp = bishMag & b->piece[opp][BISH];
    if (temp)
        return LSB_INDEX(temp);

    if ((b->piece[opp][ROOK] | b->piece[opp][QUEEN]) & getStraMoves(sqr))
        rookMag = getRookMagicMoves(sqr, obst);

    temp = rookMag & b->piece[opp][ROOK];
    if (temp)
        return LSB_INDEX(temp);

    temp = (bishMag | rookMag) & b->piece[opp][QUEEN];
    if (temp)
        return LSB_INDEX(temp);

    temp = getKingMoves(sqr) & b->piece[opp][KING];
    if (temp)
        return LSB_INDEX(temp);

    return -1;
}

//TODO: Set a flag to use SEE depending on the depth or sthng
inline void assignScores(Board* b, Move* list, const int numMoves, const Move bestFromPos, const int depth)
{
    for (int i = 0; i < numMoves; ++i)
    {
        if (list[i].promotion > 0) //Promotions have their score assigned
            continue;
        if(list[i].capture > 0) //There has been a capture
        {
            list[i].score = pVal[list[i].capture] - pVal[list[i].piece] / 10;
            /*
            if (list[i].piece == PAWN)
                list[i].score = pVal[list[i].capture] - 20;
            else
                list[i].score = 60 + seeCapture(*b, list[i]);
            */
        }

        if (compMoves(&bestFromPos, &list[i])) //It was the best refutation in the same position
        {
            list[i].score += 1000;
        }
        else
        {
            /*
            for (int j = 0; j < NUM_KM; ++j)
            {
                if (compMoves(&killerMoves[depth][j], &list[i]))
                {
                    list[i].score += 50 + j;
                    break;
                }
            }
            */
            if (compMoves(&killerMoves[depth][0], &list[i]))
                list[i].score += 50;
            else if (compMoves(&killerMoves[depth][1], &list[i]))
                list[i].score += 51;
        }
        /* TODO: gvC makes the program slower, do some testing */
        /*
        if (list[i].score < 300)
        {
            int gvC = givesCheck(b, list[i]);
            if (gvC)
                list[i].score += 50 * gvC * gvC; //Double check is more than 2 times better than a normal check, at least thats the idea
        }
        */
    }
}
inline void assignScoresQuiesce(Board* b, Move* list, const int numMoves)
{
    for (int i = 0; i < numMoves; ++i)
    {
        if (list[i].promotion > 0)
            continue;
        if(list[i].capture > 0)
        {
            if (list[i].piece == PAWN)
                list[i].score = pVal[list[i].capture] - 20;
            else
                list[i].score = 60 + seeCapture(*b, list[i]);//captureScore(&list[i]);//Bonus if it captures the piece that moved last time, it reduces the qsearch nodes about 30%
        }
    }
}
inline void addKM(const Move m, const int depth)
{
    //Keep this commented out until we stop using NUM_KM == 2
    /*
    for (int i = 1; i < NUM_KM; ++i)
        killerMoves[depth][i-1] = killerMoves[depth][i];

    killerMoves[depth][NUM_KM-1] = m;
    */
    killerMoves[depth][0] = killerMoves[depth][1];
    killerMoves[depth][1] = m;
}

void sort(Move* start, Move* end)
{
    Move* q, temp;
    for (Move* p = start + 1; p < end; ++p)
    {
        temp = *p;
        q = p - 1;

        while(q >= start && q->score < temp.score)
        {
            *(q+1) = *q;
            --q;
        }

        *(q+1) = temp;
    }
}