/* 
 * File:   main.c
 *
 * Main file, in charge of calling the mainloop
 */

#include <stdio.h>

#include "../include/global.h"
#include "../include/memoization.h"
#include "../include/board.h"
#include "../include/test.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"
#include "../include/perft.h"

#include "../include/io.h"

int main(const int argc, char* const argv[])
{
    initialize();

    Board start;
    if (argc > 1)
        start = generateFromFenEP(argv[1], argv[2], argv[3], argv[4]);
    else
        start = defaultBoard();

    drawPosition(start, 1);

    printf("[+] Is starting pos: %s\n", BOOL(validStartingPos(start)));
    printf("[+] Pieces are valid: %s\n", BOOL(validPieces(start)));

    runTests();
    //slowTests();

    printf("%d\n", perft(6, 1));

    return 0;
}