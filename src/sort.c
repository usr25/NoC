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
#include "../include/evaluation.h"

static int smallestAttackerSqr(const Board* b, const int sqr, const int col);
static inline int seeCapture(Board b, const Move m);
__attribute__((hot)) static int see(Board* b, const int to, const int pieceAtSqr);

#define NUM_KM 2

static const Move NOMOVE = (Move) {.from = -1, .to = -1};
int pVal[6];
int history[4096]; //TODO: make it 2x4096 to discriminate between stm
Move killerMoves[99][NUM_KM];

inline int compMoves(const Move* m1, const Move* m2)
{
    return m1->from == m2->from && m1->to == m2->to;
}

void initSort(void)
{
    pVal[0] = 2320;
    pVal[1] = 1267;
    pVal[2] = 609;
    pVal[3] = 385;
    pVal[4] = 372;
    pVal[5] = 116;
}

void initKM(void)
{
    for (int i = 0; i < 99; ++i)
    {
        for (int j = 0; j < NUM_KM; ++j)
            killerMoves[i][j] = NOMOVE;
    }
}

static inline int seeCapture(Board b, const Move m)
{
    makePermaMove(&b, m);
    return pVal[m.capture] - see(&b, m.to, m.piece);
}
static int see(Board* b, const int sqr, const int pieceAtSqr)
{
    const int col = b->stm;
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
    Move* end = list + numMoves;

    uint64_t pawnAtt;
    if (b->stm)
        pawnAtt = ((b->piece[BLACK][PAWN] >> 9) & 0x7f7f7f7f7f7f7f7f) | ((b->piece[BLACK][PAWN] >> 7) & 0xfefefefefefefefe);
    else
        pawnAtt = ((b->piece[WHITE][PAWN] << 9) & 0xfefefefefefefefe) | ((b->piece[WHITE][PAWN] << 7) & 0x7f7f7f7f7f7f7f7f);

    for (Move* curr = list; curr != end; ++curr)
    {
        if (curr->promotion > 0) //Promotions have their score assigned
            continue;
        if(curr->capture > 0) //There has been a capture
        {
            /*
            int subst = (curr->piece < ROOK)? pVal[ROOK] / 7 : pVal[curr->piece] / 10;
            curr->score = pVal[curr->capture] - subst;
            */
            if (curr->piece == PAWN)
                curr->score = pVal[curr->capture];
            else
                curr->score = 69 + seeCapture(*b, *curr);
        }
        else
        {
            if (pawnAtt & (1ULL << curr->to))
                curr->score -= 20; //TODO: Change this based on the moving piece values
            int add = history[(curr->from << 6) + curr->to];
            if (add > 15) add = 16;
            curr->score += add;
        }

        if (compMoves(&bestFromPos, curr)) //It was the best refutation in the same position
        {
            curr->score += 1200;
        }
        else
        {
            if (compMoves(&killerMoves[depth][0], curr))
                curr->score += 58;
            else if (compMoves(&killerMoves[depth][1], curr))
                curr->score += 59;
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
        if (curr->promotion > 0)
            continue;
        if(curr->capture > 0)
        {
            //TODO: Add bonus if it captures the last piece to move
            //TODO: That could be improved using bbs of the last pieces moved
            if (curr->piece == PAWN)
                curr->score = pVal[curr->capture] - 23;
            else
                curr->score = 69 + seeCapture(*b, *curr);
        }
    }
}
inline void addKM(const Move m, const int depth)
{
    killerMoves[depth][0] = killerMoves[depth][1];
    killerMoves[depth][1] = m;
}
inline void addHistory(const int from, const int to)
{
    history[(from << 6) + to]++;
}
void initHistory(void)
{
    int* end = history + 4096;
    for (int* p = history; p != end; ++p)
        *p = 0;
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