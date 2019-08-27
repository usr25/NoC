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


uint64_t perftRecursive(Board b, const int depth)
{
    Move moves[NMOVES];
    History h;
    uint64_t tot = 0;

    const int numMoves = legalMoves(&b, moves) >> 1;

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

uint64_t perft(Board b, const int depth, const int divide)
{
    if (depth == 0) return 1;

    Move moves[NMOVES];
    History h;
    uint64_t tot = 0;

    const int numMoves = legalMoves(&b, moves) >> 1;

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