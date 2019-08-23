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
#include "../include/sort.h"
#include "../include/search.h"
#include "../include/evaluation.h"
#include "../include/mate.h"
#include "../include/uci.h"
#include "../include/io.h"


//Depth of the null move prunning
#define R 3
//Margin for null move pruning, it is assumed that passing the move gives away some advantage. Measured in centipawns
#define MARGIN 13
//The centipawn loss it is willing to accept in order to avoid a 3fold repetition
#define RISK 11

#define PLUS_MATE    99999
#define MINS_MATE   -99999
#define PLUS_INF   9999999
#define MINS_INF  -9999999

Move bestMoveList(Board b, const int depth, int alpha, int beta, Move* list, const int numMoves, Repetition rep);
__attribute__((hot)) int pvSearch(Board b, int alpha, int beta, int depth, int null, const uint64_t prevHash, Move m, Repetition* rep);
__attribute__((hot)) int qsearch(Board b, int alpha, const int beta);

static inline const int marginDepth(const int depth);
static inline int isDraw(const Board* b, const Repetition* rep, const uint64_t newHash, const int lastMCapture);

static inline int rookVSKing(const Board b)
{
    return POPCOUNT(b.piece[b.turn][ROOK]) == 1 && POPCOUNT(b.allPieces ^ b.piece[b.turn][ROOK]) == 2;
}
static inline int onlyPawns(const Board b)
{
    return POPCOUNT(b.allPieces ^ b.piece[WHITE][PAWN] ^ b.piece[BLACK][PAWN]) == 2;
}

const Move NO_MOVE = (Move) {.from = -1, .to = -1};

clock_t startT;
clock_t timeToMoveT;
int calledTiming = 0;

uint64_t nodes = 0;
uint64_t repet = 0;
uint64_t researches = 0;
uint64_t qsearchNodes = 0;
uint64_t nullCutOffs = 0;
uint64_t betaCutOff = 0;
uint64_t betaCutOffHit = 0;

void initCall(void)
{
    betaCutOff = 0;
    betaCutOffHit = 0;
    qsearchNodes = 0;
    nullCutOffs = 0;
    repet = 0;
    researches = 0;
    initKM();
}

Move bestTime(Board b, const clock_t timeToMove, Repetition rep, int targetDepth)
{
    calledTiming = (targetDepth == 0)? 1 : 0;
    targetDepth =  (targetDepth == 0)? 99 : targetDepth;
    clock_t start = clock(), last, elapsed;

    timeToMoveT = timeToMove;
    startT = start;

    Move list[NMOVES];
    int numMoves = legalMoves(&b, list) >> 1;

    if (numMoves == 1)
        return list[0];

    initCall();

    Move best = list[0], temp, secondBest;
    int bestScore = 0;
    int delta = 5;
    for (int depth = 1; depth <= targetDepth; ++depth)
    {
        delta = max(5, delta / 5);
        int alpha = MINS_INF, beta = PLUS_INF;
        if (depth > 5)
        {
            alpha = bestScore - delta;
            beta = bestScore + delta;
        }
        nodes = 0;

        for (int i = 0; i < 4; ++i)
        {
            temp = bestMoveList(b, depth, alpha, beta, list, numMoves, rep);
            sort(list, numMoves);

            last = clock();
            elapsed = last - start;
            if ((calledTiming && elapsed > timeToMove) || temp.score >= PLUS_MATE)
                break;

            delta *= 10;
            if (temp.score >= beta)
            {
                beta += delta;
                #ifdef DEBUG
                researches++;
                #endif
            }
            else if (temp.score <= alpha)
            {
                beta = (beta + alpha) / 2;
                alpha -= delta;
                #ifdef DEBUG
                researches++;
                #endif
            }
            else
                break;
        }
        if (calledTiming && elapsed > timeToMove)
            break;
        best = temp;
        bestScore = best.score;
        secondBest = list[1];

        infoString(best, depth, nodes, 1000 * (last - start) / CLOCKS_PER_SEC);
        if ((calledTiming && (1.15f * (last - start) > timeToMove)) || best.score >= PLUS_MATE)
            break;
    }

    #ifdef DEBUG
    printf("Beta Hits: %f\n", (float)betaCutOffHit / betaCutOff);
    printf("Qsearch Nodes: %llu\n", qsearchNodes);
    printf("Null Cutoffs: %llu\n", nullCutOffs);
    printf("Repetition Cutoffs: %llu\n", repet);
    printf("Researches: %llu\n", researches);
    #endif

    if (bestScore == 0 && numMoves > 1 && -secondBest.score < RISK)
        best = secondBest;

    calledTiming = 0;
    return best;
}

int callDepth;
Move bestMoveList(Board b, const int depth, int alpha, int beta, Move* list, const int numMoves, Repetition rep)
{
    assert(depth > 0);
    nodes = 0;
    if (rookVSKing(b))
    {
        Move rookM = rookMate(b);
        for (int i = 0; i < numMoves; ++i)
        {
            if (compMoves(&list[i], &rookM) && list[i].piece == rookM.piece)
            {
                list[i].score = PLUS_MATE + 10;
                return rookM; //printf("ROOK MATE FAIL");
            }
        }
    }
    callDepth = depth;

    History h;
    Move currBest = list[0];
    int val, exit = 0;
    uint64_t hash = hashPosition(&b), newHash;

    for (int i = 0; i < numMoves && !exit; ++i)
    {
        makeMove(&b, list[i], &h);
        newHash = makeMoveHash(hash, &b, list[i], h);
        if (insuffMat(&b) || isThreeRep(&rep, newHash))
        {
            val = 0;
        }
        else
        {
            addHash(&rep, newHash);
            if (i == 0)
            {
                val = -pvSearch(b, -beta, -alpha, depth - 1, 0, newHash, list[i], &rep);
            }
            else
            {
                val = -pvSearch(b, -alpha - 1, -alpha, depth - 1, 0, newHash, list[i], &rep);
                if (val > alpha)
                    val = -pvSearch(b, -beta, -alpha, depth - 1, 0, newHash, list[i], &rep);
            }
            remHash(&rep);
        }
        undoMove(&b, list[i], &h);

        //For the sorting in later depths
        list[i].score = val;

        if (val > alpha)
        {
            currBest = list[i];
            alpha = val;
            //No need to keep searching once it has found a mate in 1
            if (val >= PLUS_MATE + depth - 1)
                exit = 1;
        }
    }

    //assert(currBest.score == alpha);
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

    assignScores(&b, list, numMoves, NO_MOVE, depth);
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
        return qsearch(b, alpha, beta);

    const int index = prevHash & MOD_ENTRIES;
    int val;
    Move bestM = NO_MOVE;
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
    const int isSafe = !isInC && !onlyP;
    //int r = R + (depth >> 3); //Make a variable r
    if (!null && depth > R && isSafe /*m.score < 320 &&*/)
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
    const int numMoves = legalMoves(&b, list) >> 1;
    if (!numMoves)
        return isInC * (MINS_MATE - depth);

    History h;
    assignScores(&b, list, numMoves, bestM, depth);
    sort(list, numMoves);

    uint64_t newHash;
    int best = MINS_INF;

    const int origAlpha = alpha;

    int spEval = 0;
    if (depth <= 3)
        spEval = eval(&b) + marginDepth(depth);

    for (int i = 0; i < numMoves; ++i)
    {
        if (depth <= 3 && spEval < alpha && isSafe && i > 4 && list[i].score < 100)
            continue;

        makeMove(&b, list[i], &h);
        newHash = makeMoveHash(prevHash, &b, list[i], h);

        if (isDraw(&b, rep, newHash, list[i].capture > 0))
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
                if (depth >= 3 && i > 4 && list[i].capture < 1 && !isInC)
                    ++reduction;
                val = -pvSearch(b, -alpha-1, -alpha, depth - reduction, null, newHash, list[i], rep);
                if (val > alpha && val < beta)
                    val = -pvSearch(b, -beta, -alpha, depth - 1, null, newHash, list[i], rep);
            }
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
            }
        }

        undoMove(&b, list[i], &h);
    }

    int flag = EXACT;

    if (best <= origAlpha)
        flag = HI;
    else if (best >= beta)
        flag = LO;

    table[index] = (Eval) {.key = prevHash, .m = bestM, .val = val, .depth = depth, .flag = flag};

    return best;
}
int qsearch(Board b, int alpha, const int beta)
{
    #ifdef DEBUG
    ++qsearchNodes;
    #endif

    const int score = eval(&b);

    if (score >= beta)
        return beta;
    else if (score > alpha)
        alpha = score;
    else if (score + VQUEEN /*+ ((m.promotion > 0)? VQUEEN : 0)*/ <= alpha)
        return alpha;

    Move list[NMOVES];
    History h;
    const int numMoves = legalMoves(&b, list) >> 1;

    assignScoresQuiesce(&b, list, numMoves);
    sort(list, numMoves);

    int val;

    for (int i = 0; i < numMoves; ++i)
    {
        //if ((list[i].score + score <= alpha) || list[i].score < 10)
        if (list[i].score < 10)
            break;

        makeMove(&b, list[i], &h);

        if (insuffMat(&b)) //No need to check for 3 fold rep
            val = 0;
        else
            val = -qsearch(b, -beta, -alpha);

        if (val >= beta)
            return beta;
        if (val > alpha)
            alpha = val;

        undoMove(&b, list[i], &h);
    }

    return alpha;
}
static inline int isDraw(const Board* b, const Repetition* rep, const uint64_t newHash, const int lastMCapture)
{
    if (lastMCapture)
        return insuffMat(b);
    else
    {
        #ifdef DEBUG
        if (isRepetition(rep, newHash)) repet++;
        #endif
        return isRepetition(rep, newHash) || isThreeRep(rep, newHash);
    }

    return 0;
}
static inline const int marginDepth(const int depth)
{
    switch (depth)
    {
        case 1:
            return VBISH;
        case 2:
            return VROOK;
        case 3:
            return VQUEEN;
    }
    return 0;
}