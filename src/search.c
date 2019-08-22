/* search.c
 * Performs the actual search to find the best move
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"
#include "../include/allmoves.h"
#include "../include/hash.h"
#include "../include/search.h"
#include "../include/evaluation.h"
#include "../include/mate.h"
#include "../include/uci.h"
#include "../include/io.h"


//Depth of the null move prunning
#define R 3
//Margin for null move pruning, it is assumed that passing the move gives away some advantage. Measured in centipawns
#define MARGIN 14

#define NUM_KM 2

#define PLUS_MATE    99999
#define MINS_MATE   -99999
#define PLUS_INF   9999999
#define MINS_INF  -9999999

Move bestMoveList(Board b, const int depth, Move* list, const int numMoves, Repetition rep);
int alphaBeta(Board b, int alpha, int beta, const int depth, int null, const uint64_t prevHash, Move m, Move* prevBest, Repetition* rep);
int pvSearch(Board b, int alpha, int beta, int depth, int null, const uint64_t prevHash, Move m, Repetition* rep);
int zwSearch(Board b, int beta, int depth);
int qsearch(Board b, int alpha, int beta, const Move m);

static inline void addKM(const Move m, const int depth);
static inline void assignScores(Move* list, const int numMoves, const Move bestFromPos, const int depth);
static inline void assignScoresQuiesce(Move* list, const int numMoves);
void sort(Move* list, const int numMoves);

static inline int rookVSKing(const Board b)
{
    return POPCOUNT(b.piece[b.turn][ROOK]) == 1 && POPCOUNT(b.allPieces ^ b.piece[b.turn][ROOK]) == 2;
}
static inline int onlyPawns(const Board b)
{
    return POPCOUNT(b.allPieces ^ b.piece[WHITE][PAWN] ^ b.piece[BLACK][PAWN]) == 2;
}
static inline int compMoves(const Move* m1, const Move* m2)
{
    return m1->from == m2->from && m1->to == m2->to;
}

clock_t startT;
clock_t timeToMoveT;
int calledTiming = 0;

uint64_t nodes = 0;
uint64_t qsearchNodes = 0;
uint64_t nullCutOffs = 0;
uint64_t betaCutOff;
uint64_t betaCutOffHit;
const Move NoMove = (Move) {.from = -1, .to = -1};


Move killerMoves[99][NUM_KM];

Move bestTime(Board b, const clock_t timeToMove, Repetition rep, int targetDepth)
{
    calledTiming = (targetDepth == 0)? 1 : 0;
    targetDepth =  (targetDepth == 0)? 99 : targetDepth;
    clock_t start = clock(), last, elapsed;

    timeToMoveT = timeToMove;
    startT = start;

    Move list[NMOVES];
    int numMoves = legalMoves(&b, list) >> 1;

    betaCutOff = 0;
    betaCutOffHit = 0;
    qsearchNodes = 0;
    nullCutOffs = 0;
    Move best = list[0], temp;
    for (int i = 0; i < 99; ++i)
    {
        for (int j = 0; j < NUM_KM; ++j)
            killerMoves[i][j] = NoMove;
    }
    for (int depth = 1; depth <= targetDepth; ++depth)
    {
        nodes = 0;
        temp = bestMoveList(b, depth, list, numMoves, rep);
        sort(list, numMoves);
        last = clock();
        elapsed = last - start;
        if (calledTiming && elapsed > timeToMove)
            break;

        best = temp;

        /*
         * Due to the exponential nature, if the time remeaning is smaller than 3 * timeTaken, break, it is unlikely that
         * the program will be able to finish another depth
         * (2 is a randomly chosen constant based on experience, it should be improved using the ratio between consecutive searches)
         * or
         * it has found mate
         */
        infoString(best, depth, nodes, 1000 * (last - start) / CLOCKS_PER_SEC);
        if ((calledTiming && (last - start > timeToMove)) || best.score >= PLUS_MATE)
            break;
    }

    #ifdef DEBUG
    printf("Beta Hits: %f\n", (float)betaCutOffHit / betaCutOff);
    printf("Qsearch Nodes: %llu\n", qsearchNodes);
    printf("Null Cutoffs: %llu\n", nullCutOffs);
    #endif

    calledTiming = 0;
    return best;
}

int callDepth;
Move bestMoveList(Board b, const int depth, Move* list, const int numMoves, Repetition rep)
{
    nodes = 0;
    if (rookVSKing(b)) return rookMate(b);
    if (depth == 0) return (Move) {};
    callDepth = depth;

    History h;

    Move currBest = list[0];
    int val, exit = 0;

    uint64_t hash = hashPosition(&b); //The position should be already added to res
    int alpha = MINS_INF;

    for (int i = 0; i < numMoves && !exit; ++i)
    {
        makeMove(&b, list[i], &h);
        uint64_t newHash = makeMoveHash(hash, &b, list[i], h);
        if (insuffMat(&b) || isThreeRep(&rep, newHash))
            val = 0;
        else
        {
            addHash(&rep, newHash);
            if (i == 0)
            {
                val = -pvSearch(b, MINS_INF, -alpha, depth - 1, 0, newHash, list[i], &rep);
            }
            else
            {
                val = -pvSearch(b, -alpha - 1, -alpha, depth - 1, 0, newHash, list[i], &rep);
                if (val > alpha)
                    val = -pvSearch(b, MINS_INF, -alpha, depth - 1, 0, newHash, list[i], &rep);
            }
            remHash(&rep);
        }
        undoMove(&b, list[i], &h);

        if (val > alpha)
        {
            currBest = list[i];
            alpha = val;
            //No need to keep searching once it has found a mate in 1
            if (val >= PLUS_MATE + depth - 1)
                exit = 1;
        }

        //For the sorting
        list[i].score = val;
    }

    return currBest;
}
Move bestMoveAB(Board b, const int depth, int divide, Repetition rep)
{
    nodes = 0;
    if (rookVSKing(b)) return rookMate(b);
    if (depth == 0) return (Move) {};
    callDepth = depth;

    Move list[NMOVES];
    History h;

    int numMoves = legalMoves(&b, list) >> 1;

    assignScores(list, numMoves, NoMove, depth);
    sort(list, numMoves);

    Move currBest = list[0];
    int val;

    uint64_t hash = hashPosition(&b); //The position should be already added to res
    int alpha = MINS_INF;

    for (int i = 0; i < numMoves; ++i)
    {
        makeMove(&b, list[i], &h);
        uint64_t newHash = makeMoveHash(hash, &b, list[i], h);
        if (insuffMat(&b) || isThreeRep(&rep, newHash))
            val = 0;
        else
        {
            addHash(&rep, newHash);
            val = -pvSearch(b, -alpha - 1, -alpha, depth - 1, 0, newHash, list[i], &rep);
            if (val > alpha)
                val = -pvSearch(b, MINS_INF, -alpha, depth - 1, 0, newHash, list[i], &rep);
            remHash(&rep);
        }
        undoMove(&b, list[i], &h);

        list[i].score = val;

        if (divide)
        {
            drawMove(list[i]);
            printf(": %d\n", val);
        }

        if (val > alpha)
        {
            currBest = list[i];
            alpha = val;
            if (val > PLUS_MATE + depth - 2) break;
        }
    }

    return currBest;
}

int pvSearch(Board b, int alpha, int beta, int depth, const int null, const uint64_t prevHash, Move m, Repetition* rep)
{
    assert(beta >= alpha);
    nodes++;
    if (calledTiming && clock() - startT > timeToMoveT)
        return 0;

    const int isInC = isInCheck(&b, b.turn);

    if (isInC)
        depth++;
    else if (depth == 0)
        return qsearch(b, alpha, beta, m);

    const int index = prevHash & MOD_ENTRIES;
    int val;
    Move bestM = NoMove;
    if (table[index].key == prevHash)
    {
        bestM = table[index].m;
        /*
        if (table[index].depth == depth)
        {
            switch (table[index].flag)
            {
                case LO:
                    alpha = max(alpha, table[index].val);
                    break;
                case HI:
                    beta = min(beta, table[index].val);
                    break;
                case EXACT:
                    val = table[index].val;
                    if (val > PLUS_MATE) val -= 1;
                    return val;
                
            }
            if (alpha >= beta)
                return table[index].val;
        }
        */
    }

    const int onlyP = onlyPawns(b);
    //int r = R + (depth >> 3); //Make a variable r
    if (!null && depth > R && !isInC && /*m.score < 320 &&*/ !onlyP)
    {
        int betaMargin = beta - MARGIN;
        Repetition _rep = (Repetition) {.index = 0};
        b.turn ^= 1;
        val = -pvSearch(b, -betaMargin, -betaMargin + 1, depth - R - 1, 1, 0, m, &_rep);
        b.turn ^= 1;

        if (val >= betaMargin)
        {
            #ifdef DEBUG
            ++nullCutOffs;
            #endif
            return beta;
        }
    }

    Move list[NMOVES];
    const int lgm = legalMoves(&b, list); //lgm is an int representing (2 * numMoves + isInCheck), in order to avoid having to check for mate
    const int numMoves = lgm >> 1;
    if (!numMoves)
        return lgm * (MINS_MATE - depth);

    History h;

    assignScores(list, numMoves, bestM, depth);
    sort(list, numMoves);

    uint64_t newHash;
    int best = MINS_INF;

    const int origAlpha = alpha;

    const int spEval = standPatEval(&b, b.turn);
    for (int i = 0; i < numMoves; ++i)
    {
        if (depth <= 3 && spEval < alpha && !isInC && !onlyP && i > 4 && list[i].score < 100)
            continue;
        makeMove(&b, list[i], &h);
        newHash = makeMoveHash(prevHash, &b, list[i], h);
        if ((m.capture > 0 && insuffMat(&b)) || isPrevPosition(rep, newHash) || isThreeRep(rep, newHash))
        {
            val = 0;
        }
        else
        {
            addHash(rep, newHash);
            if (i == 0)
            {
                val = -pvSearch(b, -beta, -alpha, depth - 1, null, newHash, list[i], rep);
            }
            else
            {
                int reduction = 1;
                if (depth > 2 && i > 4 && list[i].capture < 1 && !isInC)
                    ++reduction;
                val = -pvSearch(b, -alpha-1, -alpha, depth - reduction, null, newHash, list[i], rep);
                if (val > alpha && val < beta)
                    val = -pvSearch(b, -beta, -alpha, depth - 1, null, newHash, list[i], rep);
            }

            //if (m.promotion > 0)
            //    val += depth << 2;
            remHash(rep);
        }

        if (val > best)
        {
            best = val;
            bestM = list[i];
            if (best > alpha)
            {
                alpha = best;

                if (alpha >= beta)
                {
                    #ifdef DEBUG
                    ++betaCutOff;
                    if (i == 0)
                        ++betaCutOffHit;
                    #endif

                    if (bestM.capture < 1)
                        addKM(bestM, depth);
                    break;
                }
                else if (best >= PLUS_MATE + depth - 1)
                {
                    addKM(bestM, depth);
                    break;
                }
            }
        }

        undoMove(&b, list[i], &h);
    }

    int flag = EXACT;

    if (best <= origAlpha)
        flag = HI;
    else if (best >= beta)
        flag = LO;

    table[index] = (Eval) {.key = prevHash, .val = val, .depth = depth, .m = bestM, .flag = flag};

    return best;
}
int qsearch(Board b, int alpha, const int beta, const Move m)
{
    #ifdef DEBUG
    ++qsearchNodes;
    #endif

    const int score = b.turn? eval(&b) : -eval(&b);

    if (score >= beta)
        return beta;
    else if (score > alpha)
        alpha = score;
    else if (score + 800 + ((m.promotion > 0)? 800 : 0) <= alpha)
        return alpha;

    Move list[NMOVES];
    History h;
    const int numMoves = legalMoves(&b, list) >> 1;

    assignScoresQuiesce(list, numMoves);
    sort(list, numMoves);

    int val;

    for (int i = 0; i < numMoves; ++i)
    {
        if (list[i].score < 10)
            break;

        makeMove(&b, list[i], &h);

        if (insuffMat(&b)) //No need to check for 3 fold rep
            val = 0;
        else
            val = -qsearch(b, -beta, -alpha, list[i]);

        if (val >= beta)
            return beta;
        if (val > alpha)
            alpha = val;

        undoMove(&b, list[i], &h);
    }

    return alpha;
}

static const int score[6] = {480, 400, 320, 240, 160, 80};
static inline void captureScore(Move* m)
{
    m->score = score[m->capture] - (score[m->pieceThatMoves] >> 4);
}
inline void assignScores(Move* list, const int numMoves, const Move bestFromPos, const int depth)
{
    for (int i = 0; i < numMoves; ++i)
    {
        if(list[i].capture > 0) //There has been a capture
            captureScore(&list[i]);  //MVV - LVA
        if (compMoves(&bestFromPos, &list[i])) //It was the best refutation in the same position
            list[i].score += 500;
        else
        {
            for (int j = 0; j < NUM_KM; ++j)
            {
                if (compMoves(&killerMoves[depth][j], &list[i]))
                {
                    list[i].score += 200 + j;
                    break;
                }
            }
        }
    }
}
inline void assignScoresQuiesce(Move* list, const int numMoves)
{
    for (int i = 0; i < numMoves; ++i)
    {
        if(list[i].capture > 0)
            captureScore(&list[i]);//Bonus if it captures the piece that moved last time, it reduces the qsearch nodes about 30%
    }
}
static inline void addKM(const Move m, const int depth)
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