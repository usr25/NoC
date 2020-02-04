/* evaluation.c
 * Returns the evaluation of a given position
 * eval < 0 -> It is good for black
 * eval > 0 -> It is good for white
 */

#define OP 0
#define EG 1

const int TEMPO = 11; //Value for a passed pawn right before promotion

int V_QUEEN[2] = {1348, 1400};
int V_ROOK[2] = {604, 695};
int V_BISH[2] = {422, 417};
int V_KNIGHT[2] = {411, 389};
int V_PAWN[2] = {116, 120};

int CONNECTED_ROOKS[2] = {28, 25}; //Bonus for having connected rooks
int ROOK_OPEN_FILE[2] = {26, 22}; //Bonus for a rook on an open file (No same color pawns)
int SAFE_KING[2] = {33, 17}; //Bonus for pawns surrounding the king
int BISH_PAIR[2] = {57, 54}; //Bonus for having the bishop pair
int KNIGHT_PAWNS[2] = {32, 35}; //Bonus for the knights when there are a lot of pawns
int N_KING_OPEN_FILE[2] = {-3, -1}; //Penalization for having the king on a file with no same color pawns
int PAWN_CHAIN[2] = {27, 27}; //Bonus for making a pawn chain
int PAWN_PROTECTION[2] = {14, 15}; //Bonus for Bish / Knight protected by pawn
int ATTACKED_BY_PAWN[2] = {80, 61}; //Bonus if a pawn can attack a piece
int N_DOUBLED_PAWNS[2] = {-10, -35}; //Penalization for doubled pawns (proportional to the pawns in line - 1)

//Not yet implemented
int PASSED_PAWN[2] = {40, 70}; //Bonus for passed pawns at the 7th rank
int N_ISOLATED_PAWN[2] = {-5, -5}; //Penalization for isolated pawns
int CLOSE_TO_KING[2] = {2, 2}; //Bonus for having pieces close to the king, but not too many!
int ATTACKED_BY_PAWN_LATER[2] = {6, 5}; //Bonus if a pawn can attack a piece after moving once


#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/memoization.h"
#include "../include/magic.h"
#include "../include/io.h"
#include "../include/evaluation.h"

#include <assert.h>

typedef struct
{
    uint64_t movs[2][6];
    uint64_t all[2];
    uint64_t all2[2];
    uint64_t kingDanger[2];
    uint64_t pawnAtts[2];
    uint64_t mostPieces;
    int acc[2];
    int cnt[2][6];
    int k[2];
    int attOnK[2];
    int ph;
} Eval;


static int phase(const Eval* ev);

// Main functions
static void material(const Eval* ev);
static void pieceActivity(const Board* b, const Eval* ev);
static void passedPawns(uint64_t wp, uint64_t bp, const Eval* ev);
static void pawns(const Board* b, const Eval* ev);
static void kingSafety(const Board* b, const Eval* ev);

static void pst2(Eval* ev, const Board* b, const int color);
static void rookOnOpenFile(const Board* b);
static void minorPieces(const Eval* ev);
static void space(const Board* b, const Eval* ev, const int c);

static int kingAtts(Eval* ev, const Board* b);


//Shamelessly copied from chessprogramming (sf). 64 elements
static const int kingAtt[64] = {
    0,  0,   1,   2,   3,   5,   7,   9,  12,  15,
  18,  22,  26,  30,  35,  39,  44,  50,  56,  62,
  68,  75,  82,  85,  89,  97, 105, 113, 122, 131,
 140, 150, 169, 180, 191, 202, 213, 225, 237, 248,
 260, 272, 283, 295, 307, 319, 330, 342, 354, 366,
 377, 389, 401, 412, 424, 436, 448, 459, 471, 483,
 494, 500, 500, 500
};
static const int attWg[6] = {0, 5, 4, 3, 3};
static const int defWg[6] = {0, 1, 1, 2, 2};

static int result[2] = {0, 0};
static inline void addVal(const int* score, const int val)
{
    result[OP] += score[OP] * val;
    result[EG] += score[EG] * val;
}

static inline int taperedEval(const int ph, const int beg, const int end)
{
    assert(ph >= 0 && ph <= 256);
    return ((beg * (255 ^ ph)) + (end * ph)) / 256;
}

uint64_t auxKnightMoves(const int lsb, const uint64_t pieces)
{
    return getKnightMoves(lsb);
}

void initializeEvMov(Eval* ev, const Board* b)
{
    ev->pawnAtts[WHITE] = WHITE_PAWN_ATT(b->piece[WHITE][PAWN]);
    ev->pawnAtts[BLACK] = BLACK_PAWN_ATT(b->piece[BLACK][PAWN]);

    ev->movs[BLACK][PAWN] = ev->pawnAtts[BLACK]; ev->movs[WHITE][PAWN] = ev->pawnAtts[WHITE];
    ev->all[BLACK] = ev->pawnAtts[BLACK]; ev->all[WHITE] = ev->pawnAtts[WHITE];
    ev->all2[BLACK] = 0; ev->all2[WHITE] = 0;

    ev->acc[BLACK] = 0; ev->acc[WHITE] = 0;
    ev->attOnK[BLACK] = 0; ev->attOnK[WHITE] = 0;

    uint64_t ks = getKingMoves(LSB_INDEX(b->piece[WHITE][KING]));
    ev->kingDanger[WHITE] = ks | (ks << 8);

    ks = getKingMoves(LSB_INDEX(b->piece[BLACK][KING]));
    ev->kingDanger[BLACK] = ks | (ks >> 8);

    for (int p = QUEEN; p <= PAWN; ++p)
    {
        ev->cnt[WHITE][p] = POPCOUNT(b->piece[WHITE][p]);
        ev->cnt[BLACK][p] = POPCOUNT(b->piece[BLACK][p]);
    }

    ev->k[WHITE] = LSB_INDEX(b->piece[WHITE][KING]);
    ev->k[BLACK] = LSB_INDEX(b->piece[BLACK][KING]);
    ev->mostPieces = b->allPieces ^ b->piece[WHITE][QUEEN] ^ b->piece[BLACK][QUEEN];
}

void initEval(void)
{
}

int eval(const Board* b)
{
    assert(POPCOUNT(b->allPieces) <= 32);
    assert(((b->piece[WHITE][PAWN] | b->piece[BLACK][PAWN]) & 0xff000000000000ff) == 0);
    assert(POPCOUNT(b->piece[WHITE][KING]) == 1);
    assert(POPCOUNT(b->piece[BLACK][KING]) == 1);

    result[OP] = 0; result[EG] = 0;

    Eval ev;
    initializeEvMov(&ev, b);

    ev.ph = phase(&ev);

    material(&ev);

    pst2(&ev, b, WHITE);
    pst2(&ev, b, BLACK);

    int ka = kingAtts(&ev, b);
    result[OP] += ka;
    result[EG] += ka / 2;

    pieceActivity(b, &ev);

    kingSafety(b, &ev);

    passedPawns(b->piece[WHITE][PAWN], b->piece[BLACK][PAWN], &ev);

    pawns(b, &ev);

    space(b, &ev, WHITE);
    space(b, &ev, BLACK);

    int evaluation = taperedEval(ev.ph, result[0], result[1]);
    assert(evaluation < PLUS_MATE && evaluation > MINS_MATE);

    return TEMPO + (b->stm? evaluation : -evaluation);
}

int insuffMat(const Board* b)
{
    switch(POPCOUNT(b->color[WHITE]))
    {
        case 1: 
            
            switch(POPCOUNT(b->color[BLACK]))
            {
                case 1:
                    return 1;
                case 2:
                    return b->piece[BLACK][BISH] || b->piece[BLACK][KNIGHT];
            }

            return 0;

        case 2:

            switch(POPCOUNT(b->color[BLACK]))
            {
                case 1:
                    return b->piece[WHITE][BISH] || b->piece[WHITE][KNIGHT];
                case 2:
                    return 
                              ((ODD_TILES & b->piece[WHITE][BISH]) && (ODD_TILES & b->piece[BLACK][BISH]))
                            ||((EVEN_TILES & b->piece[WHITE][BISH]) && (EVEN_TILES & b->piece[BLACK][BISH]));
            }

            return 0;
    }

    return 0;
}

static void space(const Board* b, const Eval* ev, const int c)
{
    uint64_t mask = c? 0xffffff0000 : 0xffffff000000;
    uint64_t safe = mask & ~(ev->movs[1^c][PAWN] | b->color[c]);
    int score = POPCOUNT(safe);
    if (c)
        result[OP] += score;
    else
        result[OP] -= score;
}

static int kingAtts(Eval* ev, const Board* b)
{
    //Penalization for free queen checks
    for (int c = BLACK; c <= WHITE; ++c)
        ev->acc[c] += 6 * POPCOUNT(ev->movs[c][QUEEN] & ev->kingDanger[1^c] & ~ev->all[1^c]);

    return ((ev->attOnK[WHITE]>2)*kingAtt[min(ev->acc[WHITE], 63)] - (ev->attOnK[BLACK]>2)*kingAtt[min(ev->acc[BLACK], 63)]);
}

static int phase(const Eval* ev)
{
    const int knPh = 1;
    const int biPh = 1;
    const int roPh = 2;
    const int quPh = 4;

    const int totPh = 24;//((knPh + biPh + roPh) * 4) + (quPh * 2);

    const int currPh =
         totPh
        -knPh * (ev->cnt[WHITE][KNIGHT] + ev->cnt[BLACK][KNIGHT])
        -biPh * (ev->cnt[WHITE][BISH] + ev->cnt[BLACK][BISH])
        -roPh * (ev->cnt[WHITE][ROOK] + ev->cnt[BLACK][ROOK])
        -quPh * (ev->cnt[WHITE][QUEEN] + ev->cnt[BLACK][QUEEN]);

    return max(0, ((currPh * 256) + (totPh / 2)) / totPh);
}


static inline void material(const Eval* ev)
{
    addVal(V_QUEEN, ev->cnt[WHITE][QUEEN] - ev->cnt[BLACK][QUEEN]);
    addVal(V_ROOK, ev->cnt[WHITE][ROOK] - ev->cnt[BLACK][ROOK]);
    addVal(V_BISH, ev->cnt[WHITE][BISH] - ev->cnt[BLACK][BISH]);
    addVal(V_KNIGHT, ev->cnt[WHITE][KNIGHT] - ev->cnt[BLACK][KNIGHT]);
    addVal(V_PAWN, ev->cnt[WHITE][PAWN] - ev->cnt[BLACK][PAWN]);
}

static inline void pieceActivity(const Board* b, const Eval* ev)
{
    minorPieces(ev);
    rookOnOpenFile(b);

    addVal(CONNECTED_ROOKS, ((ev->movs[WHITE][ROOK] & b->piece[WHITE][ROOK]) != 0) - ((ev->movs[BLACK][ROOK] & b->piece[BLACK][ROOK]) != 0));
}

static inline void kingSafety(const Board* b, const Eval* ev)
{
    const uint64_t wkMoves = getKingMoves(ev->k[WHITE]);
    const uint64_t bkMoves = getKingMoves(ev->k[BLACK]);

    addVal(SAFE_KING, POPCOUNT(wkMoves & b->piece[WHITE][PAWN]) - POPCOUNT(bkMoves & b->piece[BLACK][PAWN]));
    addVal(N_KING_OPEN_FILE, ((getUpMoves(ev->k[WHITE]) & b->piece[WHITE][PAWN]) == 0) - ((getDownMoves(ev->k[BLACK]) & b->piece[BLACK][PAWN]) == 0));
    /*
    int closeW = POPCOUNT(wkMoves & b->color[WHITE] & ~wPawnBB);
    int closeB = POPCOUNT(bkMoves & b->color[BLACK] & ~bPawnBB);
    const int mult[9] = {0, 1, 3, 5, 3, 0, -1, -2, -5};
    addVal(CLOSE_TO_KING, mult[closeW] - mult[closeB]);
    */
}

static void passedPawns(uint64_t wp, uint64_t bp, const Eval* ev)
{
    const int open[8] = {0, 0,  0, 10, 15, 20, 40, 0};
    const int endg[8] = {0, 7, 15, 20, 30, 42, 70, 0};
    int lsb = 0, op = 0, end = 0;
    wp &= 0xffffffff00000000;
    bp &= 0xffffffff;
    uint64_t pos;
    int isProtected, rank, advanceBonus;
    while(wp)
    {
        lsb = LSB_INDEX(wp);
        if ((getWPassedPawn(lsb) & bp) == 0)
        {
            pos = 1ULL << lsb;
            rank = lsb >> 3;
            isProtected = (ev->pawnAtts[WHITE] & pos)? rank : 0;
            advanceBonus = ((pos << 8) & (ev->mostPieces | ev->all[BLACK]))? 0 : 12;
            op  += open[rank] + isProtected;
            end += endg[rank] + 2*isProtected + advanceBonus;
        }
        REMOVE_LSB(wp);
    }
    while (bp)
    {
        lsb = LSB_INDEX(bp);
        if ((getBPassedPawn(lsb) & wp) == 0)
        {
            pos = 1ULL << lsb;
            rank = 7 ^ (lsb >> 3);
            isProtected = (ev->pawnAtts[BLACK] & pos)? rank : 0;
            advanceBonus = ((pos >> 8) & (ev->mostPieces | ev->all[WHITE]))? 0 : 12;
            op  -= open[rank] + isProtected;
            end -= endg[rank] + 2*isProtected + advanceBonus;
        }
        REMOVE_LSB(bp);
    }

    result[OP] += op;
    result[EG] += end;
}

static void pawns(const Board* b, const Eval* ev)
{
    const uint64_t wPawnBB = b->piece[WHITE][PAWN];
    const uint64_t bPawnBB = b->piece[BLACK][PAWN];
    addVal(PAWN_CHAIN, POPCOUNT(wPawnBB & ev->pawnAtts[WHITE]) - POPCOUNT(bPawnBB & ev->pawnAtts[BLACK]));
    addVal(PAWN_PROTECTION, POPCOUNT(ev->pawnAtts[WHITE] & (b->piece[WHITE][BISH] | b->piece[WHITE][KNIGHT])) - POPCOUNT(ev->pawnAtts[BLACK] & (b->piece[BLACK][BISH] | b->piece[BLACK][KNIGHT])));
    addVal(N_DOUBLED_PAWNS, POPCOUNT(wPawnBB & ((wPawnBB << 8) | (wPawnBB << 16))) - POPCOUNT(bPawnBB & ((bPawnBB >> 8) | (bPawnBB >> 16))));
    //addVal(ATTACKED_BY_PAWN_LATER, POPCOUNT((wPawnBBAtt << 8) & b->color[BLACK]) - POPCOUNT((bPawnBBAtt >> 8) & b->color[WHITE]));

    const int wMinor = POPCOUNT(ev->pawnAtts[WHITE] & (b->piece[BLACK][KNIGHT] | b->piece[BLACK][BISH]));
    const int bMinor = POPCOUNT(ev->pawnAtts[BLACK] & (b->piece[WHITE][KNIGHT] | b->piece[WHITE][BISH]));

    const int wMajor = POPCOUNT(ev->pawnAtts[WHITE] & (b->piece[BLACK][ROOK] | b->piece[BLACK][QUEEN]));
    const int bMajor = POPCOUNT(ev->pawnAtts[BLACK] & (b->piece[WHITE][ROOK] | b->piece[WHITE][QUEEN]));

    addVal(ATTACKED_BY_PAWN, min(7, wMinor * wMinor) - min(7, bMinor * bMinor));
    addVal(ATTACKED_BY_PAWN, 2*(min(5, wMajor * wMajor) - min(5, bMajor * bMajor)));
}


static void rookOnOpenFile(const Board* b)
{
    int r = 0;
    uint64_t ray;
    uint64_t temp = b->piece[WHITE][ROOK];
    while (temp)
    {
        ray = getVert(LSB_INDEX(temp) & 7);
        r += ((ray & b->piece[WHITE][PAWN]) == 0) + ((ray & b->piece[BLACK][PAWN]) == 0);
        REMOVE_LSB(temp);
    }
    temp = b->piece[BLACK][ROOK];
    while (temp)
    {
        ray = getVert(LSB_INDEX(temp) & 7);
        r -= ((ray & b->piece[WHITE][PAWN]) == 0) + ((ray & b->piece[BLACK][PAWN]) == 0);
        REMOVE_LSB(temp);
    }

    addVal(ROOK_OPEN_FILE, r);
}
inline void minorPieces(const Eval* ev)
{
    addVal(BISH_PAIR, (ev->cnt[WHITE][BISH] > 1) - (ev->cnt[BLACK][BISH] > 1));
    addVal(KNIGHT_PAWNS, (ev->cnt[WHITE][KNIGHT] - ev->cnt[BLACK][KNIGHT]) * ((ev->cnt[WHITE][PAWN] + ev->cnt[BLACK][PAWN]) / 10));
}

const int mirror[64] = {
    56, 57, 58, 59, 60, 61, 62, 63,
    48, 49, 50, 51, 52, 53, 54, 55,
    40, 41, 42, 43, 44, 45, 46, 47,
    32, 33, 34, 35, 36, 37, 38, 39,
    24, 25, 26, 27, 28, 29, 30, 31,
    16, 17, 18, 19, 20, 21, 22, 23,
    8,  9,  10, 11, 12, 13, 14, 15,
    0,  1,  2,  3,  4,  5,  6,  7
};

static const int PST[2][6][64] = {
{
    /*KING*/    { 0, 0, 0, -10, 0, -10, 28, 0, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10},
    /*QUEEN*/   { 1, -11, -12, 3, -17, -59, -96, -15, -8, 3, 2, 5, -1, -11, -8, -2, -12, 0, 13, 2, 9, 12, 8, -6, -10, 5, 8, 10, 20, 19, 30, 4, -6, 0, 17, 28, 37, 10, 30, 13, -18, 11, 15, 28, 42, 34, 17, 30, 1, 12, 40, 0, 18, 63, 45, 66, -15, 6, -48, 0, 33, 0, 0, 118},
    /*ROOK*/    { -2, -1, 3, 1, 2, 1, 4, -9, -30, -6, 2, -2, 2, -11, -1, -33, -18, 0, 3, -3, 9, -1, 13, 3, -10, -5, 9, 16, 20, -19, 15, -15, 22, 38, 53, 66, 61, 45, 61, 18, 27, 96, 62, 87, 155, 167, 98, 87, 53, 38, 74, 71, 105, 103, 81, 120, 97, 0, 0, 42, 143, 0, 0, 177},
    /*BISHOP*/  { -8, 13, -9, -42, -35, -9, -52, -77, 17, 5, 15, -11, 1, 2, 0, 17, 5, 13, 16, 15, 11, -1, 3, 4, 1, 5, 26, 37, 24, 9, 19, 4, -1, 18, 33, 31, 42, 31, 19, 4, 8, 31, 23, 64, 105, 125, 61, 51, -27, -26, 34, 67, 75, 70, 80, 12, 0, 0, 0, 0, 0, 0, 0, 0},
    /*KNIGHT*/  { -114, -34, -76, -74, -33, -22, -70, -93, -64, -35, -32, -1, -8, -23, -48, -12, -44, -18, 0, 16, 9, 3, 3, -48, -16, 0, 2, 3, 22, 13, 38, -8, -16, -4, 28, 38, 11, 38, 16, 49, -25, 20, 69, 74, 143, 165, 63, 6, -39, 27, 62, 85, 69, 141, 2, 33, -69, 0, 0, 0, 0, 0, 0, 0},
    /*PAWN*/    { 0, 0, 0, 0, 0, 0, 0, 0, -1, -8, -12, -40, -15, 5, 3, -5, 1, 1, -6, -22, -6, -8, -4, 11, 1, 16, 9, 4, 5, 4, 11, 8, 10, 34, 26, 35, 35, 26, 19, 12, 24, 62, 83, 64, 89, 110, 82, 12, 136, 140, 200, 194, 124, 95, -18, 25, 0, 0, 0, 0, 0, 0, 0, 0 }
},
{
    { -39, -34, -32, -31, -31, -32, -34, -39, -19, -15, -12, -11, -11, -12, -15, -19, -2, 2, 4, 5, 5, 4, 2, -2, 12, 17, 19, 20, 20, 19, 17, 12, 25, 30, 32, 33, 33, 32, 30, 25, 35, 39, 42, 44, 44, 42, 39, 35, 44, 47, 51, 52, 52, 51, 47, 44, 48, 53, 55, 57, 57, 55, 53, 48},
    { -70, -63, -60, -57, -57, -60, -63, -70, -35, -30, -25, -24, -24, -25, -30, -35, -9, -3, 1, 3, 3, 1, -3, -9, 10, 16, 19, 22, 22, 19, 16, 10, 22, 27, 32, 34, 34, 32, 27, 22, 26, 32, 37, 39, 39, 37, 32, 26, 24, 30, 34, 35, 35, 34, 30, 24, 13, 19, 24, 26, 26, 24, 19, 13},
    { -37, -35, -34, -33, -33, -34, -35, -37, -31, -28, -27, -27, -27, -27, -28, -31, -17, -15, -13, -13, -13, -13, -15, -17, 1, 2, 3, 4, 4, 3, 2, 1, 17, 19, 20, 20, 20, 20, 19, 17, 28, 31, 32, 32, 32, 32, 31, 28, 31, 32, 33, 34, 34, 33, 32, 31, 18, 19, 20, 22, 22, 20, 19, 18},
    { -31, -24, -19, -17, -17, -19, -24, -31, -11, -4, 0, 2, 2, 0, -4, -11, 2, 9, 13, 16, 16, 13, 9, 2, 12, 19, 24, 26, 26, 24, 19, 12, 16, 23, 27, 30, 30, 27, 23, 16, 15, 22, 26, 28, 28, 26, 22, 15, 9, 16, 20, 23, 23, 20, 16, 9, -2, 4, 9, 11, 11, 9, 4, -2},
    { -114, -114, -109, -102, -102, -109, -114, -114, -93, -71, -56, -49, -49, -56, -71, -93, -53, -31, -17, -10, -10, -17, -31, -53, -25, -3, 11, 18, 18, 11, -3, -25, -8, 13, 28, 35, 35, 28, 13, -8, -2, 19, 34, 41, 41, 34, 19, -2, -8, 13, 28, 35, 35, 28, 13, -8, -24, -3, 11, 18, 18, 11, -3, -24},
    { 0, 0, 0, 0, 0, 0, 0, 0, -19, -19, -19, -19, -19, -19, -19, -19, -12, -12, -12, -12, -12, -12, -12, -12, -8, -8, -8, -8, -8, -8, -8, -8, 18, 18, 18, 18, 18, 18, 18, 18, 63, 63, 63, 63, 63, 63, 63, 63, 95, 95, 95, 95, 95, 95, 95, 95, 0, 0, 0, 0, 0, 0, 0, 0}
}};


static void psHelper(Eval* ev, const Board* b, const int piece, const int c, int* opening, int* endgame, int* isol, uint64_t (*move) (int, uint64_t))
{
    uint64_t bb = b->piece[c][piece], mv;

    ev->movs[c][piece] = 0;
    while(bb)
    {
        const int lsb = LSB_INDEX(bb);
        const int index = c? lsb : 63 ^ lsb;

        *opening += PST[OP][piece][index];
        *endgame += PST[EG][piece][index];

        if (piece != PAWN && piece != KING)
        {
            //The king is fairly important as a blocker
            mv = move(lsb, ev->mostPieces);

            ev->movs[c][piece] |= mv;
            ev->all2[c] |= mv & ev->all[c];
            ev->all[c] |= mv;

            if (mv & ev->kingDanger[1^c])
            {
                ev->attOnK[c]++;
                ev->acc[c] += POPCOUNT(mv & ev->kingDanger[1^c]) * attWg[piece];
            }
            if (0 && mv & ev->kingDanger[c])
            {
                ev->acc[1^c] -= defWg[piece];
            }
        }
        else if (piece == PAWN)
        {
            if (!(getPawnLanes(lsb) & b->piece[c][PAWN]))
                (*isol)++;
        }

        REMOVE_LSB(bb);
    }
}

static void pst2(Eval* ev, const Board* b, const int color)
{
    int opening = 0, endgame = 0, isol = 0;

    psHelper(ev, b, KING,   color, &opening, &endgame, &isol, auxKnightMoves);
    psHelper(ev, b, QUEEN,  color, &opening, &endgame, &isol, getQueenMagicMoves);
    psHelper(ev, b, ROOK,   color, &opening, &endgame, &isol, getRookMagicMoves);
    psHelper(ev, b, BISH,   color, &opening, &endgame, &isol, getBishMagicMoves);
    psHelper(ev, b, KNIGHT, color, &opening, &endgame, &isol, auxKnightMoves);
    psHelper(ev, b, PAWN,   color, &opening, &endgame, &isol, auxKnightMoves);

    if (color)
    {
        result[OP] += opening; result[EG] += endgame;
    }
    else
    {
        result[OP] -= opening; result[EG] -= endgame;
    }
}