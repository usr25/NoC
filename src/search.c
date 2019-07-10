#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"
#include "../include/allmoves.h"
#include "../include/search.h"
#include "../include/evaluation.h"
#include "../include/io.h"

#include <stdio.h>

#define PLUS_MATE 99999
#define MINS_MATE -99999
#define PLUS_INF 9999999
#define MINS_INF -9999999


int alphaBeta(Board b, int alpha, int beta, int depth)
{
    if (! depth) return eval(b);

    Move list[200];
    History h;
    int numMoves = legalMoves(&b, list, b.turn);

    if (! numMoves)
    {
        if (isInCheck(&b, b.turn))
            return depth * (b.turn ? MINS_MATE : PLUS_MATE);
        else
            return 0;
    }

    int val, best;
    if (b.turn)
    {
        best = MINS_INF;

        for (int i = 0; i < numMoves; ++i)
        {
            makeMove(&b, list[i], &h);
            if (isDraw(b))
                val = 0;
            else
                val = alphaBeta(b, alpha, beta, depth - 1);

            if(val > best)
            {
                best = val;
                if(val > alpha)
                {
                    alpha = val;
                    if (beta < alpha) break;
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
            if (isDraw(b))
                val = 0;
            else
                val = alphaBeta(b, alpha, beta, depth - 1);

            if(val < best)
            {
                best = val;
                if(val < beta)
                {
                    beta = val;
                    if (beta < alpha) break;
                }
            }

            undoMove(&b, list[i], &h);
        }
    }
    
    return best;
}
Move bestMoveAB(Board b, int depth, int tree)
{
    if (depth == 0) return (Move) {};
    const int color = b.turn;

    Move list[200];
    History h;

    int numMoves = legalMoves(&b, list, color);

    int best = color ? MINS_INF : PLUS_INF;
    
    Move currBest = list[0];
    int val;

    for (int i = 0; i < numMoves; ++i)
    {
        makeMove(&b, list[i], &h);
        val = alphaBeta(b, MINS_INF, PLUS_INF, depth - 1);
        undoMove(&b, list[i], &h);

        if (tree)
        {
            drawMove(list[i]);
            printf(": %d\n", val);
        }

        if (color && val > best)
        {
            currBest = list[i];
            best = val;
        }
        else if (!color && val < best)
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

    int numMoves = legalMoves(&b, list, b.turn);
    
    if (! numMoves)
    {
        if (isInCheck(&b, b.turn))
            return depth * (b.turn ? MINS_MATE : PLUS_MATE);
        else
            return 0;
    }

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
Move bestMoveBrute(Board b, int depth, int tree)
{
    Move list[200];
    History h;

    int numMoves = legalMoves(&b, list, b.turn);
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