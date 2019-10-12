/* uci.c
 * File in charge of the interaction with the user and chess interfaces
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

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

#define LEN 4096

static void uci(void);
static void isready(void);
static void perft_(Board b, int depth);
static void eval_(Board b);
static void best_(Board b, char* beg, Repetition* rep);
static void best_time(Board, char* beg, Repetition* rep);
static void help_(void);
static int move_(Board* b, char* beg, Repetition* rep);
static Board gen_(char* beg, Repetition* rep);
static Board gen_def(char* beg, Repetition* rep);

/* Main loop, listens to user input and performs the desired actions
 */
void loop(void)
{
    Board b = defaultBoard();
    Repetition rep = (Repetition) {.index = 0};

    char input[LEN];
    char* res, *beg;
    int quit = 0;

    while(!quit)
    {
        res = fgets(input, LEN, stdin);
        if (res == NULL) return;
        beg = input;

        if (strncmp(beg, "isready", 7) == 0)
            isready();

        else if (strncmp(beg, "ucinewgame", 10) == 0)
        {
            b = defaultBoard();
            rep.index = 0;
            rep.hashTable[rep.index++] = hashPosition(&b);
        }

        else if (strncmp(beg, "uci", 3) == 0)
            uci();

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

        else if (strncmp(beg, "go", 2) == 0)
            best_time(b, beg + 3, &rep);

        else if (strncmp(beg, "quit", 4) == 0)
            quit = 1;

        else if (strncmp(beg, "help", 4) == 0)
            help_();

        else
            fprintf(stdout, "# invalid command; type 'help'\n");

        fflush(stdout);
    }
}

static void uci(void)
{
    fprintf(stdout, "id name %s\n", ENGINE_NAME);
    fprintf(stdout, "id author %s\n", ENGINE_AUTHOR);
    fprintf(stdout, "uciok\n");
    fflush(stdout);
}
static void isready(void)
{
    fprintf(stdout, "readyok\n");
    fflush(stdout);
}
static void perft_(Board b, int depth)
{
    clock_t startTime = clock();
    fprintf(stdout, "Node count: %llu\n", perft(b, depth, 1));
    fprintf(stdout, "Time taken: %fs\n", (double)(clock() - startTime) / CLOCKS_PER_SEC);
    fflush (stdout);
}
static void eval_(Board b)
{
    fprintf(stdout, "%d\n", eval(&b));
    fflush(stdout);
}
static void best_time(Board b, char* beg, Repetition* rep)
{
    int callDepth = 0;
    int wtime = 0, btime = 0, winc = 0, binc = 0, movestogo = 0;

    clock_t movetime = 0;

    while (beg[1] != '\0' && beg[1] != '\n')
    {
        if (strncmp(beg, "wtime", 5) == 0) {
            beg += 6;
            wtime = atoi(beg);
        } else if (strncmp(beg, "btime", 5) == 0) {
            beg += 6;
            btime = atoi(beg);
        } else if (strncmp(beg, "winc", 4) == 0) {
            beg += 5;
            winc = atoi(beg);
        } else if (strncmp(beg, "binc", 4) == 0) {
            beg += 5;
            binc = atoi(beg);
        } else if (strncmp(beg, "movestogo", 9) == 0) {
            beg += 10;
            movestogo = atoi(beg);
        } else if (strncmp(beg, "depth", 5) == 0) {
            beg += 6;
            callDepth = atoi(beg);
        } else if (strncmp(beg, "movetime", 8) == 0) {
            beg += 9;
            movetime = (clock_t)atoi(beg);
        }

        ++beg;
    }

    Move best;
    char mv[6] = "";

    if (!callDepth)
    {
        //Play with time
        clock_t calcTime = 0;
        if (!movetime)
        {
            clock_t remTime = b.turn? wtime : btime;
            clock_t increment = b.turn? winc : binc;
            clock_t timeToMove;
            if (movestogo || increment)
            {
                if (movestogo && movestogo < 5)
                    timeToMove = min(remTime >> 1, remTime / (movestogo + 4) + (clock_t)((double)increment * .4));
                else
                    timeToMove = min(remTime >> 2, remTime / 27 + (clock_t)((double)increment * .95));
            }
            else
            {
                timeToMove = remTime / 41;
            }
            calcTime = (timeToMove * CLOCKS_PER_SEC) / 1000;
        }
        else
            calcTime = (movetime * CLOCKS_PER_SEC) / 1000;

        best = bestTime(b, calcTime, *rep, 0);
    }
    else
    {
        //Go for depth
        best = bestTime(b, 0, *rep, callDepth);
    }

    moveToText(best, mv);

    fprintf(stdout, "bestmove %s\n", mv);
    fflush(stdout);
}
static int move_(Board* b, char* beg, Repetition* rep)
{
    int prom = 0, from, to;
    from = getIndex(beg[0], beg[1]);
    to = getIndex(beg[2], beg[3]);

    Move m = (Move) {.from = from, .to = to, 
        .piece = pieceAt(b, POW2[from], b->turn),
        .capture = pieceAt(b, POW2[to], 1 ^ b->turn)};


    if(m.piece == KING)
    {
        if (abs(from - to) == 2) //Castle
        {
            if (to > from)
                m.castle = 2; //Castle queenside
            else
                m.castle = 1; //Castle kingside
        }
    }
    else if(m.piece == PAWN)
    {
        int piece = textToPiece(beg[4]);
        if(piece != NO_PIECE)
        {
            m.promotion = piece;
            prom = 1;
        }
        else if (abs(from - to) == 16)
        {
            b->enPass = to;
        }
        else if (b->enPass && abs(from - to) != 8)
        {
            if ((b->turn && (to - b->enPass == 8)) || (!b->turn && (to - b->enPass == -8)))
                m.enPass = b->enPass;
        }
    }

    makePermaMove(b, m);

    if (m.piece == PAWN || m.capture > 0)
        rep->index = 0;
    rep->hashTable[rep->index++] = hashPosition(b);

    return 4 + prom;
}

static Board gen_def(char* beg, Repetition* rep)
{
    Board b = defaultBoard();
    uint64_t startHash = hashPosition(&b);

    rep->hashTable[rep->index++] = startHash;

    if (strncmp(beg, "moves", 5) == 0)
    {
        beg += 6;
        while(beg[0] <= 'h' && beg[0] >= 'a')
            beg += move_(&b, beg, rep) + 1;
    }

    return b;
}
static Board gen_(char* beg, Repetition* rep)
{
    int counter;
    Board b = genFromFen(beg, &counter);
    uint64_t startHash = hashPosition(&b);

    rep->hashTable[rep->index++] = startHash;

    beg += counter + 1;

    while(beg[0] == ' ')
        beg += 1;

    if (strncmp(beg, "moves", 5) == 0)
    {
        beg += 6;
        while(beg[0] != ' ' && beg[0] != '\0' && beg[0] <= 'h' && beg[0] >= 'a')
            beg += move_(&b, beg, rep) + 1;
    }

    return b;
}

void infoString(const Move m, const int depth, const uint64_t nodes, const clock_t duration)
{
    char mv[6] = "";
    moveToText(m, mv);
    fprintf(stdout, "info score cp %d depth %d time %lu nodes %llu nps %llu pv %s\n", m.score, depth, duration, nodes, 100 * nodes / (duration + 1), mv);
    fflush(stdout);
}
static void help_(void)
{
    fprintf(stdout, "-====----------------====-\n");
    fprintf(stdout, "Chess Engine made by J; the commands are:\n");
    fprintf(stdout, "uci.............Print uci info\n");
    fprintf(stdout, "ucinewgame......Load starting position\n");
    fprintf(stdout, "isready.........To ensure the engine is ready to receive commands\n");
    fprintf(stdout, "eval............Static evaluation of loaded position\n");
    fprintf(stdout, "position\n");
    fprintf(stdout, "  startpos......Load starting position\n");
    fprintf(stdout, "  fen <fen>.....Load the fen's position\n");
    fprintf(stdout, "    moves.......List of moves to apply to the position\n");
    fprintf(stdout, "print...........Draw the position on the screen\n");
    fprintf(stdout, "perft #.........Count the number of legal positions at depth #\n");
    fprintf(stdout, "go\n");
    fprintf(stdout, "   depth #......Analyze at depth\n");
    fprintf(stdout, "   wtime #......Analyze until the time runs out\n");
    fprintf(stdout, "quit............Exit the engine\n");

    fprintf(stdout, "\nExample of use:\n");
    fprintf(stdout, "position startpos moves e2e4 e7e5\n");
    fprintf(stdout, "print\n");
    fprintf(stdout, "perft 6\n");
    fprintf(stdout, "go depth 7\n");
    fprintf(stdout, "-====----------------====-\n");
    fflush (stdout);
}