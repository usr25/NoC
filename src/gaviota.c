/* gaviota.c
 * This file contains functionality related to the use of the gaviota tablebases.
 * Used in endgames and, mainly, to give mate in KQvK, KPvK, KRvK
 */
#include <stdio.h>

#include "../gav/gtb-probe.h"

#include "../include/global.h"
#include "../include/board.h"
#include "../include/gaviota.h"

static int getSqr(int inBoard)
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

static void dtm_print (unsigned stm, int tb_available, unsigned info, unsigned pliestomate);
static void parseBoard(Board* b);
static void parseColor(int color, Board* b);
static void parseCastle(Board* b);
static void parseEpSqr(Board* b);

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

static int verbosity = 1;      /* initialization 0 = non-verbose, 1 = verbose */
static int scheme = tb_CP4;    /* compression scheme to be used */
static const char ** paths;    /* paths where files will be searched */
static size_t cache_size = 32*1024*1024; /* 32 MiB in this example */

/*  wdl_fraction:
fraction, over 128, that will be dedicated to wdl information. 
In other words, 96 means 3/4 of the cache will be dedicated to 
win-draw-loss info, and 1/4 dedicated to distance to mate 
information.
*/
static int wdl_fraction = 96; 

//boolean array with the availability each tablebase, 3, 4, 5 and 6
static int tablebasesAvailable[4];


void initGav(const char* path)
{
    paths = tbpaths_init();
    if (NULL == paths) printf ("Error while updating the paths");
        paths = tbpaths_add (paths, path);

    initinfo = tb_init (verbosity, scheme, paths);
    tbcache_init(cache_size, wdl_fraction);
    tbstats_reset();

    const int availability = tb_availability();

    if (!availability)
        printf("[-] No table available, pass the path to the tb as arguments Eg.: ./e /home/Chess/gtb\n");
    for (int i = 0; i < 4; ++i)
    {
        if (availability & (0b11 << (2 * i)))
            tablebasesAvailable[i] = 1;
        else
            tablebasesAvailable[i] = 0;
    }
}

int canGav(const uint64_t all)
{
    const int pc = POPCOUNT(all);
    if (pc > 6)
        return 0;
    return tablebasesAvailable[pc - 3];
}

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

static void parseBoard(Board* b)
{
    stm = b->turn? tb_WHITE_TO_MOVE : tb_BLACK_TO_MOVE;
    parseCastle(b);
    parseEpSqr(b);
    parseColor(BLACK, b);
    parseColor(WHITE, b);
}

static void parseCastle(Board* b)
{
    castling = b->castleInfo? 0 : tb_NOCASTLE;
    if (BCASTLEK & b->castleInfo) castling |= tb_BOO;
    if (BCASTLEQ & b->castleInfo) castling |= tb_BOOO;
    if (WCASTLEK & b->castleInfo) castling |= tb_WOO;
    if (WCASTLEQ & b->castleInfo) castling |= tb_WOOO;
}
static void parseEpSqr(Board* b)
{
    epsquare = tb_NOSQUARE;
}
static void parseColor(int color, Board* b)
{
    uint64_t temp = b->color[color];
    int counter = 0;
    while (temp)
    {
        int index = LSB_INDEX(temp);
        int sqr = getSqr(index);
        if (color)
        {
            ws[counter] = sqr;
            wp[counter] = pieceAtGav(b, 1ULL << index, color);
        }
        else
        {
            bs[counter] = sqr;
            bp[counter] = pieceAtGav(b, 1ULL << index, color);
        }
        counter++;
        REMOVE_LSB(temp);
    }

    if (color)
    {
        ws[counter] = tb_NOSQUARE;
        wp[counter] = tb_NOPIECE;
    }
    else
    {
        bs[counter] = tb_NOSQUARE;
        bp[counter] = tb_NOPIECE;
    }
}


static void dtm_print (unsigned stm, int tb_available, unsigned info, unsigned pliestomate)
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