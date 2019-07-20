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
#include "../include/io.h"

//position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 moves e2e4
//best results -> depth 6 capt 1

int main(const int argc, char* const argv)
{
    initialize();

    //runTests();
    //slowTests();  //57'6s
    //slowEval();   //14'1 without initialization

    //printf("%d\n", perft(6, 1)); //5'75s
    //printf("%d\n", perft(7, 1)); //2:22m

    loop();

    int a;
    Board b;
    /*
    b = genFromFen("8/3R4/8/1k6/5K2/8/8/8 w - -", &a);
    //drawPosition(b, 1);
    drawMove(rookMate(b)); printf(" == d7c7\n");

    b = genFromFen("8/3R4/1k6/8/5K2/8/8/8 w - -", &a);
    //drawPosition(b, 1);
    drawMove(rookMate(b)); printf(" == d7d1\n");
    */
    /*
    b = genFromFen("8/2R5/1k6/8/5K2/8/8/8 w - -", &a);
    //drawPosition(b, 1);
    drawMove(rookMate(b)); printf(" == c7c1\n");
    */
    /*
    b = genFromFen("8/8/1k6/8/5K2/8/8/2R5 w - -", &a);
    //drawPosition(b, 1);
    drawMove(rookMate(b)); printf(" == f4e5\n");
    */
    /*
    b = genFromFen("8/8/1k1K4/8/8/8/8/2R5 w - -", &a);
    drawPosition(b, 1);
    drawMove(rookMate(b)); printf(" == c1b1\n");
    */
    /*
    b = genFromFen("8/8/1k1K4/2R5/8/8/8/8 w - -", &a);
    drawPosition(b, 1);
    drawMove(rookMate(b)); printf(" == c5c1\n");
    */
    return 0;
}