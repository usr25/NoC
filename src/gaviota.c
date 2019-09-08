/* gaviota.c
 * This file contains functionality related to the use of the gaviota tablebases.
 * Used in endgames and, mainly, to give mate in KQvK, KPvK, KRvK
 */
#include <stdio.h>

#include "../gav/gtb-probe.h"

#include "../include/global.h"
#include "../include/board.h"
#include "../include/gaviota.h"

static int getGavSqr(int inBoard)
{
    int sqr = inBoard - inBoard % 8;
    sqr += 7 - inBoard % 8;
    return sqr;
}

static int pieceAtGav(Board* const b, const uint64_t pos, const int color)
{
    if (pos & b->piece[color][PAWN])     return tb_PAWN;
    else if (pos & b->piece[color][ROOK]) return tb_ROOK;
    else if (pos & b->piece[color][BISH]) return tb_BISHOP;
    else if (pos & b->piece[color][KNIGHT]) return tb_KNIGHT;
    else if (pos & b->piece[color][QUEEN]) return tb_QUEEN;
    else if (pos & b->piece[color][KING]) return tb_KING;

    return tb_NOPIECE;
}

static void dtmPrint (unsigned stm, int tb_available, unsigned info, unsigned pliestomate);
static void parseBoard(Board* b);
static void parsePieces(Board* b);
static void parseCastle(const Board* b);
static void parseEpSqr(const Board* b);

/* Necessary variables */
static unsigned int  stm;      /* side to move */
static unsigned int  epsquare; /* target square for an en passant capture */
static unsigned int  castling; /* castling availability, 0 => no castles */
static unsigned int  ws[17];   /* list of squares for white */
static unsigned int  bs[17];   /* list of squares for black */
static unsigned char wp[17];   /* what white pieces are on those squares */
static unsigned char bp[17];   /* what black pieces are on those squares */

static char *initinfo;             /* NULL if verbosity=0, initialization info if verbosity=1*/
static int tb_available;           /* 0 => FALSE, 1 => TRUE */
static unsigned info = tb_UNKNOWN; /* default, no tbvalue */
static unsigned pliestomate;   

static int verbosity = 0;      /* initialization 0 = non-verbose, 1 = verbose */
static int scheme = tb_CP4;    /* compression scheme to be used */
static const char ** paths;    /* paths where files will be searched */
static size_t cache_size = 64*1024*1024; /* 64 MiB in this example */

/*  wdl_fraction:
fraction, over 128, that will be dedicated to wdl information. 
In other words, 96 means 3/4 of the cache will be dedicated to 
win-draw-loss info, and 1/4 dedicated to distance to mate 
information.
*/
static int wdl_fraction = 128; //The DTM info is only used once per call, no need to waste cache

//boolean array with the availability of each tablebase, 3, 4, 5 and 6 respectively
static int tablebasesAvailable[4];


void initGav(const char* path)
{
    paths = tbpaths_init();

    if (NULL == paths) printf ("Error while updating the paths");
    paths = tbpaths_add (paths, "gav/gtb"); //This is the default for 3 piece
    if (NULL == paths) printf ("Error while updating the paths");
    paths = tbpaths_add (paths, path);

    initinfo = tb_init (verbosity, scheme, paths);
    tbcache_init(cache_size, wdl_fraction);
    tbstats_reset();

    const int availability = tb_availability();

    if (!availability)
        fprintf(stdout, "[-] No tablebases available, the engine will proceed without them. Ensure $cwd is ../Engine/ or pass the path to the tb as arguments Eg.: ./e /home/Chess/gtb");
    else
        fprintf(stdout, "[+] Gaviota tablebases available (~ == incomplete): ");

    for (int i = 0; i < 4; ++i)
    {
        if (availability & (0b11 << (2 * i)))
        {
            tablebasesAvailable[i] = 1;
            if (availability & (0b10 << (2 * i)))
                fprintf(stdout, "%d ", i + 3);
            else if (availability & (0b1 << (2 * i)))
                fprintf(stdout, "~%d ", i + 3);
        }
        else
            tablebasesAvailable[i] = 0;
    }

    fprintf(stdout, "\n");
    fflush(stdout);
}

inline int canGav(const uint64_t all)
{
    const int pc = POPCOUNT(all);
    if (pc > 6)
        return 0;
    return tablebasesAvailable[pc - 3];
}

/* Returns the score of a position based on the distance to mate using
 * the gaviota tablebases
 */
int gavScore(Board b, int* tbIsAv)
{
    parseBoard(&b);

    *tbIsAv = tb_probe_hard (stm, epsquare, castling, ws, bs, wp, bp, &info, &pliestomate);

    int multiplier = 0;
    if (b.turn)
    {
        if (info == tb_WMATE) multiplier = 1;
        else if (info == tb_BMATE) multiplier = -1;
        else multiplier = 0;
    }
    else
    {
        if (info == tb_WMATE) multiplier = -1;
        else if (info == tb_BMATE) multiplier = 1;
        else multiplier = 0;   
    }

    return multiplier * pliestomate;
}

/* Returns the query of the position with WDL info.
 * From the perspective of white: 1 -> white win, 0 -> draw, -1 -> black win
 */
int gavWDL(Board b, int* tbIsAv)
{
    parseBoard(&b);

    *tbIsAv = tb_probe_WDL_hard(stm, epsquare, castling, ws, bs, wp, bp, &info);

    if (info == tb_WMATE)
        return 1;
    else if (info == tb_BMATE)
        return -1;
    else
        return 0;
}

int gavWDLSoft(Board b, int* tbIsAv)
{
    parseBoard(&b);

    *tbIsAv = tb_probe_WDL_soft(stm, epsquare, castling, ws, bs, wp, bp, &info);

    if (info == tb_WMATE)
        return 1;
    else if (info == tb_BMATE)
        return -1;
    else
        return 0;
}

static void parseBoard(Board* b)
{
    stm = b->turn? tb_WHITE_TO_MOVE : tb_BLACK_TO_MOVE;
    parseCastle(b);
    parseEpSqr(b);
    parsePieces(b);
}

static void parseCastle(const Board* b)
{
    castling = tb_NOCASTLE;
    if (!b->castleInfo)
        return;
    if (BCASTLEK & b->castleInfo) castling |= tb_BOO;
    if (BCASTLEQ & b->castleInfo) castling |= tb_BOOO;
    if (WCASTLEK & b->castleInfo) castling |= tb_WOO;
    if (WCASTLEQ & b->castleInfo) castling |= tb_WOOO;
}
static void parseEpSqr(const Board* b)
{
    if (b->enPass > 8 && b->enPass < 56)
        epsquare = getGavSqr(b->enPass - (b->turn? -8 : 8));
    else
        epsquare = tb_NOSQUARE;
}
static void parsePieces(Board* b)
{
    uint64_t temp = b->allPieces, indexBB;
    int counterB = 0, counterW = 0, index, sqr;
    while (temp)
    {
        index = LSB_INDEX(temp);
        indexBB = 1ULL << index;
        sqr = getGavSqr(index);

        if (indexBB & b->color[WHITE])
        {
            ws[counterW] = sqr;
            wp[counterW] = pieceAtGav(b, indexBB, WHITE);
            counterW++;
        }
        else
        {
            bs[counterB] = sqr;
            bp[counterB] = pieceAtGav(b, indexBB, BLACK);
            counterB++;
        }
        REMOVE_LSB(temp);
    }


    ws[counterW] = tb_NOSQUARE;
    wp[counterW] = tb_NOPIECE;

    bs[counterB] = tb_NOSQUARE;
    bp[counterB] = tb_NOPIECE;
}

static void dtmPrint (unsigned stm, int tb_available, unsigned info, unsigned pliestomate)
{
    if (tb_available)
    {
        if (info == tb_DRAW)
            printf ("Draw\n");
        else if (info == tb_WMATE && stm == tb_WHITE_TO_MOVE)
            printf ("White mates, plies=%u\n", pliestomate);
        else if (info == tb_BMATE && stm == tb_BLACK_TO_MOVE)
            printf ("Black mates, plies=%u\n", pliestomate);
        else if (info == tb_WMATE && stm == tb_BLACK_TO_MOVE)
            printf ("Black is mated, plies=%u\n", pliestomate);
        else if (info == tb_BMATE && stm == tb_WHITE_TO_MOVE)
            printf ("White is mated, plies=%u\n", pliestomate);
        else
        {
            printf ("FATAL ERROR, This should never be reached\n");
            exit(EXIT_FAILURE);
        }
        printf ("\n");
    }
    else
    {
        printf ("Tablebase info not available\n\n");   
    }
}