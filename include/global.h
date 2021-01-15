#define ENGINE_AUTHOR "Jorge"
#define ENGINE_NAME "NoC 9.20 NNUE"

#include <stdint.h>

#define CHECK_MALLOC(ptr) if (!(ptr)) { fprintf(stderr, "Malloc failed in %s %d\n", __FILE__, __LINE__); \
                            exit(66);};

#define REMOVE_LSB(bb) bb &= bb - 1ULL
#define POPCOUNT(ll) (int)__builtin_popcountll(ll)
#define LSB_INDEX(ll) (int)__builtin_ctzll(ll)
#define MSB_INDEX(ll) (63 ^ (int)__builtin_clzll(ll))

#define WHITE_PAWN_ATT(bb) (((bb) << 9) & 0xfefefefefefefefeULL) | (((bb) << 7) & 0x7f7f7f7f7f7f7f7fULL)
#define BLACK_PAWN_ATT(bb) (((bb) >> 9) & 0x7f7f7f7f7f7f7f7fULL) | (((bb) >> 7) & 0xfefefefefefefefeULL)

#define BASE_64(a, b) (((a) << 6) + (b))
#define RANGE_64(a) ((a) >= 0 && (a) < 64)

#define NO_PIECE -1
#define KING    0
#define QUEEN   1
#define ROOK    2
#define BISH    3
#define KNIGHT  4
#define PAWN    5

//1 ^ color to change the color
//2 | color to change to AV
//3 - color to change to opposite AV
#define BLACK 0
#define WHITE 1
#define AV_BLACK 2
#define AV_WHITE 3

#define BCASTLEK    0b1
#define BCASTLEQ   0b10
#define WCASTLEK  0b100
#define WCASTLEQ 0b1000

#define C_MASK_WK 0x6ULL
#define C_MASK_WQ 0x70ULL
#define C_MASK_BK 0x600000000000000ULL
#define C_MASK_BQ 0x7000000000000000ULL

#define NMOVES 200

//Relevant positions on the board
#define EVEN_TILES  0x55aa55aa55aa55aaULL
#define ODD_TILES   0xaa55aa55aa55aa55ULL
#define EDGE        0xff818181818181ffULL
#define EDGE_RANKS  0xff000000000000ffULL
#define CENTRE      0x1818000000ULL
#define BIG_CENTRE  0x3c3c3c3c0000ULL

#define PLUS_MATE   1159998
#define MINS_MATE  -1159998
#define PLUS_INF    9999999
#define MINS_INF   -9999999

#define MAX_PLY 75

#define TT_MOVE     0xf000
#define GOOD_CAP    0xe000
#define KILLER      0xd000
#define COUNTER     0xc000
#define BAD_CAP     0xb000

#define SCORE_MASK   0xfff
#define TYPE_MASK   0xf000

const inline int min(const int a, const int b) {return (a < b)? a : b;}
const inline int max(const int a, const int b) {return (a > b)? a : b;}

extern uint64_t POW2[64];