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
#include "../include/gaviota.h"


//Depth of the null move prunning
#define R 3
//Margin for null move pruning, it is assumed that passing the move gives away some advantage. Measured in centipawns
#define MARGIN 11
//The centipawn loss it is willing to accept in order to avoid a 3fold repetition
#define RISK 11

#define PLUS_MATE    99999
#define MINS_MATE   -99999
#define PLUS_INF   9999999
#define MINS_INF  -9999999

static Move bestMoveList(Board b, const int depth, int alpha, int beta, Move* list, const int numMoves, Repetition rep);
__attribute__((hot)) static int pvSearch(Board b, int alpha, int beta, int depth, const int height, int null, const uint64_t prevHash, Repetition* rep);
__attribute__((hot)) static int qsearch(Board b, int alpha, const int beta);

static void expensiveSort(Board b, Move* list, const int numMoves, int alpha, const int beta, const int depth, const int height, const uint64_t prevHash, Repetition* rep);
static Move tableLookUp(Board b, int* tbAv);
static int nullMove(Board b, const int depth, const int beta, const uint64_t prevHash);
static inline const int marginDepth(const int depth);
static inline int isDraw(const Board* b, const Repetition* rep, const uint64_t newHash, const int lastMCapture);

/* Dertermine the type of position
 */
static inline int rookVSKing(const Board b)
{
    return POPCOUNT(b.piece[b.turn][ROOK]) == 1 && POPCOUNT(b.allPieces ^ b.piece[b.turn][ROOK]) == 2;
}
static inline int noZugz(const Board b)
{
    return POPCOUNT(b.color[b.turn] ^ b.piece[b.turn][PAWN]) <= 2;
}

const Move NO_MOVE = (Move) {.from = -1, .to = -1};

/* Time management */
clock_t startT = 0;
clock_t timeToMoveT = 0;
int calledTiming = 0;

/* Info string */
uint64_t nodes = 0;

/* Debug info */
uint64_t noMoveGen = 0;
uint64_t repe = 0;
uint64_t researches = 0;
uint64_t qsearchNodes = 0;
uint64_t nullCutOffs = 0;
uint64_t betaCutOff = 0;
uint64_t betaCutOffHit = 0;
uint64_t queries = 0;

int exitFlag = 0;

void initCall(void)
{
    queries = 0;
    betaCutOff = 0;
    betaCutOffHit = 0;
    qsearchNodes = 0;
    nullCutOffs = 0;
    researches = 0;
    repe = 0;
    noMoveGen = 0;
    exitFlag = 0;

    initKM();
}

Move bestTime(Board b, const clock_t timeToMove, Repetition rep, int targetDepth)
{
    /* All the time management */
    calledTiming = (targetDepth == 0)? 1 : 0;
    targetDepth  = (targetDepth == 0)? 99 : targetDepth;
    clock_t start = clock(), last, elapsed;

    timeToMoveT = timeToMove;
    startT = start;

    Move list[NMOVES];
    const int numMoves = legalMoves(&b, list) >> 1;

    //It there is only one possible move, return it
    if (numMoves == 1)
        return list[0];

    //If there is little time, caching can give problems
    if (canGav(b.allPieces))
    {
        int tbAv; //This is to ensure that the engine still works without tb
        Move tb = tableLookUp(b, &tbAv);
        if (tbAv)
        {
            infoString(tb, 0, 0, 1000 * (clock() - start) / CLOCKS_PER_SEC);
            return tb;
        }
    }

    /*
    TODO: Test if this works
    assignScores(&b, list, numMoves, NO_MOVE, 0);
    sort(list, list+numMoves);
    */
    initCall();

    Move best = list[0], temp, secondBest;
    int bestScore = 0;
    int previousBestScore = -1;
    int delta = 11;
    for (int depth = 1; depth <= targetDepth; ++depth)
    {
        //delta = max(25, delta * .8f);
        int alpha = MINS_INF, beta = PLUS_INF;
        if (depth >= 5)
        {
            alpha = bestScore - delta;
            beta = bestScore + delta;
        }
        nodes = 0;

        for (int i = 0; i < 5; ++i)
        {
            //TODO: Try placing sort before this call
            temp = bestMoveList(b, depth, alpha, beta, list, numMoves, rep);
            sort(list, list+numMoves);

            last = clock();
            elapsed = last - start;

            if ((calledTiming && elapsed > timeToMove) || temp.score >= PLUS_MATE)
                break;
            if (temp.score >= beta)
            {
                delta *= 2;
                beta += delta;
                researches++;
            }
            else if (temp.score <= alpha)
            {
                delta *= 2;
                beta = (beta + alpha) / 2;
                alpha -= delta;
                researches++;
            }
            else
                break;
        }
        /*if (calledTiming && elapsed > timeToMove)
        {
            for (int i = 0; i < numMoves - 1; ++i)
            {
                if (compMoves(&temp, &list[i]))
                {
                    if (list[i+1].score != 0)
                        best = temp;
                    break;
                }
            }
        }*/
        if (calledTiming && elapsed > timeToMove)
            break;

        best = temp;
        //previousBestScore = bestScore;
        bestScore = best.score;
        //secondBest = list[1];

        infoString(best, depth, nodes, 1000 * (last - start) / CLOCKS_PER_SEC);
        if ((calledTiming && (1.15f * (last - start) > timeToMove)) || best.score >= PLUS_MATE)
            break;
    }

    #ifdef DEBUG
    printf("Beta Hits: %f\n", (float)betaCutOffHit / betaCutOff);
    printf("Qsearch Nodes: %llu\n", qsearchNodes);
    printf("Null Cutoffs: %llu\n", nullCutOffs);
    printf("Researches: %llu\n", researches);
    printf("Repetitions: %llu\n", repe);
    printf("Queries: %llu\n", queries);
    #endif

    //Choose a worse move in order to avoid 3fold repetition, if the risk is low enough
    //Another (better) alternative is to include random noise when the score is assigned if it is a 3fold rep, to avoid messing up other evaluations
    //if (bestScore == 0 && previousBestScore == 0 && numMoves > 1 && -secondBest.score < RISK)
    //    best = secondBest;

    calledTiming = 0;
    return best;
}

int callDepth;
static Move bestMoveList(Board b, const int depth, int alpha, int beta, Move* list, const int numMoves, Repetition rep)
{
    assert(depth > 0);
    if (rookVSKing(b))
    {
        Move rookM = rookMate(b);
        for (int i = 0; i < numMoves; ++i)
        {
            if (compMoves(&list[i], &rookM) && list[i].piece == rookM.piece)
            {
                list[i].score = PLUS_MATE + 10;
                return rookM;
            }
            //else
                //printf("ROOK MATE FAIL");
        }
    }
    callDepth = depth;

    History h;
    Move currBest = list[0];
    int val;
    uint64_t hash = hashPosition(&b), newHash;

    for (int i = 0; i < numMoves; ++i)
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
                val = -pvSearch(b, -beta, -alpha, depth - 1, 1, 0, newHash, &rep);
            }
            else
            {
                val = -pvSearch(b, -alpha - 1, -alpha, depth - 1, 1, 0, newHash, &rep);
                if (val > alpha && val < beta)
                    val = -pvSearch(b, -beta, -alpha, depth - 1, 1, 0, newHash, &rep);
            }
            remHash(&rep);
        }
        undoMove(&b, list[i], &h);

        //For the sorting at later depths
        list[i].score = val;

        if (val > alpha)
        {
            currBest = list[i];
            alpha = val;

            if (val >= PLUS_MATE + depth - 1 || val >= beta)
                break;
        }
    }

    return currBest;
}
static int pvSearch(Board b, int alpha, int beta, int depth, const int height, const int null, const uint64_t prevHash, Repetition* rep)
{
    const int pv = beta - alpha > 1;
    //assert(beta >= alpha);
    nodes++;
    const int index = prevHash & MOD_ENTRIES;

    if (canGav(b.allPieces))
    {
        int usable;
        int gavScore = gavWDL(b, &usable) * (PLUS_MATE - 110 + height);
        if (usable)
        {
            #ifdef DEBUG
            queries++;
            #endif
            return b.turn? gavScore : -gavScore;
        }
    }

    if (calledTiming && (exitFlag || (nodes & 4095) == 0 && clock() - startT > timeToMoveT))
    {
        exitFlag = 1;
        return 0;
    }

    const int isInC = isInCheck(&b, b.turn);

    if (isInC)
        depth++;
    else if (depth == 0)
        return qsearch(b, alpha, beta);

    int val;
    Move bestM = NO_MOVE;
    if (table[index].key == prevHash)
    {
        if (table[index].depth >= depth)
        {
            switch (table[index].flag)
            {
                case LO:
                    alpha = max(alpha, table[index].val);
                    break;
                case HI:
                    beta = min(beta, table[index].val);
                    break;
                    /*
                case EXACT:
                    val = table[index].val;
                    if (val > PLUS_MATE) val -= 1;
                    return val;
                    */
            }
            if (alpha >= beta)
                return table[index].val;
        }
        bestM = table[index].m;
    }

    const int ev = eval(&b);
    const int nZg = noZugz(b);
    const int isSafe = !isInC && !nZg;

    if (isSafe)
    {
        /* Razoring */
        //if (depth == 1 && ev + VROOK + 101 <= alpha && isSafe)
        //    return qsearch(b, alpha, beta);
        /* Static pruning */
        if (!pv && depth <= 4 && ev - 100 * depth >= beta && ev < 8000)
            return ev;

        if (!pv && depth <= 6 && ev - 180 * depth >= beta && ev < 7500)
            return ev;

        /* Null move pruning */
        //int r = R + (depth >> 3); //Make a variable r
        if (!null && depth > R)
        {
            if (nullMove(b, depth, beta, prevHash))
            {
                #ifdef DEBUG
                ++nullCutOffs;
                #endif
                return beta;
            }
        }
    }

    Move list[NMOVES];
    const int numMoves = legalMoves(&b, list) >> 1;
    if (!numMoves)
        return isInC * (MINS_MATE - 100 + height);

    const int newHeight = height + 1;
    uint64_t newHash;
    int best = MINS_INF;
    const int origAlpha = alpha;

    assignScores(&b, list, numMoves, bestM, depth);
    sort(list, list+numMoves);
    Move mt = list[0];
/*
    if (depth >= 5)
    {
        makeMove(&b, mt, &h);
        newHash = makeMoveHash(prevHash, &b, mt, h);
        best = -pvSearch(b, -beta, -alpha, depth - 1, newHeight, null, newHash, rep);
        if (best > alpha)
        {
            alpha = best;
            if (best >= beta)
                return best;
        }
        undoMove(&b, mt, &h);
        expensiveSort(b, list, numMoves, alpha, beta, depth / 5, newHeight, prevHash, rep);
        expSort = 1;
    }
*/


    int spEval = 0;
    if (depth <= 3)
        spEval = ev + marginDepth(depth);

    const int canBreak = depth <= 3 && spEval <= alpha && isSafe;

    History h;
    for (int i = 0; i < numMoves; ++i)
    {
        if (canBreak && i > 4 && list[i].score < 100)
            break;
        //if (expSort && compMoves(&mt, &list[i]))
        //  continue;

        makeMove(&b, list[i], &h);
        newHash = makeMoveHash(prevHash, &b, list[i], h);

        if (isDraw(&b, rep, newHash, list[i].capture > 0))
        {
            val = 0;
        }
        else
        {
            int reduction = 1;
            addHash(rep, newHash);
            if (i == 0)
            {
                val = -pvSearch(b, -beta, -alpha, depth - 1, newHeight, null, newHash, rep);
            }
            else
            {
                if (depth >= 3 && i > 4 && list[i].score < 100 && !isInC)
                    reduction++;
                if (!pv && list[i].piece == KING && isInC && list[i].capture < 1)
                    reduction++;
                if (pv && list[i].score > 60 && list[i].capture > 0)
                    reduction--;
                //if (list[i].piece == KING && list[i].castle)
                //    reduction--;

                if (reduction > depth)
                    reduction = depth;

                val = -pvSearch(b, -alpha-1, -alpha, depth - reduction, newHeight, null, newHash, rep);
                if (val > alpha && val < beta)
                    val = -pvSearch(b, -beta, -alpha, depth - 1, newHeight, null, newHash, rep);
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

    table[index] = (Eval) {.key = prevHash, .m = bestM, .val = best, .depth = depth, .flag = flag};

    return best;
}

static int qsearch(Board b, int alpha, const int beta)
{
    #ifdef DEBUG
    ++qsearchNodes;
    #endif

    const int score = eval(&b);

    if (score >= beta)
        return beta;
    else if (score > alpha)
        alpha = score;
    else if (score + VQUEEN /*+ ((m.promotion > 0)? VQUEEN : 0)*/ <= alpha) /*TODO: Check for zugz*/
        return alpha;

    Move list[NMOVES];
    const int numMoves = legalMovesQuiesce(&b, list) >> 1;
    History h;

    assignScoresQuiesce(&b, list, numMoves);
    sort(list, list+numMoves);

    int val;

    for (int i = 0; i < numMoves; ++i)
    {
        if ((list[i].score + score + 125 <= alpha) || list[i].score < 60)
        //if (list[i].score < 60)
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

/* In this function there are no assumptions made about the sorting of the list
 */
static void expensiveSort(Board b, Move* list, const int numMoves, int alpha, const int beta, const int depth, const int height, const uint64_t prevHash, Repetition* rep)
{
    assert(depth > 0);
    //assert(beta >= alpha);

    uint64_t newHash;
    int val;

    History h;
    for (int i = 0; i < numMoves; ++i)
    {
        makeMove(&b, list[i], &h);
        newHash = makeMoveHash(prevHash, &b, list[i], h);

        if (isDraw(&b, rep, newHash, list[i].capture > 0))
        {
            val = 0;
        }
        else
        {
            addHash(rep, newHash);
            val = -pvSearch(b, -beta - 25, -alpha + 25, depth - 1, 0, 0, newHash, rep);
            remHash(rep);
        }

        list[i].score = val;

        undoMove(&b, list[i], &h);
    }

    sort(list, list+numMoves);
}

static Move tableLookUp(Board b, int* tbAv)
{
    Move list[NMOVES];
    Move temp[NMOVES];

    const int numMoves = legalMoves(&b, list) >> 1;
    int bestScore = -1;
    Move bestM = list[0];

    History h;
    for (int i = 0; i < numMoves; ++i)
    {
        makeMove(&b, list[i], &h);
        const int lgm = legalMoves(&b, temp);
        int score;
        if (lgm == 1)
        {
            //Tablebases dont work the same when there is mate
            bestScore = PLUS_MATE - 1;
            bestM = list[i];
            break;
        }
        else if (lgm == 0)
        {
            score = 0;
        }
        else
            score = -gavScore(b, tbAv);

        if (score > 0)
            score = PLUS_MATE - score;
        if (score < bestScore && bestScore < 0) //Find the longest losing mate, hoping the opponent will draw or 50 move rule
        {
            bestScore = score;
            bestM = list[i];
        }
        else if (score == 0 && bestScore < 0) //Go for draw if we are losing
        {
            bestScore = score;
            bestM = list[i];
        }
        else if (score > 0 && score > bestScore) //Shortest distance to mate
        {
            bestScore = score;
            bestM = list[i];
        }

        undoMove(&b, list[i], &h);
    }

    bestM.score = bestScore;
    if (bestScore > 0)
        bestM.score = 2 * PLUS_MATE - bestScore;

    return bestM;
}

static int nullMove(Board b, const int depth, const int beta, const uint64_t prevHash)
{
    const int betaMargin = beta - MARGIN;
    Repetition _rep = (Repetition) {.index = 0};
    b.turn ^= 1;
    //In stead of depth - R - 1 do depth / 5, it is cheaper and gives about 30 elo
    const int val = -pvSearch(b, -betaMargin, -betaMargin + 1, depth / 5, 1, 1, changeTurn(prevHash), &_rep);
    b.turn ^= 1;

    return val >= betaMargin;
}
static inline int isDraw(const Board* b, const Repetition* rep, const uint64_t newHash, const int lastMCapture)
{
    if (lastMCapture)
        return insuffMat(b);
    else
    {
        #ifdef DEBUG
        if (isRepetition(rep, newHash)) repe++;
        #endif
        //Repetition pruning, it doesnt seem to give or take elo, but since I came up with it it is going to stay
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