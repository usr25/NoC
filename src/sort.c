/* sort.c
 * Assigns scores to the moves available from a position and sort the list
 * This considerably improves the alphabeta's efficiency and LMR effectiveness
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <time.h>

#include "../include/global.h"
#include "../include/board.h"
#include "../include/memoization.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"
#include "../include/sort.h"
#include "../include/magic.h"
#include "../include/evaluation.h"

int history[2][4096];

static int smallestAttackerSqr(const Board* b, const int sqr, const int col, const uint64_t diag, const uint64_t stra);
__attribute__((hot)) static int see(Board* b, const int to, const int pieceAtSqr, const uint64_t diag, const uint64_t stra);

#define NUM_KM 2

static Move NOMOVE = (Move) {.from = -1, .to = -1};
static int pVal[6];
Move killerMoves[MAX_PLY][NUM_KM];

Move counterMove[2][4096];

inline int compMoves(const Move* m1, const Move* m2)
{
    return m1->from == m2->from && m1->to == m2->to;
}

void initSort(void)
{
    //This is so that evaluation tweaks don't affect the ordering
    pVal[0] = 5000;
    pVal[1] = 1200;
    pVal[2] = 600;
    pVal[3] = 385;
    pVal[4] = 375;
    pVal[5] = 116;
}

void initKM(void)
{
    for (int i = 0; i < MAX_PLY; ++i)
    {
        for (int j = 0; j < NUM_KM; ++j)
            killerMoves[i][j] = NOMOVE;
    }
}

int seeCapture(Board b, const Move m)
{
    makePermaMove(&b, m);
    return pVal[m.capture] - see(&b, m.to, m.piece, getDiagMoves(m.to), getStraMoves(m.to));
}
static int see(Board* b, const int sqr, const int pieceAtSqr, const uint64_t diag, const uint64_t stra)
{
    const int col = b->stm;
    const int from = smallestAttackerSqr(b, sqr, 1 ^ col, diag, stra);

    int value = 0;

    if (from != -1)
    {
        const int attacker = pieceAt(b, POW2[from], col);

        const Move move = (Move){.from=from, .to=sqr, .piece=attacker, .capture=pieceAtSqr, .castle=0};

        makePermaMove(b, move);
        const int score = pVal[pieceAtSqr] - see(b, sqr, attacker, diag, stra);
        value = (score > 0)? score : 0;
    }

    return value;
}

static int smallestAttackerSqr(const Board* b, const int sqr, const int col, const uint64_t diag, const uint64_t stra)
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

    if ((b->piece[opp][BISH] | b->piece[opp][QUEEN]) & diag)
        bishMag = getBishMagicMoves(sqr, obst);

    temp = bishMag & b->piece[opp][BISH];
    if (temp)
        return LSB_INDEX(temp);

    if ((b->piece[opp][ROOK] | b->piece[opp][QUEEN]) & stra)
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

//TODO: Set a flag to use SEE depending on the depth or sthng like that
inline void assignScores(Board* b, Move* list, const int numMoves, const Move bestFromPos, const int depth)
{
    Move* end = list + numMoves;

    uint64_t pawnAtt;
    if (b->stm)
        pawnAtt = BLACK_PAWN_ATT(b->piece[BLACK][PAWN]);
    else
        pawnAtt = WHITE_PAWN_ATT(b->piece[WHITE][PAWN]);

    for (Move* curr = list; curr != end; ++curr)
    {
        assert(RANGE_64(curr->from) && RANGE_64(curr->to));
        if (curr->promotion > 0) //Promotions have their score assigned
            continue;
        if(IS_CAP(*curr)) //There has been a capture
        {
            /*
            //Simple MVV-LVA
            int subst = (curr->piece < ROOK)? pVal[ROOK] / 7 : pVal[curr->piece] / 10;
            curr->score = pVal[curr->capture] - subst;
            */
            //SEE
            if (curr->piece == PAWN)
                curr->score = pVal[curr->capture];
            else if (curr->piece == KING)
                curr->score = pVal[curr->capture] - 100;
            else
                curr->score = 69 + seeCapture(*b, *curr);
        }
        else
        {
            if (pawnAtt & (1ULL << curr->to))
                curr->score -= 25 + 5*(PAWN - curr->piece);
            int add = history[b->stm][BASE_64(curr->from, curr->to)];
            if (add > 0)
                add = (int)sqrt(add) / 2;
            else
                add = -(int)sqrt(-add) / 2;
            curr->score += add;
        }

        if (compMoves(&bestFromPos, curr)) //It was the best refutation in the same position
        {
            curr->score += 2200;
        }
        else
        {
            if (compMoves(&killerMoves[depth][0], curr))
                curr->score += 59;
            else if (compMoves(&killerMoves[depth][1], curr))
                curr->score += 58;
        }
        /*
        if (curr->score < 300)
        {
            int gvC = givesCheck(b, *curr);
            if (gvC)
                curr->score += 50 * gvC * gvC; //Double check is more than 2 times better than a normal check, at least thats the idea
        }
        */
    }
}
inline void assignScoresQuiesce(Board* b, Move* list, const int numMoves)
{
    Move* end = list + numMoves;
    for (Move* curr = list; curr != end; ++curr)
    {
        if(IS_CAP(*curr) && !(curr->piece == PAWN && curr->promotion != 0))
        {
            //TODO: Add bonus if it captures the last piece to move
            //TODO: That could be improved using bbs of the last pieces moved
            if (curr->piece == KING)
                curr->score = pVal[curr->capture] - 50;
            else
                curr->score = seeCapture(*b, *curr);
        }
    }
}
inline void addKM(const Move m, const int depth)
{
    killerMoves[depth][0] = killerMoves[depth][1];
    killerMoves[depth][1] = m;
}
inline void addHistory(const int from, const int to, const int n, const int stm)
{
    assert(RANGE_64(from) && RANGE_64(to));
    int* h = &history[stm][BASE_64(from, to)];
    *h += n;
    if (*h > 7000)
        *h /= 10;
}
inline void decHistory(const int from, const int to, const int n, const int stm)
{
    assert(RANGE_64(from) && RANGE_64(to));
    int* h = &history[stm][BASE_64(from, to)];
    *h -= n;
    if (*h < -7000)
        *h /= 10;
}
void initHistory(void)
{
    int* end = history[0] + 4096;
    for (int* p = history[BLACK], *q = history[WHITE]; p != end; ++p, ++q)
    {
        *p = 0;
        *q = 0;
    }
}

/* Moves the largest score to the beggining of the list
 * [1 5 3 4 2] -> [5 1 4 3 2]
 * it is equivalent to one bubble sort iteration
 */
void bestMoveFst(Move* start, Move* end)
{
    Move best = *(end - 1);
    for (Move* p = end - 2; p >= start; --p)
    {
        if (best.score >= p->score){
            *(p+1) = *p;
        } else {
            *(p+1) = best;
            best = *p;
        }
    }

    *start = best;
}

/* Efficient insertion sort implementation
 * it is better than quicksort for smaller lists
 */
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

void moveToFst(Move* ls, int idx)
{
    const Move tmp = ls[idx--];
    for (; idx >= 0; --idx)
        ls[idx+1] = ls[idx];
    ls[0] = tmp;
}