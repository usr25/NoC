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
#include "../include/mate.h"
#include "../include/evaluation.h"
#include "../include/io.h"
#include "../include/magic.h"

//position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 moves e2e4
//best results -> depth 6 capt 1

int main(const int argc, char* const argv)
{
    initialize();
    populateMagics();

    //runTests();
    //slowTests();  //51'3s
    //slowEval();   //8'7 without initialization

    //printf("%d\n", perft(6, 1)); //5s
    //printf("%d\n", perft(7, 1)); //2:1m

    loop();

    return 0;
}