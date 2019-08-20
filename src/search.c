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
#define MARGIN 3

#define PLUS_MATE    99999
#define MINS_MATE   -99999
#define PLUS_INF   9999999
#define MINS_INF  -9999999

Move bestMoveList(Board b, const int depth, Move* list, const int numMoves, Repetition rep);
int alphaBeta(Board b, int alpha, int beta, const int depth, int null, const uint64_t prevHash, Move m, Move* prevBest, Repetition* rep);
int pvSearch(Board b, int alpha, int beta, const int depth, int null, const uint64_t prevHash, Move m, Move* prevBest, Repetition* rep);
int bestMoveBruteValue(Board b, const int depth);
int qsearch(Board b, int alpha, int beta, const Move m);

static inline void assignScores(Move* list, const int numMoves, const int to, const Move* prevBest, const Move bestFromPos);
static inline void assignScoresQuiesce(Move* list, const int numMoves, const int to);
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
const Move NoMove = (Move) {.from = -1, .to = -1};

unsigned int betaCutOff;
unsigned int betaCutOffHit;

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
    Move best = list[0], temp;
    for (int depth = 1; depth <= targetDepth; ++depth)
    {
        nodes = 0;
        sort(list, numMoves);
        temp = bestMoveList(b, depth, list, numMoves, rep);
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
        if ((calledTiming && (2 * (last - start) > timeToMove)) || abs(best.score) >= PLUS_MATE)
            break;
    }

    #ifdef DEBUG
    printf("Beta Hits: %f\n", (float)betaCutOffHit / betaCutOff);
    printf("Qsearch Nodes: %llu\n", qsearchNodes);
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
    Move prevRefutation = NoMove;
    int val, exit = 0;

    uint64_t hash = hashPosition(&b); //The position should be already added to res
    int alpha = MINS_INF;
    int pvS = 1;

    for (int i = 0; i < numMoves && !exit; ++i)
    {
        makeMove(&b, list[i], &h);
        uint64_t newHash = makeMoveHash(hash, &b, list[i], h);
        if (insuffMat(b) || isThreeRep(&rep, newHash))
            val = 0;
        else
        {
            rep.hashTable[rep.index++] = newHash;
            if (pvS)
            {
                val = -pvSearch(b, MINS_INF, -alpha, depth - 1, 0, newHash, list[i], &prevRefutation, &rep);
            }
            else
            {
                val = -pvSearch(b, -alpha - 1, -alpha, depth - 1, 0, newHash, list[i], &prevRefutation, &rep);
                if (val > alpha)
                    val = -pvSearch(b, MINS_INF, -alpha, depth - 1, 0, newHash, list[i], &prevRefutation, &rep);
            }
            --rep.index;
        }
        undoMove(&b, list[i], &h);

        if (val > alpha)
        {
            currBest = list[i];
            alpha = val;
            pvS = 0;
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

    assignScores(list, numMoves, -1, &NoMove, NoMove);
    sort(list, numMoves);

    Move currBest = list[0];
    Move prevRefutation = NoMove;
    int val;

    uint64_t hash = hashPosition(&b); //The position should be already added to res
    int alpha = MINS_INF;

    for (int i = 0; i < numMoves; ++i)
    {
        makeMove(&b, list[i], &h);
        uint64_t newHash = makeMoveHash(hash, &b, list[i], h);
        if (insuffMat(b) || isThreeRep(&rep, newHash))
            val = 0;
        else
        {
            rep.hashTable[rep.index++] = newHash;
            val = -pvSearch(b, MINS_INF, -alpha, depth - 1, 0, newHash, list[i], &prevRefutation, &rep);
            --rep.index;
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

int pvSearch(Board b, int alpha, int beta, const int depth, int null, const uint64_t prevHash, Move m, Move* prevBest, Repetition* rep)
{
    nodes++;
    if (calledTiming && clock() - startT > timeToMoveT)
        return 0;

    const int isInC = isInCheck(&b, b.turn);

    if (depth == 0)
    {
        if (isInC)
            return pvSearch(b, alpha, beta, 1, 0, prevHash, m, prevBest, rep);
        else
            return qsearch(b, alpha, beta, m);
    }

    const int index = prevHash & MOD_ENTRIES;
    int val;
    Move bestM = NoMove;
    if (table[index].key == prevHash)
    {
        bestM = table[index].m;
        if (table[index].depth == depth)
        {
            switch (table[index].flag)
            {
                case LO:
                    alpha = (alpha > table[index].val) ? alpha : table[index].val;
                    break;
                case HI:
                    beta = (beta < table[index].val) ? beta : table[index].val;
                    break;

                case EXACT:
                    val = table[index].val;
                    if (val > PLUS_MATE) val -= 1;
                    return val;
            }

            if (alpha >= beta)
                return table[index].val;
        }
    }

    if (!null && depth > R && !isInC && !onlyPawns(b))
    {
        Repetition _rep = (Repetition) {.index = 0};
        b.turn ^= 1;
        val = -pvSearch(b, -beta, -beta + 1, depth - R - 1, 1, 0, m, prevBest, &_rep);
        b.turn ^= 1;

        if (val >= beta)
            return beta;
    }

    Move list[NMOVES];
    const int lgm = legalMoves(&b, list); //lgm is an int representing (2 * numMoves + isInCheck), in order to avoid having to check for mate
    const int numMoves = lgm >> 1;
    if (!numMoves)
        return lgm * (MINS_MATE - depth);

    History h;

    assignScores(list, numMoves, m.to, prevBest, bestM);
    sort(list, numMoves);

    uint64_t newHash;
    int best = MINS_INF;
    int pvS = 1;
    Move refutation = NoMove;
    const int origAlpha = alpha;

    for (int i = 0; i < numMoves; ++i)
    {
        makeMove(&b, list[i], &h);
        newHash = makeMoveHash(prevHash, &b, list[i], h);
        if ((m.capture > 0 && insuffMat(b)) || isThreeRep(rep, newHash))
        {
            val = 0;
        }
        else
        {
            rep->hashTable[rep->index++] = newHash;
            if (pvS)
            {
                val = -pvSearch(b, -beta, -alpha, depth - 1, null, newHash, list[i], &refutation, rep);
            }
            else
            {
                int reduction = 1;
                val = -pvSearch(b, -alpha-1, -alpha, depth - reduction, null, newHash, list[i], &refutation, rep);
                if (val > alpha && val < beta)
                    val = -pvSearch(b, -beta, -alpha, depth - 1, null, newHash, list[i], &refutation, rep);
            }
            if (m.promotion > 0)
                val += depth << 2;
            --rep->index;
        }

        if (val > best)
        {
            best = val;
            bestM = list[i];
            if (best > alpha)
            {
                alpha = best;
                pvS = 0;

                if (alpha >= beta)
                {
                    #ifdef DEBUG
                    ++betaCutOff;
                    if (i == 0)
                        ++betaCutOffHit;
                    #endif

                    if (list[i].capture < 1)
                        *prevBest = list[i];
                    break;
                }
                else if (best >= PLUS_MATE + depth - 1)
                {
                    *prevBest = list[i];
                    break;
                }
                else if (prevBest->from == -1)
                    *prevBest = list[i];
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

int qsearch(Board b, int alpha, int beta, const Move m)
{
    #ifdef DEBUG
    ++qsearchNodes;
    #endif

    const int score = b.turn? eval(b) : -eval(b);

    if (score >= beta)
        return beta;
    if (score > alpha)
        alpha = score;

    Move list[NMOVES];
    History h;
    const int numMoves = legalMoves(&b, list) >> 1;

    assignScoresQuiesce(list, numMoves, m.to);
    sort(list, numMoves);

    int val;

    for (int i = 0; i < numMoves; ++i)
    {
        if (list[i].capture < 1)
            continue;

        makeMove(&b, list[i], &h);

        if (insuffMat(b)) //No need to check for 3 fold rep
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

static const int score[6] = {80, 160, 240, 320, 400, 480};
inline void assignScores(Move* list, const int numMoves, const int to, const Move* prevRefutation, const Move bestFromPos)
{
    for (int i = 0; i < numMoves; ++i)
    {
        if(list[i].capture > 0) //There has been a capture
            list[i].score =
                score[list[i].pieceThatMoves] - (score[list[i].capture] >> 4);  //LVA - MVV
        if (compMoves(prevRefutation, &list[i]) || compMoves(&bestFromPos, &list[i]))
            list[i].score += 500;
    }
}
inline void assignScoresQuiesce(Move* list, const int numMoves, const int to)
{
    for (int i = 0; i < numMoves; ++i)
    {
        if(list[i].capture > 0) //There has been a capture
            list[i].score =
                score[list[i].pieceThatMoves] - (score[list[i].capture] >> 4)
                +((to == list[i].to) << 9); //Bonus if it captures the piece that moved last time, it reduces the qsearch nodes about 30%
    }
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