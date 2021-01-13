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
#include "../include/evaluation.h"
#include "../include/search.h"
#include "../include/sort.h"
#include "../include/movegen.h"
#include "../include/nnue.h"
#ifdef USE_TB
#include "../include/gaviota.h"
#endif
#ifdef TRAIN
#include "../include/train.h"
#endif

#ifndef NNUE_PATH
#define NNUE_PATH "nn-f4838ada61cc.nnue"
#endif

//TODO: In move, use uint64_t in .from and .to to make faster makeMove / undoMove and implement syzygy
int main(const int argc, char** const argv)
{
    #ifdef NDEBUG
    printf("    _   __      ______\n   / | / /___  / ____/\n  /  |/ / __ \\/ /     \n / /|  / /_/ / /___   \n/_/ |_/\\____/\\____/  \n\n");
    printf("%s (RELEASE) uci chess engine made by %s\n", ENGINE_NAME, ENGINE_AUTHOR);
    #else
    printf("%s (DEBUG) uci chess engine made by %s\n", ENGINE_NAME, ENGINE_AUTHOR);
    #endif

    initMemo();
    initMagics();

    #ifdef TRAIN
    setVariables(argc, argv);
    readValues(argv[1]);
    txlTrain();
    exit(EXIT_SUCCESS);
    #endif

    initEval();
    initSort();

    #ifdef USE_TB
    if (argc > 1)
        initGav(argv[1]);
    else
        initGav("\0");
    #endif

    //chooseTest(6);

    loop();

    exit(EXIT_SUCCESS);
}
