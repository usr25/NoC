/* 
 * File:   main.c
 *
 * Main file, in charge of calling the mainloop
 */

#include <stdio.h>
#include <stdlib.h>

#include "../include/global.h"
#include "../include/board.h"
#include "../include/test.h"
#include "../include/node.h"
#include "../include/memoization.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"
#include "../include/allmoves.h"
#include "../include/perft.h"

#include "../include/evaluation.h"
#include "../include/io.h"

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
    //slowTests();

    //printf("%d\n", perft(5, 0));
    //printf("%d\n", perft(6, 0));

    return 0;
}