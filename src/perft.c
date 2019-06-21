#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"

#include <stdio.h>

uint64_t results[] = {1ULL, 20ULL, 400ULL, 8902ULL, 197281ULL, 4865609ULL, 119060324ULL};

//No pawns
//50, 2125, 96062, 4200525, 191462298
uint64_t perftRecursive(Board b, int depth, const int color)
{
    if (depth == 0) return 1;

    Move moves[100];
    int numMoves, tot = 0;

    numMoves = allMoves(&b, moves, 0, color);
    for (int i = 0; i < numMoves; ++i)
    {        
        makeMove(&b, &moves[i], color);
        if (isInCheck(&b, color) == NO_PIECE)
            tot += perftRecursive(b, depth - 1, 1 ^ color);

        undoMove(&b, moves[i], color);
    }
    
    return tot;
}

int perft(Board b, int depth, int tree)
{
    if (depth == 0) return 1;
    
    Move moves[100];
    int numMoves, tot = 0;

    numMoves = allMoves(&b, moves, 0, 1);
    for (int i = 0; i < numMoves; ++i)
    {        
        makeMove(&b, &moves[i], 1);

        if (isInCheck(&b, 1) == NO_PIECE){
            int temp = perftRecursive(b, depth - 1, 0);

            if (tree)
            {
                char a = 'a' + (moves[i].from % 8);
                int a1 = (moves[i].from / 8) + 1;
                char b = 'a' + (moves[i].to % 8);
                int b1 = (moves[i].to / 8) + 1;

                printf("%c%d%c%d: %d\n", a, a1, b, b1, temp);
            }
            tot += temp;
        }

        undoMove(&b, moves[i], 1);
    }
    
    return results[depth] == tot;
}