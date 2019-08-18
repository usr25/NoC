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
#define MARGIN 2

#define PLUS_MATE    99999
#define MINS_MATE   -99999
#define PLUS_INF  99999999
#define MINS_INF -99999999

Move bestMoveList(Board b, const int depth, Move* list, const int numMoves, Repetition rep);
int alphaBeta(Board b, int alpha, int beta, const int depth, int null, const uint64_t prevHash, Move m, Move* prevBest, Repetition* rep);
int bestMoveBruteValue(Board b, const int depth);
int qsearch(Board b, int alpha, int beta, const Move m);

static inline void assignScores(Move* list, const int numMoves, const int to, Move* prevBest);
void sort(Move* list, const int numMoves);

static inline int rookVSKing(const Board b)
{
    return POPCOUNT(b.piece[b.turn][ROOK]) == 1 && POPCOUNT(b.allPieces ^ b.piece[b.turn][ROOK]) == 2;
}
static inline int onlyPawns(const Board b)
{
    return POPCOUNT(b.allPieces ^ b.piece[WHITE][PAWN] ^ b.piece[BLACK][PAWN]) == 2;
}

clock_t startT;
clock_t timeToMoveT;
int calledTiming = 0;
uint64_t nodes = 0;

Move bestTime(Board b, const clock_t timeToMove, Repetition rep)
{
    calledTiming = 1;
    timeToMoveT = timeToMove;
    clock_t start = clock(), last;
    startT = start;

    Move list[200];
    int numMoves = legalMoves(&b, list) >> 1;

    Move best = list[0], temp;
    for (int depth = 2; depth <= 20; ++depth)
    {
        nodes = 0;
        sort(list, numMoves);
        temp = bestMoveList(b, depth, list, numMoves, rep);
        last = clock();
        clock_t elapsed = last - start;
        if (elapsed > timeToMove)
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
        if (2 * (last - start) > timeToMove || abs(best.score) >= PLUS_MATE)
            break;
    }

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
    //initializeTable();

    History h;

    Move currBest = list[0];
    Move prevBest = (Move) {.from = -1, .to = -1};
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
            val = -alphaBeta(b, MINS_INF, -alpha, depth - 1, 0, newHash, list[i], &prevBest, &rep);
            --rep.index;
        }
        undoMove(&b, list[i], &h);

        if (val > alpha)
        {
            currBest = list[i];
            alpha = val;
            //No need to keep searching once it has found a mate in 1
            if (val > PLUS_MATE + depth - 2) break;
        }

        //For the sorting
        list[i].score = val;
    }

    //infoString(currBest, depth, nodes);
    return currBest;
}
Move bestMoveAB(Board b, const int depth, int divide, Repetition rep)
{
    nodes = 0;
    if (rookVSKing(b)) return rookMate(b);
    if (depth == 0) return (Move) {};
    callDepth = depth;
    //initializeTable();

    Move list[200];
    History h;

    int numMoves = legalMoves(&b, list) >> 1;
    Move prevBest = (Move) {.from = -1, .to = -1};

    assignScores(list, numMoves, -1, &prevBest);
    sort(list, numMoves);

    Move currBest = list[0];
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
            val = -alphaBeta(b, MINS_INF, -alpha, depth - 1, 0, newHash, list[i], &prevBest, &rep);
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
            //No need to keep searching once it has found a mate in 1
            if (val > PLUS_MATE + depth - 2) break;
        }
    }

    return currBest;
}


int alphaBeta(Board b, int alpha, int beta, const int depth, int null, const uint64_t prevHash, Move m, Move* prevBest, Repetition* rep)
{
    nodes++;
    if (calledTiming && clock() - startT > timeToMoveT)
        return 0;

    int isInC = isInCheck(&b, b.turn);

    if (depth == 0)
        if (isInC)
            return alphaBeta(b, alpha, beta, 1, 0, prevHash, m, prevBest, rep);
        else
            return qsearch(b, alpha, beta, m);

    if (!null && depth > R && m.score < 10 && !isInC && !onlyPawns(b))
    {
        Repetition _rep = (Repetition) {.index = 0};
        b.turn ^= 1;
        int val = -alphaBeta(b, -beta, -beta + 1, depth - R - 1, 1, 0, m, prevBest, &_rep);
        b.turn ^= 1;

        if (val >= beta - MARGIN)
            return beta;
    }

    Move list[200];
    int lgm = legalMoves(&b, list); //lgm is an int representing (2 * numMoves + isInCheck), in order to avoid having to check for mate
    int numMoves = lgm >> 1;
    if (!numMoves)
        return lgm * (MINS_MATE - depth);

    History h;

    assignScores(list, numMoves, m.to, prevBest);
    sort(list, numMoves);

    uint64_t newHash;
    int best = MINS_INF, val, index;
    Move currBest = (Move) {.from = -1, .to = -1};

    for (int i = 0; i < numMoves; ++i)
    {
        makeMove(&b, list[i], &h);
        newHash = makeMoveHash(prevHash, &b, list[i], h);
        if (m.capture > 0 && insuffMat(b))
            val = 0;
        else if(isThreeRep(rep, newHash))
            val = 0;
        else
        {
            rep->hashTable[rep->index++] = newHash;
            index = newHash & MOD_ENTRIES;
            if (table[index].key == newHash && table[index].depth == depth && table[index].val < alpha)
            {
                val = table[index].val;
                if (val > PLUS_MATE) val -= 1;
            }
            else
            {
                val = -alphaBeta(b, -beta, -alpha, depth - 1, null, newHash, list[i], &currBest, rep);
                table[index] = (Eval) {.key = newHash, .val = val, .depth = depth};
                if (m.promotion > 0)
                    val += depth << 2;
            }
            --rep->index;
        }

        if (val > best)
        {
            best = val;
            if (best > alpha)
            {
                alpha = best;

                if (alpha >= beta)
                {
                    *prevBest = list[i];
                    break;
                }
            }
        }

        undoMove(&b, list[i], &h);
    }

    return best;
}

int qsearch(Board b, int alpha, int beta, const Move m)
{
    const int score = b.turn? eval(b) : -eval(b);

    if (score >= beta)
        return beta + MARGIN;

    if (score > alpha)
        alpha = score;

    Move list[200];
    History h;
    int numMoves = legalMoves(&b, list) >> 1;

    Move temp = (Move) {.from = -1, .to = -1};
    assignScores(list, numMoves, m.to, &temp);
    sort(list, numMoves);

    int val;

    for (int i = 0; i < numMoves; ++i)
    {
        if (list[i].capture < 1)
            continue;

        makeMove(&b, list[i], &h);

        if (insuffMat(b))
            val = 0;
        else
            val = -qsearch(b, -beta, -alpha, list[i]);

        if (val >= beta)
            return beta + MARGIN;

        if (val > alpha)
            alpha = val;

        undoMove(&b, list[i], &h);
    }

    return alpha - MARGIN;
}

static const int score[6] = {80, 160, 240, 320, 400, 480};
inline void assignScores(Move* list, const int numMoves, const int to, Move* prevBest)
{
    for (int i = 0; i < numMoves; ++i)
    {
        if(list[i].capture > 0) //There has been a capture
            list[i].score = 
                score[list[i].pieceThatMoves] - (score[list[i].capture] >> 4)  //LVA - MVV
                +((to == list[i].to) << 7); //Bonus if it captures the piece that moved last time
        if (prevBest->from == list[i].from && prevBest->to == list[i].to)
            list[i].score += 500;
    }
}

/* Sorts all the moves based on their score
 * It is currently based on the MVV - LVA and a bonus if the piece captures the piece that moved the last time,
 * since it is likely it wont be protected
 */
void sort(Move* list, const int numMoves)
{
    //Insertion sort
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