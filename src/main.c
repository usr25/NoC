/* main.c
 *
 * Main file, initializes memoization and calls loop, along with some tests if necessary
 */

#include <stdio.h>

#include "../include/global.h"
#include "../include/memoization.h"
#include "../include/board.h"
#include "../include/test.h"
#include "../include/moves.h"
#include "../include/hash.h"
#include "../include/perft.h"
#include "../include/search.h"
#include "../include/uci.h"
#include "../include/io.h"

//position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 moves e2e4
//best results -> depth 6 capt 1

int main(const int argc, char* const argv)
{
    initialize();

    //runTests();
    //slowTests();  //57'6s
    //slowEval();   //-, 14'1 without initialization

    //printf("%d\n", perft(6, 1)); //5'75s
    //printf("%d\n", perft(7, 1)); //2:22m

    loop();

    return 0;
}