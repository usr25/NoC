/* search.c
 * Performs the actual search to find the best move
 */

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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
//#include <math.h>

//If there is a capture, this is the search will continue for CAPT_DEPTH
#define CAPT_DEPTH 1
//Depth of the null move prunning
#define R 3

#define PLUS_MATE    99999
#define MINS_MATE   -99999
#define PLUS_INF  99999999
#define MINS_INF -99999999

int alphaBeta(Board b, int alpha, int beta, const int depth, int capt, int null, const uint64_t prevHash, Move m, Repetition* rep);
int bestMoveBruteValue(Board b, const int depth);

void sort(Move* list, const int numMoves, const int to);

static inline int rookVSKing(const Board b)
{
    return POPCOUNT(b.piece[b.turn][ROOK]) == 1 && POPCOUNT(b.allPieces ^ b.piece[b.turn][ROOK]) == 2;
}
static inline int onlyPawns(const Board b)
{
    return POPCOUNT(b.allPieces ^ b.piece[WHITE][PAWN] ^ b.piece[BLACK][PAWN]) == 2;
}

double startT;
double timeToMoveT;
int calledTiming = 0;
uint64_t nodes = 0;
//TODO: Use Move.score to return if it has mate, so that it can end sooner
//TODO: Sort the moves base on the result of the previous iteration
//Pass timeToMove with a small buffer
Move bestTime(Board b, const double timeToMove, Repetition rep)
{
    calledTiming = 1;
    timeToMoveT = timeToMove;
    clock_t start = clock();
    startT = start;
    
    Move best, temp;
    for (int i = 3; i <= 20; ++i)
    {
        temp = bestMoveAB(b, i, 0, rep);
        clock_t last = clock();
        clock_t elapsed = (double)(last - start);
        if (elapsed > timeToMove)
            break;

        best = temp;

        /*Due to the exponential nature, if the time remeaning is smaller than 3 * timeTaken, break, it is unlikely that
         *the program will be able to finish another depth
         *(3 is a randomly chosen constant based on experience, it should be improved using the ratio between consecutive searches)
         * or
         * it has found mate
         */
        if (3 * (double)(last - start) > timeToMove || abs(best.score) >= PLUS_MATE)
            break;
    }

    calledTiming = 0;
    return best;
}


int callDepth;
int notCallDepthParity;
Move bestMoveAB(Board b, const int depth, int divide, Repetition rep)
{
    nodes = 0;
    if (rookVSKing(b)) return rookMate(b);
    if (depth == 0) return (Move) {};
    callDepth = depth;
    notCallDepthParity = 1 ^ (depth & 1);
    //initializeTable();
    
    const int color = b.turn;

    Move list[200];
    History h;

    int numMoves = legalMoves(&b, list) >> 1;

    sort(list, numMoves, -1);
    
    Move currBest = list[0];
    int val;

    uint64_t hash = hashPosition(&b); //The position should be already added to res
    int alpha = MINS_INF, beta = PLUS_INF;

    for (int i = 0; i < numMoves; ++i)
    {
        makeMove(&b, list[i], &h);
        uint64_t newHash = makeMoveHash(hash, &b, list[i], h);
        if (insuffMat(b) || isThreeRep(&rep, newHash))
            val = 0;
        else
        {
            rep.hashTable[rep.index++] = newHash;
            val = alphaBeta(b, alpha, beta, depth - 1, CAPT_DEPTH, 0, newHash, list[i], &rep);
            --rep.index;
        }
        undoMove(&b, list[i], &h);

        list[i].score = val;

        if (divide)
        {
            drawMove(list[i]);
            printf(": %d\n", val);
        }

        if (color && val > alpha)
        {
            currBest = list[i];
            alpha = val;
            //No need to keep searching once it has found a mate in 1
            if (val > PLUS_MATE + depth - 2)
                break;
        }
        else if (!color && val < beta)
        {
            currBest = list[i];
            beta = val;
            if (val < MINS_MATE - depth + 2)
                break;
        }
    }

    infoString(currBest, depth, nodes);
    return currBest;
}
int alphaBeta(Board b, int alpha, int beta, const int depth, int capt, int null, const uint64_t prevHash, Move m, Repetition* rep)
{
    if (calledTiming && (double)clock() - startT > timeToMoveT)
        return 0;

    nodes++;
    //Null move pruning
    if (!null && depth > R && m.capture < 1 && !isInCheck(&b, b.turn) && !onlyPawns(b))
    {
        int score;
        Repetition rep_ = (Repetition){.index = 0};
        if (b.turn)
        {
            b.turn ^= 1;
            score = alphaBeta(b, beta + 1, beta, depth - R, capt, 1, 0, m, &rep_);
            b.turn ^= 1;
            if (score >= beta)
                return beta;
        }
        else
        {
            b.turn ^= 1;
            score = alphaBeta(b, alpha, alpha - 1, depth - R, capt, 1, 0, m, &rep_);
            b.turn ^= 1;
            if (score <= alpha)
                return alpha;
        }
    }

    Move list[200];
    History h;
    int lgm = legalMoves(&b, list); //lgm is an int representing (2 * numMoves + isInCheck), in order to avoid having to check for mate

    int numMoves = lgm >> 1;
    if (! numMoves)
        return lgm * (b.turn ? MINS_MATE - depth : PLUS_MATE + depth);

    sort(list, numMoves, m.to);

    int val, index, exit = 1;
    uint64_t newHash;

    int best = b.turn? MINS_INF : PLUS_INF;
    int mate = b.turn? PLUS_MATE : MINS_MATE;

    for (int i = 0; i < numMoves && exit; ++i)
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
            if (depth == 1)
            {
                nodes++;
                if (table[index].key == newHash)
                {
                    val = table[index].val;
                    if (val > mate) val -= table[index].depth + 1;
                }
                else if (capt && list[i].capture > 0)
                {
                    val = alphaBeta(b, alpha, beta, 1, capt - 1, null, newHash, list[i], rep);
                    table[index] = (Eval) {.key = newHash, .val = val, .depth = 1};
                }
                else
                {
                    val = eval(b);
                }
            }
            else
            {
                val = alphaBeta(b, alpha, beta, depth - 1, capt, null, newHash, list[i], rep);
                table[index] = (Eval) {.key = newHash, .val = val, .depth = depth};
            }
            --rep->index;
        }

        if(!b.turn && val > best)
        {
            best = val;
            if(val > alpha)
            {
                alpha = val;
                if (beta <= alpha || val > PLUS_MATE + depth - 2)
                    break; //Pruning or it has found a mate in 1, either case, break
            }
        }
        else if(b.turn && val < best)
        {
            best = val;
            if(val < beta)
            {
                beta = val;
                if (beta <= alpha || val < MINS_MATE - depth + 2)
                    break; //Prunning or it has found a mate in 1, either case, break
            }
        }

        undoMove(&b, list[i], &h);
    }

    return best;
}


/* Sorts all the moves based on their score
 * It is currently based on the MVV - LVA and a bonus if the piece captures the piece that moved the last time,
 * since it is likely it wont be protected
 */
static const int score[6] = {80, 160, 240, 320, 400, 480};

void sort(Move* list, const int numMoves, const int to)
{
    for (int i = 0; i < numMoves; ++i)
    {
        if(list[i].capture > 0) //There has been a capture
            list[i].score = 
                score[list[i].pieceThatMoves] - (score[list[i].capture] >> 4)  //LVA - MVV
                +((to == list[i].to) << 7); //Bonus if it captures the piece that moved last time
    }

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