#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include "../include/global.h"
#include "../include/board.h"
#include "../include/memoization.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"
#include "../include/evaluation.h"
#include "../include/hash.h"
#include "../include/sort.h"
#include "../include/search.h"
#include "../include/magic.h"
#include "../include/io.h"

int smallestAttackerSqr(const Board* b, const int sqr, const int col);
inline int seeCapture(Board b, const Move m);
__attribute__((hot)) int see(Board* b, const int to, const int pieceAtSqr);

const Move NOMOVE = (Move) {.from = -1, .to = -1};
const int pVal[6] = {1500, VQUEEN, VROOK, VBISH, VKNIGHT, VPAWN};
Move killerMoves[99][NUM_KM];

void initKM(void)
{
    for (int i = 0; i < 99; ++i)
    {
        for (int j = 0; j < 2; ++j)
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

int smallestAttackerSqr(const Board* b, const int sqr, const int col)
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

inline int compMoves(const Move* m1, const Move* m2)
{
    return m1->from == m2->from && m1->to == m2->to;
}

inline void assignScores(Board* b, Move* list, const int numMoves, const Move bestFromPos, const int depth)
{
    for (int i = 0; i < numMoves; ++i)
    {
        if (list[i].promotion > 1) //Promotions have their score assigned
            continue;
        if(list[i].capture > 0) //There has been a capture
            list[i].score = 60 + seeCapture(*b, list[i]);

        if (compMoves(&bestFromPos, &list[i])) //It was the best refutation in the same position
        {
            list[i].score += 500;
        }
        else
        {
            for (int j = 0; j < NUM_KM; ++j)
            {
                if (compMoves(&killerMoves[depth][j], &list[i]))
                {
                    list[i].score += 50 + j;
                    break;
                }
            }
        }
        int gvC = list[i].score < 300 && givesCheck(b, list[i]);
        if (gvC)
            list[i].score += 100 * gvC * gvC;
    }
}
inline void assignScoresQuiesce(Board* b, Move* list, const int numMoves)
{
    for (int i = 0; i < numMoves; ++i)
    {
        if (list[i].promotion > 1)
            continue;
        if(list[i].capture > 0)
        {
            if (list[i].piece == PAWN)
                list[i].score = pVal[list[i].capture] - 10;
            else
                list[i].score = 60 + seeCapture(*b, list[i]);//captureScore(&list[i]);//Bonus if it captures the piece that moved last time, it reduces the qsearch nodes about 30%
        }
    }
}
inline void addKM(const Move m, const int depth)
{
    for (int i = 1; i < NUM_KM; ++i)
        killerMoves[depth][i-1] = killerMoves[depth][i];

    killerMoves[depth][NUM_KM-1] = m;
}
/* Sorts all the moves based on their score, the algorithm is insertion sort
 */
void sort(Move* list, const int numMoves)
{
    int j;
    Move temp;
    for (int i = 1; i < numMoves; ++i)
    {
        temp = list[i];
        j = i - 1;

        while(j > -1 && list[j].score < temp.score)
        {
            list[j + 1] = list[j];
            --j;
        }

        list[j + 1] = temp;
    }
}