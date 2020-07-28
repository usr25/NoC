/* perft.c
 * Generates the perft for any given position
 */

#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"
#include "../include/allmoves.h"
#include "../include/movegen.h"
#include "../include/hash.h"
#include "../include/io.h"
#include "../include/perft.h"

#include <stdio.h>
#include <assert.h>


uint64_t perftRecursive(Board b, const int depth)
{
    if (depth == 0) return 1;

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
    uint64_t tot = 0, temp;

    const int numMoves = legalMoves(&b, moves) >> 1;

    if (depth == 1)
        return numMoves;

    for (int i = 0; i < numMoves; ++i)
    {
        assert(moveIsValidBasic(&b, &moves[i]));
        makeMove(&b, moves[i], &h);

        temp = perftRecursive(b, depth-1);

        if (divide)
        {
            drawMove(moves[i]);
            printf(": %llu\n", temp);
        }

        tot += temp;

        undoMove(&b, moves[i], &h);
    }

    return tot;
}

uint64_t perftMovegen(Board b, const int depth, const int divide) {

    if (depth == 0) return 1;

    MoveGen mg = newMG(&b);
    Move m;
    History h;
    uint64_t tot = 0, temp = 0;

    while (1) {
        m = next(&mg, &b);
        if (mg.state == Exhausted) break;
        assert(m.from != -1);
        makeMove(&b, m, &h);

        temp = perftMovegen(b, depth-1, 0);

        if (divide)
        {
            drawMove(m);
            printf(": %llu\n", temp);
        }

        tot += temp;

        undoMove(&b, m, &h);
    }

    return tot;
}

/* This is an especial version of the perft to ensure that the zobrist
 * hash update works
 */
int hashPerft(Board b, const int depth, const uint64_t prevHash)
{
    if (depth == 0) return 1;

    Move moves[NMOVES];
    History h;

    const int numMoves = legalMoves(&b, moves) >> 1;

    for (int i = 0; i < numMoves; ++i)
    {
        makeMove(&b, moves[i], &h);
        uint64_t newHash = makeMoveHash(prevHash, &b, moves[i], h);

        if (newHash != hashPosition(&b) && !hashPerft(b, depth - 1, newHash))
            return 0;

        undoMove(&b, moves[i], &h);
    }

    return 1;
}