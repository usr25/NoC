#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"
#include "../include/io.h"

#include <stdio.h>

uint64_t results[] = {1ULL, 20ULL, 400ULL, 8902ULL, 197281ULL, 4865609ULL, 119060324ULL};

//No pawns
<<<<<<< HEAD
//50, 2125, 96062, 4200525, 191462298 // WORKS 4200525

//No pawns no castle -> rnbqkbnr/8/8/8/8/8/8/RNBQKBNR w - -
//50, 2125, 96062, 4200525, 191419375 //WORKS ALL

//Castle -> r3k3/8/8/8/8/3b4/8/R3K2R b KQkq -
//27, 586, 13643 //WORKS 586

=======
//50, 2125, 96062, 4200525, 191462298
//No pawns no castle -> rnbqkbnr/8/8/8/8/8/8/RNBQKBNR w - -
//50, 2125, 96062, 4200525, 191419375 //WORKS
//Castle -> r3k3/8/8/8/8/3b4/8/R3K2R b KQkq -
//27, 586, 13643
>>>>>>> 7ee8f5e84b1cd97cad048116681915519a1d2f2e
uint64_t perftRecursive(Board b, const int depth, const int color)
{
    if (depth == 0) return 1;

    Move moves[200];
    int numMoves, tot = 0;

    numMoves = allMoves(&b, moves, 0, color);
    
    for (int i = 0; i < numMoves; ++i)
    {        
        makeMove(&b, &moves[i]);
        if (isInCheck(&b, color) == NO_PIECE)
            tot += perftRecursive(b, depth - 1, 1 ^ color);

        undoMove(&b, moves[i]);
    }
    
    return tot;
}

int perft(Board b, int depth, int tree)
{
    if (depth == 0) return 1;
    
    Move moves[200];
    int numMoves, tot = 0;

    numMoves = allMoves(&b, moves, 0, WHITE);
    for (int i = 0; i < numMoves; ++i)
    {        
        makeMove(&b, &moves[i]);

        if (isInCheck(&b, 1) == NO_PIECE){
            int temp = perftRecursive(b, depth - 1, 0);

            if (tree)
            {
                drawMove(moves[i]);
                printf(": %d\n", temp);
            }

            tot += temp;
        }

        undoMove(&b, moves[i]);
    }
    
    return results[depth] == tot;
}