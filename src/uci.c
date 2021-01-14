/* uci.c
 * File in charge of the interaction with the user and chess interfaces
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>

#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"
#include "../include/hash.h"
#include "../include/search.h"
#include "../include/io.h"
#include "../include/evaluation.h"
#include "../include/uci.h"
#include "../include/mate.h"
#include "../include/nnue.h"
#include "../include/perft.h"

#define LEN 4096

static void uci(void);
static void isready(void);
static void perft_(Board b, int depth);
static void mate_(Board b, int depth);
static void eval_(Board b);
static void go_(Board b, char* beg, Repetition* rep);
static void help_(void);
static int move_(Board* b, char* beg, Repetition* rep);
static Board gen_(char* beg, Repetition* rep);
static Board gen_def(char* beg, Repetition* rep);

#ifndef NNUE_PATH
#define NNUE_PATH "nn-f4838ada61cc.nnue"
#endif

/* Main loop, listens to user input and performs the desired actions
 */
void loop(void)
{
    #ifdef USE_NNUE
    initNNUE(NNUE_PATH);
    #endif
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

        else if (strncmp(beg, "go", 2) == 0)
            go_(b, beg + 3, &rep);

        else if (strncmp(beg, "position startpos", 17) == 0)
            b = gen_def(beg + 18, &rep);

        else if (strncmp(beg, "position fen", 12) == 0)
            b = gen_(beg + 13, &rep);

        else if(strncmp(beg, "position", 8) == 0)
            b = gen_(beg + 9, &rep);

        else if (strncmp(beg, "ucinewgame", 10) == 0)
        {
            b = defaultBoard();
            rep.hashTable[0] = hashPosition(&b);
            rep.index = 1;
        }

        else if (strncmp(beg, "uci", 3) == 0)
            uci();

        else if (strncmp(beg, "print", 5) == 0)
            drawPosition(b, 1);

        else if (strncmp(beg, "perft", 5) == 0)
            perft_(b, atoi(beg + 6));

        else if (strncmp(beg, "eval", 4) == 0)
            eval_(b);

        else if (strncmp(beg, "mate", 4) == 0)
            mate_(b, atoi(beg + 5));

        else if (strncmp(beg, "loadnnue", 8) == 0)
        {
            #ifdef USE_NNUE
            char* b = beg;
            while (*b != '\n') b++;
            *b = '\0';
            initNNUE(beg + 9);
            #else
            fprintf(stderr, "USE_NNUE hasn't been defined, not using NNUE\n");
            fflush(stderr);
            #endif
        }

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
    fprintf(stdout, "Node count: %lu\n", perft(b, depth, 1));
    fprintf(stdout, "Time taken: %fs\n", (double)(clock() - startTime) / CLOCKS_PER_SEC);
    fflush (stdout);
}
static void mate_(Board b, int depth)
{
    clock_t startTime = clock();
    char mv[6] = "";
    Move m = findMate(b, depth);
    moveToText(m, mv);

    fprintf(stdout, "mate %s in %d\n", mv, m.score);
    fprintf(stdout, "Time taken: %fs\n", 
        (double)(clock() - startTime) / CLOCKS_PER_SEC);
    fflush (stdout);
}
static void eval_(Board b)
{
    //TODO: Implement this once nn is in nnue.c
    #ifdef USE_NNUE
    const int ev = evaluateNNUE(&b, 0);
    #else
    const int ev = eval(&b);
    #endif
    fprintf(stdout, "%d\n", ev);
    fflush(stdout);
}
static void go_(Board b, char* beg, Repetition* rep)
{
    SearchParams sp = {.depth = 0, .timeToMove = 0, .extraTime = 0};
    int wtime = 0, btime = 0, winc = 0, binc = 0, movestogo = 0;

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
            sp.depth = atoi(beg);
        } else if (strncmp(beg, "movetime", 8) == 0) {
            beg += 9;
            sp.timeToMove = (clock_t)atoi(beg) * CLOCKS_PER_SEC / 1000;
        }

        ++beg;
    }

    Move best;
    char mv[6] = "";

    if (!sp.depth)
    {
        //Play with time
        if (!sp.timeToMove)
        {
            clock_t remTime   = max(b.stm? wtime : btime, 100) - 50; //To avoid problems with lag
            clock_t increment = b.stm? winc  : binc;
            clock_t timeInSecs, timeInTicks, remTimeInTicks;
            remTimeInTicks = remTime * CLOCKS_PER_SEC / 1000;

            if (movestogo)
                timeInSecs = min(remTime / 5, remTime / (movestogo + 4) + (clock_t)((double)increment * .95));
            else if (increment)
                timeInSecs = min(remTime / 5, remTime / 61 + (clock_t)((double)increment * .95));
            else
                timeInSecs = remTime / 51;

            timeInTicks = timeInSecs * CLOCKS_PER_SEC / 1000;

            sp.timeToMove = timeInTicks;
            sp.extraTime = min(timeInTicks / 2, (remTimeInTicks - timeInTicks) / 4);
        }
    }

    assert(sp.timeToMove >= 0);
    assert(sp.extraTime >= 0);
    best = bestTime(b, *rep, sp);

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
        .piece = pieceAt(b, POW2[from], b->stm),
        .capture = pieceAt(b, POW2[to], 1 ^ b->stm)};


    if(m.piece == KING)
    {
        if (to == from + 2) //Queenside castle
            m.castle = 2;
        else if (to == from - 2) //Kingside castle
            m.castle = 1;
        else
            m.castle = 0;
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
            if ((b->stm && (to - b->enPass == 8)) || (!b->stm && (to - b->enPass == -8)))
                m.enPass = b->enPass;
        }
    }

    if (m.piece < KING || m.piece > PAWN || ((m.capture < KING || m.capture > PAWN) && m.capture != NO_PIECE))
    {
        printf("%d\n", b->stm);
        printf("%s\n", beg);
        printf("%d\n", m.piece);
        drawPosition(*b, 1);
        drawBitboard(b->piece[WHITE][BISH]);
        drawBitboard(b->piece[WHITE][PAWN]);
        drawBitboard(b->piece[BLACK][PAWN]);
        drawBitboard(b->allPieces);
    }

    makePermaMove(b, m);

    if (m.piece == PAWN || IS_CAP(m))
        rep->index = 0;
    rep->hashTable[rep->index++] = hashPosition(b);

    return 4 + prom;
}

static Board gen_def(char* beg, Repetition* rep)
{
    Board b = defaultBoard();
    uint64_t startHash = hashPosition(&b);

    rep->index = 0;
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

    rep->index = 0;
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
    fprintf(stdout, "info score cp %d depth %d time %lu nodes %lu nps %lu pv %s\n", 
        100 * m.score / V_PAWN[0], depth, duration, nodes, 1000 * nodes / (duration + 1), mv);
    fflush(stdout);
}
static void help_(void)
{
    fprintf(stdout, "-====----------------====-\n");
    fprintf(stdout, "Chess Engine made by Jorge; the commands are:\n");
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
    fprintf(stdout, "mate #..........Determine the shortest mate within # plies\n");
    fprintf(stdout, "loadnnue <path>.Load the NNUE file <path>\n");
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