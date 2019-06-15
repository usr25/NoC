/* 
 * File:   main.c
 *
 * Main file, in charge of calling the mainloop
 */

#include <stdio.h>
#include <stdlib.h>

#include "../include/io.h"
#include "../include/board.h"
#include "../include/test.h"

#define BOOL(x) ((x)?"true":"false")

//rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1

int main(const int argc, char* const argv[])
{
    Board start;
    if (argc >= 2)
        start = generateFromFen(argv[1], argv[2], argv[3]);
    else
        start = defaultBoard();
    drawPosition(start);
    printf("[+] Is starting pos: %s\n", BOOL(validStartingPos(start)));
    printf("[+] Pieces are valid: %s\n", BOOL(validPieces(start)));

    return (EXIT_SUCCESS);
}