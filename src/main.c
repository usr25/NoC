/* main.c
 * Main file, initializes memoization and calls loop, along with some tests if necessary
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <argp.h>

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
#include "../include/argparser.h"
#ifdef USE_TB
#include "../include/gaviota.h"
#endif
#ifdef TRAIN
#include "../include/train.h"
#endif

//TODO: In move, use uint64_t in .from and .to to make faster makeMove / undoMove and implement syzygy
int main(const int argc, char** const argv)
{
    #ifdef NDEBUG
    printf("    _   __      ______   _   ___   ____  ________\n   / | / /___  / ____/  / | / / | / / / / / ____/\n  /  |/ / __ \\/ /      /  |/ /  |/ / / / / __/   \n / /|  / /_/ / /___   / /|  / /|  / /_/ / /___   \n/_/ |_/\\____/\\____/  /_/ |_/_/ |_/\\____/_____/   \n\n");
    printf("%s (RELEASE) uci chess engine made by %s\n", ENGINE_NAME, ENGINE_AUTHOR);
    printf("NoC Copyright (C) 2021 Jorge Fernandez\n");
    #else
    printf("%s (DEBUG) uci chess engine made by %s\n", ENGINE_NAME, ENGINE_AUTHOR);
    #endif
    printf("Use --help for more information\n");

    Arguments arguments = parseArguments(argc, argv);

    initMemo();
    initMagics();
    initializeTable();

    #ifdef TRAIN
    if (arguments.train != NULL) {
        setVariables(argc, argv);
        readValues(arguments.train);
        txlTrain();
        exit(EXIT_SUCCESS);
    } else {
        fprintf(stderr, "No --train argument was provided\n");
        exit(100);
    }
    #endif

    initEval();
    initSort();

    #ifdef USE_NNUE
    if (arguments.nnue != NULL) {
        initNNUE(arguments.nnue);
    } else {
        fprintf(stderr, "No --nnue argument was provided. Initializing without.\n");
        exit(101);
    }
    #endif

    #ifdef USE_TB
    if (arguments.gaviota != NULL){
        initGav(arguments.gaviota);
    } else {
        fprintf(stdout, "No --gaviota argument was provided. Initializing without.\n");
        initGav("\0");
    }
    #endif

    //chooseTest(6);

    loop();

    exit(EXIT_SUCCESS);
}
