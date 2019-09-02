/* main.c
 * Main file, initializes memoization and calls loop, along with some tests if necessary
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../include/global.h"
#include "../include/memoization.h"
#include "../include/board.h"
#include "../include/test.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"
#include "../include/hash.h"
#include "../include/uci.h"
#include "../include/io.h"
#include "../include/magic.h"
#include "../include/search.h"
#include "../include/sort.h"
#include "../include/gaviota.h"


//position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 moves e2e4
//TODO: In move, use uint64_t in .from and .to to make faster makeMove / undoMove and implement syzygy
int main(const int argc, char** const argv)
{
    initMemo();
    initMagics();
    if (argc > 1)
        initGav(argv[1]);
    else
        initGav("\0");

    //0, 1->51'1, 2->3'5s, 3->2:4, 4->51s, 5-> -
    //chooseTest(1);

    loop();

    return 0;
}