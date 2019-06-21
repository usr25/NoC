/* 
 * File:   main.c
 *
 * Main file, in charge of calling the mainloop
 */

#include <stdio.h>

#include "../include/global.h"
#include "../include/board.h"
#include "../include/io.h"
#include "../include/test.h"
#include "../include/node.h"
#include "../include/memoization.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"
#include "../include/perft.h"

#include "../include/evaluation.h"


//rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1  Starting pos
//8/pppppppp/rnbqkbNr/8/8/RNBQKBnR/PPPPPPPP/8 w KQkq - 0 1  All pieces in front of pawns
//5N2/5Pkq/5p2/3P4/3pKQ2/3n4/8/8 w - -                      King moves
//8/2n2N2/8/1br2rb1/n1Q2q1N/1BR2RB1/8/8 w - -               Queen moves
//8/8/2Br2R1/8/8/2BR2r1/8/8 w - -                           Rook moves
//8/2r4R/3B2b1/8/8/3B2b1/2R4r/8 w - -                       Bish moves
//8/1p1p1Pk1/2N3K1/n3N3/2n3p1/qP1PQ3/8/8 w - -              Knight moves
//8/4p3/3Q1q2/4q3/4q3/pq3P1p/P5P1/8 w - -                   Pawn moves

void initializePOW2()
{
    for (int i = 0; i < 64; ++i)
        POW2[i] = 1ULL << i;
}

int main(const int argc, char* const argv[])
{
    initializePOW2();

    Board start;
    if (argc >= 2)
        start = generateFromFen(argv[1], argv[2], argv[3]);
    else
        start = defaultBoard();

    drawPosition(start, 1);
    initialize();

    printf("[+] Is starting pos: %s\n", BOOL(validStartingPos(start)));
    printf("[+] Pieces are valid: %s\n", BOOL(validPieces(start)));

    runTests();
    

    return 0;
}