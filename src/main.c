/* main.c
 * Main file, initializes memoization and calls loop, along with some tests if necessary
 */

#include <stdio.h>
#include <time.h>

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
//TODO: In move, use uint64_t in .from and .to to make faster makeMove / undoMove
int main(const int argc, char* const argv)
{
    initialize();
    populateMagics();

    //runTests();
    //slowTests();  //58'4s
    //slowEval();   //6'4s null, 8'7s

    //printf("%d\n", perft(6, 1)); //3'8s
    //printf("%d\n", perft(7, 1)); //1:36m

    loop();

    return 0;
}