#include "../include/global.h"
#include "../include/memoization.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/allmoves.h"
#include "../include/boardmoves.h"
#include "../include/mate.h"
#include "../include/io.h"

#include <stdio.h>
#include <stdlib.h>

static int determineMate(Board b, int depth, int height, int alpha, int beta);

//TODO: Use BFS. Hint: Use a queue
//TODO: Use a pv
//TODO: Return all the moves that give mate
Move findMate(Board b, int depth)
{
    int alpha = -99, beta = 99;
    int best = -100;
    Move bestM;
    Move list[250];
    History h;
    const int numMoves = legalMoves(&b, list) >> 1;
    for (int i = 0; i < numMoves; ++i)
    {
        makeMove(&b, list[i], &h);
        int score = -determineMate(b, depth, 1, -beta, -alpha);
        undoMove(&b, list[i], &h);

        if (score > best)
        {
            best = score;
            bestM = list[i];

            if (score > alpha) alpha = score;
        }
    }

    bestM.score = best;
    return bestM;
}

static int determineMate(Board b, int depth, int height, int alpha, int beta)
{
    if (depth <= 0) return 0;
    int best = -100;
    Move list[250];
    History h;
    const int lgm = legalMoves(&b, list);
    const int numMoves = lgm >> 1;
    if (numMoves == 0)
    {
        return -height * lgm;
    }
    for (int i = 0; i < numMoves; ++i)
    {
        makeMove(&b, list[i], &h);
        int score = -determineMate(b, depth-1, height+1, -beta, -alpha);
        undoMove(&b, list[i], &h);

        if (score > best)
        {
            best = score;

            if (score > alpha){
                alpha = score;
                if (score >= beta)
                    break;
            }
        }
    }

    return best;
}