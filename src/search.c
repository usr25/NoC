/* search.c
 * Performs the search to find the best move
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"
#include "../include/memoization.h"
#include "../include/allmoves.h"
#include "../include/hash.h"
#include "../include/sort.h"
#include "../include/search.h"
#include "../include/evaluation.h"
#include "../include/mate.h"
#include "../include/uci.h"
#include "../include/io.h"
#include "../include/nnue.h"
#ifdef USE_TB
#include "../include/gaviota.h"
#endif

//Depth of the null move prunning
#define R 3


static Move bestMoveList(Board b, const int depth, int alpha, int beta, Move* list, const int numMoves, Repetition rep);
__attribute__((hot)) static int pvSearch(Board b, int alpha, int beta, int depth, const int height, int null, const uint64_t prevHash, Repetition* rep, const int isInC);

static void internalIterDeepening(Board b, Move* list, const int numMoves, int alpha, const int beta, const int depth, const int height, const uint64_t prevHash, Repetition* rep);
static Move tableLookUp(Board b, int* tbAv);
static int nullMove(Board b, const int depth, const int beta, const uint64_t prevHash);
static inline int isDraw(const Board* b, const Repetition* rep, const uint64_t newHash, const int lastMCapture);


static const inline int mate(int height)
{
    return PLUS_MATE + 100 - height;
}
static inline int zugz(const Board b)
{
    return POPCOUNT(b.color[b.stm] ^ b.piece[b.stm][PAWN]) <= 2;
}
static inline int isAdvancedPassedPawn(const Move m, const uint64_t oppPawns, const int color)
{
    if (color)
        return m.to > 39 && ((getWPassedPawn(m.to) & oppPawns) == 0);
    else
        return m.to < 24 && ((getBPassedPawn(m.to) & oppPawns) == 0);
}

const Move NO_MOVE = (Move) {.from = -1, .to = -1};

/* Time management */
static clock_t stopAt = 0;
static clock_t timeToMove = 0;
static int playWithTime = 0;
static int foundBeforeTimesUp = 0;
static int finishingTime = 0;
static int requestedExtraTime = 0;

/* Info string */
static uint64_t nodes = 0;

/* Debug info */
static uint64_t noMoveGen = 0;
static uint64_t repe = 0;
static uint64_t researches = 0;
static uint64_t qsearchNodes = 0;
static uint64_t nullCutOffs = 0;
static uint64_t betaCutOff = 0;
static uint64_t betaCutOffHit = 0;
static uint64_t queries = 0;

static int exitFlag = 0;

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
    nodes = 0;
    finishingTime = 0;
    requestedExtraTime = 0;
    foundBeforeTimesUp = 0;

    initHistory();
    initKM();
}

static int us;
Move bestTime(Board b, Repetition rep, SearchParams sp)
{
    initCall();

    SearchData sd = (SearchData) {.lastMove = NO_MOVE, .lastScore = 0, .consecutiveMove = 0, .consecutiveScore = 0};
    assert(sp.timeToMove >= 0);
    assert(sp.extraTime >= 0);
    assert(sp.depth >= 0);
    assert(rep.index >= 0 && rep.index < 128);
    /* Adjust the depth if necessary */
    playWithTime = (sp.depth == 0)? 1 : 0;
    sp.depth     = (sp.depth == 0)? MAX_PLY : sp.depth;

    if (sp.depth > MAX_PLY)
    {
        printf("[-] Target depth > MAX_PLY, changing value\n");
        sp.depth = MAX_PLY;
    }

    clock_t start = clock(), last, elapsed;

    stopAt = sp.timeToMove + start;
    timeToMove = sp.timeToMove;

    us = b.stm;

    Move list[NMOVES];
    const int numMoves = legalMoves(&b, list) >> 1;

    //If there is only one possible move, return it
    if (numMoves == 1)
        return list[0];

    #ifdef USE_TB
    //If there is little time, caching can give problems
    if (canGav(b.allPieces))
    {
        int tbAv;
        Move tb = tableLookUp(b, &tbAv);
        if (tbAv)
        {
            infoString(tb, 0, 0, 1000 * (clock() - start) / CLOCKS_PER_SEC);
            return tb;
        }
    }
    #endif

    assignScores(&b, list, numMoves, NO_MOVE, 0);

    Move best = list[0], temp;
    int bestScore = 0;
    int delta;
    int alpha = MINS_INF, beta = PLUS_INF;

    for (int depth = 1; depth <= sp.depth; ++depth)
    {
        delta = 45;
        if (depth >= 6)
        {
            alpha = bestScore - delta;
            beta = bestScore + delta;
        }

        sort(list, list+numMoves);
        while (1)
        {
            foundBeforeTimesUp = 0;
            temp = bestMoveList(b, depth, alpha, beta, list, numMoves, rep);

            last = clock();
            elapsed = last - start;

            if (temp.score >= beta)
            {
                beta += delta;
                delta += delta / 2;
                researches++;

                if (!requestedExtraTime && last < stopAt)
                {
                    requestedExtraTime = 1;
                    stopAt += sp.extraTime;
                    timeToMove += sp.extraTime;
                }
            }
            else if (temp.score <= alpha)
            {
                beta = (beta + alpha) / 2;
                alpha -= delta;
                delta += delta / 2;
                researches++;

                if (!requestedExtraTime && last < stopAt)
                {
                    requestedExtraTime = 1;
                    stopAt += 2*sp.extraTime;
                    timeToMove += 2*sp.extraTime;
                }
            }
            else
            {
                if (exitFlag && foundBeforeTimesUp)
                    best = list[foundBeforeTimesUp];
                break;
            }
            if (exitFlag || finishingTime)
                break;
        }
        if (exitFlag)
            break;

        sd.lastMove = best;
        sd.lastScore = best.score;
        best = temp;
        bestScore = best.score;

        infoString(best, depth, nodes, 1000 * elapsed / CLOCKS_PER_SEC);

        if (compMoves(&sd.lastMove, &best))
            sd.consecutiveMove++;
        else
            sd.consecutiveMove = 0;

        if (abs(sd.lastScore - bestScore) < 11)
            sd.consecutiveScore++;
        else
            sd.consecutiveScore = 0;

        //A premature exit when we are playing with time
        if (playWithTime &&
               (best.score >= PLUS_MATE //We have found a mate
            || (playWithTime && ((clock_t)(1.35f * elapsed) > timeToMove)) //There isn't enough time for another iter
            || finishingTime)) //We have consumed the extra time
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

    playWithTime = 0;
    return best;
}

static double percentage = 0;
static Move moveStack[MAX_PLY+10]; //To avoid possible overflow errors
static int evalStack[MAX_PLY+10];
static Move bestMoveList(Board b, const int depth, int alpha, int beta, Move* list, const int numMoves, Repetition rep)
{
    assert(depth > 0);
    assert(numMoves > 0);
    assert(rep.index >= 0 && rep.index < 128);

    Move currBest = list[0];
    History h;
    int val, inC;
    uint64_t hash = hashPosition(&b), newHash;
    int subtreeSize[NMOVES];

    #ifdef USE_NNUE
    initQueueEval(&b);
    evalStack[0] = evaluateNNUE(&b, 1);
    #else
    evalStack[0] = eval(&b);
    #endif

    NNUEChangeQueue q = (NNUEChangeQueue) {.idx = 0};

    int undo;
    for (int i = 0; i < numMoves; ++i)
    {
        undo = 0;
        long initNodes = nodes;
        //If the move leads to being mated, break (since the moves are ordered based on score)
        if (list[i].score < MINS_MATE)
            break;
        moveStack[0] = list[i];

        percentage = i / (double) numMoves;
        assert(percentage >= 0 && percentage <= 1.1);

        makeMove(&b, list[i], &h);

        newHash = makeMoveHash(hash, &b, list[i], h);
        inC = isInCheck(&b, b.stm);

        if (insuffMat(&b) || isThreeRep(&rep, newHash))
        {
            val = 0;
        }
        else
        {
            updateDo(&q, list[i], &b);
            undo = 1;
            addHash(&rep, newHash);
            if (i == 0)
            {
                val = -pvSearch(b, -beta, -alpha, depth - 1, 1, 0, newHash, &rep, inC);
            }
            else
            {
                val = -pvSearch(b, -alpha - 1, -alpha, depth - 1, 1, 0, newHash, &rep, inC);
                if (val > alpha)
                    val = -pvSearch(b, -beta, -alpha, depth - 1, 1, 0, newHash, &rep, inC);
            }
            remHash(&rep);
        }

        undoMove(&b, list[i], &h);

        if (undo) updateUndo(&q, &b);

        //For the sorting at later depths
        list[i].score = val;
        subtreeSize[i] = (int)((nodes - initNodes) / 2);

        if (val > alpha)
        {
            if (!exitFlag)
                foundBeforeTimesUp = i;
            currBest = list[i];
            alpha = val;

            if (val >= beta)
            {
                if (!exitFlag)
                    moveToFst(list, i);
                break;
            }
        }
    }
    /*
    if (!exitFlag)
    {
        for (int j = 0; j < numMoves; ++j)
        {
            list[j].score = subtreeSize[j];
        }
        moveToFst(list, foundBeforeTimesUp);
        sort(list+1, list+numMoves);
        assert(compMoves(&list[0], &currBest));
    }
    */

    return currBest;
}

static const int marginDepth[4] = {0, 400, 600, 1200};
static int pvSearch(Board b, int alpha, int beta, int depth, const int height, const int null, const uint64_t prevHash, Repetition* rep, const int isInC)
{
    assert(rep->index >= 0 && rep->index < 128);
    assert(beta >= alpha);
    assert(b.fifty >= 0);
    assert(height > 0 && height <= MAX_PLY);
    assert(depth >= 0);

    nodes++;
    const int pv = beta - alpha > 1;
    const int index = prevHash % NUM_ENTRIES;
    assert(index >= 0 && index < NUM_ENTRIES);

    #ifdef USE_TB
    if (canGav(b.allPieces))
    {
        int usable;
        int gavScore = gavWDL(b, &usable) * (mate(height) - 20);
        if (usable)
        {
            #ifdef DEBUG
            queries++;
            #endif
            return b.stm? gavScore : -gavScore;
        }
    }
    #endif

    if (exitFlag)
        return 0;
    if (playWithTime && (nodes & 1023) == 0 && clock() > stopAt)
    {
        if (percentage > .86f && !finishingTime)
        {
            finishingTime = 1;
            stopAt += timeToMove / 5;
        }
        else
        {
            exitFlag = 1;
            return 0;
        }
    }

    //Mate distance pruning
    const int origAlpha = alpha;
    alpha = max(-mate(height), alpha);
    beta = min(mate(height+1), beta);
    if (alpha >= beta)
        return alpha;

    if (height >= MAX_PLY)
        #ifdef USE_NNUE
        return evaluateNNUE(&b, 1);
        #else
        return eval(&b);
        #endif

    if (isInC && (depth < 5 || IS_CAP(moveStack[height-1])))
        depth++;
    else if (depth == 0)
        return qsearch(b, alpha, beta, -1);

    int val, ttHit = 0, ev = -2000;
    Move bestM = NO_MOVE;
    Eval* tableEntry = &table[index];
    if (tableEntry->key == prevHash)
    {
        if (height > 3 && tableEntry->depth >= depth)
        {
            switch (tableEntry->flag)
            {
                case LO:
                    alpha = max(alpha, tableEntry->val);
                    break;
                case HI:
                    beta = min(beta, tableEntry->val);
                    break;
                case EXACT:
                    val = tableEntry->val;
                    if (val < PLUS_MATE - 100)
                        return val;
            }
            if (alpha >= beta)
                return tableEntry->val;
        }

        bestM = tableEntry->m;
        ev = tableEntry->eval;
        ttHit = moveIsValidBasic(&b, &bestM);
    }

    if (!(isInC || ttHit))
        #ifdef USE_NNUE
        ev = evaluateNNUE(&b, 1);
        #else
        ev = eval(&b);
        #endif
    evalStack[height] = ev;

    int fprune = 0;
    const int fmargin[8] = {0, 150, 250, 350, 450, 550, 650, 750};
    if (!isInC && !pv)
    {
        // Razoring
        if (depth == 1 && ev + V_ROOK[0] + 101 <= alpha)
        {
            const int razScore = qsearch(b, alpha, beta, -1);
            if (razScore >= beta)
                return razScore;
        }

        // Static pruning
        if (depth <= 4 && ev - 180 * depth >= beta && ev < 9000)
            return ev;

        // Null move pruning
        if (!null && ev >= beta && depth > R && !zugz(b))
        {
            if (nullMove(b, depth, beta, prevHash))
            {
                #ifdef DEBUG
                ++nullCutOffs;
                #endif
                return beta;
            }
        }

        //Futility pruning
        if (depth <= 7 && abs(alpha) <= 9000 && ev + fmargin[depth] <= alpha)
        	fprune = 1;
    }

    uint64_t newHash;
    int best = MINS_INF;
    const int newHeight = height + 1;
    History h;

    NNUEChangeQueue q = (NNUEChangeQueue) {.idx = 0};
    int undo = 0;

    /*
    if (ttHit)
    {
        makeMove(&b, bestM, &h);
        newHash = makeMoveHash(prevHash, &b, bestM, h);

        if (isDraw(&b, rep, newHash, IS_CAP(bestM)))
        {
            val = (height < 5)? 0 : 8 - (newHash & 15);
            assert(val >= -10 && val <= 10);
        }
        else
        {
            updateDo(&q, bestM, &b);
            undo = 1;
            addHash(rep, newHash);
            val = -pvSearch(b, -beta, -alpha, depth - 1, newHeight, null, newHash, rep, isInCheck(&b, b.stm));
            remHash(rep);
        }
        undoMove(&b, bestM, &h);
        if (undo) updateUndo(&q, &b);

        best = val;
        if (best > alpha)
        {
            alpha = best;
            if (alpha >= beta)
            {
                #ifdef DEBUG
                ++betaCutOff;
                ++betaCutOffHit;
                #endif
                if (!IS_CAP(bestM))
                {
                    addHistory(bestM.from, bestM.to, depth, 1^b.stm);
                    addKM(bestM, depth);
                }
                goto end;
            }
        }
    }
    */

    Move list[NMOVES];
    const int numMoves = legalMoves(&b, list) >> 1;
    if (!numMoves)
        return isInC * -mate(height);

    const int improving = height > 1 && ev > evalStack[height-2] + 20 && !isInC && !null;
    const int notImproving = height > 1 && ev < evalStack[height-2] - 75 && !isInC && !null;

    assignScores(&b, list, numMoves, bestM, depth);
    sort(list, list+numMoves);

    int iid = 0;
    if (iid = (depth >= 5 && list[0].score < 290 && numMoves > 3))
    {
        const int targD = pv? depth / 2 : depth / 4;
        internalIterDeepening(b, list, numMoves, alpha, beta, targD, newHeight, prevHash, rep);
    }

    const int canBreak = depth <= 3 && ev + marginDepth[depth] <= alpha && !isInC;
    //const int fewMovesExt = b.stm != us && numMoves < 5;

    int inC;
    Move m;

    const int prev = b.stm;
    for (int i = /*ttHit*/0; i < numMoves; ++i)
    {
        undo = 0;

	    int SEEscore = 0;
        m = list[i];
        moveStack[height] = m;
        assert(RANGE_64(m.from) && RANGE_64(m.to));
        if (canBreak && !IS_CAP(m) && (i > 3 + depth || (i > 3 && !pv)))
            break;

        assert(b.stm == prev);
        makeMove(&b, m, &h);

        inC = isInCheck(&b, b.stm);

        if (0 && IS_CAP(m) && m.piece != PAWN && !inC) {
            SEEscore = seeCapture(b, m);
            if (depth <= 8 && best > MINS_MATE && SEEscore < -80*depth*depth){
                continue;
            }
        }

        newHash = makeMoveHash(prevHash, &b, m, h);

        if (isDraw(&b, rep, newHash, IS_CAP(m)))
        {
            val = (height < 5)? 0 : 8 - (newHash & 15);
        }
        else
        {
            updateDo(&q, m, &b);
            undo = 1;

            addHash(rep, newHash);
            if (i == 0)
            {
                val = -pvSearch(b, -beta, -alpha, depth - 1, newHeight, null, newHash, rep, inC);
            }
            else
            {
                int reduction = 1;
                if (depth > 1 && !inC && !isInC)
                {
                    if (i > 3 + 2*pv)
                    {
                        int hv = history[1^b.stm][BASE_64(m.from, m.to)];
                        reduction += 1 - (!pv && improving) + depth / 3 - (hv > 1250);
                    }

                    if (!pv && notImproving)
                        reduction++;
                    if ((IS_CAP(m) && m.capture < PAWN) || (moveStack[height-1].to == m.to && depth < 4))
                        reduction--;
                    else if (m.piece == PAWN && isAdvancedPassedPawn(m, b.piece[b.stm][PAWN], 1 ^ b.stm))
                        reduction--;
                    //else if (fewMovesExt)
                    //    reduction--;
                    //if (list[i].piece == KING && list[i].castle)
                    //    reduction--;

                    if (reduction > depth) reduction = depth; //TODO: Try removing this and setting depth <= 0
                }

                assert(depth - reduction >= 0);
                val = -pvSearch(b, -alpha-1, -alpha, depth - reduction, newHeight, null, newHash, rep, inC);
                if (val > alpha && reduction > 1)
                    val = -pvSearch(b, -alpha-1, -alpha, depth - 1, newHeight, null, newHash, rep, inC);
                if (pv && val > alpha && val < beta)
                    val = -pvSearch(b, -beta, -alpha, depth - 1, newHeight, null, newHash, rep, inC);
            }

            remHash(rep);
            assert(rep->index >= 0);
            assert(compMoves(&moveStack[height], &m) && moveStack[height].piece == m.piece);
        }

        undoMove(&b, m, &h);
        if (undo) updateUndo(&q, &b);

        if (val > best)
        {
            best = val;
            bestM = m;
            if (best > alpha)
            {
                alpha = best;

                if (alpha >= beta)
                {
                    #ifdef DEBUG
                    ++betaCutOff;
                    if (i == 0) ++betaCutOffHit;
                    #endif

                    if (!IS_CAP(bestM))
                    {
                        addHistory(bestM.from, bestM.to, depth*depth, b.stm);
                        addKM(bestM, depth);
                    }

                    if (depth < 6)
                    {
                        for (int j = 0; j < i; ++j)
                            decHistory(list[j].from, list[j].to, (!IS_CAP(list[j]))*depth, b.stm);
                    }
                    break;
                }
            }
        }
    }

    end: ;
    int flag = EXACT;

    if (best <= origAlpha)
        flag = HI;
    else if (best >= beta)
        flag = LO;

    table[index] = (Eval) {.key = prevHash, .m = bestM, .val = best, .eval = ev, .depth = depth, .flag = flag};

    return best;
}

int qsearch(Board b, int alpha, const int beta, const int d)
{
    assert(beta >= alpha);
    #ifdef DEBUG
    ++qsearchNodes;
    #endif

    #ifdef USE_NNUE
    const int score = evaluateNNUE(&b, 1);
    #else
    const int score = eval(&b);
    #endif

    if (score >= beta)
        return beta;
    else if (score > alpha)
        alpha = score;
    else if (score + V_QUEEN[0] <= alpha)
        return alpha;

    if (d == 0)
        return alpha;

    Move list[NMOVES];
    const int nMvsAndChck = legalMovesQuiesce(&b, list);
    const int numMoves = nMvsAndChck >> 1;
    History h;

    assignScoresQuiesce(&b, list, numMoves);
    sort(list, list+numMoves);

    int val;

    int undo = 0;
    NNUEChangeQueue q = (NNUEChangeQueue) {.idx = 0};

    for (int i = 0; i < numMoves; ++i)
    {
        undo = 0;
        if (!(nMvsAndChck & 1) && i > 2 && list[i].score + score < alpha)
            break;

        makeMove(&b, list[i], &h);

        if (insuffMat(&b)) //No need to check for 3 fold rep
            val = 0;
        else
        {
            updateDo(&q, list[i], &b);
            undo = 1;
            val = -qsearch(b, -beta, -alpha, d - 1 /*+ (list[i].capture < 3)*/);
        }

        undoMove(&b, list[i], &h);

        if (undo) updateUndo(&q, &b);

        if (val > alpha)
        {
            alpha = val;
            if (val >= beta)
                return beta;
        }
    }

    return alpha;
}

/* In this function there are no assumptions made about the sorting of the list
 */
static void internalIterDeepening(Board b, Move* list, const int numMoves, int alpha, const int beta, const int depth, const int height, const uint64_t prevHash, Repetition* rep)
{
    assert(beta >= alpha);
    assert(depth >= 1);
    assert(height > 0 && height <= MAX_PLY);

    uint64_t newHash;
    int val;

    History h;
    NNUEChangeQueue q = (NNUEChangeQueue) {.idx = 0};

    int undo;
    for (int i = 0; i < numMoves; ++i)
    {
        undo = 0;

        makeMove(&b, list[i], &h);

        newHash = makeMoveHash(prevHash, &b, list[i], h);

        if (isDraw(&b, rep, newHash, IS_CAP(list[i])))
        {
            val = 0;
        }
        else
        {
            updateDo(&q, list[i], &b);
            undo = 1;
            addHash(rep, newHash);
            val = -pvSearch(b, -beta, -alpha, depth - 1, height, 1, newHash, rep, isInCheck(&b, b.stm));
            remHash(rep);
        }

        list[i].score = val;

        undoMove(&b, list[i], &h);
        if (undo) updateUndo(&q, &b);
    }

    sort(list, list+numMoves);
}

#ifdef USE_TB
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
#endif

static int nullMove(Board b, const int depth, const int beta, const uint64_t prevHash)
{
    assert(depth >= R);
    Repetition _rep = (Repetition) {.index = 0};
    b.stm ^= 1;
    const int td = (depth < 6)? depth - R : depth / 3 + 1;
    const int val = -pvSearch(b, -beta, -beta + 1, td, MAX_PLY - 15, 1, changeTurn(prevHash), &_rep, 0);
    b.stm ^= 1;

    return val >= beta;
}
static inline int isDraw(const Board* b, const Repetition* rep, const uint64_t newHash, const int lastMCapture)
{
    if (lastMCapture)
    {
        return insuffMat(b);
    }
    else if (rep->index > 5)
    {
        #ifdef DEBUG
        if (isRepetition(rep, newHash)) repe++;
        #endif

        return rep->index >= 100 || b->fifty >= 100 || isRepetition(rep, newHash) || isThreeRep(rep, newHash);
    }

    return 0;
}