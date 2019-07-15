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
#include "../include/allmoves.h"
#include "../include/perft.h"
#include "../include/uci.h"
#include "../include/search.h"
#include "../include/evaluation.h"
#include "../include/hash.h"

#include "../include/io.h"

//position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 moves e2e4

int main(const int argc, char* const argv[])
{
    initialize();

    //runTests();
    //slowTests();  //58'8s
    //slowEval();   //47'9s

    //printf("%d\n", perft(6, 1)); //5'9s

    loop();
    
    return 0;
}