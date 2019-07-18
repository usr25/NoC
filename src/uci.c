/* uci.c
 * File in charge of the interaction with the user and chess interfaces
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../include/global.h"
#include "../include/board.h"
#include "../include/perft.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"
#include "../include/hash.h"
#include "../include/search.h"
#include "../include/io.h"
#include "../include/evaluation.h"
#include "../include/uci.h"

#define ENGINE_AUTHOR "usr"
#define ENGINE_NAME "DEV"
#define LEN 4096
#define DEPTH 6

void uci(void);
void isready(void);
void perft_(Board b, int depth);
void eval_(Board b);
void best_(Board b, char* beg, Repetition* rep);
int move_(Board* b, char* beg, Repetition* rep, uint64_t prevHash);
Board gen_(char* beg, Repetition* rep);
Board gen_def(char* beg, Repetition* rep);

Move evalPos(char* beg);

/*
 * Main loop, listens to user input and performs the desired actions
 */
//TODO: Implement time management
void loop(void)
{
    Board b;
    Repetition rep = (Repetition) {.index = 0};

    char input[LEN];
    char* res, *beg;
    int quit = 0;

    while(1)
    {
        res = fgets(input, LEN, stdin);
        if (res == NULL) return;
        beg = input;

        if (strncmp(beg, "ucinewgame", 10) == 0){
            b = defaultBoard();
            rep.hashTable[rep.index++] = hashPosition(&b);
            break;
        }
        else if (strncmp(beg, "uci", 3) == 0){
            uci();
            break;
        }
        else if (strncmp(beg, "isready", 7) == 0){
            isready();
            break;
        }
        else
            return;
    }

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
        
        else if (strncmp(beg, "position startpos", 17) == 0)
            b = gen_def(beg + 18, &rep);

        else if (strncmp(beg, "position fen", 12) == 0)
            b = gen_(beg + 13, &rep);

        else if(strncmp(beg, "position", 8) == 0)
            b = gen_(beg + 9, &rep);

        else if (strncmp(beg, "eval", 4) == 0)
            eval_(b);
        
        else if (strncmp(beg, "best", 4) == 0 || strncmp(beg, "go", 2) == 0 || strncmp(beg, "bestmove", 8) == 0)
            best_(b, beg + 5, &rep);

        else if (strncmp(beg, "quit", 4) == 0)
            quit = 1;

        else{
            fprintf(stdout, "Invalid command\n");
            fflush(stdout);
        }
    }
}

void uci(void)
{
    fprintf(stdout, "id name %s\n", ENGINE_NAME);
    fprintf(stdout, "id author %s\n", ENGINE_AUTHOR);
    fprintf(stdout, "uciok\n");
    fflush(stdout);
}
void isready(void)
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
void best_(Board b, char* beg, Repetition* rep)
{
    Move best;
    char mv[6] = "";

    best = bestMoveAB(b, DEPTH, 0, *rep); //WARNING: Remove the 1
    
    moveToText(best, mv);
    fprintf(stdout, "bestmove %s\n", mv);
    fflush(stdout);
}
//TODO: make use of prevhash, for convinience it is recalculated
int move_(Board* b, char* beg, Repetition* rep, uint64_t prevHash)
{
    int prom = 0, from, to;
    from = getIndex(beg[0], beg[1]);
    to = getIndex(beg[2], beg[3]);

    Move m = (Move) {.from = from, .to = to, 
        .pieceThatMoves = pieceAt(b, POW2[from], b->turn),
        .capture = pieceAt(b, POW2[to], 1 ^ b->turn)};

    
    if(m.pieceThatMoves == KING)
    {
        if (abs(from - to) == 2) //Castle
        {
            if (to > from)
                m.castle = 2; //Castle queenside
            else
                m.castle = 1; //Castle kingside
        }
    }

    if(m.pieceThatMoves == PAWN)
    {
        int piece = textToPiece(beg[4]);
        if(piece != NO_PIECE)
        {
            m.promotion = piece;
            prom++;
        }
        if (abs(from - to) == 16)
            b->enPass = to;

        if (b->enPass - from == 1 && (from & 7) != 7 && (b->piece[1 ^ b->turn][PAWN] & POW2[b->enPass]))
            m.enPass = b->enPass;
        else if (b->enPass - from == -1 && (from & 7) != 0 && (b->piece[1 ^b->turn][PAWN] & POW2[b->enPass]))
            m.enPass = b->enPass;
    }

    History h;
    makeMove(b, m, &h);

    if (m.pieceThatMoves == PAWN || m.capture > 0)
        rep->index = 0;
    else
        rep->hashTable[rep->index++] = hashPosition(b);

    return 4 + prom;
}
Board gen_def(char* beg, Repetition* rep)
{
    Board b = defaultBoard();
    uint64_t startHash = hashPosition(&b);

    rep->hashTable[rep->index++] = startHash;

    if (strncmp(beg, "moves", 5) == 0)
    {
        beg += 6;
        while(beg[0] != ' ' && beg[0] != '\0' && beg[0] <= 'h' && beg[0] >= 'a')
            beg += move_(&b, beg, rep, rep->hashTable[rep->index - 1]) + 1;
    }

    return b;
}

Board gen_(char* beg, Repetition* rep)
{
    int counter;
    Board b = genFromFen(beg, &counter);
    uint64_t startHash = hashPosition(&b);
    
    rep->hashTable[rep->index++] = startHash;
    
    beg += counter + 1;

    if (strncmp(beg, "moves", 5) == 0)
    {
        beg += 6;
        while(beg[0] != ' ' && beg[0] != '\0' && beg[0] <= 'h' && beg[0] >= 'a')
            beg += move_(&b, beg, rep, rep->hashTable[rep->index - 1]) + 1;
    }

    return b;
}

Move evalPos(char* beg)
{
    int a;
    Board b = genFromFen(beg, &a);
    Move best = bestMoveAB(b, DEPTH, 1, (Repetition){});
    drawMove(best);
    printf("\n");
    return best;
}