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
#include "../include/io.h"

#include <stdio.h>

//If there is a capture, this is the search will continue for CAPT_DEPTH
#define CAPT_DEPTH 1

#define PLUS_MATE    99999
#define MINS_MATE   -99999
#define PLUS_INF  99999999
#define MINS_INF -99999999

int alphaBeta(Board b, int alpha, int beta, const int depth, int capt, const uint64_t prevHash, Move m, Repetition* rep);
int bestMoveBruteValue(Board b, const int depth);

void sort(Move* list, const int numMoves, const int to);

static inline int rookVSKing(Board b)
{
    return POPCOUNT(b.piece[b.turn][ROOK]) == 1 && POPCOUNT(b.allPieces ^ b.piece[b.turn][ROOK]) == 2;
}

Move bestMoveAB(Board b, const int depth, int tree, Repetition rep)
{
    if (rookVSKing(b)) return rookMate(b);
    if (depth == 0) return (Move) {};
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
            val = alphaBeta(b, alpha, beta, depth - 1, CAPT_DEPTH, newHash, list[i], &rep);
            --rep.index;
        }
        undoMove(&b, list[i], &h);

        if (tree)
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

    return currBest;
}
int alphaBeta(Board b, int alpha, int beta, const int depth, int capt, const uint64_t prevHash, Move m, Repetition* rep)
{
    Move list[200];
    History h;
    int lgm = legalMoves(&b, list); //lgm is an int representing (2 * numMoves + isInCheck), in order to avoid having to check for mate

    int numMoves = lgm >> 1;
    if (! numMoves)
        return lgm * (b.turn ? MINS_MATE - depth : PLUS_MATE + depth);

    sort(list, numMoves, m.to);

    int val, best, index;
    uint64_t newHash;
    if (b.turn)
    {
        best = MINS_INF;

        for (int i = 0; i < numMoves; ++i)
        {
            makeMove(&b, list[i], &h);
            newHash = makeMoveHash(prevHash, &b, list[i], h);
            index = newHash & MOD_ENTRIES;
            
            if (insuffMat(b) || isThreeRep(rep, newHash))
            {
                val = 0;
            }
            else if (table[index].key == newHash && table[index].depth >= depth)
            {
                val = table[index].val;
                if (val > PLUS_MATE) val -= depth;
            }
            else
            {
                rep->hashTable[rep->index++] = newHash;
                if (depth == 1)
                {
                    if (capt && list[i].capture > 0)
                        val = alphaBeta(b, alpha, beta, 1, capt - 1, newHash, list[i], rep);
                    else
                        val = eval(b);
                }
                else
                    val = alphaBeta(b, alpha, beta, depth - 1, capt, newHash, list[i], rep);
                table[index] = (Eval) {.key = newHash, .val = val, .depth = depth};
                --rep->index;
            }

            if(val > best)
            {
                best = val;
                if(val > alpha)
                {
                    alpha = val;
                    if (beta < alpha || val > PLUS_MATE + depth - 2)
                        break; //Pruning or it has found a mate in 1
                }
            }

            undoMove(&b, list[i], &h);
        }
    }
    else
    {
        best = PLUS_INF;

        for (int i = 0; i < numMoves; ++i)
        {
            makeMove(&b, list[i], &h);
            newHash = makeMoveHash(prevHash, &b, list[i], h);
            index = newHash & MOD_ENTRIES;

            if (insuffMat(b) || isThreeRep(rep, newHash)){
                val = 0;
            }
            else if (table[index].key == newHash && table[index].depth >= depth)
            {
                val = table[index].val;
                if (val < MINS_MATE) val += depth;
            }
            else
            {
                rep->hashTable[rep->index++] = newHash;
                if (depth == 1)
                {
                    if (capt && list[i].capture > 0)
                        val = alphaBeta(b, alpha, beta, 1, capt - 1, newHash, list[i], rep);
                    else
                        val = eval(b);
                }
                else
                    val = alphaBeta(b, alpha, beta, depth - 1, capt, newHash, list[i], rep);
                table[index] = (Eval) {.key = newHash, .val = val, .depth = depth};
                --rep->index;
            }

            if(val < best)
            {
                best = val;
                if(val < beta)
                {
                    beta = val;
                    if (beta < alpha || val < MINS_MATE - depth + 2)
                        break; //Prunning or it has found a mate in 1
                }
            }

            undoMove(&b, list[i], &h);
        }
    }
    
    return best;
}

Move bestMoveBrute(Board b, const int depth, int tree)
{
    if (rookVSKing(b)) return rookMate(b);
    Move list[200];
    History h;

    int numMoves = legalMoves(&b, list) >> 1;
    int best = b.turn ? MINS_INF : PLUS_INF;
    int val;
    Move currBest = list[0];

    for (int i = 0; i < numMoves; ++i)
    {
        makeMove(&b, list[i], &h);
        val = bestMoveBruteValue(b, depth - 1);
        undoMove(&b, list[i], &h);

        if (tree)
        {
            drawMove(list[i]);
            printf(": %d\n", val);
        }

        if (b.turn && val > best)
        {
            currBest = list[i];
            best = val;
        }
        else if (!b.turn && val < best)
        {
            currBest = list[i];
            best = val;
        }
    }

    return currBest;
}
int bestMoveBruteValue(Board b, int depth)
{
    if (depth == 0) return eval(b);

    Move list[200];
    History h;

    int lgm = legalMoves(&b, list);
    int numMoves = lgm >> 1;
    
    if (! numMoves)
        return lgm * (b.turn ? MINS_MATE - depth : PLUS_MATE + depth);


    int best = b.turn ? MINS_INF : PLUS_INF;
    int val;

    for (int i = 0; i < numMoves; ++i)
    {
        makeMove(&b, list[i], &h);
        val = bestMoveBruteValue(b, depth - 1);
        undoMove(&b, list[i], &h);

        if (b.turn && val > best)
            best = val;
        else if (!b.turn && val < best)
            best = val;
    }

    return best;
}


/* Sorts all the moves based on their score
 * It is currently based on the LVA - MVV and a bonus if the piece captures the piece that moved the last time,
 * since it is likely it wont be protected
 */
static const int score[6] = {80, 160, 240, 320, 400, 480};

void sort(Move* list, const int numMoves, const int to)
{
    for (int i = 0; i < numMoves; ++i)
    {
        if(list[i].capture != NO_PIECE && list[i].capture)
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