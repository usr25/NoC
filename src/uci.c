#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../include/global.h"
#include "../include/uci.h"
#include "../include/board.h"
#include "../include/perft.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"
#include "../include/search.h"
#include "../include/io.h"
#include "../include/evaluation.h"

#define ENGINE_AUTHOR "usr"
#define ENGINE_NAME "Engine"
#define LEN 256

void uci();
void isready();
void perft_(Board b, int depth);
void eval_(Board b);
void best_(Board b, char* beg);
void move_(Board* b, char* beg);

void loop()
{
    Board b;

    char input[LEN];
    char* res, *beg;
    int quit = 0;

    res = fgets(input, LEN, stdin);
    if (res == NULL) return;
    beg = input;

    if (strncmp(beg, "ucinewgame", 10) == 0)
        b = defaultBoard();
    else if (strncmp(beg, "uci", 3) == 0)
        uci();
    else if (strncmp(beg, "isready", 7) == 0)
        isready();
    else
        return;

    while(! quit)
    {
        res = fgets(input, LEN, stdin);

        if (res == NULL) return;

        beg = input;

        if (strncmp(beg, "isready", 7) == 0)
            isready();

        else if (strncmp(beg, "ucinewgame", 10) == 0) 
            b = defaultBoard();
        
        else if (strncmp(beg, "print", 5) == 0) 
            drawPosition(b, 1);
        
        else if (strncmp(beg, "perft", 5) == 0)
            perft_(b, atoi(beg + 6));
        
        else if (strncmp(beg, "position", 8) == 0)
            b = genFromFen(beg + 9);
        
        else if (strncmp(beg, "eval", 4) == 0)
            eval_(b);
        
        else if (strncmp(beg, "best", 4) == 0)
            best_(b, beg + 5);

        else if (strncmp(beg, "move", 4) == 0)
            move_(&b, beg + 5);

        else if (strncmp(beg, "quit", 4) == 0 || beg[0] == 'q')//"quit" or something has gone wrong, either way exit
            quit = 1;

        else{
            fprintf(stdout, "Invalid command\n");
            fflush(stdout);
        }
    }
}

void uci()
{
    fprintf(stdout, "id name %s\n", ENGINE_NAME);
    fprintf(stdout, "id author %s\n", ENGINE_AUTHOR);
    fprintf(stdout, "uciok\n");
    fflush(stdout);
}
void isready()
{
    fprintf(stdout, "readyok\n");
    fflush(stdout);
}
void perft_(Board b, int depth)
{
    printf("%llu\n", perftRecursive(b, depth));
}
void eval_(Board b)
{
    printf("%d\n", eval(b));
}
void best_(Board b, char* beg)
{
    if (strncmp(beg, "tree", 4) == 0)
        drawMove(bestMoveAB(b, 5, 1));
    else
        drawMove(bestMoveAB(b, 5, 0));
    printf("\n");
}
void move_(Board* b, char* beg)
{
    Move m = (Move) {.from = getIndex(beg[0], beg[1]), .to = getIndex(beg[2], beg[3])};

    m.pieceThatMoves = pieceAt(b, POW2[m.from], b->turn);

    History h;
    makeMove(b, m, &h);
}