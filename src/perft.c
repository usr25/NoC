#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"
#include "../include/allmoves.h"
#include "../include/io.h"

#include <stdio.h>

uint64_t results[] = {1ULL, 20ULL, 400ULL, 8902ULL, 197281ULL, 4865609ULL, 119060324ULL, 3195901860ULL, 84998978956ULL, 2439530234167ULL};

uint64_t perftRecursive(Board b, const int depth)
{
    Move moves[200];
    History h;
    int numMoves, tot = 0;
    numMoves = legalMoves(&b, moves, b.turn);

    if (depth == 1)
        tot = numMoves;
    else
    {
        for (int i = 0; i < numMoves; ++i)
        {        
            makeMove(&b, moves[i], &h);
            tot += perftRecursive(b, depth - 1);
            undoMove(&b, moves[i], &h);
        }
    }
    
    return tot;
}

//Returns 1 if the perft of the starting position at a given depth is the number it should be
int perft(int depth, int tree)
{
    if (depth == 0) return 1;
    
    Move moves[200];
    Board b = defaultBoard();
    History h;
    int numMoves, tot = 0;

    numMoves = legalMoves(&b, moves, b.turn);
    for (int i = 0; i < numMoves; ++i)
    {        
        makeMove(&b, moves[i], &h);

        int temp = (depth == 1) ? 1 : perftRecursive(b, depth - 1);

        if (tree)
        {
            drawMove(moves[i]);
            printf(": %d\n", temp);
        }

        tot += temp;

        undoMove(&b, moves[i], &h);
    }
    
    return tot == results[depth];
}