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

int main(int argc, char const *argv[])
{
    drawPosition(defaultBoard());
    printf("[+] Starting pos is valid: %s\n", BOOL(validStartingPos(defaultBoard())));
    return (EXIT_SUCCESS);
}