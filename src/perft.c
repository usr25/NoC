#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"

#include <stdio.h>
#include <stdlib.h>

uint64_t results[] = {1ULL, 20ULL, 400ULL, 8092ULL, 197281ULL, 4865609ULL, 119060324ULL};
/*
uint64_t perftRecursive(Board b, int depth, int color)
{
    Move moves[256];
    int numMoves, tot = 0;

    if (depth == 0)
    {
        if (isInCheck(&b, 1 ^ color))
            return 0ULL;

        return 1ULL;
    }

    if (color)
    {
        numMoves = allMovesWhite(&b, moves, 0);
       
        for (int i = 0; i < numMoves; ++i)
        {
            makeMoveWhite(&b, &moves[i], 0);
            tot += perftRecursive(b, depth - 1, 0);

            undoMoveWhite(&b, &moves[i], 0);
        }
    }
    else
    {
        numMoves = allMovesBlack(&b, moves, 0);

        for (int i = 0; i < numMoves; ++i)
        {
            makeMoveBlack(&b, &moves[i], 0);
            tot += perftRecursive(b, depth - 1, 1);
            
            undoMoveBlack(&b, &moves[i], 0);
        }
    }

    return tot;
}
*/
int perft(Board b, int depth)
{
    return 0;//return perftRecursive(b, depth, 1); //== results[depth];
}