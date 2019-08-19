/* perft.c
 * Generates the perft for any given position
 */

#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"
#include "../include/allmoves.h"
#include "../include/io.h"

#include <stdio.h>

//The perft results from the starting position (until perft 9)
uint64_t results[10] = {1ULL, 20ULL, 400ULL, 8902ULL, 197281ULL, 4865609ULL, 119060324ULL, 3195901860ULL, 84998978956ULL, 2439530234167ULL};

uint64_t perftRecursive(Board b, const int depth)
{
    Move moves[NMOVES];
    History h;
    uint64_t tot = 0;

    int numMoves = legalMoves(&b, moves) >> 1;

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

uint64_t perft(Board b, const int depth, int divide)
{
    if (depth == 0) return 1;

    Move moves[NMOVES];
    History h;
    uint64_t tot = 0;

    int numMoves = legalMoves(&b, moves) >> 1;
    
    if (depth == 1)
        return numMoves;
    
    for (int i = 0; i < numMoves; ++i)
    {        
        makeMove(&b, moves[i], &h);

        int temp = perftRecursive(b, depth - 1);

        if (divide)
        {
            drawMove(moves[i]);
            printf(": %d\n", temp);
        }

        tot += temp;

        undoMove(&b, moves[i], &h);
    }

    return tot;
}