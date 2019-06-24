#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"
#include "../include/allmoves.h"
#include "../include/io.h"

#include <stdio.h>

uint64_t results[] = {1ULL, 20ULL, 400ULL, 8902ULL, 197281ULL, 4865609ULL, 119060324ULL};

//No pawns
//50, 2125, 96062, 4200525, 191462298 // WORKS ALL

//No pawns no castle -> rnbqkbnr/8/8/8/8/8/8/RNBQKBNR w - -
//50, 2125, 96062, 4200525, 191419375 //WORKS ALL

//Castle -> r3k3/8/8/8/8/3b4/8/R3K2R b KQkq -
//27, 586, 13643, 322232, 7288108 //WORKS ALL

//Bish & Knight -> 4k3/1b2nbb1/3n4/8/8/4N3/1B1N1BB1/4K3 w -
//35, 1252, 42180, 1467048, 48483119 //WORKS ALL

//Queen -> 4kq2/4q3/8/8/8/8/1Q6/Q3K3 w - -
//4, 105, 2532, 79266 //WORKS ALL

//Promotion -> 8/4P1K1/8/8/8/1k6/3p4/8 w - -
//12, 142, 1788, 22121, 334791, 4791402

//Deep -> 8/8/8/1P2K1p1/P6p/3k4/8/8 w - -
//7, 62, 549, 5075, 46842, 424286, 3899674, 34987984

//En passant -> 8/1p3k2/7K/8/2P5/8/8/8 w - -
//4, 32, 185, 1382, 9120, 67430, 482124, 3558853 //WORKS ALL

uint64_t perftRecursive(Board b, const int depth, const int color)
{
    Move moves[200];
    History h = (History) {.color = color};
    int numMoves, tot = 0;
    numMoves = legalMoves(&b, moves, color);

    if (depth == 1)
        tot = numMoves;
    else if (depth == 0)
        tot = 1;
    else
    {
        for (int i = 0; i < numMoves; ++i)
        {        
            makeMove(&b, moves[i], &h);
            tot += perftRecursive(b, depth - 1, 1 ^ color);
            undoMove(&b, moves[i], &h);
        }
    }
    
    return tot;
}

int perft(int depth, int tree)
{
    if (depth == 0) return 1;
    
    Move moves[200];
    Board b = defaultBoard();
    History h = (History) {.color = b.posInfo & 1};
    int numMoves, tot = 0;

    numMoves = allMoves(&b, moves, b.posInfo & 1);
    for (int i = 0; i < numMoves; ++i)
    {        
        makeMove(&b, moves[i], &h);

        if (isInCheck(&b, 1) == NO_PIECE){
            int temp = perftRecursive(b, depth - 1, 1 ^ (b.posInfo & 1));

            if (tree)
            {
                drawMove(moves[i]);
                printf(": %d\n", temp);
            }

            tot += temp;
        }

        undoMove(&b, moves[i], &h);
    }
    
    return results[depth] == tot;
}